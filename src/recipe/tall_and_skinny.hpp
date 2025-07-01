// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TALL_AND_SKINNY_20240422_HPP
#define TALL_AND_SKINNY_20240422_HPP

#include "../recipe.hpp"
#include "tinytc/types.h"

#include <cstdint>

namespace tinytc {

class binary;
class prog;

enum class tall_and_skinny_kernel : int { gemm = 0, gemm_beta0 = 1, num_kernels = 2 };
auto tall_and_skinny_kernel_name(tall_and_skinny_kernel k) -> char const *;

struct tall_and_skinny_recipe : ::tinytc_recipe {
  public:
    tall_and_skinny_recipe(prog prg, binary bin, tinytc_type_t ty, std::int64_t M, std::int64_t ldA,
                           std::int64_t ldB, std::int64_t ldC, std::int32_t M_block_size);
    auto num_kernels() const -> int override;
    auto kernel_name(int kernel_num) const -> char const * override;

    inline auto ty() const -> tinytc_type_t { return ty_; }
    inline auto M_block_size() const -> std::int32_t { return M_block_size_; }

    inline auto is_M_dynamic() const -> bool { return M_dyn_; }
    inline auto is_ldA_dynamic() const -> bool { return ldA_dyn_; }
    inline auto is_ldB_dynamic() const -> bool { return ldB_dyn_; }
    inline auto is_ldC_dynamic() const -> bool { return ldC_dyn_; }

  private:
    tinytc_type_t ty_;
    bool M_dyn_, ldA_dyn_, ldB_dyn_, ldC_dyn_;
    std::int32_t M_block_size_;
};

} // namespace tinytc

#endif // TALL_AND_SKINNY_20240422_HPP
