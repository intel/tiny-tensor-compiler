// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "spv/matrix_walker.hpp"
#include "coopmatrix_layout.hpp"
#include "node/type.hpp"
#include "spv/converter_aux.hpp"
#include "spv/defs.hpp"
#include "spv/enums.hpp"
#include "spv/instructions.hpp"
#include "spv/module.hpp"
#include "spv/uniquifier.hpp"
#include "tinytc/types.hpp"

namespace tinytc::spv {

matrix_walker::matrix_walker(uniquifier &unique, std::int32_t sgs, coopmatrix_layout const &layout,
                             spv_inst *pos0, spv_inst *pos1, spv_inst *shape0, spv_inst *shape1,
                             spv_inst *stride0, spv_inst *stride1, checked_flag chk,
                             std::int32_t constant_p)
    : unique_{unique}, layout_{layout}, chk_{chk} {
    index_ty_ = get_spv_index_ty(unique, layout.sty->context());

    auto &mod = unique_.mod();
    auto crows = unique.constant(layout_.rows);
    row_inc_ = mod.add<OpIMul>(index_ty_, crows, stride0);
    col_inc_factor_ = sgs / layout.rows;
    col_inc_ = mod.add<OpIMul>(index_ty_, unique.constant(col_inc_factor_), stride1);

    spv_inst *p = constant_p >= 0 ? unique.constant(constant_p)
                                  : unique.load_builtin(BuiltIn::SubgroupLocalInvocationId);
    p = mod.add<OpSConvert>(index_ty_, p);

    row_ = layout.rows < sgs ? mod.add<OpSRem>(index_ty_, p, crows) : p;
    row_ = mod.add<OpIAdd>(index_ty_, row_, pos0);
    row_ = mod.add<OpIMul>(index_ty_, row_, stride0);

    auto c0 = unique.null_constant(index_ty_);
    col0_ = layout.rows < sgs ? mod.add<OpSDiv>(index_ty_, p, crows) : c0;
    col0_ = mod.add<OpIAdd>(index_ty_, col0_, pos1);
    col0_ = mod.add<OpIMul>(index_ty_, col0_, stride1);
    col_ = col0_;

    if (rows_checked()) {
        row_max_ = mod.add<OpIMul>(index_ty_, shape0, stride0);
    }
    if (may_need_mask() || cols_checked()) {
        col_max_ = mod.add<OpIMul>(index_ty_, shape1, stride1);
    }
}

void matrix_walker::advance_block() {
    col_ = col0_;
    col_no_ = 0;
    row_ = unique_.mod().add<OpIAdd>(index_ty_, row_, row_inc_);
    ++block_no_;
}
void matrix_walker::advance_column() {
    col_ = unique_.mod().add<OpIAdd>(index_ty_, col_, col_inc_);
    ++col_no_;
}

auto matrix_walker::component_no(std::int32_t col_no) const -> std::int32_t {
    return layout_.component_no(col_no, block_no_);
}
auto matrix_walker::component_no() const -> std::int32_t { return component_no(col_no_); }
auto matrix_walker::offset() const -> spv_inst * {
    return unique_.mod().add<OpIAdd>(index_ty_, row_, col_);
};
auto matrix_walker::rows_checked() const -> bool {
    return chk_ == checked_flag::both || chk_ == checked_flag::rows;
}
auto matrix_walker::cols_checked() const -> bool {
    return chk_ == checked_flag::both || chk_ == checked_flag::cols;
}
auto matrix_walker::needs_mask() const -> bool {
    return (col_no_ + 1) * col_inc_factor_ > layout_.shape1;
}
auto matrix_walker::may_need_mask() const -> bool {
    return layout_.cols * col_inc_factor_ > layout_.shape1;
}

auto matrix_walker::col_ok() const -> spv_inst * {
    auto c0 = unique_.null_constant(index_ty_);
    auto bool_ty = unique_.bool_ty();
    auto &mod = unique_.mod();
    auto check1 = mod.add<OpSLessThanEqual>(bool_ty, c0, col_);
    auto check2 = mod.add<OpSLessThan>(bool_ty, col_, col_max_);
    return mod.add<OpLogicalAnd>(bool_ty, check1, check2);
}
auto matrix_walker::row_ok() const -> spv_inst * {
    auto c0 = unique_.null_constant(index_ty_);
    auto bool_ty = unique_.bool_ty();
    auto &mod = unique_.mod();
    auto check1 = mod.add<OpSLessThanEqual>(bool_ty, c0, row_);
    auto check2 = mod.add<OpSLessThan>(bool_ty, row_, row_max_);
    return mod.add<OpLogicalAnd>(bool_ty, check1, check2);
}

} // namespace tinytc::spv
