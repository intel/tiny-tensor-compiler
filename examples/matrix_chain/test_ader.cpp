// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "test_ader.hpp"

#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <utility>

using namespace sycl;
using namespace tinytc;

template <typename T>
test_ader<T>::test_ader(std::int64_t N, std::int64_t P, std::int64_t howmany, std::size_t alignment,
                        queue q)
    : N_(N), P_(P), howmany_(howmany), alignment_(alignment), q_(std::move(q)),
      dev_info_(make_core_info(q_.get_device())), I_ref_(Bd(), P_, Bd_aligned(), howmany_, q_),
      I_opt_(Bd(), P_, Bd_aligned(), howmany_, q_),
      tmp_(Bd(), P_, Bd_aligned(N_ - 1), howmany_, q_),
      A_(dim, matrix_batch<T>(P_, P_, P_, howmany_, q_)),
      K_(dim, matrix_batch<T>(Bd(), Bd(), Bd_aligned(N_ - 1), 1, q_)), dQ_(make_dQ()),
      opt_bundle_(make_optimized_kernel()), opt_kernel_(make_kernel(opt_bundle_, "ader_kernel")) {
    I_ref_.random();
    I_opt_.random();
    for (auto &a : A_) {
        a.random();
    }
    for (auto &k : K_) {
        k.random();
    }
    auto d = dQ_.begin();
    (d++)->random();
    for (; d != dQ_.end(); ++d) {
        d->fill(0);
    }

    for (std::int64_t n = 1; n <= N_; ++n) {
        auto bn = Bd_aligned(N_ - n);
        g_.emplace_back(make_recipe_handler(
            q_, make_small_gemm_batched(dev_info_, to_scalar_type_v<T>, transpose::N, transpose::N,
                                        bn, P_, Bd(N_ - n + 1), K_[0].ld(), 0, dQ_[n - 1].ld(),
                                        dQ_[n - 1].stride(), bn, bn * P_)));
        g_.emplace_back(make_recipe_handler(
            q_, make_small_gemm_batched(dev_info_, to_scalar_type_v<T>, transpose::N, transpose::N,
                                        bn, P_, P_, bn, bn * P_, A_[0].ld(), A_[0].stride(),
                                        dQ_[n].ld(), dQ_[n].stride())));
    }
}

template <typename T> std::vector<matrix_batch<T>> test_ader<T>::make_dQ() {
    auto result = std::vector<matrix_batch<T>>{};
    for (std::int64_t n = 0; n <= N_; ++n) {
        result.emplace_back(matrix_batch<T>(Bd(N_ - n), P_, Bd_aligned(N_ - n), howmany_, q_));
    }
    return result;
}

template <typename T>
auto test_ader<T>::make_optimized_kernel() -> sycl::kernel_bundle<sycl::bundle_state::executable> {
    constexpr auto real_t = to_scalar_type_v<T>;
    auto opt_kernel = [&](function_builder &fb) {
        T dt = 1.01;
        T num = T(1.0);
        int denom = 1;
        std::array<value, dim> A;
        std::array<value, dim> K;
        for (std::size_t i = 0; i < dim; ++i) {
            A[i] = fb.argument(A_[i].type(), "A");
        }
        for (std::size_t i = 0; i < dim; ++i) {
            K[i] = fb.argument(K_[i].type(), "K");
        }
        auto Q = fb.argument(dQ_[0].type(), "dQ");
        auto I = fb.argument(I_opt_.type(), "I");
        fb.body([&](region_builder &bb) {
            auto const gid = bb.add(make_group_id());
            auto const offsets3 = std::vector<value>{make_index(0), make_index(0), gid};
            auto const size3 = std::vector<value>{make_dynamic(), make_dynamic(), value{}};
            auto dq = bb.add(make_subview(Q, offsets3, size3));
            for (std::size_t d = 0; d < dim; ++d) {
                A[d] = bb.add(make_subview(A[d], offsets3, size3));
            }
            auto i = bb.add(make_subview(I, offsets3, size3));
            bb.add(make_axpby(transpose::N, false, make_imm(num / denom), dq, make_imm(T(1.0)), i));

            auto const offsets2 = std::vector<value>{make_index(0), make_index(0)};
            for (std::int64_t n = 1; n <= N_; ++n) {
                num *= dt;
                denom *= n + 1;
                auto bn = Bd_aligned(N_ - n);
                auto dq_next = bb.add(make_alloca(dQ_[n].type(false)));
                auto dq_nextv =
                    bb.add(make_subview(dq_next, offsets2, {make_index(bn), make_index(P_)}));
                auto tmp = bb.add(make_alloca(make_memref(real_t, {bn, P_}, {1, bn})));
                for (std::size_t d = 0; d < dim; ++d) {
                    auto Kv = bb.add(
                        make_subview(K[d], offsets2, {make_index(bn), make_index(Bd(N_ - n + 1))}));
                    bb.add(make_gemm(transpose::N, transpose::N, false, make_imm(T(1.0)), Kv, dq,
                                     make_imm(T(0.0)), tmp));
                    bb.add(make_gemm(transpose::N, transpose::N, false, make_imm(T(1.0)), tmp, A[d],
                                     make_imm(T(d > 0 ? 1.0 : 0.0)), dq_nextv));
                }
                auto iv =
                    bb.add(make_subview(i, offsets2, {make_index(Bd(N_ - n)), make_index(P_)}));
                bb.add(make_axpby(transpose::N, false, make_imm(num / denom), dq_next,
                                  make_imm(T(1.0)), iv));
                dq = dq_next;
            }
        });
    };
    auto pb = program_builder{};
    pb.create("ader_kernel", opt_kernel);

    return make_kernel_bundle(q_.get_context(), q_.get_device(),
                              compile_to_opencl(pb.get_product(), dev_info_));
}

