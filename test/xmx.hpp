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

template <typename T> struct poison {
    constexpr static T value = -1293.f;
};
template <> struct poison<std::int8_t> {
    constexpr static std::int8_t value = -42;
};
template <typename T> constexpr T poison_v = poison<T>::value;

template <typename T> class test_matrix {
  public:
    using value_type = T;
    constexpr static T poison = poison_v<T>;

    test_matrix(std::int64_t rows, std::int64_t cols, T initial_value = poison)
        : rows_{rows}, cols_{cols}, data_(rows * cols, initial_value) {}

    inline auto rows() const -> std::int64_t { return rows_; }
    inline auto cols() const -> std::int64_t { return cols_; }
    inline auto bytes() const -> std::size_t { return rows_ * cols_ * sizeof(T); }

    inline auto data() -> T * { return data_.data(); }
    inline auto data() const -> T const * { return data_.data(); }

    inline auto operator()(std::int64_t i, std::int64_t j) -> T & { return data_[i + j * rows_]; }
    inline auto operator()(std::int64_t i, std::int64_t j) const -> T const & {
        return data_[i + j * rows_];
    }

  private:
    std::int64_t rows_, cols_;
    std::vector<T> data_;
};

template <typename... T>
void run_custom_test_case(std::string const &code, char const *kernel_name, T &...matrices) {
    struct buffer_pair {
        void *host;
        std::size_t size;
        bool is_const;
        runtime_class::mem_t device;
    };

    auto gpu_rt = std::make_shared<runtime_class>();

    auto buffers = std::vector<buffer_pair>{};
    buffers.reserve(sizeof...(T));

    (buffers.emplace_back(buffer_pair{const_cast<typename T::value_type *>(matrices.data()),
                                      matrices.bytes(), std::is_const_v<T>, nullptr}),
     ...);

    for (auto &bp : buffers) {
        bp.device = gpu_rt->create_buffer(bp.size);
        gpu_rt->memcpy_h2d(bp.device, bp.host, bp.size);
    }

    auto ctx = make_compiler_context();
    set_error_reporter(ctx, [](char const *what, const tinytc_location_t *, void *) {
        std::cerr << what << std::endl;
    });
    auto bundle = gpu_rt->get_kernel_bundle(parse_string(code, ctx),
                                            tinytc_core_feature_flag_large_register_file);
    auto kernel = gpu_rt->get_kernel(bundle, kernel_name);

    for (std::size_t i = 0; i < buffers.size(); ++i) {
        gpu_rt->set_mem_arg(kernel, i, buffers[i].device, auto_mem_type_v<runtime_class::mem_t>);
    }
    gpu_rt->submit(kernel);
    gpu_rt->synchronize();

    for (auto &bp : buffers) {
        if (!bp.is_const) {
            gpu_rt->memcpy_d2h(bp.host, bp.device, bp.size);
        }
        gpu_rt->free_buffer(bp.device);
    }
}

TEST_CASE(RUNTIME_NAME " store block2d f16") {
    const std::string code = R"TinyTL(
func @store_block2d(%A: memref<f16x128x128> {alignment=128})
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

    constexpr std::int64_t N = 128;

    auto A = test_matrix<half>(N, N);

    run_custom_test_case(code, "store_block2d", A);

    for (std::int64_t j = 0; j < A.cols(); ++j) {
        for (std::int64_t i = 0; i < A.rows(); ++i) {
            if (i >= 32 && i < 48 && j >= 64 && j < 72) {
                REQUIRE(A(i, j) == half(42.0f));
            } else if (i >= 62 && i < 94 && j >= 17 && j < 33) {
                REQUIRE(A(i, j) == half(43.0f));
            } else {
                REQUIRE(A(i, j) == A.poison);
            }
        }
    }
}

