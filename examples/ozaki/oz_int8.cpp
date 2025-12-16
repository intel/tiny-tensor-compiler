// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "oz_int8.hpp"
#include "memory.hpp"

#include <oneapi/mkl.hpp>
#include <tinytc/tinytc_sycl.hpp>

#include <iostream>
#include <sstream>
#include <type_traits>

using namespace sycl;
using namespace tinytc;

kernel2d::kernel2d(sycl::kernel k, std::int32_t block_size_m, std::int32_t block_size_n)
    : kernel{std::move(k)}, group_size{get_group_size(kernel)}, BM{block_size_m}, BN{block_size_n} {
}

auto kernel2d::execution_range(std::int64_t N) -> nd_range<3u> {
    const std::size_t wg_z = 1 + (N - 1) / BM;
    const std::size_t wg_y = 1 + (N - 1) / BN;
    const auto global_range = get_global_size(range<3u>{1, wg_y, wg_z}, group_size);
    return nd_range{global_range, group_size};
}

// Number of bits per split
constexpr auto alpha = 7;

auto make_context() {
    auto ctx = create_compiler_context();
    set_error_reporter(ctx.get(), [](char const *what, const tinytc_location_t *, void *) {
        std::cerr << what << std::endl;
    });
    return ctx;
}

auto compile_module(tinytc_compiler_context_t ctx, char const *code, queue q,
                    tinytc_core_feature_flags_t flags = 0) {
    auto program = parse_string(code, ctx);
    return create_kernel_bundle(q.get_context(), q.get_device(), program.get(), flags);
}

auto make_split_int8_kernel(examples::test_type ty, tinytc_compiler_context_t ctx, queue q) {
    extern const std::uint8_t _binary_split_int8_tinytc_start, _binary_split_int8_tinytc_end;
    std::uint8_t const *start = &_binary_split_int8_tinytc_start;
    std::uint8_t const *end = &_binary_split_int8_tinytc_end;
    const std::size_t size = end - start;

    constexpr std::int32_t BM0 = 64, BN0 = 256;
    constexpr std::int32_t BM1 = 256, BN1 = 64;
    auto oss = std::stringstream{};
    oss << "$ty = " << examples::to_string(ty) << "\n";
    oss << "$BM0 = " << BM0 << "\n";
    oss << "$BN0 = " << BN0 << "\n";
    oss << "$BM1 = " << BM1 << "\n";
    oss << "$BN1 = " << BN1 << "\n";
    if (ty == examples::test_type::f32) {
        oss << "$ity = i32\n";
        oss << "$man_bits = 23\n";
        oss << "$exp_bits = 8\n";
    } else {
        oss << "$ity = i64\n";
        oss << "$man_bits = 52\n";
        oss << "$exp_bits = 11\n";
    }
    oss << "$alpha = " << alpha << "\n";
    oss.write((char const *)start, size);
    auto bundle = compile_module(ctx, std::move(oss).str().c_str(), q);
    auto k0 = kernel2d{create_kernel(bundle, "reduce_exp_max"), BM0, BN0};
    auto k1 = kernel2d{create_kernel(bundle, "split_int8"), BM1, BN1};
    return std::array<kernel2d, 2u>{k0, k1};
}

auto make_acc_f_kernel(examples::test_type ty, tinytc_compiler_context_t ctx, queue q) {
    extern const std::uint8_t _binary_acc_f_tinytc_start, _binary_acc_f_tinytc_end;
    std::uint8_t const *start = &_binary_acc_f_tinytc_start;
    std::uint8_t const *end = &_binary_acc_f_tinytc_end;
    const std::size_t size = end - start;

    constexpr std::int32_t BM = 256, BN = 64;
    auto oss = std::stringstream{};
    oss << "$ty = " << examples::to_string(ty) << "\n";
    oss << "$BM = " << BM << "\n";
    oss << "$BN = " << BN << "\n";
    oss << "$alpha = " << alpha << "\n";
    oss.write((char const *)start, size);
    auto bundle = compile_module(ctx, std::move(oss).str().c_str(), q);
    return kernel2d{create_kernel(bundle, "acc_f"), BM, BN};
}

