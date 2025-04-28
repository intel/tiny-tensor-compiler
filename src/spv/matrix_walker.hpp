// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MATRIX_WALKER_20250428_HPP
#define MATRIX_WALKER_20250428_HPP

#include <cstdint>

namespace tinytc {
enum class checked_flag;
enum class matrix_use;
} // namespace tinytc

namespace tinytc::spv {
struct coopmatrix_layout;
class spv_inst;
class uniquifier;

class matrix_walker {
  public:
    matrix_walker(uniquifier &unique, std::int32_t sgs, coopmatrix_layout const &layout,
                  matrix_use use, spv_inst *pos0, spv_inst *pos1, spv_inst *shape0,
                  spv_inst *shape1, spv_inst *stride0, spv_inst *stride1, checked_flag chk,
                  std::int32_t constant_p = -1);

    void advance_block();
    void advance_column();

    auto component_no(std::int32_t col_no) -> std::int32_t;
    auto component_no() -> std::int32_t;
    auto offset() -> spv_inst *;
    auto rows_checked() -> bool;
    auto cols_checked() -> bool;
    auto needs_mask() -> bool;
    auto may_need_mask() -> bool;

    auto col_ok() -> spv_inst *;
    auto row_ok() -> spv_inst *;

  private:
    uniquifier &unique_;
    coopmatrix_layout const &layout_;
    checked_flag chk_;
    spv_inst *index_ty_;
    spv_inst *row_inc_;
    std::int64_t col_inc_factor_;
    spv_inst *col_inc_;
    spv_inst *row_;
    spv_inst *col0_;
    spv_inst *col_;
    bool transpose_;
    spv_inst *row_max_ = nullptr;
    spv_inst *col_max_ = nullptr;
    std::int32_t block_no_ = 0;
    std::int32_t col_no_ = 0;
};

} // namespace tinytc::spv

#endif // MATRIX_WALKER_20250428_HPP
