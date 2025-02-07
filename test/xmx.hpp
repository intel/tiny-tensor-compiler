// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef XMX_20250120_HPP
#define XMX_20250120_HPP

#include "runtime_concept.hpp"
#include "tinytc/tinytc.hpp"

#include <cstdint>
#include <doctest/doctest.h>
#include <iostream>
#include <sstream>
#include <type_traits>

using runtime_class = RUNTIME_CLASS;
using namespace tinytc;

TEST_CASE(RUNTIME_NAME " store block2d f16") {
    auto gpu_rt = std::make_shared<runtime_class>();

    const std::string code = R"TinyTL(
func @store_block2d(%A: memref<f16x128x128> {align=128})
    attributes{subgroup_size=16,work_group_size=[16,1]} {
    parallel {
        %0 = constant 42.0 : coopmatrix<f16x16x8,matrix_acc>
        %1 = constant 32 : index
        %2 = constant 64 : index
        cooperative_matrix_store %0, %A[%1,%2]
        %3 = constant 43.0 : coopmatrix<f16x32x16,matrix_acc>
        %4 = constant 62 : index
        %5 = constant 17 : index
        cooperative_matrix_store %3, %A[%4,%5]
    }
})TinyTL";

    constexpr auto poison = half(-1293.0f);
    auto A_host = std::vector<half>(128 * 128, poison);

    auto A = gpu_rt->create_buffer(128 * 128 * sizeof(half));
    gpu_rt->memcpy_h2d(A, A_host.data(), A_host.size() * sizeof(half));

    auto ctx = make_compiler_context();
    ctx.set_error_reporter(
        [](char const *what, const tinytc_location_t *, void *) { std::cerr << what << std::endl; },
        nullptr);
    auto bundle = gpu_rt->get_kernel_bundle(parse_string(code, ctx));
    auto kernel = gpu_rt->get_kernel(bundle, "store_block2d");

    gpu_rt->set_mem_arg(kernel, 0, A, auto_mem_type_v<runtime_class::mem_t>);
    gpu_rt->submit(kernel);
    gpu_rt->synchronize();

    gpu_rt->memcpy_d2h(A_host.data(), A, A_host.size() * sizeof(half));

    for (std::size_t j = 0; j < 128; ++j) {
        for (std::size_t i = 0; i < 128; ++i) {
            if (i >= 32 && i < 48 && j >= 64 && j < 72) {
                REQUIRE(A_host[i + j * 128] == half(42.0f));
            } else if (i >= 62 && i < 94 && j >= 17 && j < 33) {
                REQUIRE(A_host[i + j * 128] == half(43.0f));
            } else {
                REQUIRE(A_host[i + j * 128] == poison);
            }
        }
    }

    gpu_rt->free_buffer(A);
}

TEST_CASE(RUNTIME_NAME " load block2d f16") {
    auto gpu_rt = std::make_shared<runtime_class>();

    const std::string code = R"TinyTL(
func @load_block2d(%A: memref<f16x128x128> {align=128},
                   %B: memref<f16x128x128> {align=128})
    attributes{subgroup_size=16,work_group_size=[16,1]} {
    parallel {
        %0 = constant 4 : index
        %1 = constant 8 : index
        %2 = cooperative_matrix_load.n %A[%0,%1] : coopmatrix<f16x32x16,matrix_acc>
        cooperative_matrix_store %2, %B[%0,%1]
    }
})TinyTL";

    constexpr auto poison = half(-1293.0f);
    auto A_host = std::vector<half>(128 * 128, poison);
    auto B_host = std::vector<half>(128 * 128, poison);
    for (std::size_t j = 8; j < 24; ++j) {
        for (std::size_t i = 4; i < 36; ++i) {
            A_host[i + j * 128] = half(static_cast<float>(i + j * 128));
        }
    }

    auto A = gpu_rt->create_buffer(128 * 128 * sizeof(half));
    auto B = gpu_rt->create_buffer(128 * 128 * sizeof(half));
    gpu_rt->memcpy_h2d(A, A_host.data(), A_host.size() * sizeof(half));
    gpu_rt->memcpy_h2d(B, B_host.data(), B_host.size() * sizeof(half));

    auto ctx = make_compiler_context();
    ctx.set_error_reporter(
        [](char const *what, const tinytc_location_t *, void *) { std::cerr << what << std::endl; },
        nullptr);
    auto bundle = gpu_rt->get_kernel_bundle(parse_string(code, ctx));
    auto kernel = gpu_rt->get_kernel(bundle, "load_block2d");

    gpu_rt->set_mem_arg(kernel, 0, A, auto_mem_type_v<runtime_class::mem_t>);
    gpu_rt->set_mem_arg(kernel, 1, B, auto_mem_type_v<runtime_class::mem_t>);
    gpu_rt->submit(kernel);
    gpu_rt->synchronize();

    gpu_rt->memcpy_d2h(B_host.data(), B, B_host.size() * sizeof(half));

    for (std::size_t j = 0; j < 128; ++j) {
        for (std::size_t i = 0; i < 128; ++i) {
            REQUIRE(A_host[i + j * 128] == B_host[i + j * 128]);
        }
    }

    gpu_rt->free_buffer(A);
    gpu_rt->free_buffer(B);
}

