// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "../smm.hpp"
#include "../tensor3.hpp"
#include "test_runtime.hpp"

#include "doctest/doctest.h"

using namespace tinytc;

TEST_CASE_TEMPLATE("sycl packed alpha=1 beta=0", T, TEST_PRECISIONS) {
    auto KK = std::vector<std::uint32_t>{1, 9};
    auto MM = std::vector<std::uint32_t>{1, 13, 33, 65};
    auto NN = std::vector<std::uint32_t>{1, 5, 37};
    auto HH = std::vector<std::uint32_t>{1, 100};

    std::uint32_t M, N, K, howmany;
    DOCTEST_TENSOR4_TEST(MM, NN, KK, HH);

    check_small_gemm_batched<T, sycl_test_runtime>(ir::transpose::N, ir::transpose::N, M, N, K, M,
                                                   M * K, K, K * N, M, M * N, 1.0, 0.0, howmany);
}

TEST_CASE_TEMPLATE("sycl non-packed alpha=1 beta=0", T, TEST_PRECISIONS) {
    std::uint32_t M = 16, N = 32, K = 8, howmany = 10;
    std::uint32_t ldA = 20, ldB = 9, ldC = 24;

    check_small_gemm_batched<T, sycl_test_runtime>(ir::transpose::N, ir::transpose::N, M, N, K, ldA,
                                                   ldA * ldB, ldB, ldB * 2 * N, ldC, ldC * 3 * N,
                                                   1.0, 0.0, howmany);
}

TEST_CASE_TEMPLATE("sycl packed alpha=1 beta=1", T, TEST_PRECISIONS) {
    std::uint32_t M = 6, N = 33, K = 8, howmany = 5;

    check_small_gemm_batched<T, sycl_test_runtime>(ir::transpose::N, ir::transpose::N, M, N, K, M,
                                                   M * K, K, K * N, M, M * N, 1.0, 1.0, howmany);
}

TEST_CASE_TEMPLATE("sycl packed alpha=-1 beta=2", T, TEST_PRECISIONS) {
    std::uint32_t M = 8, N = 16, K = 16, howmany = 5;

    check_small_gemm_batched<T, sycl_test_runtime>(ir::transpose::N, ir::transpose::N, M, N, K, M,
                                                   M * K, K, K * N, M, M * N, -1.0, 2.0, howmany);
}

TEST_CASE_TEMPLATE("sycl non-packed alpha=1 beta=0 transA transB", T, TEST_PRECISIONS) {
    std::uint32_t M = 16, N = 32, K = 8, howmany = 10;
    std::uint32_t ldA = 10, ldB = 32, ldC = 24;

    check_small_gemm_batched<T, sycl_test_runtime>(ir::transpose::T, ir::transpose::T, M, N, K, ldA,
                                                   ldA * ldB, ldB, ldB * 2 * N, ldC, ldC * 3 * N,
                                                   1.0, 0.0, howmany);
}