TEST_CASE(RUNTIME_NAME " load block2d f16") {
    const std::string code = R"TinyTL(
func @load_block2d(%A: memref<f16x128x128> {alignment=128},
                   %B: memref<f16x128x128> {alignment=128})
    attributes{subgroup_size=16,work_group_size=[16,1]} {
    parallel {
        %0 = constant 4 : index
        %1 = constant 8 : index
        %2 = cooperative_matrix_load %A[%0,%1] : coopmatrix<f16x32x16,matrix_acc>
        cooperative_matrix_store %2, %B[%0,%1]
    }
})TinyTL";

    constexpr std::int64_t N = 128;

    const auto A = [] {
        auto A = test_matrix<half>(N, N);
        for (std::int64_t j = 8; j < 24; ++j) {
            for (std::int64_t i = 4; i < 36; ++i) {
                A(i, j) = half(static_cast<float>(i + j * A.rows()));
            }
        }
        return A;
    }();
    auto B = test_matrix<half>(N, N);

    run_custom_test_case(code, "load_block2d", A, B);

    for (std::int64_t j = 0; j < A.cols(); ++j) {
        for (std::int64_t i = 0; i < A.rows(); ++i) {
            REQUIRE(A(i, j) == B(i, j));
        }
    }
}

TEST_CASE(RUNTIME_NAME " matmul dpas f16") {
    auto gpu_rt = std::make_shared<runtime_class>();

    const std::string code = R"TinyTL(
func @matmul_dpas(%A: memref<f16x64x64> {alignment=64},
                  %B: memref<f16x64x64> {alignment=64},
                  %C: memref<f32x64x64> {alignment=64})
    attributes{subgroup_size=16,work_group_size=[16,1]} {
    parallel {
        %0 = constant 0 : index
        %1 = cooperative_matrix_load %A[%0,%0] : coopmatrix<f16x32x32,matrix_a>
        %2 = cooperative_matrix_load %B[%0,%0] : coopmatrix<f16x32x16,matrix_b>
        %3 = constant 0.0 : coopmatrix<f32x32x16,matrix_acc>
        %4 = cooperative_matrix_mul_add %1, %2, %3 : coopmatrix<f32x32x16,matrix_acc>
        cooperative_matrix_store %4, %C[%0,%0]
    }
})TinyTL";

    constexpr std::int64_t N = 64;

    constexpr double f16_eps = 0.0009765625;
    const auto A = [] {
        auto A = test_matrix<half>(N, N);
        for (std::int64_t j = 0; j < N; ++j) {
            for (std::int64_t i = 0; i < N; ++i) {
                A(i, j) = half(static_cast<float>((1.0 + i * f16_eps) * (j + 1) / 32));
            }
        }
        return A;
    }();
    const auto B = [] {
        auto B = test_matrix<half>(N, N);
        for (std::int64_t j = 0; j < N; ++j) {
            for (std::int64_t i = 0; i < N; ++i) {
                B(i, j) = half(static_cast<float>(1.0 / ((i + 1) * (1 + j * f16_eps))));
            }
        }
        return B;
    }();
    auto C = test_matrix<float>(N, N);

    run_custom_test_case(code, "matmul_dpas", A, B, C);

    constexpr auto tol = 0.00195790082582908423;
    for (std::int64_t j = 0; j < 16; ++j) {
        for (std::int64_t i = 0; i < 32; ++i) {
            REQUIRE(C(i, j) ==
                    doctest::Approx(static_cast<float>(1 + i * f16_eps) / (1 + j * f16_eps))
                        .epsilon(tol));
        }
    }
}