TEST_CASE(RUNTIME_NAME " matmul dpas f16") {
    auto gpu_rt = std::make_shared<runtime_class>();

    const std::string code = R"TinyTL(
func @matmul_dpas(%A: memref<f16x64x64> {align=64},
                  %B: memref<f16x64x64> {align=64},
                  %C: memref<f32x64x64> {align=64})
    attributes{subgroup_size=16,work_group_size=[16,1]} {
    parallel {
        %0 = constant 0 : index
        %1 = cooperative_matrix_load.n %A[%0,%0] : coopmatrix<f16x32x32,matrix_a>
        %2 = cooperative_matrix_load.n %B[%0,%0] : coopmatrix<f16x32x16,matrix_b>
        %3 = constant 0.0 : coopmatrix<f32x32x16,matrix_acc>
        %4 = cooperative_matrix_mul_add %1, %2, %3 : coopmatrix<f32x32x16,matrix_acc>
        cooperative_matrix_store %4, %C[%0,%0]
    }
})TinyTL";

    constexpr auto poison = half(-1293.0f);
    auto A_host = std::vector<half>(64 * 64, poison);
    auto B_host = std::vector<half>(64 * 64, poison);
    auto C_host = std::vector<float>(64 * 64, poison);
    constexpr double f16_eps = 0.0009765625;
    for (std::size_t j = 0; j < 64; ++j) {
        for (std::size_t i = 0; i < 64; ++i) {
            A_host[i + j * 64] = half(static_cast<float>((1.0 + i * f16_eps) * (j + 1) / 32));
            B_host[i + j * 64] = half(static_cast<float>(1.0 / ((i + 1) * (1 + j * f16_eps))));
        }
    }

    auto A = gpu_rt->create_buffer(64 * 64 * sizeof(half));
    auto B = gpu_rt->create_buffer(64 * 64 * sizeof(half));
    auto C = gpu_rt->create_buffer(64 * 64 * sizeof(float));
    gpu_rt->memcpy_h2d(A, A_host.data(), A_host.size() * sizeof(half));
    gpu_rt->memcpy_h2d(B, B_host.data(), B_host.size() * sizeof(half));
    gpu_rt->memcpy_h2d(C, C_host.data(), C_host.size() * sizeof(float));

    auto ctx = make_compiler_context();
    ctx.set_error_reporter(
        [](char const *what, const tinytc_location_t *, void *) { std::cerr << what << std::endl; },
        nullptr);
    auto bundle = gpu_rt->get_kernel_bundle(parse_string(code, ctx));
    auto kernel = gpu_rt->get_kernel(bundle, "matmul_dpas");

    gpu_rt->set_mem_arg(kernel, 0, A, auto_mem_type_v<runtime_class::mem_t>);
    gpu_rt->set_mem_arg(kernel, 1, B, auto_mem_type_v<runtime_class::mem_t>);
    gpu_rt->set_mem_arg(kernel, 2, C, auto_mem_type_v<runtime_class::mem_t>);
    gpu_rt->submit(kernel);
    gpu_rt->synchronize();

    gpu_rt->memcpy_d2h(C_host.data(), C, C_host.size() * sizeof(float));

    constexpr auto tol = 0.00195790082582908423;
    for (std::size_t j = 0; j < 16; ++j) {
        for (std::size_t i = 0; i < 32; ++i) {
            REQUIRE(C_host[i + j * 64] ==
                    doctest::Approx(static_cast<float>(1 + i * f16_eps) / (1 + j * f16_eps))
                        .epsilon(tol));
        }
    }

    gpu_rt->free_buffer(A);
    gpu_rt->free_buffer(B);
    gpu_rt->free_buffer(C);
}

#endif // XMX_20250120_HPP
