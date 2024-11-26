// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CODEGEN_TOOLS_20240229_HPP
#define CODEGEN_TOOLS_20240229_HPP

#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <cstdint>
#include <functional>
#include <optional>

namespace tinytc {

// tools for OpenCL codegen

short bits(scalar_type ty);

using sgs_loop_body_builder = std::function<void(region_builder &, value, bool, value)>;
using uniform_loop_body_builder = std::function<void(region_builder &, value, value)>;

void tile_loop_by_sgs(region_builder &bb, value loop_trip_count, int sgs, int num_tiles,
                      value sg_id, sgs_loop_body_builder const &body);

void tile_loop_uniformly(region_builder &bb, value loop_trip_count, int block_size, int num_tiles,
                         value sg_id, uniform_loop_body_builder const &body);

auto mixed_precision_arithmetic(region_builder &bb, arithmetic operation, value a, value b,
                                location const &loc) -> value;
auto mixed_precision_coopmatrix_scale(region_builder &bb, value a, value b,
                                      location const &loc) -> value;

auto get_atomic_store_flag(value beta) -> std::optional<store_flag>;
void blas_update(region_builder &bb, bool atomic, value alpha, value ab, value beta, value C,
                 array_view<value> index_list, location const &loc);

auto instant_constant_fold_add(region_builder &bb, inst i) -> value;
auto get_bool_constant(tinytc_value_t val) -> std::optional<bool>;
auto get_int_constant(tinytc_value_t val) -> std::optional<std::int64_t>;

} // namespace tinytc

#endif // CODEGEN_TOOLS_20240229_HPP
