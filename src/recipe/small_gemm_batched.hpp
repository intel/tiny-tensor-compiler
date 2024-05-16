// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SMALL_GEMM_BATCHED_20240419_HPP
#define SMALL_GEMM_BATCHED_20240419_HPP

#include "../recipe.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

namespace tinytc {

enum class small_gemm_batched_kernel : int { gemm = 0, gemm_beta0 = 1, num_kernels = 2 };
auto small_gemm_batched_kernel_name(small_gemm_batched_kernel k) -> char const *;

struct small_gemm_batched_recipe : ::tinytc_recipe {
  public:
    small_gemm_batched_recipe(prog prg, source src, scalar_type ty);
    auto num_kernels() const -> int override;
    auto kernel_name(int kernel_num) const -> char const * override;

    inline auto ty() const -> scalar_type { return ty_; }

  private:
    scalar_type ty_;
};

} // namespace tinytc

#endif // SMALL_GEMM_BATCHED_20240419_HPP
