// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "doctest_util.hpp"
#include "linalg_blas_a2.hpp"
#include "linalg_blas_a3.hpp"
#include "linalg_runner.hpp"
#include "linalg_types.hpp"

#include "doctest/doctest.h"

#include <algorithm>
#include <complex>
#include <cstdint>

using runtime_class = RUNTIME_CLASS;
using namespace tinytc;

TEST_CASE_TEMPLATE(RUNTIME_NAME " axpby 0d", T, TEST_PRECISIONS) {
    auto op = test::axpby<T, T, T, T>(transpose::N, {{}}, {{}});
    test::test_blas_a2<runtime_class>(op, 1, 0);
}

TEST_CASE_TEMPLATE(RUNTIME_NAME " axpby 1d", T, TEST_PRECISIONS) {
    auto MM = std::vector<std::int64_t>{18, 16, 32};

    std::int64_t M = {};
    DOCTEST_TENSOR1_TEST(MM);

    auto op = test::axpby<T, T, T, T>(transpose::N, {{M}}, {{M}});
    test::test_blas_a2<runtime_class>(op, 1, 0);
}

TEST_CASE_TEMPLATE(RUNTIME_NAME " axpby 2d", T, TEST_PRECISIONS) {
    auto MM = std::vector<std::int64_t>{18, 16, 32};
    auto NN = std::vector<std::int64_t>{5, 17};

    std::int64_t M = {}, N = {};
    DOCTEST_TENSOR2_TEST(MM, NN);

    auto op = test::axpby<T, T, T, T>(transpose::N, {{M, N}}, {{M, N}});
    test::test_blas_a2<runtime_class>(op, 1, 0);
}

TEST_CASE_TEMPLATE(RUNTIME_NAME " axpby 2d trans", T, TEST_PRECISIONS) {
    auto MM = std::vector<std::int64_t>{18, 16, 32};
    auto NN = std::vector<std::int64_t>{5, 17};

    std::int64_t M = {}, N = {};
    DOCTEST_TENSOR2_TEST(MM, NN);

    auto op = test::axpby<T, T, T, T>(transpose::T, {{N, M}}, {{M, N}});
    test::test_blas_a2<runtime_class>(op, 1, 0);
}

TEST_CASE_TEMPLATE(RUNTIME_NAME " cumsum 1d", T, TEST_PRECISIONS) {
    auto MM = std::vector<std::int64_t>{18, 16, 32, 123};

    std::int64_t M = {};
    DOCTEST_TENSOR1_TEST(MM);

    auto op = test::cumsum<T, T, T, T>({{M}}, 0, {{M}});
    test::test_blas_a2<runtime_class>(op, 1, 0);
}

TEST_CASE_TEMPLATE(RUNTIME_NAME " cumsum 1d work_group_size=[128,1]", T, TEST_PRECISIONS) {
    auto MM = std::vector<std::int64_t>{123, 435};

    std::int64_t M = {};
    DOCTEST_TENSOR1_TEST(MM);

    auto op = test::cumsum<T, T, T, T>({{M}}, 0, {{M}}, 128);
    test::test_blas_a2<runtime_class>(op, 1, 0);
}

TEST_CASE_TEMPLATE(RUNTIME_NAME " cumsum 2d work_group_size=[64,1]", T, TEST_PRECISIONS) {
    auto MM = std::vector<std::int64_t>{17, 76};
    auto NN = std::vector<std::int64_t>{5, 135};
    auto modes = std::vector<std::int64_t>{0, 1};

    std::int64_t M = {}, N = {}, K = {};
    DOCTEST_TENSOR3_TEST(MM, NN, modes);

    auto op = test::cumsum<T, T, T, T>({{M, N}}, K, {{M, N}}, 64);
    test::test_blas_a2<runtime_class>(op, 1, 0);
}

TEST_CASE_TEMPLATE(RUNTIME_NAME " cumsum 3d work_group_size=[64,1]", T, TEST_PRECISIONS) {
    auto MM = std::vector<std::int64_t>{65};
    auto NN = std::vector<std::int64_t>{65};
    auto KK = std::vector<std::int64_t>{65};
    auto modes = std::vector<std::int64_t>{0, 1, 2};

    std::int64_t M = {}, N = {}, K = {}, howmany = {};
    DOCTEST_TENSOR4_TEST(MM, NN, KK, modes);

    auto op = test::cumsum<T, T, T, T>({{M, N, K}}, howmany, {{M, N, K}}, 64);
    test::test_blas_a2<runtime_class>(op, 1, 0);
}

TEST_CASE_TEMPLATE(RUNTIME_NAME " gemm packed alpha=1 beta=0", T, TEST_PRECISIONS) {
    auto KK = std::vector<std::int64_t>{56};
    auto MM = std::vector<std::int64_t>{20, 32, 53};
    auto NN = std::vector<std::int64_t>{5, 16, 23};

    std::int64_t M = {}, N = {}, K = {};
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

    std::int64_t M = {}, N = {}, K = {};
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

    std::int64_t M = {}, N = {}, K = {};
    DOCTEST_TENSOR3_TEST(MM, NN, KK);

    auto op = test::gemm<std::int16_t, std::int16_t, std::int16_t, float, float>(
        transpose::N, transpose::N, {{M, K}}, {{K, N}}, {{M, N}});
    test::test_blas_a3<runtime_class>(op, 1, 0);
}

