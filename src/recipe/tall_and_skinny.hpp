// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TALL_AND_SKINNY_20240422_HPP
#define TALL_AND_SKINNY_20240422_HPP

#include "../recipe.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <cstdint>

namespace tinytc {

enum class tall_and_skinny_kernel : int { gemm = 0, gemm_beta0 = 1, num_kernels = 2 };
auto tall_and_skinny_kernel_name(tall_and_skinny_kernel k) -> char const *;

struct tall_and_skinny_recipe : ::tinytc_recipe {
  public:
    tall_and_skinny_recipe(prog prg, source src, scalar_type ty, std::int32_t M_block_size);
    auto num_kernels() const -> int override;
    auto kernel_name(int kernel_num) const -> char const * override;

    inline auto ty() const -> scalar_type { return ty_; }
    inline auto M_block_size() const -> std::int32_t { return M_block_size_; }

  private:
    scalar_type ty_;
    std::int32_t M_block_size_;
};

} // namespace tinytc

#endif // TALL_AND_SKINNY_20240422_HPP
