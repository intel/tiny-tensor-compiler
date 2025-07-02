// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CONVERTER_AUX_20250416_HPP
#define CONVERTER_AUX_20250416_HPP

#include "coopmatrix_layout.hpp"
#include "node/inst_view.hpp"
#include "spv/enums.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <functional>

namespace tinytc::spv {

class spv_inst;
class uniquifier;

auto get_spv_index_ty(uniquifier &unique, tinytc_compiler_context_t ctx) -> spv_inst *;
auto get_spv_ty(uniquifier &unique, memref_type const *ty) -> spv_inst *;
auto get_spv_pointer_index_ty(uniquifier &unique, tinytc_compiler_context_t ctx,
                              address_space addrspace = address_space::global) -> spv_inst *;
auto get_spv_ty_non_coopmatrix(uniquifier &unique, tinytc_type_t ty) -> spv_inst *;

auto get_last_label(tinytc_spv_mod &mod) -> spv_inst *;
auto make_binary_op(uniquifier &unique, tinytc_type_t operand_ty, IK op, spv_inst *a, spv_inst *b,
                    location const &loc) -> spv_inst *;
auto make_binary_op_mixed_precision(uniquifier &unique, tinytc_type_t result_ty, IK op,
                                    tinytc_type_t a_ty, spv_inst *a, tinytc_type_t b_ty,
                                    spv_inst *b, location const &loc) -> spv_inst *;
auto make_cast(uniquifier &unique, tinytc_type_t to_ty, tinytc_type_t a_ty, spv_inst *a,
               location const &loc) -> spv_inst *;
auto make_complex_mul(uniquifier &unique, spv_inst *ty, spv_inst *a, spv_inst *b,
                      bool conj_b = false) -> spv_inst *;
auto make_compare_op(uniquifier &unique, tinytc_type_t operand_ty, IK op, spv_inst *a, spv_inst *b,
                     location const &loc) -> spv_inst *;
auto make_constant(uniquifier &unique, tinytc_type_t ty, constant_value_type const &val)
    -> spv_inst *;
void make_conditional_execution(uniquifier &unique, spv_inst *condition,
                                std::function<void(tinytc_spv_mod &)> then);
auto make_conditional_execution(uniquifier &unique, spv_inst *return_ty, spv_inst *condition,
                                std::function<spv_inst *(tinytc_spv_mod &)> then,
                                spv_inst *otherwise, location const &loc) -> spv_inst *;
auto make_conditional_execution(uniquifier &unique, spv_inst *return_ty, spv_inst *condition,
                                std::function<spv_inst *(tinytc_spv_mod &)> then,
                                std::function<spv_inst *(tinytc_spv_mod &)> otherwise,
                                location const &loc) -> spv_inst *;
auto make_math_unary_op(uniquifier &unique, tinytc_type_t operand_ty, IK op, spv_inst *a,
                        location const &loc) -> spv_inst *;
void make_store(uniquifier &unique, store_flag flag, tinytc_type_t val_ty, address_space as,
                spv_inst *pointer, spv_inst *value, location const &loc);
auto make_unary_op(uniquifier &unique, tinytc_type_t operand_ty, IK op, spv_inst *a,
                   location const &loc) -> spv_inst *;
auto make_subgroup_op(uniquifier &unique, tinytc_type_t operand_ty, IK op, spv_inst *a,
                      location const &loc) -> spv_inst *;

} // namespace tinytc::spv

#endif // CONVERTER_AUX_20250416_HPP
