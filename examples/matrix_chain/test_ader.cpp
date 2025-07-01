// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "test_ader.hpp"
#include <tinytc/builder.hpp>

#include <array>
#include <cmath>
#include <limits>
#include <sstream>
#include <utility>

using namespace sycl;
using namespace tinytc;

template <typename T>
test_ader<T>::test_ader(std::int64_t N, std::int64_t P, std::int64_t howmany, std::size_t alignment,
                        queue q, bool dump)
    : N_(N), P_(P), howmany_(howmany), alignment_(alignment), q_(std::move(q)),
      dev_info_(make_core_info(q_.get_device())), I_ref_(Bd(), P_, Bd_aligned(), howmany_, q_),
      I_opt_(Bd(), P_, Bd_aligned(), howmany_, q_),
      tmp_(Bd(), P_, Bd_aligned(N_ - 1), howmany_, q_),
      A_(dim, matrix_batch<T>(P_, P_, P_, howmany_, q_)),
      K_(dim, matrix_batch<T>(Bd(), Bd(), Bd_aligned(N_ - 1), 1, q_)), dQ_(make_dQ()),
      opt_bundle_(make_optimized_kernel(dump)),
      opt_kernel_(make_kernel(opt_bundle_, "ader_kernel")) {
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
auto test_ader<T>::make_optimized_kernel(bool dump_code)
    -> sycl::kernel_bundle<sycl::bundle_state::executable> {
    constexpr auto real_t = to_scalar_type_v<T>;
    auto opt_kernel = [&](tinytc_compiler_context_t ctx) {
        auto element_ty = get<number_type>(ctx, real_t);
        std::array<tinytc_type_t, 2 * dim + 3> param_types;
        param_types[0] = element_ty;
        for (std::size_t i = 0; i < dim; ++i) {
            param_types[1 + i] = A_[i].type(element_ty);
        }
        for (std::size_t i = 0; i < dim; ++i) {
            param_types[1 + dim + i] = K_[i].type(element_ty);
        }
        param_types[1 + 2 * dim + 0] = dQ_[0].type(element_ty);
        param_types[1 + 2 * dim + 1] = I_opt_.type(element_ty);

        auto void_ty = get<void_type>(ctx);
        auto f = make_func("ader_kernel", param_types, void_ty);
        auto fn_body = get_body(f);

        std::array<tinytc_value_t, 2 * dim + 3> params;
        get_parameters(fn_body, params);

        auto dt = params[0];
        set_name(dt, "dt");
        auto A = [&params](std::size_t i) -> tinytc_value_t & { return params[1 + i]; };
        auto K = [&params](std::size_t i) -> tinytc_value_t & { return params[1 + dim + i]; };
        auto Q = params[1 + 2 * dim + 0];
        auto I = params[1 + 2 * dim + 1];
        for (std::size_t i = 0; i < dim; ++i) {
            set_name(A(i), (std::ostringstream{} << 'A' << i).str());
            set_name(K(i), (std::ostringstream{} << 'K' << i).str());
        }
        set_name(Q, "Q");
        set_name(I, "I");

        auto bb = region_builder{fn_body};
        auto const c0 = bb.constant_zero(element_ty);
        auto const c1 = bb.constant_one(element_ty);
        auto const gid =
            bb.create<group_id_inst>(comp3::x, get<number_type>(ctx, scalar_type::index));
        auto const static_offsets3 = std::array<std::int64_t, 3u>{0, 0, dynamic};
        auto const static_sizes3 = [](matrix_batch<T> const &b) -> std::array<std::int64_t, 3u> {
            return {b.nrows(), b.ncols(), 0};
        };
        auto const static_sizes2 = [](matrix_batch<T> const &b) -> std::array<std::int64_t, 2u> {
            return {b.nrows(), b.ncols()};
        };
        auto const offsets3 = array_view<tinytc_value_t>(gid);
        const auto dynamic_stride = std::array{std::int64_t{1}, dynamic};
        auto dqt = get<memref_type>(element_ty, static_sizes2(dQ_[0]), dynamic_stride,
                                    address_space::global);
        auto dq = bb.create<subview_inst>(static_offsets3, static_sizes3(dQ_[0]), Q, offsets3,
                                          array_view<tinytc_value_t>{}, dqt);
        for (std::size_t d = 0; d < dim; ++d) {
            auto At = get<memref_type>(element_ty, static_sizes2(A_[d]), array_view<std::int64_t>{},
                                       address_space::global);
            A(d) = bb.create<subview_inst>(static_offsets3, static_sizes3(A_[d]), A(d), offsets3,
                                           array_view<tinytc_value_t>{}, At);
        }
        auto it = get<memref_type>(element_ty, static_sizes2(I_opt_), dynamic_stride,
                                   address_space::global);
        auto i = bb.create<subview_inst>(static_offsets3, static_sizes3(I_opt_), I, offsets3,
                                         array_view<tinytc_value_t>{}, it);
        bb.create<axpby_inst>(false, transpose::N, c1, dq, c1, i);

        int denom = 1;
        auto cnum = c1;
        auto const static_offsets2 = std::array<std::int64_t, 2u>{0, 0};
        for (std::int64_t n = 1; n <= N_; ++n) {
            cnum = bb.create<mul_inst>(cnum, dt, get_type(dt));
            denom *= n + 1;
            auto cdenom = bb.create<constant_inst>(static_cast<double>(denom), element_ty);
            auto cfactor = bb.create<div_inst>(cnum, cdenom, get_type(cnum));
            auto bn = Bd_aligned(N_ - n);
            auto dq_next = bb.create<alloca_inst>(dQ_[n].local_type(element_ty));
            auto dq_nextvt = get<memref_type>(element_ty, std::array{bn, P_}, dynamic_stride,
                                              address_space::local);
            auto dq_nextv = bb.create<subview_inst>(static_offsets2, array_view{bn, P_}, dq_next,
                                                    array_view<tinytc_value_t>{},
                                                    array_view<tinytc_value_t>{}, dq_nextvt);
            auto tmp = bb.create<alloca_inst>(get<memref_type>(
                element_ty, std::array{bn, P_}, dynamic_stride, address_space::local));
            for (std::size_t d = 0; d < dim; ++d) {
                auto Kvt = get<memref_type>(element_ty, std::array{bn, Bd(N_ - n + 1)},
                                            dynamic_stride, address_space::global);
                auto Kv = bb.create<subview_inst>(static_offsets2, array_view{bn, Bd(N_ - n + 1)},
                                                  K(d), array_view<tinytc_value_t>{},
                                                  array_view<tinytc_value_t>{}, Kvt);
                bb.create<gemm_inst>(false, transpose::N, transpose::N, c1, Kv, dq, c0, tmp);
                bb.create<gemm_inst>(false, transpose::N, transpose::N, c1, tmp, A(d),
                                     d > 0 ? c1 : c0, dq_nextv);
            }
            auto ivt = get<memref_type>(element_ty, std::array{Bd(N_ - n), P_}, dynamic_stride,
                                        address_space::global);
            auto iv = bb.create<subview_inst>(static_offsets2, array_view{Bd(N_ - n), P_}, i,
                                              array_view<tinytc_value_t>{},
                                              array_view<tinytc_value_t>{}, ivt);
            bb.create<axpby_inst>(false, transpose::N, cfactor, dq_next, c1, iv);
            dq = dq_next;
        }

        return f;
    };
    auto ctx = make_compiler_context();
    set_error_reporter(ctx, [](char const *what, const tinytc_location_t *, void *) {
        std::cerr << what << std::endl;
    });
    auto p = make_prog(ctx);
    add_function(p, opt_kernel(ctx.get()));
    if (dump_code) {
        dump(p);
    }
    return make_kernel_bundle(q_.get_context(), q_.get_device(),
                              compile_to_spirv_and_assemble(p, dev_info_));
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
    T dt = 1.01;
    auto exe_range = get_execution_range(
        opt_kernel_, sycl::range<3u>{1u, 1u, static_cast<std::size_t>(howmany_)});
    return {q_.submit([&](handler &h) {
        h.set_args(dt, A_[0].get(), howmany_, A_[1].get(), howmany_, A_[2].get(), howmany_,
                   K_[0].get(), K_[1].get(), K_[2].get(), dQ_[0].get(), howmany_, I_opt_.get(),
                   howmany_);
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

