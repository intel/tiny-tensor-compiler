// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SMALL_GEMM_BATCHED_20240419_HPP
#define SMALL_GEMM_BATCHED_20240419_HPP

#include "../recipe.hpp"
#include "tinytc/tinytc.hpp"

#include <cstdint>

namespace tinytc {

enum class small_gemm_batched_kernel : std::uint32_t {
    gemm = 0u,
    gemm_beta0 = 1u,
    num_kernels = 2u
};
auto small_gemm_batched_kernel_name(small_gemm_batched_kernel k) -> char const *;

struct small_gemm_batched_recipe : ::tinytc_recipe {
  public:
    small_gemm_batched_recipe(prog prg, binary bin, scalar_type ty);
    auto num_kernels() const -> std::uint32_t override;
    auto kernel_name(std::uint32_t kernel_num) const -> char const * override;

    inline auto ty() const -> scalar_type { return ty_; }

  private:
    scalar_type ty_;
};

} // namespace tinytc

#endif // SMALL_GEMM_BATCHED_20240419_HPP