TEST_CASE(RUNTIME_NAME " matmul transposed dpas f16") {
    auto gpu_rt = std::make_shared<runtime_class>();

    const std::string code = R"TinyTL(
func @matmul_dpas(%A: memref<f16x64x64>,
                  %B: memref<f16x32x64>,
                  %C: memref<f32x64x128>)
    attributes{subgroup_size=16,work_group_size=[16,1]} {
    parallel {
        %0 = constant 0 : index
        %1 = cooperative_matrix_load %A[%0,%0] : coopmatrix<f16x32x16,matrix_a>
        %2 = cooperative_matrix_load.t %B[%0,%0] : coopmatrix<f16x16x32,matrix_b>
        %3 = constant 0.0 : coopmatrix<f32x32x32,matrix_acc>
        %4 = cooperative_matrix_mul_add %1, %2, %3 : coopmatrix<f32x32x32,matrix_acc>
        cooperative_matrix_store %4, %C[%0,%0]

        %5 = cooperative_matrix_load %A[%0,%0] : coopmatrix<f16x32x32,matrix_a>
        %6 = cooperative_matrix_load.t %B[%0,%0] : coopmatrix<f16x32x16,matrix_b>
        %7 = constant 0.0 : coopmatrix<f32x32x16,matrix_acc>
        %8 = cooperative_matrix_mul_add %5, %6, %7 : coopmatrix<f32x32x16,matrix_acc>
        %c32 = constant 32 : index
        cooperative_matrix_store %8, %C[%0,%c32]

        %9 = cooperative_matrix_load %A[%0,%0] : coopmatrix<f16x32x32,matrix_a>
        %10 = cooperative_matrix_load.t %B[%0,%0] : coopmatrix<f16x32x32,matrix_b>
        %11 = constant 0.0 : coopmatrix<f32x32x32,matrix_acc>
        %12 = cooperative_matrix_mul_add %9, %10, %11 : coopmatrix<f32x32x32,matrix_acc>
        %c64 = constant 64 : index
        cooperative_matrix_store %12, %C[%0,%c64]

        %13 = cooperative_matrix_load.t %B[%0,%0] : coopmatrix<f16x32x16,matrix_a>
        %14 = cooperative_matrix_load %A[%0,%0] : coopmatrix<f16x16x32,matrix_b>
        %15 = cooperative_matrix_mul_add %13, %14, %3 : coopmatrix<f32x32x32,matrix_acc>
        cooperative_matrix_store %15, %C[%c32,%0]

        %16 = cooperative_matrix_load.t %B[%0,%0] : coopmatrix<f16x16x32,matrix_a>
        %17 = cooperative_matrix_load %A[%0,%0] : coopmatrix<f16x32x32,matrix_b>
        %18 = constant 0.0 : coopmatrix<f32x16x32,matrix_acc>
        %19 = cooperative_matrix_mul_add %16, %17, %18 : coopmatrix<f32x16x32,matrix_acc>
        cooperative_matrix_store %19, %C[%c32,%c32]

        %20 = cooperative_matrix_load.t %B[%0,%0] : coopmatrix<f16x32x32,matrix_a>
        %21 = cooperative_matrix_load %A[%0,%0] : coopmatrix<f16x32x32,matrix_b>
        %22 = cooperative_matrix_mul_add %20, %21, %11 : coopmatrix<f32x32x32,matrix_acc>
        cooperative_matrix_store %22, %C[%c32,%c64]
    }
})TinyTL";

    constexpr std::int64_t M = 64, N = 32, K = 64;

    const auto A = [] {
        auto A = test_matrix<half>(M, K, 0.0f);
        for (std::int64_t i = 0; i < M; ++i) {
            A(i, i) = half(1.0f);
        }
        return A;
    }();
    const auto B = [] {
        auto B = test_matrix<half>(N, K);
        for (std::int64_t j = 0; j < K; ++j) {
            for (std::int64_t i = 0; i < N; ++i) {
                B(i, j) = half(static_cast<float>(i + j * N));
            }
        }
        return B;
    }();
    auto C = test_matrix<float>(M, 128);

    run_custom_test_case(code, "matmul_dpas", A, B, C);

    auto const check = [&](std::int64_t i0, std::int64_t i1, std::int64_t j0, std::int64_t j1) {
        for (std::int64_t j = j0; j < j1; ++j) {
            for (std::int64_t i = i0; i < i1; ++i) {
                REQUIRE(C(i, j) == B(j - j0, i - i0));
            }
        }
    };
    check(0, 16, 0, 32);
    check(0, 32, 32, 48);
    check(0, 32, 64, 96);
    check(32, 64, 0, 16);
    check(32, 48, 32, 64);
    check(32, 64, 64, 96);
}