auto make_gemm_kernel(examples::test_type ty, examples::test_type acc_ty,
                      tinytc_compiler_context_t ctx, queue q) {
    extern const std::uint8_t _binary_gemm_tinytc_start, _binary_gemm_tinytc_end;
    std::uint8_t const *start = &_binary_gemm_tinytc_start;
    std::uint8_t const *end = &_binary_gemm_tinytc_end;
    const std::size_t size = end - start;

    std::int32_t BM = 256, BN = 128, bm = 32, bn = 32, bk = 4, sgs = 16;
    std::int32_t A_stride_gcd = 16 / examples::size(ty);
    std::int32_t B_stride_gcd = 16 / examples::size(ty);
    std::int32_t C_stride_gcd = 16 / examples::size(acc_ty);
    if (ty == examples::test_type::i8 && acc_ty == examples::test_type::i32) {
        BM = 256;
        BN = 256;
        bm = 64;
        bn = 32;
        bk = 32;
    } else if (acc_ty == examples::test_type::f32) {
        sgs = 32;
        bk = 8;
    }

    auto oss = std::stringstream{};
    oss << "$ty = " << examples::to_string(ty) << "\n";
    oss << "$acc_ty = " << examples::to_string(acc_ty) << "\n";
    oss << "$bm = " << bm << "\n";
    oss << "$bn = " << bn << "\n";
    oss << "$bk = " << bk << "\n";
    oss << "$BM = " << BM << "\n";
    oss << "$BN = " << BN << "\n";
    oss << "$sgs = " << sgs << "\n";
    oss << "$A_stride_gcd = " << A_stride_gcd << "\n";
    oss << "$B_stride_gcd = " << B_stride_gcd << "\n";
    oss << "$C_stride_gcd = " << C_stride_gcd << "\n";
    if (acc_ty == examples::test_type::i32) {
        oss << "$init = 0\n";
    } else {
        oss << "$init = 0.0\n";
    }
    oss.write((char const *)start, size);
    auto bundle = compile_module(ctx, std::move(oss).str().c_str(), q,
                                 tinytc_core_feature_flag_large_register_file);
    return kernel2d{create_kernel(bundle, "gemm"), BM, BN};
}

oz_int8::oz_int8(examples::test_type ty, std::int64_t N, std::int64_t num_splits, queue q)
    : ty_size_{size(ty)}, N_{N}, num_splits_{num_splits}, q_{std::move(q)}, ctx_{make_context()},
      split_int8_{make_split_int8_kernel(ty, ctx_.get(), q_)},
      acc_f_{make_acc_f_kernel(ty, ctx_.get(), q_)},
      gemm_{make_gemm_kernel(ty, ty, ctx_.get(), q_)},
      gemm_s8s8s32_{
          make_gemm_kernel(examples::test_type::i8, examples::test_type::i32, ctx_.get(), q_)} {}

auto oz_int8::split_i8(void *A, std::size_t stride0, std::size_t stride1)
    -> std::pair<sycl_unique_ptr<std::int8_t>, sycl_unique_ptr<void>> {
    auto As = malloc_device_unique<std::int8_t>(N_ * N_ * num_splits_, q_);
    auto e = malloc_device_unique<void>(N_ * ty_size_, q_);

    q_.fill(e.get(), 0, N_);

    q_.submit([&](sycl::handler &h) {
        h.set_args(A, N_, N_, N_,                   // A
                   std::int32_t{stride0 > stride1}, // isB
                   e.get(), N_                      // e
        );
        h.parallel_for(split_int8_[0].execution_range(N_), split_int8_[0].kernel);
    });

    q_.submit([&](sycl::handler &h) {
        h.set_args(A, N_, N_, N_,                             // A
                   std::int32_t{stride0 > stride1},           // isB
                   e.get(), N_,                               // e
                   As.get(), N_, N_, num_splits_, N_, N_ * N_ // As
        );
        h.parallel_for(split_int8_[1].execution_range(N_), split_int8_[1].kernel);
    });

    return {std::move(As), std::move(e)};
}

void oz_int8::ref(void *A, void *B, void *C) {
    q_.submit([&](sycl::handler &h) {
        h.set_args(A, N_, N_, N_, // A
                   B, N_, N_, N_, // B
                   C, N_, N_, N_  // C
        );
        h.parallel_for(gemm_.execution_range(N_), gemm_.kernel);
    });
}

void oz_int8::operator()(void *A, void *B, void *C) {
    auto [As, eA] = split_i8(A, 1, N_);
    auto [Bs, eB] = split_i8(B, N_, 1);

    auto Ctmp = malloc_device_unique<std::int32_t>(N_ * N_, q_);
    std::int32_t *Ctmp_ptr = Ctmp.get();
    for (std::int64_t i = 0; i < num_splits_; ++i) {
        std::int8_t *As_i = As.get() + i * N_ * N_;
        for (std::int64_t j = 0; j < num_splits_ - i; ++j) {
            std::int8_t *Bs_j = Bs.get() + j * N_ * N_;

            q_.submit([&](sycl::handler &h) {
                h.set_args(As_i, N_, N_, N_,    // A
                           Bs_j, N_, N_, N_,    // B
                           Ctmp_ptr, N_, N_, N_ // C
                );
                h.parallel_for(gemm_s8s8s32_.execution_range(N_), gemm_s8s8s32_.kernel);
            });

            q_.submit([&](sycl::handler &h) {
                h.set_args(Ctmp_ptr, N_, N_, N_, // C_tmp
                           C, N_, N_, N_,        // C
                           eA.get(), N_,         // eA
                           eB.get(), N_,         // eB
                           i + j                 // step
                );
                h.parallel_for(acc_f_.execution_range(N_), acc_f_.kernel);
            });
        }
    }
    q_.wait();
}

