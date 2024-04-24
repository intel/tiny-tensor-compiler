// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TALL_AND_SKINNY_20240422_HPP
#define TALL_AND_SKINNY_20240422_HPP

#include "../recipe.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <cstdint>

namespace tinytc {

enum class tall_and_skinny_kernel : std::uint32_t { gemm = 0u, gemm_beta0 = 1u, num_kernels = 2u };
auto tall_and_skinny_kernel_name(tall_and_skinny_kernel k) -> char const *;

struct tall_and_skinny_recipe : ::tinytc_recipe {
  public:
    tall_and_skinny_recipe(prog prg, binary bin, scalar_type ty, std::uint32_t M_block_size);
    auto num_kernels() const -> std::uint32_t override;
    auto kernel_name(std::uint32_t kernel_num) const -> char const * override;

    inline auto ty() const -> scalar_type { return ty_; }
    inline auto M_block_size() const -> std::uint32_t { return M_block_size_; }

  private:
    scalar_type ty_;
    std::uint32_t M_block_size_;
};

} // namespace tinytc

#endif // TALL_AND_SKINNY_20240422_HPP
