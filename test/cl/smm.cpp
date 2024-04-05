// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "../smm.hpp"
#include "../tensor3.hpp"
#include "test_runtime.hpp"

#include "doctest/doctest.h"

using namespace tinytc;

TEST_CASE_TEMPLATE("level zero packed alpha=1 beta=0", T, TEST_PRECISIONS) {
    auto KK = std::vector<std::uint32_t>{71};
    auto MM = std::vector<std::uint32_t>{27, 43};
    auto NN = std::vector<std::uint32_t>{3, 33};
    auto HH = std::vector<std::uint32_t>{1, 51};

    std::uint32_t M, N, K, howmany;
    DOCTEST_TENSOR4_TEST(MM, NN, KK, HH);

    check_small_gemm_batched<T, opencl_test_runtime>(ir::transpose::N, ir::transpose::N, M, N, K, M,
                                                     M * K, K, K * N, M, M * N, 1.0, 0.0, howmany);
}
