// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef OZ_INT8_20251216_HPP
#define OZ_INT8_20251216_HPP

#include "../gemm_common.hpp"
#include "memory.hpp"

#include <sycl/sycl.hpp>
#include <tinytc/tinytc.hpp>

#include <array>
#include <cstdint>
#include <utility>

struct kernel2d {
    kernel2d(sycl::kernel k, std::int32_t block_size_m, std::int32_t block_size_n);
    auto execution_range(std::int64_t N) -> sycl::nd_range<3u>;

    ::sycl::kernel kernel;
    ::sycl::range<3u> group_size;
    std::int32_t BM, BN;
};

class oz_int8 {
  public:
    oz_int8(tinytc::examples::test_type ty, std::int64_t N, std::int64_t num_splits, sycl::queue q);

    void ref(void *A, void *B, void *C);
    void operator()(void *A, void *B, void *C);

  private:
    auto split_i8(void *A, std::size_t stride0, std::size_t stride1)
        -> std::pair<sycl_unique_ptr<std::int8_t>, sycl_unique_ptr<void>>;

    std::int32_t ty_size_;
    std::int64_t N_, num_splits_;
    sycl::queue q_;
    tinytc::shared_handle<tinytc_compiler_context_t> ctx_;
    std::array<kernel2d, 2u> split_int8_;
    kernel2d acc_f_, gemm_, gemm_s8s8s32_;
};

#endif // OZ_INT8_20251216_HPP
