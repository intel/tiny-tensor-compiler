// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "test_volume.hpp"
#include "util.hpp"

#include <array>
#include <cmath>
#include <limits>
#include <sstream>
#include <utility>

using namespace sycl;
using namespace tinytc;

template <typename T>
test_volume<T>::test_volume(std::int64_t N, std::int64_t P, std::int64_t howmany,
                            std::size_t alignment, queue q, bool dump)
    : B3_(num_basis(N, dim)), B2_(num_basis(N - 1, dim)), P_(P), howmany_(howmany),
      B3_aligned_(aligned<T>(B3_, alignment)), B2_aligned_(aligned<T>(B2_, alignment)),
      q_(std::move(q)), dev_info_(make_core_info(q_.get_device())),
      Q_ref_(B3_, P_, B3_aligned_, howmany_, q_), Q_opt_(B3_, P_, B3_aligned_, howmany_, q_),
      I_(B3_, P_, B3_aligned_, howmany_, q_), tmp_(B3_, P_, B2_aligned_, howmany_, q_),
      A_(dim, matrix_batch<T>(P_, P_, P_, howmany_, q_)),
      K_(dim, matrix_batch<T>(B3_, B3_, B3_aligned_, 1, q_)), ctx_(make_compiler_context()),
      opt_bundle_(make_optimized_kernel(dump)),
      opt_kernel_(make_kernel(opt_bundle_, "volume_kernel")) {
    Q_ref_.random();
    Q_opt_.random();
    I_.random();
    tmp_.random();
    for (auto &a : A_) {
        a.random();
    }
    for (auto &k : K_) {
        k.random();
    }

    g_.emplace_back(make_recipe_handler(
        q_, make_small_gemm_batched(dev_info_, to_scalar_type_v<T>, transpose::N, transpose::N,
                                    B2_aligned_, P_, P_, B3_aligned_, B3_aligned_ * P_, P_, P_ * P_,
                                    B2_aligned_, B2_aligned_ * P_, ctx_)));
    g_.emplace_back(make_recipe_handler(
        q_, make_small_gemm_batched(dev_info_, to_scalar_type_v<T>, transpose::N, transpose::N,
                                    B3_aligned_, P_, B2_, B3_aligned_, 0, B2_aligned_,
                                    B2_aligned_ * P_, B3_aligned_, B3_aligned_ * P_, ctx_)));
}

template <typename T> auto test_volume<T>::make_compiler_context() -> compiler_context {
    auto ctx = ::tinytc::make_compiler_context();
    set_error_reporter(ctx, [](char const *what, const tinytc_location_t *, void *) {
        std::cerr << what << std::endl;
    });
    return ctx;
}