TEST_CASE(RUNTIME_NAME " load and store block2d on local memory f16") {
    auto gpu_rt = std::make_shared<runtime_class>();

    const std::string code = R"TinyTL(
func @load_store_block2d_slm(%A: memref<f16x128x128> {alignment=128},
                             %B: memref<f16x128x128> {alignment=128})
    attributes{subgroup_size=16,work_group_size=[16,1]} {
    %tmp = alloca {alignment=64} : memref<f16x32x32,local>
    parallel {
        %0 = constant 4 : index
        %1 = constant 8 : index
        %2 = cooperative_matrix_load %A[%0,%1] : coopmatrix<f16x16x8,matrix_acc>
        barrier.global
        %3 = constant 16 : index
        %4 = constant 8 : index
        cooperative_matrix_store %2, %tmp[%3,%4]
        barrier.local
        %6 = cooperative_matrix_load %tmp[%3,%4] : coopmatrix<f16x16x8,matrix_acc>
        cooperative_matrix_store %6, %B[%0,%1]

        %7 = constant 64 : index
        %8 = constant 32 : index
        %9 = cooperative_matrix_load %A[%7,%8] : coopmatrix<f16x32x32,matrix_acc>
        barrier.global
        %c0 = constant 0 : index
        cooperative_matrix_store %9, %tmp[%c0,%c0]
        barrier.local
        %10 = cooperative_matrix_load %tmp[%c0,%c0] : coopmatrix<f16x32x32,matrix_acc>
        barrier.local
        cooperative_matrix_store %10, %B[%7,%8]
    }
})TinyTL";

    constexpr std::int64_t N = 128;

    const auto A = [] {
        auto A = test_matrix<half>(N, N);
        for (std::int64_t j = 8; j < 16; ++j) {
            for (std::int64_t i = 4; i < 20; ++i) {
                A(i, j) = half(static_cast<float>(i + j * N));
            }
        }
        for (std::int64_t j = 32; j < 64; ++j) {
            for (std::int64_t i = 64; i < 96; ++i) {
                A(i, j) = half(static_cast<float>(i + j * N));
            }
        }
        return A;
    }();
    auto B = test_matrix<half>(N, N);

    run_custom_test_case(code, "load_store_block2d_slm", A, B);

    for (std::int64_t j = 0; j < N; ++j) {
        for (std::int64_t i = 0; i < N; ++i) {
            REQUIRE(A(i, j) == B(i, j));
        }
    }
}

TEST_CASE(RUNTIME_NAME " load block2d on local memory with vnni f16") {
    auto gpu_rt = std::make_shared<runtime_class>();

    const std::string code = R"TinyTL(
func @dpas_slm(%B: memref<f16x128x128>,
               %C: memref<f32x128x128>)
    attributes{subgroup_size=16,work_group_size=[16,1]} {
    %0 = constant 0 : index
    %A = alloca {alignment=128} : memref<f16x32x32,local>
    %n = constant 32 : index
    %N = constant 128 : index
    foreach (%i,%j)=(%0,%0),(%n,%n) {
        %1 = mul %j, %n : index
        %2 = add %i, %1 : index
        %3 = cast %2 : f16
        store %3, %A[%i,%j]
    }
    parallel {
        barrier.local
        %1 = cooperative_matrix_load.t %A[%0,%0] : coopmatrix<f16x32x16,matrix_a>
        %2 = cooperative_matrix_load %B[%0,%0] : coopmatrix<f16x16x32,matrix_b>
        %3 = constant 0.0 : coopmatrix<f32x32x32,matrix_acc>
        %4 = cooperative_matrix_mul_add %1, %2, %3 : coopmatrix<f32x32x32,matrix_acc>
        cooperative_matrix_store %4, %C[%0,%0]

        %5 = cooperative_matrix_load.t %A[%0,%0] : coopmatrix<f16x16x32,matrix_a>
        %6 = cooperative_matrix_load %B[%0,%0] : coopmatrix<f16x32x32,matrix_b>
        %7 = constant 0.0 : coopmatrix<f32x16x32,matrix_acc>
        %8 = cooperative_matrix_mul_add %5, %6, %7 : coopmatrix<f32x16x32,matrix_acc>
        %c32 = constant 32 : index
        cooperative_matrix_store %8, %C[%c32,%0]

        %9 = cooperative_matrix_load.t %A[%0,%0] : coopmatrix<f16x32x32,matrix_a>
        %10 = cooperative_matrix_load %B[%0,%0] : coopmatrix<f16x32x32,matrix_b>
        %11 = cooperative_matrix_mul_add %9, %10, %3 : coopmatrix<f32x32x32,matrix_acc>
        cooperative_matrix_store %11, %C[%c32,%c32]
    }
})TinyTL";

    constexpr std::int64_t n = 32;
    constexpr std::int64_t N = 128;

    const auto B = [] {
        auto B = test_matrix<half>(N, N, half{0.0f});
        for (std::int64_t i = 0; i < N; ++i) {
            B(i, i) = half{1.0f};
        }
        return B;
    }();
    auto C = test_matrix<float>(N, N);

    run_custom_test_case(code, "dpas_slm", B, C);

    auto const check = [&](std::int64_t i0, std::int64_t i1, std::int64_t j0, std::int64_t j1) {
        for (std::int64_t j = j0; j < j1; ++j) {
            for (std::int64_t i = i0; i < i1; ++i) {
                REQUIRE(C(i, j) == static_cast<float>(j - j0 + (i - i0) * n));
            }
        }
    };
    check(0, 32, 0, 16);
    check(32, 48, 0, 32);
    check(32, 64, 32, 64);
}

