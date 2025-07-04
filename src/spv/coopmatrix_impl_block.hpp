// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef COOPMATRIX_IMPL_BLOCK_20250428_HPP
#define COOPMATRIX_IMPL_BLOCK_20250428_HPP

#include "spv/coopmatrix_impl.hpp"
#include "tinytc/types.h"

#include <cstdint>

namespace tinytc::spv {

class coopmatrix_impl_block : public coopmatrix_impl {
  public:
    using coopmatrix_impl::coopmatrix_impl;

    auto load(cooperative_matrix_load_inst in, dope_vector const &odv, spv_inst *operand,
              spv_inst *pos0, spv_inst *pos1) -> spv_inst * override;
    void store(cooperative_matrix_store_inst in, dope_vector const &odv, spv_inst *val,
               spv_inst *operand, spv_inst *pos0, spv_inst *pos1) override;

  private:
    auto get_io_sty(tinytc_type_t ty) -> tinytc_type_t;
    auto is_aligned(std::int32_t alignment, tinytc_value const &operand, tinytc_value const &pos0)
        -> bool;
};

} // namespace tinytc::spv

#endif // COOPMATRIX_IMPL_BLOCK_20250428_HPP
