// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CODEGEN_TOOLS_20240229_HPP
#define CODEGEN_TOOLS_20240229_HPP

#include "device_info.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <clir/builder.hpp>
#include <clir/builtin_type.hpp>
#include <clir/expr.hpp>
#include <clir/var.hpp>

#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <utility>

namespace tinytc {

short bits(scalar_type ty);
clir::expr constant(scalar_type ty, std::int64_t value);
clir::expr constant(scalar_type ty, double value);
clir::expr multiply(scalar_type ty_a, scalar_type ty_b, clir::expr a, clir::expr b);
clir::expr vload_helper(short vec_size, clir::expr offset, clir::expr ptr);
clir::expr sub_group_block_read_helper(clir::expr pointer, scalar_type ty, clir::address_space as);
clir::expr sub_group_block_write_helper(clir::expr pointer, clir::expr data, scalar_type ty,
                                        clir::address_space as);

void store_helper(clir::block_builder &bb, bool is_atomic, clir::expr dst, scalar_type ty,
                  clir::address_space as, clir::expr value, scalar_type beta_ty, clir::expr beta);
void atomic_store_helper(clir::block_builder &bb, clir::expr dst, scalar_type ty,
                         clir::address_space as, clir::expr value, scalar_type beta_ty,
                         clir::expr beta);

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

class block_accessor {
  public:
    virtual ~block_accessor() = default;
    virtual auto get(int m_block, int k) const -> clir::expr = 0;
};

class block_accessor_regular : public block_accessor {
  public:
    block_accessor_regular(clir::expr block, int Kb);
    auto get(int m_block, int k) const -> clir::expr override;
    inline void offset(clir::expr offset) { offset_ = std::move(offset); }

  private:
    clir::expr block_, offset_;
    int Kb_;
};

class block_accessor_vector : public block_accessor {
  public:
    block_accessor_vector(clir::expr block);
    auto get(int m_block, int k) const -> clir::expr override;

  private:
    clir::expr block_;
};

struct matrix_block_description {
    scalar_type ty;         ///< Matrix scalar type
    clir::address_space as; ///< Matrix address space
    int Mb;                 ///< Number of rows if M_mode == 0; number of columns if M_mode == 1
    int Kb;                 ///< Number of columns if M_mode == 0; number of rows if M_mode == 0
    clir::expr pointer;     ///< Pointer to block start
    clir::expr M;           ///< Size of row mode if M_mode == 0; size of column mode if M_mode == 1
    std::array<clir::expr, 2u> stride; ///< Matrix stride

    int first_block_with_check(std::int32_t subgroup_size) const;
    clir::expr condition(int m_block, std::int32_t subgroup_size) const;
    bool is_unit_stride(int mode) const;
};

auto read_matrix_block_regular(clir::block_builder &bb, matrix_block_description const &d,
                               int M_mode, core_config const &core_cfg,
                               char const *block_name) -> std::unique_ptr<block_accessor>;
auto read_matrix_block_vector(clir::block_builder &bb, matrix_block_description const &d,
                              int M_mode, core_config const &core_cfg,
                              char const *block_name) -> std::unique_ptr<block_accessor>;

// Read MbxKb block
auto read_matrix_block(clir::block_builder &bb, matrix_block_description const &d, int M_mode,
                       core_config const &core_cfg,
                       char const *block_name) -> std::unique_ptr<block_accessor>;

// Write MbxKb block
void write_matrix_block(clir::block_builder &bb, block_accessor const &block,
                        matrix_block_description const &d, bool is_atomic, scalar_type beta_ty,
                        clir::expr beta, core_config const &core_cfg);

using sgs_loop_body_builder_new =
    std::function<void(region_builder &, value const &, bool, value const &)>;
using uniform_loop_body_builder_new =
    std::function<void(region_builder &, value const &, value const &)>;

void tile_loop_by_sgs_new(region_builder &bb, value const &loop_trip_count, int sgs, int num_tiles,
                          value const &sg_id, sgs_loop_body_builder_new const &body);
void tile_loop_by_sgs_new_constant(region_builder &bb, std::int64_t loop_trip_count, int sgs,
                                   int num_tiles, value const &sg_id,
                                   sgs_loop_body_builder_new const &body);
void tile_loop_by_sgs_new_dynamic(region_builder &bb, value const &loop_trip_count, int sgs,
                                  int num_tiles, value const &sg_id,
                                  sgs_loop_body_builder_new const &body);

void tile_loop_uniformly_new(region_builder &bb, value const &loop_trip_count, int block_size,
                             int num_tiles, value const &sg_id,
                             uniform_loop_body_builder_new const &body);
void tile_loop_uniformly_new_constant(region_builder &bb, std::int64_t loop_trip_count,
                                      int block_size, int num_tiles, value const &sg_id,
                                      uniform_loop_body_builder_new const &body);
void tile_loop_uniformly_new_dynamic(region_builder &bb, value const &loop_trip_count,
                                     int block_size, int num_tiles, value const &sg_id,
                                     uniform_loop_body_builder_new const &body);

} // namespace tinytc

#endif // CODEGEN_TOOLS_20240229_HPP