TEST_CASE(RUNTIME_NAME " load block2d on local memory with transpose f16") {
    auto gpu_rt = std::make_shared<runtime_class>();

    const std::string code = R"TinyTL(
func @dpas_slm(%A: memref<f16x128x128>,
               %C: memref<f32x128x128>)
    attributes{subgroup_size=16,work_group_size=[16,1]} {
    %0 = constant 0 : index
    %B = alloca {alignment=128} : memref<f16x32x32,local>
    %n = constant 32 : index
    %N = constant 128 : index
    foreach (%i,%j)=(%0,%0),(%n,%n) {
        %1 = mul %j, %n : index
        %2 = add %i, %1 : index
        %3 = cast %2 : f16
        store %3, %B[%i,%j]
    }
    parallel {
        barrier.local
        %1 = cooperative_matrix_load %A[%0,%0] : coopmatrix<f16x32x16,matrix_a>
        %2 = cooperative_matrix_load.t %B[%0,%0] : coopmatrix<f16x16x32,matrix_b>
        %3 = constant 0.0 : coopmatrix<f32x32x32,matrix_acc>
        %4 = cooperative_matrix_mul_add %1, %2, %3 : coopmatrix<f32x32x32,matrix_acc>
        cooperative_matrix_store %4, %C[%0,%0]

        %5 = cooperative_matrix_load %A[%0,%0] : coopmatrix<f16x32x32,matrix_a>
        %6 = cooperative_matrix_load.t %B[%0,%0] : coopmatrix<f16x32x16,matrix_b>
        %7 = constant 0.0 : coopmatrix<f32x32x16,matrix_acc>
        %8 = cooperative_matrix_mul_add %5, %6, %7 : coopmatrix<f32x32x16,matrix_acc>
        %c32 = constant 32 : index
        cooperative_matrix_store %8, %C[%c32,%0]

        %9 = cooperative_matrix_load %A[%0,%0] : coopmatrix<f16x32x32,matrix_a>
        %10 = cooperative_matrix_load.t %B[%0,%0] : coopmatrix<f16x32x32,matrix_b>
        %11 = cooperative_matrix_mul_add %9, %10, %3 : coopmatrix<f32x32x32,matrix_acc>
        cooperative_matrix_store %11, %C[%c32,%c32]
    }
})TinyTL";

    constexpr std::int64_t n = 32;
    constexpr std::int64_t N = 128;

    const auto A = [] {
        auto A = test_matrix<half>(N, N, half{0.0f});
        for (std::int64_t i = 0; i < N; ++i) {
            A(i, i) = half{1.0f};
        }
        return A;
    }();
    auto C = test_matrix<float>(N, N);

    run_custom_test_case(code, "dpas_slm", A, C);

    auto const check = [&](std::int64_t i0, std::int64_t i1, std::int64_t j0, std::int64_t j1) {
        for (std::int64_t j = j0; j < j1; ++j) {
            for (std::int64_t i = i0; i < i1; ++i) {
                REQUIRE(C(i, j) == static_cast<float>(j - j0 + (i - i0) * n));
            }
        }
    };
    check(0, 16, 0, 32);
    check(32, 64, 0, 16);
    check(32, 64, 32, 64);
}

