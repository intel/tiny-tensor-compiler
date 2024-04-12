// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CODEGEN_TOOLS_20240229_HPP
#define CODEGEN_TOOLS_20240229_HPP

#include <cstdint>
#include <functional>

namespace clir {
enum class address_space;
class block_builder;
class expr;
class var;
}; // namespace clir

namespace tinytc {

enum class scalar_type;

clir::expr vload_helper(short vec_size, clir::expr offset, clir::expr ptr);

void store_helper(clir::block_builder &bb, bool is_atomic, clir::expr dst, scalar_type ty,
                  clir::address_space as, clir::expr value, clir::expr beta);
void atomic_store_helper(clir::block_builder &bb, clir::expr dst, scalar_type ty,
                         clir::address_space as, clir::expr value, clir::expr beta);

void dispatch_constant_dynamic(clir::expr e, std::function<void(std::int64_t)> const &const_case,
                               std::function<void(clir::expr)> const &dyn_case);

using sgs_loop_body_builder =
    std::function<void(clir::block_builder &, clir::expr, bool, clir::expr)>;
using uniform_loop_body_builder =
    std::function<void(clir::block_builder &, clir::expr, clir::expr)>;

void tile_loop_by_sgs(clir::block_builder &bb, clir::expr loop_trip_count, unsigned sgs,
                      unsigned num_tiles, clir::var sg_id, sgs_loop_body_builder const &body);
void tile_loop_by_sgs_constant(clir::block_builder &bb, unsigned loop_trip_count, unsigned sgs,
                               unsigned num_tiles, clir::var sg_id,
                               sgs_loop_body_builder const &body);
void tile_loop_by_sgs_dynamic(clir::block_builder &bb, clir::expr loop_trip_count, unsigned sgs,
                              unsigned num_tiles, clir::var sg_id,
                              sgs_loop_body_builder const &body);

unsigned tile_loop_uniformly_max_block_size(unsigned loop_trip_count, unsigned block_size,
                                            unsigned num_tiles);
void tile_loop_uniformly(clir::block_builder &bb, clir::expr loop_trip_count, unsigned block_size,
                         unsigned num_tiles, clir::var sg_id,
                         uniform_loop_body_builder const &body);
void tile_loop_uniformly_constant(clir::block_builder &bb, unsigned loop_trip_count,
                                  unsigned block_size, unsigned num_tiles, clir::var sg_id,
                                  uniform_loop_body_builder const &body);
void tile_loop_uniformly_dynamic(clir::block_builder &bb, clir::expr loop_trip_count,
                                 unsigned block_size, unsigned num_tiles, clir::var sg_id,
                                 uniform_loop_body_builder const &body);

} // namespace tinytc

#endif // CODEGEN_TOOLS_20240229_HPP
