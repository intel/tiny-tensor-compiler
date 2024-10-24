// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "doctest_util.hpp"
#include "linalg_ops.hpp"
#include "linalg_runner.hpp"
#include "linalg_types.hpp"

#include "doctest/doctest.h"

#include <algorithm>
#include <complex>

using runtime_class = RUNTIME_CLASS;
using namespace tinytc;

TEST_CASE_TEMPLATE(RUNTIME_NAME " gemm packed alpha=1 beta=0", T, TEST_PRECISIONS) {
    auto KK = std::vector<std::int64_t>{56};
    auto MM = std::vector<std::int64_t>{20, 32, 53};
    auto NN = std::vector<std::int64_t>{5, 16, 23};

    std::int64_t M, N, K;
    DOCTEST_TENSOR3_TEST(MM, NN, KK);

    auto op = test::gemm<T, T, T, T, T>(transpose::N, transpose::N, {{M, K}}, {{K, N}}, {{M, N}});
    test::test_blas_a3<runtime_class>(op, 1, 0);
}

TEST_CASE_TEMPLATE(RUNTIME_NAME " gemm non-packed alpha=1 beta=0", T, TEST_PRECISIONS) {
    std::int64_t M = 16, N = 32, K = 8;
    std::int64_t ldA = 20, ldB = 9, ldC = 24;

    auto op = test::gemm<T, T, T, T, T>(transpose::N, transpose::N, {{M, K}, {1, ldA}},
                                        {{K, N}, {1, ldB}}, {{M, N}, {1, ldC}});
    test::test_blas_a3<runtime_class>(op, 1, 0);
}

TEST_CASE_TEMPLATE(RUNTIME_NAME " gemm packed alpha=1 beta=1", T, TEST_PRECISIONS) {
    std::int64_t M = 6, N = 33, K = 8;

    auto op = test::gemm<T, T, T, T, T>(transpose::N, transpose::N, {{M, K}}, {{K, N}}, {{M, N}});
    test::test_blas_a3<runtime_class>(op, 1, 1);
}

TEST_CASE_TEMPLATE(RUNTIME_NAME " gemm packed alpha=-1 beta=2", T, TEST_PRECISIONS) {
    std::int64_t M = 8, N = 16, K = 16;

    auto op = test::gemm<T, T, T, T, T>(transpose::N, transpose::N, {{M, K}}, {{K, N}}, {{M, N}});
    test::test_blas_a3<runtime_class>(op, -1, 2);
}

TEST_CASE_TEMPLATE(RUNTIME_NAME " gemm non-packed alpha=1 beta=0 transA transB", T,
                   TEST_PRECISIONS) {
    std::int64_t M = 16, N = 32, K = 8;
    std::int64_t ldA = 10, ldB = 32, ldC = 24;

    auto op = test::gemm<T, T, T, T, T>(transpose::T, transpose::T, {{K, M}, {1, ldA}},
                                        {{N, K}, {1, ldB}}, {{M, N}, {1, ldC}});
    test::test_blas_a3<runtime_class>(op, 1, 0);
}

TEST_CASE_TEMPLATE(RUNTIME_NAME " gemm non-static M", T, TEST_PRECISIONS) {
    std::int64_t M = 63, N = 43, K = 23;

    auto op = test::gemm<T, T, T, T, T>(
        transpose::N, transpose::N, {{M, K}, {1, M}, {dynamic, K}, {1, dynamic}}, {{K, N}, {1, K}},
        {{M, N}, {1, M}, {dynamic, N}, {1, dynamic}});
    test::test_blas_a3<runtime_class>(op, 1, 1);
}

TEST_CASE_TEMPLATE(RUNTIME_NAME " gemm non-static N", T, TEST_PRECISIONS) {
    std::int64_t M = 63, N = 43, K = 23;

    auto op = test::gemm<T, T, T, T, T>(transpose::N, transpose::N, {{M, K}, {1, M}},
                                        {{K, N}, {1, K}, {K, dynamic}, {1, K}},
                                        {{M, N}, {1, M}, {M, dynamic}, {1, M}});
    test::test_blas_a3<runtime_class>(op, 1, 1);
}

TEST_CASE_TEMPLATE(RUNTIME_NAME " gemm non-static", T, TEST_PRECISIONS) {
    std::int64_t M = 63, N = 43, K = 23;

    auto op = test::gemm<T, T, T, T, T>(transpose::N, transpose::N,
                                        {{M, K}, {1, M}, {dynamic, dynamic}, {1, dynamic}},
                                        {{K, N}, {1, K}, {dynamic, dynamic}, {1, dynamic}},
                                        {{M, N}, {1, M}, {dynamic, dynamic}, {1, dynamic}});
    test::test_blas_a3<runtime_class>(op, 1, 1);
}

TEST_CASE_TEMPLATE(RUNTIME_NAME " gemm packed complex alpha=1 beta=0", T, TEST_PRECISIONS) {
    auto KK = std::vector<std::uint32_t>{53};
    auto MM = std::vector<std::uint32_t>{21, 42};
    auto NN = std::vector<std::uint32_t>{7, 11};

    std::int64_t M, N, K;
    DOCTEST_TENSOR3_TEST(MM, NN, KK);

    using CT = std::complex<T>;
    auto op =
        test::gemm<CT, CT, CT, CT, CT>(transpose::N, transpose::N, {{M, K}}, {{K, N}}, {{M, N}});
    test::test_blas_a3<runtime_class>(op, 1, 0);
}

TEST_CASE_TEMPLATE(RUNTIME_NAME " gemm packed complex alpha=(-1,-2) beta=(2,3)", T,
                   TEST_PRECISIONS) {
    std::int64_t M = 8, N = 16, K = 16;

    using CT = std::complex<T>;
    auto op =
        test::gemm<CT, CT, CT, CT, CT>(transpose::N, transpose::N, {{M, K}}, {{K, N}}, {{M, N}});
    test::test_blas_a3<runtime_class>(op, {-1.0, -2.0}, {2.0, 3.0});
}

TEST_CASE(RUNTIME_NAME " gemm packed mixed precision") {
    auto KK = std::vector<std::uint32_t>{53};
    auto MM = std::vector<std::uint32_t>{21, 42};
    auto NN = std::vector<std::uint32_t>{7, 11};

    std::int64_t M, N, K;
    DOCTEST_TENSOR3_TEST(MM, NN, KK);

    auto op = test::gemm<int, int, int, float, float>(transpose::N, transpose::N, {{M, K}},
                                                      {{K, N}}, {{M, N}});
    test::test_blas_a3<runtime_class>(op, 1, 0);
}

TEST_CASE_TEMPLATE(RUNTIME_NAME " ger packed alpha=1 beta=0", T, TEST_PRECISIONS) {
    auto MM = std::vector<std::int64_t>{10, 32, 45};
    auto NN = std::vector<std::int64_t>{1, 16, 17, 48};

    std::int64_t M, N;
    DOCTEST_TENSOR2_TEST(MM, NN);

    auto op = test::ger<T, T, T, T, T>({{M}}, {{N}}, {{M, N}});
    test::test_blas_a3<runtime_class>(op, 1, 0);
}

TEST_CASE_TEMPLATE(RUNTIME_NAME " hadamard packed alpha=1 beta=0", T, TEST_PRECISIONS) {
    auto MM = std::vector<std::int64_t>{10, 32, 45};

    std::int64_t M;
    DOCTEST_TENSOR1_TEST(MM);

    auto op = test::hadamard<T, T, T, T, T>({{M}}, {{M}}, {{M}});
    test::test_blas_a3<runtime_class>(op, 1, 0);
}