TEST_CASE(RUNTIME_NAME " load block2d i32") {
    const std::string code = R"TinyTL(
func @load_block2d(%A: memref<i32x128x128>,
                   %B: memref<i32x128x128>)
    attributes{subgroup_size=16,work_group_size=[16,1]} {
    parallel {
        %0 = constant 4 : index
        %1 = constant 8 : index
        %2 = cooperative_matrix_load %A[%0,%1] : coopmatrix<i32x32x16,matrix_acc>
        cooperative_matrix_store %2, %B[%0,%1]
    }
})TinyTL";

    constexpr std::int64_t N = 128;

    const auto A = [] {
        auto A = test_matrix<std::int32_t>(N, N);
        for (std::int64_t j = 8; j < 24; ++j) {
            for (std::int64_t i = 4; i < 36; ++i) {
                A(i, j) = i + j * A.rows();
            }
        }
        return A;
    }();
    auto B = test_matrix<std::int32_t>(N, N);

    run_custom_test_case(code, "load_block2d", A, B);

    for (std::int64_t j = 0; j < A.cols(); ++j) {
        for (std::int64_t i = 0; i < A.rows(); ++i) {
            REQUIRE(A(i, j) == B(i, j));
        }
    }
}

TEST_CASE(RUNTIME_NAME " matmul dpas i8") {
    auto gpu_rt = std::make_shared<runtime_class>();

    const std::string code = R"TinyTL(
func @matmul_dpas(%A: memref<i8x64x64>,
                  %B: memref<i8x64x64>,
                  %C: memref<i32x64x64>)
    attributes{subgroup_size=16,work_group_size=[16,1]} {
    parallel {
        %0 = constant 0 : index
        %1 = cooperative_matrix_load %A[%0,%0] : coopmatrix<i8x32x32,matrix_a>
        %2 = cooperative_matrix_load %B[%0,%0] : coopmatrix<i8x32x16,matrix_b>
        %3 = constant 0 : coopmatrix<i32x32x16,matrix_acc>
        %4 = cooperative_matrix_mul_add %1, %2, %3 : coopmatrix<i32x32x16,matrix_acc>
        cooperative_matrix_store %4, %C[%0,%0]
    }
})TinyTL";

    constexpr std::int64_t N = 64;
    constexpr std::int64_t K = 32;

    const auto A = [] {
        auto A = test_matrix<std::int8_t>(N, N);
        for (std::int64_t j = 0; j < N; ++j) {
            for (std::int64_t i = 0; i < N; ++i) {
                A(i, j) = i + j;
                // A(i, j) = i == j ? 1 : 0;
            }
        }
        return A;
    }();
    const auto B = [] {
        auto B = test_matrix<std::int8_t>(N, N);
        for (std::int64_t j = 0; j < N; ++j) {
            for (std::int64_t i = 0; i < N; ++i) {
                B(i, j) = i - j;
                // B(i, j) = i == j ? 1 : 0;
            }
        }
        return B;
    }();
    auto C = test_matrix<std::int32_t>(N, N);

    run_custom_test_case(code, "matmul_dpas", A, B, C);

    for (std::int64_t j = 0; j < 16; ++j) {
        for (std::int64_t i = 0; i < 32; ++i) {
            std::int32_t ref =
                (i - j) * (K - 1) * K / 2 - i * j * K + (K - 1) * K * (2 * K - 1) / 6;
            REQUIRE(C(i, j) == ref);
        }
    }
}

