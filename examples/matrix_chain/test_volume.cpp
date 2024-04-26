// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "test_volume.hpp"
#include "util.hpp"

#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <utility>

using namespace sycl;
using namespace tinytc;

template <typename T>
test_volume<T>::test_volume(std::int64_t N, std::int64_t P, std::int64_t howmany,
                            std::size_t alignment, queue q)
    : B3_(num_basis(N, dim)), B2_(num_basis(N - 1, dim)), P_(P), howmany_(howmany),
      B3_aligned_(aligned<T>(B3_, alignment)), B2_aligned_(aligned<T>(B2_, alignment)),
      q_(std::move(q)), dev_info_(make_core_info(q_.get_device())),
      Q_ref_(B3_, P_, B3_aligned_, howmany_, q_), Q_opt_(B3_, P_, B3_aligned_, howmany_, q_),
      I_(B3_, P_, B3_aligned_, howmany_, q_), tmp_(B3_, P_, B2_aligned_, howmany_, q_),
      A_(dim, matrix_batch<T>(P_, P_, P_, howmany_, q_)),
      K_(dim, matrix_batch<T>(B3_, B3_, B3_aligned_, 1, q_)), opt_bundle_(make_optimized_kernel()),
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
                                    B2_aligned_, B2_aligned_ * P_)));
    g_.emplace_back(make_recipe_handler(
        q_, make_small_gemm_batched(dev_info_, to_scalar_type_v<T>, transpose::N, transpose::N,
                                    B3_aligned_, P_, B2_, B3_aligned_, 0, B2_aligned_,
                                    B2_aligned_ * P_, B3_aligned_, B3_aligned_ * P_)));
}

template <typename T>
auto test_volume<T>::make_optimized_kernel()
    -> sycl::kernel_bundle<sycl::bundle_state::executable> {
    constexpr auto real_t = to_scalar_type_v<T>;
    /**
     * With B3_ = 56, B3_aligned_ = 64, B2_ = 35, B2_aligned_ = 48, P_ = 9
     *
     * func chain(A0: batch<memref<f32x9x9>, distance<81>>,
     *            A1: batch<memref<f32x9x9>, distance<81>>,
     *            A2: batch<memref<f32x9x9>, distance<81>>,
     *            K0: memref<f32x56x9,1,64>,
     *            K1: memref<f32x56x9,1,64>,
     *            K2: memref<f32x56x9,1,64>,
     *            Q: batch<memref<f32x56x9,1,64>, distance<576>>,
     *            I: batch<memref<f32x56x9,1,64>, distance<576>>) {
     *      a0 = get_work_item A0
     *      a1 = get_work_item A1
     *      a2 = get_work_item A2
     *      q = get_work_item Q
     *      i = get_work_item i
     *      tmp = alloca matrix<f32x56x9, 1x64>;
     *      K0v = submatrix K0[0:64,0:35]
     *      K1v = submatrix K1[0:64,0:35]
     *      K2v = submatrix K2[0:64,0:35]
     *      qv = submatrix Q[0:64,0:9]
     *      iv = submatrix I[0:48,0:9]
     *      tmpv = submatrix tmp[0:48,0:9]
     *      matmul(iv, a0, tmpv, 1.0, 0.0);
     *      matmul(K0v, tmp, qv, 1.0, 0.0);
     *      matmul(iv, a1, tmpv, 1.0, 0.0);
     *      matmul(K1v, tmp, qv, 1.0, 0.0);
     *      matmul(iv, a2, tmpv, 1.0, 0.0);
     *      matmul(K2v, tmp, qv, 1.0, 0.0);
     * }
     */
    // Optimized kernel
    auto opt_kernel = [&](function_builder &fb) {
        auto A0 = fb.argument(A_[0].type(), "A0");
        auto A1 = fb.argument(A_[1].type(), "A1");
        auto A2 = fb.argument(A_[2].type(), "A2");
        auto K0 = fb.argument(K_[0].type(), "K0");
        auto K1 = fb.argument(K_[1].type(), "K1");
        auto K2 = fb.argument(K_[2].type(), "K2");
        auto Q = fb.argument(Q_opt_.type(), "Q");
        auto I = fb.argument(I_.type(), "I");
        fb.body([&](region_builder &bb) {
            auto gid = bb.add(make_group_id());
            auto const offsets2 = std::vector<value>{make_imm(0), make_imm(0)};
            auto const offsets3 = std::vector<value>{make_imm(0), make_imm(0), gid};
            auto const size3 = std::vector<value>{make_dynamic(), make_dynamic(), value{}};
            auto const sizeK2 = std::vector<value>{make_imm(B3_aligned_), make_imm(B2_)};
            auto tmp = bb.add(make_alloca(make_memref(real_t, {B2_, P_}, {1, B2_aligned_})));
            auto a0 = bb.add(make_subview(A0, offsets3, size3));
            auto a1 = bb.add(make_subview(A1, offsets3, size3));
            auto a2 = bb.add(make_subview(A2, offsets3, size3));
            auto K0v = bb.add(make_subview(K0, offsets2, sizeK2));
            auto K1v = bb.add(make_subview(K1, offsets2, sizeK2));
            auto K2v = bb.add(make_subview(K2, offsets2, sizeK2));
            auto qv =
                bb.add(make_subview(Q, offsets3, {make_imm(B3_aligned_), make_imm(P_), value{}}));
            auto iv =
                bb.add(make_subview(I, offsets3, {make_imm(B2_aligned_), make_imm(P_), value{}}));
            auto tmpv = bb.add(make_subview(tmp, offsets2, {make_imm(B2_aligned_), make_imm(P_)}));
            auto const s0 = make_imm(T(0.0));
            auto const s1 = make_imm(T(1.0));
            bb.add(make_gemm(transpose::N, transpose::N, false, s1, iv, a0, s0, tmpv));
            bb.add(make_gemm(transpose::N, transpose::N, false, s1, K0v, tmp, s1, qv));
            bb.add(make_gemm(transpose::N, transpose::N, false, s1, iv, a1, s0, tmpv));
            bb.add(make_gemm(transpose::N, transpose::N, false, s1, K1v, tmp, s1, qv));
            bb.add(make_gemm(transpose::N, transpose::N, false, s1, iv, a2, s0, tmpv));
            bb.add(make_gemm(transpose::N, transpose::N, false, s1, K2v, tmp, s1, qv));
        });
    };
    auto pb = program_builder{};
    pb.create("volume_kernel", opt_kernel);

    return make_kernel_bundle(
        q_.get_context(), q_.get_device(),
        compile_to_binary(pb.get_product(), dev_info_, bundle_format::native));
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
    auto exe_range = get_execution_range(opt_kernel_, howmany_);
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

