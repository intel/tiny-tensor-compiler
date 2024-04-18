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
      dev_info_(create_core_info(q_.get_device())), I_ref_(Bd(), P_, Bd_aligned(), howmany_, q_),
      I_opt_(Bd(), P_, Bd_aligned(), howmany_, q_),
      tmp_(Bd(), P_, Bd_aligned(N_ - 1), howmany_, q_),
      A_(dim, matrix_batch<T>(P_, P_, P_, howmany_, q_)),
      K_(dim, matrix_batch<T>(Bd(), Bd(), Bd_aligned(N_ - 1), 1, q_)), dQ_(make_dQ()),
      opt_bundle_(make_optimized_kernel()), opt_kernel_(opt_bundle_.get("ader_kernel")) {
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
        g_.emplace_back(recipe::small_gemm_batched<T, sycl_runtime>(
            dev_info_, transpose::N, transpose::N, bn, P_, Bd(N_ - n + 1), K_[0].ld(), 0,
            dQ_[n - 1].ld(), dQ_[n - 1].stride(), bn, bn * P_, q_.get_context(), q_.get_device()));
        g_.emplace_back(recipe::small_gemm_batched<T, sycl_runtime>(
            dev_info_, transpose::N, transpose::N, bn, P_, P_, bn, bn * P_, A_[0].ld(),
            A_[0].stride(), dQ_[n].ld(), dQ_[n].stride(), q_.get_context(), q_.get_device()));
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
auto test_ader<T>::make_optimized_kernel() -> tensor_kernel_bundle<sycl_runtime> {
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
            auto gid = bb.add(create_group_id());
            auto dq = bb.add(create_subview(Q, {0, 0, gid}, {dynamic, dynamic, nullptr}));
            for (std::size_t d = 0; d < dim; ++d) {
                A[d] = bb.add(create_subview(A[d], {0, 0, gid}, {dynamic, dynamic, nullptr}));
            }
            auto i = bb.add(create_subview(I, {0, 0, gid}, {dynamic, dynamic, nullptr}));
            bb.add(create_axpby(transpose::N, false, num / denom, dq, T(1.0), i));
            for (std::int64_t n = 1; n <= N_; ++n) {
                num *= dt;
                denom *= n + 1;
                auto bn = Bd_aligned(N_ - n);
                auto dq_next = bb.add(create_alloca(dQ_[n].type(false)));
                auto dq_nextv = bb.add(create_subview(dq_next, {0, 0}, {bn, P_}));
                auto tmp = bb.add(create_alloca(create_memref(real_t, {bn, P_}, {1, bn})));
                for (std::size_t d = 0; d < dim; ++d) {
                    auto Kv = bb.add(create_subview(K[d], {0, 0}, {bn, Bd(N_ - n + 1)}));
                    bb.add(create_gemm(transpose::N, transpose::N, false, T(1.0), Kv, dq, T(0.0),
                                       tmp));
                    bb.add(create_gemm(transpose::N, transpose::N, false, T(1.0), tmp, A[d],
                                       T(d > 0 ? 1.0 : 0.0), dq_nextv));
                }
                auto iv = bb.add(create_subview(i, {0, 0}, {Bd(N_ - n), P_}));
                bb.add(create_axpby(transpose::N, false, num / denom, dq_next, T(1.0), iv));
                dq = dq_next;
            }
        });
    };
    auto pb = program_builder{};
    pb.create("ader_kernel", opt_kernel);

    return tensor_kernel_bundle(
        compile_to_binary(pb.get_product(), dev_info_, bundle_format::native), q_.get_context(),
        q_.get_device());
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
            e[0] = g_[2 * (n - 1)](howmany_, T(1.0), K_[d].get(), dQ_[n - 1].get(), T(0.0),
                                   tmp_.get(), q_, {e});
            e[0] = g_[2 * n - 1](howmany_, T(1.0), tmp_.get(), A_[d].get(), T(1.0), dQ_[n].get(),
                                 q_, {e});
        }
        e[0] = taylor_sum(I_ref_, dQ_[n], num / denom, e);
    }
    return e;
}

template <typename T> std::vector<event> test_ader<T>::optimized() {
    opt_kernel_.set_args(A_[0].get(), howmany_, A_[1].get(), howmany_, A_[2].get(), howmany_,
                         K_[0].get(), K_[1].get(), K_[2].get(), dQ_[0].get(), howmany_,
                         I_opt_.get(), howmany_);
    return {opt_kernel_.submit(howmany_, q_)};
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