TEST_CASE(RUNTIME_NAME " matrix use cast f16") {
    auto gpu_rt = std::make_shared<runtime_class>();

    const std::string code = R"TinyTL(
func @use_cast(%A: memref<f16x128x128>,
               %B: memref<f32x128x128>,
               %C: memref<f32x128x128>)
    attributes{subgroup_size=16,work_group_size=[16,1]} {
    parallel {
        ; B use conversion
        %0 = constant 0 : index
        %1 = cooperative_matrix_load %A[%0,%0] : coopmatrix<f16x32x32,matrix_a>
        %2 = cooperative_matrix_load %B[%0,%0] : coopmatrix<f32x32x32,matrix_acc>
        %3 = constant 0.0 : coopmatrix<f32x32x32,matrix_acc>
        %4 = cast %2 : coopmatrix<f16x32x32,matrix_b>
        %5 = cooperative_matrix_mul_add %1, %4, %3 : coopmatrix<f32x32x32,matrix_acc>
        cooperative_matrix_store %5, %C[%0,%0]

        ; A use conversion
        %6 = constant 32 : index
        %7 = cooperative_matrix_load %B[%0,%0] : coopmatrix<f32x32x32,matrix_acc>
        %8 = cooperative_matrix_load %A[%0,%0] : coopmatrix<f16x32x32,matrix_b>
        %10 = cast %7 : coopmatrix<f16x32x32,matrix_a>
        %11 = cooperative_matrix_mul_add %10, %8, %3 : coopmatrix<f32x32x32,matrix_acc>
        cooperative_matrix_store %11, %C[%6,%0]
    }
})TinyTL";

    constexpr std::int64_t n = 32;
    constexpr std::int64_t N = 128;

    const auto A = [] {
        auto A = test_matrix<half>(N, N, half{0.0f});
        for (std::int64_t i = 0; i < N; ++i) {
            A(i, i) = half{1.0f};
        }
        return A;
    }();
    const auto B = [] {
        auto B = test_matrix<float>(N, N);
        for (std::int64_t j = 0; j < N; ++j) {
            for (std::int64_t i = 0; i < N; ++i) {
                B(i, j) = static_cast<float>(i + j * n);
            }
        }
        return B;
    }();
    auto C = test_matrix<float>(N, N);

    run_custom_test_case(code, "use_cast", A, B, C);

    auto const check = [&](std::int64_t i0, std::int64_t i1, std::int64_t j0, std::int64_t j1) {
        for (std::int64_t j = j0; j < j1; ++j) {
            for (std::int64_t i = i0; i < i1; ++i) {
                REQUIRE(C(i, j) == static_cast<float>((i - i0) + (j - j0) * n));
            }
        }
    };
    check(0, 32, 0, 32);
    check(32, 64, 0, 32);
}

TEST_CASE(RUNTIME_NAME " matrix use cast i8") {
    auto gpu_rt = std::make_shared<runtime_class>();

    const std::string code = R"TinyTL(
func @use_cast(%A: memref<i8x128x128>,
               %B: memref<i32x128x128>,
               %C: memref<i32x128x128>)
    attributes{subgroup_size=16,work_group_size=[16,1]} {
    parallel {
        ; B use conversion
        %0 = constant 0 : index
        %1 = cooperative_matrix_load %A[%0,%0] : coopmatrix<i8x16x32,matrix_a>
        %2 = cooperative_matrix_load %B[%0,%0] : coopmatrix<i32x32x8,matrix_acc>
        %3 = constant 0 : coopmatrix<i32x16x8,matrix_acc>
        %4 = cast %2 : coopmatrix<i8x32x8,matrix_b>
        %5 = cooperative_matrix_mul_add %1, %4, %3 : coopmatrix<i32x16x8,matrix_acc>
        cooperative_matrix_store %5, %C[%0,%0]

        ; A use conversion
        %6 = constant 32 : index
        %7 = cooperative_matrix_load %B[%0,%0] : coopmatrix<i32x16x32,matrix_acc>
        %8 = cooperative_matrix_load %A[%0,%0] : coopmatrix<i8x32x8,matrix_b>
        %10 = cast %7 : coopmatrix<i8x16x32,matrix_a>
        %11 = cooperative_matrix_mul_add %10, %8, %3 : coopmatrix<i32x16x8,matrix_acc>
        cooperative_matrix_store %11, %C[%6,%0]
    }
})TinyTL";

    constexpr std::int64_t N = 128;

    const auto A = [] {
        auto A = test_matrix<std::int8_t>(N, N, std::int8_t{0});
        for (std::int64_t i = 0; i < N; ++i) {
            A(i, i) = std::int8_t{1};
        }
        return A;
    }();
    const auto B = [] {
        auto B = test_matrix<std::int32_t>(N, N);
        for (std::int64_t j = 0; j < N; ++j) {
            for (std::int64_t i = 0; i < N; ++i) {
                B(i, j) = static_cast<std::int32_t>(i + j);
            }
        }
        return B;
    }();
    auto C = test_matrix<std::int32_t>(N, N);

    run_custom_test_case(code, "use_cast", A, B, C);

    auto const check = [&](std::int64_t i0, std::int64_t i1, std::int64_t j0, std::int64_t j1) {
        for (std::int64_t j = j0; j < j1; ++j) {
            for (std::int64_t i = i0; i < i1; ++i) {
                REQUIRE(C(i, j) == static_cast<float>((i - i0) + (j - j0)));
            }
        }
    };
    check(0, 16, 0, 8);
    check(32, 48, 0, 8);
}