template <typename T>
event test_ader<T>::taylor_sum(matrix_batch<T> &I, matrix_batch<T> &dQ, T factor,
                               std::vector<event> const &dep_events) {
    auto Iptr = I.get();
    auto dQptr = dQ.get();
    std::size_t Ild = I.ld();
    std::size_t Istride = I.stride();
    std::size_t dQld = dQ.ld();
    std::size_t dQstride = dQ.stride();
    std::size_t nrows = dQ.nrows();
    std::size_t ncols = dQ.ncols();
    return q_.parallel_for(range{static_cast<std::size_t>(howmany_), ncols, nrows}, dep_events,
                           [=](id<3> i) {
                               Iptr[i[2] + i[1] * Ild + i[0] * Istride] +=
                                   factor * dQptr[i[2] + i[1] * dQld + i[0] * dQstride];
                           });
}

template <typename T> std::vector<event> test_ader<T>::reference() {
    T dt = 1.01;
    T num = T(1.0);
    int denom = 1;
    auto e = std::vector<event>{taylor_sum(I_ref_, dQ_[0], num / denom)};
    for (std::int64_t n = 1; n <= N_; ++n) {
        num *= dt;
        denom *= n + 1;
        for (std::size_t d = 0; d < dim; ++d) {
            small_gemm_batched::set_args(g_[2 * (n - 1)], howmany_, T(1.0), K_[d].get(),
                                         dQ_[n - 1].get(), T(0.0), tmp_.get());
            e[0] = g_[2 * (n - 1)].submit(q_, e);
            small_gemm_batched::set_args(g_[2 * n - 1], howmany_, T(1.0), tmp_.get(), A_[d].get(),
                                         T(1.0), dQ_[n].get());
            e[0] = g_[2 * n - 1].submit(q_, e);
        }
        e[0] = taylor_sum(I_ref_, dQ_[n], num / denom, e);
    }
    return e;
}

template <typename T> std::vector<event> test_ader<T>::optimized() {
    auto exe_range = get_execution_range(opt_kernel_, howmany_);
    return {q_.submit([&](handler &h) {
        h.set_args(A_[0].get(), howmany_, A_[1].get(), howmany_, A_[2].get(), howmany_, K_[0].get(),
                   K_[1].get(), K_[2].get(), dQ_[0].get(), howmany_, I_opt_.get(), howmany_);
        h.parallel_for(exe_range, opt_kernel_);
    })};
}

template <typename T> bool test_ader<T>::check() {
    I_ref_.random();
    auto d = dQ_.begin();
    (d++)->random();
    for (; d != dQ_.end(); ++d) {
        d->fill(0);
    }
    for (auto &e : reference()) {
        e.wait();
    }

    I_opt_.random();
    dQ_[0].random();
    for (auto &e : optimized()) {
        e.wait();
    }
    return is_approx_equal(I_opt_, I_ref_, q_);
}

template <typename T> std::int64_t test_ader<T>::flop() {
    std::int64_t flops = 2 * Bd() * P_; // Taylor sum first
    for (std::int64_t n = 1; n <= N_; ++n) {
        // derivative
        flops += dim * 2 * Bd(N_ - n) * P_ * (Bd(N_ - n + 1) + P_);
        // taylor sum
        flops += 2 * Bd(N_ - n) * P_;
    }
    return flops * howmany_;
}
template <typename T> std::int64_t test_ader<T>::flop_aligned() {
    std::int64_t flops = 2 * Bd() * P_; // Taylor sum first
    for (std::int64_t n = 1; n <= N_; ++n) {
        // derivative
        flops += dim * 2 * Bd_aligned(N_ - n) * P_ * (Bd(N_ - n + 1) + P_);
        // taylor sum
        flops += 2 * Bd(N_ - n) * P_;
    }
    return flops * howmany_;
}
template <typename T> std::int64_t test_ader<T>::bytes() {
    std::int64_t elements_read = dim * Bd(N_ - 1) * Bd(N_); // K
    elements_read += howmany_ * dim * P_ * P_;              // A
    elements_read += howmany_ * Bd() * P_;                  // dQ(0)
    // for (std::int64_t n = 0; n < N_; ++n) {
    // elements_read += howmany_ * Bd(N_ - n) * P_; // dQ(0), ..., dQ(N-1)
    //}
    elements_read += howmany_ * Bd() * P_; // I
    std::int64_t elements_write = 0;
    // for (std::int64_t n = 1; n <= N_; ++n) {
    // elements_write += howmany_ * Bd(N_ - n) * P_; // dQ(1), ..., dQ(N)
    //}
    elements_write += howmany_ * Bd() * P_; // I
    return sizeof(T) * (elements_read + elements_write);
}

template class test_ader<float>;
template class test_ader<double>;