template <typename T>
auto test_volume<T>::make_optimized_kernel(bool dump_code)
    -> sycl::kernel_bundle<sycl::bundle_state::executable> {
    constexpr auto real_t = to_scalar_type_v<T>;
    // Optimized kernel
    auto opt_kernel = [&](compiler_context const &ctx) {
        auto element_ty = get_scalar(ctx, real_t);
        std::array<data_type, 2 * dim + 2> param_types;
        for (std::size_t i = 0; i < dim; ++i) {
            param_types[i] = A_[i].type(element_ty);
        }
        for (std::size_t i = 0; i < dim; ++i) {
            param_types[dim + i] = K_[i].type(element_ty);
        }
        param_types[2 * dim + 0] = Q_opt_.type(element_ty);
        param_types[2 * dim + 1] = I_.type(element_ty);

        auto f = make_func("volume_kernel", param_types, get_void(ctx));
        auto fn_body = get_body(f);

        std::array<value, 2 * dim + 2> params;
        get_parameters(fn_body, params);

        auto A = [&params](std::size_t i) -> value & { return params[i]; };
        auto K = [&params](std::size_t i) -> value & { return params[dim + i]; };
        auto Q = params[2 * dim + 0];
        auto I = params[2 * dim + 1];

        for (std::size_t i = 0; i < dim; ++i) {
            set_name(A(i), (std::ostringstream{} << 'A' << i).str());
            set_name(K(i), (std::ostringstream{} << 'K' << i).str());
        }
        set_name(Q, "Q");
        set_name(I, "I");

        auto bb = region_builder{fn_body};
        auto gid = bb.create<group_id_inst>(comp3::x, get_scalar(ctx, scalar_type::index));
        auto const static_offsets2 = std::array<std::int64_t, 2u>{0, 0};
        auto const static_offsets3 = std::array<std::int64_t, 3u>{0, 0, dynamic};
        auto const static_sizes2 = [](matrix_batch<T> const &b) -> std::array<std::int64_t, 2u> {
            return {b.nrows(), b.ncols()};
        };
        auto const static_sizes3 = [](matrix_batch<T> const &b) -> std::array<std::int64_t, 3u> {
            return {b.nrows(), b.ncols(), 0};
        };
        auto const offsets3 = array_view<value>(gid);
        auto const sizeK2 = std::array<std::int64_t, 2u>{B3_aligned_, B2_};
        auto tmp = bb.create<alloca_inst>(
            get_memref(element_ty, {B2_aligned_, P_}, {}, address_space::local));

        auto a0t = get_memref(element_ty, static_sizes2(A_[0]));
        auto a1t = get_memref(element_ty, static_sizes2(A_[1]));
        auto a2t = get_memref(element_ty, static_sizes2(A_[2]));
        auto k0t = get_memref(element_ty, sizeK2);
        auto k1t = get_memref(element_ty, sizeK2);
        auto k2t = get_memref(element_ty, sizeK2);
        auto qvt = get_memref(element_ty, {B3_aligned_, P_});
        auto ivt = get_memref(element_ty, {B2_aligned_, P_}, {1, dynamic});
        auto tmpvt = get_memref(element_ty, {B2_, P_}, {}, address_space::local);
        auto a0 = bb.create<subview_inst>(static_offsets3, static_sizes3(A_[0]), A(0), offsets3,
                                          array_view<value>{}, a0t);
        auto a1 = bb.create<subview_inst>(static_offsets3, static_sizes3(A_[1]), A(1), offsets3,
                                          array_view<value>{}, a1t);
        auto a2 = bb.create<subview_inst>(static_offsets3, static_sizes3(A_[2]), A(2), offsets3,
                                          array_view<value>{}, a2t);
        auto k0 = bb.create<subview_inst>(static_offsets2, sizeK2, K(0), array_view<value>{},
                                          array_view<value>{}, k0t);
        auto k1 = bb.create<subview_inst>(static_offsets2, sizeK2, K(1), array_view<value>{},
                                          array_view<value>{}, k1t);
        auto k2 = bb.create<subview_inst>(static_offsets2, sizeK2, K(2), array_view<value>{},
                                          array_view<value>{}, k2t);
        auto qv =
            bb.create<subview_inst>(static_offsets3, array_view{B3_aligned_, P_, std::int64_t{0}},
                                    Q, offsets3, array_view<value>{}, qvt);
        auto iv =
            bb.create<subview_inst>(static_offsets3, array_view{B2_aligned_, P_, std::int64_t{0}},
                                    I, offsets3, array_view<value>{}, ivt);
        auto tmpv = bb.create<subview_inst>(static_offsets2, array_view{B2_, P_}, tmp,
                                            array_view<value>{}, array_view<value>{}, tmpvt);
        auto const c0 = bb.constant_zero(element_ty);
        auto const c1 = bb.constant_one(element_ty);
        bb.create<gemm_inst>(false, transpose::N, transpose::N, c1, iv, a0, c0, tmp);
        bb.create<gemm_inst>(false, transpose::N, transpose::N, c1, k0, tmpv, c1, qv);
        bb.create<gemm_inst>(false, transpose::N, transpose::N, c1, iv, a1, c0, tmp);
        bb.create<gemm_inst>(false, transpose::N, transpose::N, c1, k1, tmpv, c1, qv);
        bb.create<gemm_inst>(false, transpose::N, transpose::N, c1, iv, a2, c0, tmp);
        bb.create<gemm_inst>(false, transpose::N, transpose::N, c1, k2, tmpv, c1, qv);

        return f;
    };
    auto p = make_prog(ctx_);
    add_function(p, opt_kernel(ctx_));
    if (dump_code) {
        dump(p);
    }
    return make_kernel_bundle(q_.get_context(), q_.get_device(),
                              compile_to_spirv_and_assemble(p, dev_info_));
}

template <typename T> std::vector<event> test_volume<T>::reference() {
    auto e = std::vector<event>{};
    for (std::size_t d = 0; d < dim; ++d) {
        small_gemm_batched::set_args(g_[0], howmany_, T(1.0), I_.get(), A_[d].get(), T(0.0),
                                     tmp_.get());
        e.emplace_back(g_[0].submit(q_, e));
        e.front() = e.back();
        e.pop_back();
        small_gemm_batched::set_args(g_[1], howmany_, T(1.0), K_[d].get(), tmp_.get(), T(1.0),
                                     Q_ref_.get());
        e.emplace_back(g_[1].submit(q_, e));
    }
    return e;
}

template <typename T> std::vector<event> test_volume<T>::optimized() {
    auto exe_range = get_execution_range(
        opt_kernel_, sycl::range<3u>{1u, 1u, static_cast<std::size_t>(howmany_)});
    return {q_.submit([&](handler &h) {
        h.set_args(A_[0].get(), howmany_, A_[1].get(), howmany_, A_[2].get(), howmany_, K_[0].get(),
                   K_[1].get(), K_[2].get(), Q_opt_.get(), howmany_, I_.get(), howmany_);
        h.parallel_for(exe_range, opt_kernel_);
    })};
}

template <typename T> bool test_volume<T>::check() {
    Q_ref_.random();
    Q_opt_.random();
    for (auto &e : reference()) {
        e.wait();
    }
    for (auto &e : optimized()) {
        e.wait();
    }
    return is_approx_equal(Q_opt_, Q_ref_, q_);
}

template <typename T> std::int64_t test_volume<T>::flop() {
    return dim * 2 * (B3_ * P_ * B2_ + B2_ * P_ * P_) * howmany_;
}
template <typename T> std::int64_t test_volume<T>::flop_aligned() {
    return dim * 2 * (B3_aligned_ * P_ * B2_ + B2_aligned_ * P_ * P_) * howmany_;
}
template <typename T> std::int64_t test_volume<T>::bytes() {
    auto const bytes_read =
        sizeof(T) * (howmany_ * (2 * B3_ * P_ + P_ * P_ * dim) + B3_ * B3_ * dim);
    auto const bytes_write = sizeof(T) * howmany_ * B3_ * P_;
    return bytes_read + bytes_write;
}

template class test_volume<float>;
template class test_volume<double>;