TEST_CASE_TEMPLATE(RUNTIME_NAME " gemv packed alpha=1 beta=0", T, TEST_PRECISIONS) {
    auto NN = std::vector<std::uint32_t>{21};
    auto MM = std::vector<std::uint32_t>{16, 23};

    std::int64_t M = {}, N = {};
    DOCTEST_TENSOR2_TEST(MM, NN);

    auto op = test::gemv<T, T, T, T, T>(transpose::N, {{M, N}}, {{N}}, {{M}});
    test::test_blas_a3<runtime_class>(op, 1, 0);
}

TEST_CASE_TEMPLATE(RUNTIME_NAME " gemv packed trans alpha=1 beta=0", T, TEST_PRECISIONS) {
    std::int64_t M = 19, N = 32;

    auto op = test::gemv<T, T, T, T, T>(transpose::T, {{N, M}}, {{N}}, {{M}});
    test::test_blas_a3<runtime_class>(op, 1, 0);
}

TEST_CASE_TEMPLATE(RUNTIME_NAME " gemv packed complex alpha=1 beta=0", T, TEST_PRECISIONS) {
    auto NN = std::vector<std::uint32_t>{5};
    auto MM = std::vector<std::uint32_t>{8, 37};

    std::int64_t M = {}, N = {};
    DOCTEST_TENSOR2_TEST(MM, NN);

    using CT = std::complex<T>;
    auto op = test::gemv<CT, CT, CT, CT, CT>(transpose::N, {{M, N}}, {{N}}, {{M}});
    test::test_blas_a3<runtime_class>(op, 1, 0);
}

TEST_CASE_TEMPLATE(RUNTIME_NAME " ger packed alpha=1 beta=0", T, TEST_PRECISIONS) {
    auto MM = std::vector<std::int64_t>{10, 32, 45};
    auto NN = std::vector<std::int64_t>{1, 16, 17, 48};

    std::int64_t M = {}, N = {};
    DOCTEST_TENSOR2_TEST(MM, NN);

    auto op = test::ger<T, T, T, T, T>({{M}}, {{N}}, {{M, N}});
    test::test_blas_a3<runtime_class>(op, 1, 0);
}

TEST_CASE_TEMPLATE(RUNTIME_NAME " hadamard packed alpha=1 beta=0", T, TEST_PRECISIONS) {
    auto MM = std::vector<std::int64_t>{10, 32, 45};

    std::int64_t M = {};
    DOCTEST_TENSOR1_TEST(MM);

    auto op = test::hadamard<T, T, T, T, T>({{M}}, {{M}}, {{M}});
    test::test_blas_a3<runtime_class>(op, 1, 0);
}

TEST_CASE_TEMPLATE(RUNTIME_NAME " hadamard 2d packed alpha=1 beta=0", T, TEST_PRECISIONS) {
    auto MM = std::vector<std::int64_t>{10, 32, 45};
    auto NN = std::vector<std::int64_t>{5, 16, 42};

    std::int64_t M = {}, N = {};
    DOCTEST_TENSOR2_TEST(MM, NN);

    auto op = test::hadamard<T, T, T, T, T>({{M, N}}, {{M, N}}, {{M, N}});
    test::test_blas_a3<runtime_class>(op, 1, 0);
}

TEST_CASE_TEMPLATE(RUNTIME_NAME " sum 1d", T, TEST_PRECISIONS) {
    auto MM = std::vector<std::int64_t>{18, 16, 32, 123};

    std::int64_t M = {};
    DOCTEST_TENSOR1_TEST(MM);

    auto op = test::sum<T, T, T, T>(transpose::N, {{M}}, {{}});
    test::test_blas_a2<runtime_class>(op, 1, 0);
}

TEST_CASE_TEMPLATE(RUNTIME_NAME " sum 1d work_group_size=[128,1]", T, TEST_PRECISIONS) {
    auto MM = std::vector<std::int64_t>{123, 435};

    std::int64_t M = {};
    DOCTEST_TENSOR1_TEST(MM);

    auto op = test::sum<T, T, T, T>(transpose::N, {{M}}, {{}}, 128);
    test::test_blas_a2<runtime_class>(op, 1, 0);
}

TEST_CASE_TEMPLATE(RUNTIME_NAME " sum 2d", T, TEST_PRECISIONS) {
    auto MM = std::vector<std::int64_t>{18, 16, 32};
    auto NN = std::vector<std::int64_t>{5, 17};

    std::int64_t M = {}, N = {};
    DOCTEST_TENSOR2_TEST(MM, NN);

    auto op = test::sum<T, T, T, T>(transpose::N, {{M, N}}, {{M}});
    test::test_blas_a2<runtime_class>(op, 1, 0);
}

TEST_CASE_TEMPLATE(RUNTIME_NAME " sum 2d trans", T, TEST_PRECISIONS) {
    auto MM = std::vector<std::int64_t>{18, 16, 32};
    auto NN = std::vector<std::int64_t>{5, 17};

    std::int64_t M = {}, N = {};
    DOCTEST_TENSOR2_TEST(MM, NN);

    auto op = test::sum<T, T, T, T>(transpose::T, {{N, M}}, {{M}});
    test::test_blas_a2<runtime_class>(op, 1, 0);
}
