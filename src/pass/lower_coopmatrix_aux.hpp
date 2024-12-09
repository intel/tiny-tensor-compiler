// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef LOWER_COOPMATRIX_AUX_20241209_HPP
#define LOWER_COOPMATRIX_AUX_20241209_HPP

#include "codegen_tools.hpp"
#include "node/data_type_node.hpp"
#include "node/inst_node.hpp"
#include "node/value_node.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace tinytc {

template <typename Builder, typename F>
void make_conditional_execution(Builder &bb, tinytc_value_t condition, F &&conditional_exe,
                                location const &loc) {
    auto ii = std::make_unique<if_inst>(condition, array_view<tinytc_data_type_t>{}, loc);
    auto bb_then = region_builder{&ii->then()};
    conditional_exe(bb_then);
    bb.add(inst{ii.release()});
}
template <typename Builder, typename F>
auto make_conditional_execution(Builder &bb, tinytc_value_t condition, F &&conditional_val,
                                tinytc_data_type_t return_ty, location const &loc) -> value {
    auto ii = std::make_unique<if_inst>(condition, array_view<data_type>(return_ty), loc);
    auto bb_then = region_builder{&ii->then()};
    auto val = conditional_val(bb_then);
    bb_then.add(make_yield(val, loc));
    auto bb_otherwise = region_builder{&ii->otherwise()};
    auto zero = bb_otherwise.add(make_constant_zero(return_ty, loc));
    bb_otherwise.add(make_yield(zero, loc));
    return bb.add(inst{ii.release()});
}
template <typename Builder, typename F>
auto make_conditional_execution(Builder &bb, tinytc_value_t condition, F &&conditional_vals,
                                std::size_t num_returned_val, tinytc_data_type_t return_ty,
                                location const &loc) -> std::vector<value> {
    auto ii = std::make_unique<if_inst>(condition,
                                        std::vector<data_type>(num_returned_val, return_ty), loc);
    auto bb_then = region_builder{&ii->then()};
    auto vals = conditional_vals(bb_then);
    bb_then.add(make_yield(vals, loc));
    auto bb_otherwise = region_builder{&ii->otherwise()};
    auto zero = bb_otherwise.add(make_constant_zero(return_ty, loc));
    bb_otherwise.add(make_yield(std::vector<value>(num_returned_val, zero), loc));
    return bb.add_multivalued(inst{ii.release()});
}

template <typename Builder>
auto get_matrix_fibre(Builder &bb, value operand, std::array<value, 2u> dyn_offsets, int omode,
                      std::array<std::int64_t, 2u> const &shape, value subgroup_local_id,
                      location const &loc) -> value {
    auto index_ty = scalar_data_type::get(operand->context(), scalar_type::index);
    auto ot = get_memref_type(*operand);
    auto offsets = std::array<std::int64_t, 2u>{dynamic, dynamic};

    dyn_offsets[omode] =
        bb.add(make_arith(arithmetic::add, dyn_offsets[omode], subgroup_local_id, index_ty, loc));

    auto sizes = std::array<std::int64_t, 2u>{0, 0};
    sizes[1 - omode] = shape[1];

    auto subt = get_memref(ot->element_data_ty(), {shape[1]}, {ot->stride(1 - omode)},
                           ot->addrspace(), loc);
    return bb.add(make_subview(operand, offsets, sizes, dyn_offsets, {}, subt, loc));
}

class check_condition_generator {
  public:
    inline check_condition_generator(value operand, std::array<value, 2u> const &pos)
        : operand_(operand), pos_(pos), rem_{nullptr, nullptr} {}

    template <typename Builder>
    auto operator()(Builder &bb, value offset, int mode, location const &loc) -> value {
        auto bool_ty = boolean_data_type::get(offset->context());
        auto index_ty = scalar_data_type::get(offset->context(), scalar_type::index);
        if (rem_[mode].get() == nullptr) {
            auto size = bb.add(make_size(operand_, mode, index_ty, loc));
            rem_[mode] = bb.add(make_arith(arithmetic::sub, size, pos_[mode], index_ty, loc));
        }
        auto neg_offset = bb.add(make_arith(arithmetic_unary::neg, offset, index_ty, loc));
        auto check1 = bb.add(make_cmp(cmp_condition::le, neg_offset, pos_[mode], bool_ty, loc));
        auto check2 = bb.add(make_cmp(cmp_condition::lt, offset, rem_[mode], bool_ty, loc));
        return bb.add(make_arith(arithmetic::and_, check1, check2, bool_ty, loc));
    }

  private:
    value operand_;
    std::array<value, 2u> pos_;
    std::array<value, 2u> rem_;
};

auto normalize_checked_flag(checked_flag checked, matrix_use use) -> checked_flag;
auto normalize_shape(std::array<std::int64_t, 2u> shape,
                     matrix_use use) -> std::array<std::int64_t, 2u>;
auto normalize_transpose(transpose trans, matrix_use use) -> transpose;

} // namespace tinytc

#endif // LOWER_COOPMATRIX_AUX_20241209_HPP
