// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CODEGEN_TOOLS_20240229_HPP
#define CODEGEN_TOOLS_20240229_HPP

#include "device_info.hpp"
#include "node/inst_view.hpp"
#include "tiling.hpp"
#include "tinytc/builder.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <cstdint>
#include <functional>
#include <optional>
#include <utility>
#include <vector>

namespace tinytc {

class coopmatrix_data_type;
class memref_data_type;

auto get_core_config_and_tiling(tinytc_func const &fn, const_tinytc_core_info_t info)
    -> std::pair<core_config, local_tiling>;

using sgs_loop_body_builder = std::function<void(region_builder &, value, bool, value)>;
using uniform_loop_body_builder = std::function<void(region_builder &, value, value)>;

void tile_loop_by_sgs(region_builder &bb, value loop_trip_count, int sgs, int num_tiles,
                      value sg_id, sgs_loop_body_builder const &body,
                      attr for_attributes = nullptr);

void tile_loop_uniformly(region_builder &bb, value loop_trip_count, int block_size, int num_tiles,
                         value sg_id, uniform_loop_body_builder const &body,
                         attr for_attributes = nullptr);

auto promote_binop_operands(region_builder &bb, scalar_type result_ty, value a, value b,
                            location const &loc) -> std::pair<value, value>;
template <typename Binop>
auto mixed_precision_arithmetic(region_builder &bb, scalar_type result_ty, value a, value b,
                                location const &loc) -> value {
    auto [a_p, b_p] = promote_binop_operands(bb, result_ty, a, b, loc);
    return bb.create<Binop>(a_p, b_p, scalar_data_type::get(a_p->context(), result_ty), loc);
}
auto mixed_precision_coopmatrix_scale(region_builder &bb, value a, value b, location const &loc)
    -> value;

auto get_atomic_store_flag(value beta) -> std::optional<store_flag>;
void blas_update(region_builder &bb, bool atomic, value alpha, value ab, value beta, value C,
                 array_view<value> index_list, location const &loc);

auto instant_constant_fold_add(region_builder &bb, inst i) -> value;
auto get_bool_constant(tinytc_value_t val) -> std::optional<bool>;
auto get_int_constant(const_tinytc_value_t val) -> std::optional<std::int64_t>;
auto get_int_constant(tinytc_value const &val) -> std::optional<std::int64_t>;
auto get_coopmatrix_type(tinytc_value const &v) -> coopmatrix_data_type const *;
auto get_memref_type(tinytc_value const &v) -> memref_data_type const *;
auto get_scalar_type(tinytc_value const &v) -> scalar_type;
auto get_yield(location const &loc, tinytc_region &reg) -> yield_inst;

template <typename T> auto get_int_constants(T &&val_range) -> std::vector<std::int64_t> {
    auto result = std::vector<std::int64_t>{};
    result.reserve(val_range.size());
    for (auto &val : val_range) {
        const auto cst = get_int_constant(val);
        result.emplace_back(cst ? *cst : dynamic);
    }
    return result;
}

auto add_check(checked_flag flag, checked_flag new_flag) -> checked_flag;

class work_group_op {
  public:
    work_group_op(std::int32_t num_tiles, std::int32_t subgroup_size, data_type ty);

    void setup(region_builder &bb, location const &loc);
    void teardown(region_builder &bb);

    inline auto num_tiles() const -> std::int32_t { return num_tiles_; }
    inline auto subgroup_size() const -> std::int32_t { return subgroup_size_; }
    inline auto ty() const -> data_type { return ty_; }

  protected:
    std::int32_t num_tiles_, subgroup_size_;
    data_type ty_;
    value tmp_;
};

class work_group_reduce : public work_group_op {
  public:
    using work_group_op::work_group_op;

    auto make(region_builder &bb, value a, location const &loc) -> value;
};

class work_group_inclusive_scan : public work_group_op {
  public:
    using work_group_op::work_group_op;

    auto make(region_builder &bb, value a, bool compute_sum, location const &loc)
        -> std::pair<value, value>;
};

} // namespace tinytc

#endif // CODEGEN_TOOLS_20240229_HPP
