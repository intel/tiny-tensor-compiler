// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CODEGEN_TOOLS_20240229_HPP
#define CODEGEN_TOOLS_20240229_HPP

#include "device_info.hpp"
#include "node/inst_view.hpp"
#include "tiling.hpp"
#include "tinytc/builder.hpp"
#include "tinytc/core.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <cstdint>
#include <functional>
#include <optional>
#include <utility>
#include <vector>

namespace tinytc {

auto get_core_config_and_tiling(tinytc_func const &fn, const_tinytc_core_info_t info)
    -> std::pair<core_config, local_tiling>;

using sgs_loop_body_builder =
    std::function<void(region_builder &, tinytc_value_t, bool, tinytc_value_t)>;
using uniform_loop_body_builder =
    std::function<void(region_builder &, tinytc_value_t, tinytc_value_t)>;

void tile_loop_by_sgs(region_builder &bb, tinytc_value_t loop_trip_count, int sgs, int num_tiles,
                      tinytc_value_t sg_id, sgs_loop_body_builder const &body,
                      tinytc_attr_t for_attributes = nullptr);

void tile_loop_uniformly(region_builder &bb, tinytc_value_t loop_trip_count, int block_size,
                         int num_tiles, tinytc_value_t sg_id, uniform_loop_body_builder const &body,
                         tinytc_attr_t for_attributes = nullptr);

auto promote_binop_operands(region_builder &bb, tinytc_type_t result_ty, tinytc_value_t a,
                            tinytc_value_t b, location const &loc)
    -> std::pair<tinytc_value_t, tinytc_value_t>;
template <typename Binop>
auto mixed_precision_arithmetic(region_builder &bb, tinytc_type_t result_ty, tinytc_value_t a,
                                tinytc_value_t b, location const &loc) -> tinytc_value_t {
    auto [a_p, b_p] = promote_binop_operands(bb, result_ty, a, b, loc);
    return bb.create<Binop>(a_p, b_p, result_ty, loc);
}
auto mixed_precision_coopmatrix_scale(region_builder &bb, tinytc_value_t a, tinytc_value_t b,
                                      location const &loc) -> tinytc_value_t;

void blas_update(region_builder &bb, bool atomic, tinytc_value_t alpha, tinytc_value_t ab,
                 tinytc_value_t beta, tinytc_value_t C, array_view<tinytc_value_t> index_list,
                 location const &loc);

auto instant_constant_fold_add(region_builder &bb, unique_handle<tinytc_inst_t> &&i)
    -> tinytc_value_t;
auto get_bool_constant(tinytc_value_t val) -> std::optional<bool>;
auto get_int_constant(const_tinytc_value_t val) -> std::optional<std::int64_t>;
auto get_int_constant(tinytc_value const &val) -> std::optional<std::int64_t>;
auto get_coopmatrix_type(tinytc_value const &v) -> coopmatrix_type *;
auto get_memref_type(tinytc_value const &v) -> memref_type *;
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
    work_group_op(std::int32_t num_tiles, std::int32_t subgroup_size, tinytc_type_t ty);

    void setup(region_builder &bb, location const &loc);
    void teardown(region_builder &bb);

    inline auto num_tiles() const -> std::int32_t { return num_tiles_; }
    inline auto subgroup_size() const -> std::int32_t { return subgroup_size_; }
    inline auto ty() const -> tinytc_type_t { return ty_; }

  protected:
    std::int32_t num_tiles_, subgroup_size_;
    tinytc_type_t ty_;
    tinytc_value_t tmp_;
};

class work_group_reduce : public work_group_op {
  public:
    using work_group_op::work_group_op;

    auto make(region_builder &bb, tinytc_value_t a, location const &loc) -> tinytc_value_t;
};

class work_group_inclusive_scan : public work_group_op {
  public:
    using work_group_op::work_group_op;

    auto make(region_builder &bb, tinytc_value_t a, bool compute_sum, location const &loc)
        -> std::pair<tinytc_value_t, tinytc_value_t>;
};

} // namespace tinytc

#endif // CODEGEN_TOOLS_20240229_HPP