TEST_CASE(RUNTIME_NAME " matrix reduction i16") {
    auto gpu_rt = std::make_shared<runtime_class>();

    const std::string code = R"TinyTL(
func @reduction(%A: memref<i16x128x128>,
                %B: memref<i16x128x128>)
    attributes{subgroup_size=16,work_group_size=[16,1]} {
    parallel {
        %0 = constant 0 : index
        %1 = cooperative_matrix_load %A[%0,%0] : coopmatrix<i16x16x16,matrix_acc>
        %2 = cooperative_matrix_reduce_add.column %1 : coopmatrix<i16x1x16,matrix_acc>
        cooperative_matrix_store %2, %B[%0,%0]

        %c1 = constant 1 : index
        %3 = cooperative_matrix_load %A[%0,%0] : coopmatrix<i16x16x7,matrix_acc>
        %4 = cooperative_matrix_reduce_add.column %3 : coopmatrix<i16x1x7,matrix_acc>
        cooperative_matrix_store %4, %B[%c1,%0]

        %c2 = constant 2 : index
        %5 = cooperative_matrix_load %A[%0,%0] : coopmatrix<i16x32x32,matrix_acc>
        %6 = cooperative_matrix_reduce_add.column %5 : coopmatrix<i16x1x32,matrix_acc>
        cooperative_matrix_store %6, %B[%c2,%0]

        %c3 = constant 3 : index
        %7 = cooperative_matrix_load %A[%0,%0] : coopmatrix<i16x16x16,matrix_acc>
        %8 = cooperative_matrix_reduce_add.row %7 : coopmatrix<i16x16x1,matrix_acc>
        cooperative_matrix_store %8, %B[%c3,%0]

        %9 = cooperative_matrix_load %A[%0,%0] : coopmatrix<i16x16x7,matrix_acc>
        %10 = cooperative_matrix_reduce_add.row %9 : coopmatrix<i16x16x1,matrix_acc>
        cooperative_matrix_store %10, %B[%c3,%c1]

        %11 = cooperative_matrix_load %A[%0,%0] : coopmatrix<i16x32x32,matrix_acc>
        %12 = cooperative_matrix_reduce_add.row %11 : coopmatrix<i16x32x1,matrix_acc>
        cooperative_matrix_store %12, %B[%c3,%c2]

        ;%13 = cooperative_matrix_load %A[%0,%0] : coopmatrix<i16x4x42,matrix_acc>
        ;%14 = cooperative_matrix_reduce_add.row %13 : coopmatrix<i16x4x1,matrix_acc>
        ;cooperative_matrix_store %14, %B[%c3,%c3]
    }
})TinyTL";

    constexpr std::int64_t N = 128;

    const auto A = [] {
        auto A = test_matrix<std::int16_t>(N, N);
        for (std::int64_t j = 0; j < N; ++j) {
            for (std::int64_t i = 0; i < N; ++i) {
                A(i, j) = i + j;
            }
        }
        return A;
    }();
    auto B = test_matrix<std::int16_t>(N, N);

    run_custom_test_case(code, "reduction", A, B);

    auto const sum = [&](std::int64_t n) { return (n - 1) * n / 2; };
    auto const check_column = [&](std::int64_t i, std::int64_t j0, std::int64_t j1,
                                  std::int64_t M) {
        for (std::int64_t j = j0; j < j1; ++j) {
            REQUIRE(B(i, j) == sum(M) + (j - j0) * M);
        }
    };
    check_column(0, 0, 16, 16);
    check_column(1, 0, 7, 16);
    check_column(2, 0, 32, 32);

    auto const check_row = [&](std::int64_t i, std::int64_t j0, std::int64_t j1, std::int64_t M) {
        for (std::int64_t j = j0; j < j1; ++j) {
            REQUIRE(B(j, i) == sum(M) + (j - j0) * M);
        }
    };

    check_row(0, 3, 19, 16);
    check_row(1, 3, 19, 7);
    check_row(2, 3, 35, 32);
    // check_row(3, 3, 7, 42);
}

#endif // XMX_20250120_HPP
