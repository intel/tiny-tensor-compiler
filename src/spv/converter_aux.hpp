// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CONVERTER_AUX_20250416_HPP
#define CONVERTER_AUX_20250416_HPP

#include "error.hpp"
#include "node/inst_view.hpp"
#include "node/type.hpp"
#include "number.hpp"
#include "spv/defs.hpp"
#include "spv/instructions.hpp"
#include "spv/module.hpp"
#include "spv/uniquifier.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"
#include "util/casting.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <vector>

namespace tinytc::spv {

auto get_spv_index_ty(uniquifier &unique, tinytc_compiler_context_t ctx) -> spv_inst *;
auto get_spv_ty(uniquifier &unique, memref_type const *ty) -> spv_inst *;
auto get_spv_pointer_index_ty(uniquifier &unique, tinytc_compiler_context_t ctx,
                              address_space addrspace = address_space::global) -> spv_inst *;
auto get_spv_ty_non_coopmatrix(uniquifier &unique, tinytc_type_t ty) -> spv_inst *;

auto get_last_label(tinytc_spv_mod &mod) -> spv_inst *;

auto split_re_im(uniquifier &unique, tinytc_type_t val_ty, address_space as, spv_inst *pointer,
                 spv_inst *value) -> std::array<std::array<spv_inst *, 2u>, 2u>;
auto make_atomic_load(uniquifier &unique, memory_scope scope, memory_semantics semantics,
                      tinytc_type_t result_ty, address_space as, spv_inst *pointer,
                      location const &loc) -> spv_inst *;
void make_atomic_store(uniquifier &unique, memory_scope scope, memory_semantics semantics,
                       tinytc_type_t val_ty, address_space as, spv_inst *pointer, spv_inst *value,
                       location const &loc);
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
auto make_unary_op(uniquifier &unique, tinytc_type_t operand_ty, IK op, spv_inst *a,
                   location const &loc) -> spv_inst *;
auto make_subgroup_op(uniquifier &unique, tinytc_type_t operand_ty, IK op, spv_inst *a,
                      location const &loc) -> spv_inst *;

template <typename SpvIOp, typename SpvFOp>
auto make_atomic_update(uniquifier &unique, memory_scope scope, memory_semantics semantics,
                        tinytc_type_t val_ty, address_space as, spv_inst *pointer, spv_inst *value,
                        location const &loc) -> spv_inst * {
    if ((isa<i8_type>(*val_ty) || isa<i16_type>(*val_ty) || isa<bf16_type>(*val_ty))) {
        throw compilation_error(loc, status::spirv_unsupported_atomic_data_type);
    }

    auto &mod = unique.mod();
    auto result_ty = get_spv_ty_non_coopmatrix(unique, val_ty);
    auto c_scope = unique.constant(static_cast<std::int32_t>(scope));
    auto c_semantics = unique.constant(static_cast<std::int32_t>(semantics));
    if (isa<complex_type>(*val_ty)) {
        auto re_im = split_re_im(unique, val_ty, as, pointer, value);
        auto float_ty = component_type(val_ty);
        auto spv_float_ty = get_spv_ty_non_coopmatrix(unique, float_ty);
        auto re = mod.add<SpvFOp>(spv_float_ty, re_im[0][0], c_scope, c_semantics, re_im[0][1]);
        auto im = mod.add<SpvFOp>(spv_float_ty, re_im[1][0], c_scope, c_semantics, re_im[1][1]);
        auto dummy = mod.add<OpUndef>(result_ty);
        auto tmp = mod.add<OpCompositeInsert>(result_ty, re, dummy, std::vector<LiteralInteger>{0});
        return mod.add<OpCompositeInsert>(result_ty, im, tmp, std::vector<LiteralInteger>{1});
    } else if (isa<float_type>(*val_ty)) {
        return mod.add<SpvFOp>(result_ty, pointer, c_scope, c_semantics, value);
    } else if (isa<integer_type>(*val_ty)) {
        return mod.add<SpvIOp>(result_ty, pointer, c_scope, c_semantics, value);
    }
    throw compilation_error(loc, status::spirv_unsupported_atomic_data_type);
}

} // namespace tinytc::spv

#endif // CONVERTER_AUX_20250416_HPP
