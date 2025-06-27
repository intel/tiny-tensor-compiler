// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CONVERTER_AUX_20250416_HPP
#define CONVERTER_AUX_20250416_HPP

#include "node/inst_view.hpp"
#include "spv/enums.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <functional>

namespace tinytc::spv {

class spv_inst;
class uniquifier;

auto get_last_label(tinytc_spv_mod &mod) -> spv_inst *;
auto make_binary_op(uniquifier &unique, scalar_type sty, IK op, spv_inst *a, spv_inst *b,
                    location const &loc) -> spv_inst *;
auto make_binary_op_mixed_precision(uniquifier &unique, scalar_type result_ty, IK op,
                                    scalar_type a_ty, spv_inst *a, scalar_type b_ty, spv_inst *b,
                                    location const &loc) -> spv_inst *;
auto make_cast(uniquifier &unique, scalar_type to_ty, scalar_type a_ty, spv_inst *a,
               location const &loc) -> spv_inst *;
auto make_complex_mul(uniquifier &unique, spv_inst *ty, spv_inst *a, spv_inst *b,
                      bool conj_b = false) -> spv_inst *;
auto make_constant(uniquifier &unique, scalar_type sty, constant_value_type const &val)
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
void make_store(uniquifier &unique, store_flag flag, scalar_type sty, address_space as,
                spv_inst *pointer, spv_inst *value, location const &loc);
auto make_unary_op(uniquifier &unique, scalar_type sty, IK op, spv_inst *a, location const &loc)
    -> spv_inst *;
auto make_subgroup_op(uniquifier &unique, scalar_type sty, IK op, spv_inst *a, location const &loc)
    -> spv_inst *;

} // namespace tinytc::spv

#endif // CONVERTER_AUX_20250416_HPP
