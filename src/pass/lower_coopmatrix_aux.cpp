// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/lower_coopmatrix_aux.hpp"
#include "codegen_tools.hpp"
#include "support/casting.hpp"
#include "tinytc/types.hpp"

#include <array>
#include <cstdint>
#include <utility>

namespace tinytc {

auto get_matrix_fibre(region_builder &bb, value operand, std::array<value, 2u> dyn_offsets,
                      int omode, std::array<std::int64_t, 2u> const &shape, value subgroup_local_id,
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

auto get_return_types(inst_node &in,
                      std::int32_t subgroup_size) -> std::vector<tinytc_data_type_t> {
    auto return_types = std::vector<tinytc_data_type_t>{};
    return_types.reserve(in.num_results());

    for (auto &res : in.results()) {
        if (auto ct = dyn_cast<coopmatrix_data_type>(res.ty()); ct) {
            for (std::int64_t i = 0; i < ct->length(subgroup_size); ++i) {
                return_types.emplace_back(ct->ty());
            }
        } else {
            return_types.emplace_back(res.ty());
        }
    }
    return return_types;
}

auto normalize_checked_flag(checked_flag checked, matrix_use use) -> checked_flag {
    if (use == matrix_use::b) {
        switch (checked) {
        case checked_flag::none:
            return checked_flag::none;
        case checked_flag::cols:
            return checked_flag::rows;
        case checked_flag::rows:
            return checked_flag::cols;
        case checked_flag::both:
            return checked_flag::both;
        }
    }
    return checked;
}
auto normalize_shape(std::array<std::int64_t, 2u> shape,
                     matrix_use use) -> std::array<std::int64_t, 2u> {
    if (use == matrix_use::b) {
        std::swap(shape[0], shape[1]);
    }
    return shape;
}
auto normalize_transpose(transpose trans, matrix_use use) -> transpose {
    if (use == matrix_use::b) {
        switch (trans) {
        case transpose::T:
            return transpose::N;
        case transpose::N:
            return transpose::T;
        }
    }
    return trans;
}

} // namespace tinytc
