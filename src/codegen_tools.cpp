// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "codegen_tools.hpp"
#include "error.hpp"
#include "scalar_type.hpp"
#include "util.hpp"

#include <clir/attr_defs.hpp>
#include <clir/builtin_function.hpp>
#include <clir/data_type.hpp>
#include <clir/handle.hpp>
#include <clir/internal/expr_node.hpp>
#include <clir/visit.hpp>

#include <cassert>
#include <memory>
#include <utility>

using namespace clir;

namespace tinytc {

expr as_type(builtin_type ty, expr e) {
    switch (ty) {
    case builtin_type::char_t:
        return as_char(std::move(e));
    case builtin_type::uchar_t:
        return as_uchar(std::move(e));
    case builtin_type::short_t:
        return as_short(std::move(e));
    case builtin_type::ushort_t:
        return as_ushort(std::move(e));
    case builtin_type::int_t:
        return as_int(std::move(e));
    case builtin_type::uint_t:
        return as_uint(std::move(e));
    case builtin_type::long_t:
        return as_long(std::move(e));
    case builtin_type::ulong_t:
        return as_ulong(std::move(e));
    case builtin_type::float_t:
        return as_float(std::move(e));
    case builtin_type::double_t:
        return as_double(std::move(e));
    default:
        break;
    }
    return e;
}

expr vload_helper(short vec_size, expr offset, expr ptr) {
    switch (vec_size) {
    case 1:
        return ptr[std::move(offset)];
    case 2:
        return vload2(std::move(offset), std::move(ptr));
    case 3:
        return vload3(std::move(offset), std::move(ptr));
    case 4:
        return vload4(std::move(offset), std::move(ptr));
    case 8:
        return vload8(std::move(offset), std::move(ptr));
    case 16:
        return vload16(std::move(offset), std::move(ptr));
    default:
        break;
    };
    return nullptr;
}

builtin_type block_rw_op_type(builtin_type scalar_ty) {
    switch (scalar_ty) {
    case builtin_type::short_t:
        return builtin_type::ushort_t;
    case builtin_type::int_t:
    case builtin_type::float_t:
        return builtin_type::uint_t;
    case builtin_type::long_t:
    case builtin_type::double_t:
        return builtin_type::ulong_t;
    default:
        break;
    }
    return scalar_ty;
}

expr sub_group_block_read_helper(expr pointer, builtin_type scalar_ty, address_space as) {
    auto const make_read = [](builtin_type bt, expr pointer) -> expr {
        switch (bt) {
        case builtin_type::short_t:
        case builtin_type::ushort_t:
            return intel_sub_group_block_read_us(std::move(pointer));
        case builtin_type::int_t:
        case builtin_type::uint_t:
        case builtin_type::float_t:
            return intel_sub_group_block_read_ui(std::move(pointer));
        case builtin_type::long_t:
        case builtin_type::ulong_t:
        case builtin_type::double_t:
            return intel_sub_group_block_read_ul(std::move(pointer));
        default:
            break;
        }
        return pointer[get_sub_group_local_id()];
    };
    auto const bt = block_rw_op_type(scalar_ty);
    pointer = cast(pointer_to(clir::data_type(bt, as)), std::move(pointer));
    auto inst = make_read(bt, std::move(pointer));
    if (bt != scalar_ty) {
        return as_type(scalar_ty, std::move(inst));
    }
    return inst;
}
expr sub_group_block_write_helper(expr pointer, expr data, builtin_type scalar_ty,
                                  address_space as) {
    auto const make_write = [](builtin_type bt, expr pointer, expr data) -> expr {
        switch (bt) {
        case builtin_type::short_t:
        case builtin_type::ushort_t:
            return intel_sub_group_block_write_us(std::move(pointer), std::move(data));
        case builtin_type::int_t:
        case builtin_type::uint_t:
        case builtin_type::float_t:
            return intel_sub_group_block_write_ui(std::move(pointer), std::move(data));
        case builtin_type::long_t:
        case builtin_type::ulong_t:
        case builtin_type::double_t:
            return intel_sub_group_block_write_ul(std::move(pointer), std::move(data));
        default:
            break;
        }
        return pointer[get_sub_group_local_id()] = std::move(data);
    };
    auto const bt = block_rw_op_type(scalar_ty);
    pointer = cast(pointer_to(clir::data_type(bt, as)), std::move(pointer));
    if (bt != scalar_ty) {
        data = as_type(bt, std::move(data));
    }
    return make_write(bt, std::move(pointer), std::move(data));
}

void store_helper(block_builder &bb, bool is_atomic, expr dst, scalar_type ty, address_space as,
                  expr value, expr beta) {
    if (is_atomic) {
        atomic_store_helper(bb, std::move(dst), ty, as, std::move(value), std::move(beta));
    } else {
        bb.assign(dereference(dst), std::move(value) + std::move(beta) * dereference(dst));
    }
}

void atomic_store_helper(block_builder &bb, expr dst, scalar_type ty, address_space as, expr value,
                         expr beta) {
    int mode = -1;
    visit(overloaded{
              [&](clir::internal::int_imm &c) {
                  mode = c.value() == 0 ? 0 : (c.value() == 1 ? 1 : -1);
              },
              [&](clir::internal::uint_imm &c) {
                  mode = c.value() == 0u ? 0 : (c.value() == 1u ? 1 : -1);
              },
              [&](clir::internal::float_imm &c) {
                  mode = c.value() == 0.0 ? 0 : (c.value() == 1.0 ? 1 : -1);
              },
              [&](auto &) {},
          },
          *beta);
    auto pointer_ty = pointer_to(to_clir_atomic_ty(ty, as, type_qualifier::volatile_t));
    auto atomic_dst = cast(std::move(pointer_ty), dst);
    if (mode == 0) {
        bb.add(call_builtin(builtin_function::atomic_store_explicit,
                            {std::move(atomic_dst), std::move(value), memory_order::relaxed,
                             memory_scope::work_group}));
    } else if (mode == 1) {
        bb.add(call_builtin(builtin_function::atomic_fetch_add_explicit,
                            {std::move(atomic_dst), std::move(value), memory_order::relaxed,
                             memory_scope::work_group}));
    } else {
        auto expected = bb.declare_assign(to_clir_ty(ty), "expected", dereference(dst));
        auto desired = bb.declare(to_clir_ty(ty), "desired");
        auto cmpxchg =
            call_builtin(builtin_function::atomic_compare_exchange_strong_explicit,
                         {std::move(atomic_dst), address_of(std::move(expected)), desired,
                          memory_order::relaxed, memory_order::relaxed, memory_scope::work_group});
        bb.add(while_loop_builder(std::move(cmpxchg), true)
                   .body([&](block_builder &bb) { bb.assign(desired, value + beta * expected); })
                   .get_product());
    }
}

void dispatch_constant_dynamic(expr e, std::function<void(std::int64_t)> const &const_case,
                               std::function<void(expr)> const &dyn_case) {
    visit(
        overloaded{
            [&](clir::internal::int_imm &c) { const_case(c.value()); },
            [&](clir::internal::uint_imm &c) { const_case(static_cast<std::int64_t>(c.value())); },
            [&](auto &) { dyn_case(std::move(e)); },
        },
        *e);
}

void tile_loop_by_sgs(block_builder &bb, expr loop_trip_count, unsigned sgs, unsigned num_tiles,
                      var sg_id, sgs_loop_body_builder const &body) {
    dispatch_constant_dynamic(
        std::move(loop_trip_count),
        [&](std::int64_t c) {
            tile_loop_by_sgs_constant(bb, c, sgs, num_tiles, std::move(sg_id), body);
        },
        [&](expr d) {
            tile_loop_by_sgs_dynamic(bb, std::move(d), sgs, num_tiles, std::move(sg_id), body);
        });
}

void tile_loop_by_sgs_constant(block_builder &bb, unsigned loop_trip_count, unsigned sgs,
                               unsigned num_tiles, var sg_id, sgs_loop_body_builder const &body) {
    auto blocks = loop_trip_count / sgs;
    auto rem = loop_trip_count % sgs;

    auto block = bb.declare(generic_uint(), "blck");
    if (blocks > 0) {
        bb.add(for_loop_builder(assignment(block, sgs * sg_id), block < sgs * blocks,
                                add_into(block, sgs * num_tiles))
                   .body([&](block_builder &bb) { body(bb, block, false, sgs); })
                   .attribute(opencl_unroll_hint(1))
                   .get_product());
    }
    if (rem > 0) {
        bb.assign(block, blocks * sgs);
        bb.add(if_selection_builder(sg_id == num_tiles - 1u)
                   .then([&](block_builder &bb) { body(bb, block, true, rem); })
                   .get_product());
    }
}

void tile_loop_by_sgs_dynamic(block_builder &bb, expr loop_trip_count, unsigned sgs,
                              unsigned num_tiles, var sg_id, sgs_loop_body_builder const &body) {
    auto blocks = bb.declare_assign(generic_uint(), "blocks", loop_trip_count / sgs);
    auto rem = bb.declare_assign(generic_uint(), "rem", std::move(loop_trip_count) % sgs);

    auto block = bb.declare(generic_uint(), "blck");
    bb.add(for_loop_builder(assignment(block, sgs * sg_id), block < sgs * blocks,
                            add_into(block, sgs * num_tiles))
               .body([&](block_builder &bb) { body(bb, block, false, sgs); })
               .attribute(opencl_unroll_hint(1))
               .get_product());

    bb.add(if_selection_builder(rem > 0)
               .then([&](block_builder &bb) {
                   bb.assign(block, blocks * sgs);
                   bb.add(if_selection_builder(sg_id == num_tiles - 1u)
                              .then([&](block_builder &bb) { body(bb, block, true, rem); })
                              .get_product());
               })
               .get_product());
}

unsigned tile_loop_uniformly_max_block_size(unsigned loop_trip_count, unsigned block_size,
                                            unsigned num_tiles) {
    auto blocks = 1 + (loop_trip_count - 1) / block_size;
    blocks = (1 + (blocks - 1) / num_tiles) * num_tiles;
    auto bs = loop_trip_count / blocks;
    auto rem = loop_trip_count % blocks;
    return rem > 0 ? bs + 1 : bs;
}

void tile_loop_uniformly(block_builder &bb, expr loop_trip_count, unsigned block_size,
                         unsigned num_tiles, var sg_id, uniform_loop_body_builder const &body) {
    dispatch_constant_dynamic(
        std::move(loop_trip_count),
        [&](std::int64_t c) {
            tile_loop_uniformly_constant(bb, c, block_size, num_tiles, std::move(sg_id), body);
        },
        [&](expr d) {
            tile_loop_uniformly_dynamic(bb, std::move(d), block_size, num_tiles, std::move(sg_id),
                                        body);
        });
}

void tile_loop_uniformly_constant(block_builder &bb, unsigned loop_trip_count, unsigned block_size,
                                  unsigned num_tiles, var sg_id,
                                  uniform_loop_body_builder const &body) {
    // Find minimum number of blocks such that the block sizes are smaller or equal block_size
    auto blocks = 1 + (loop_trip_count - 1) / block_size;
    // Increase the number of blocks if such that the number of blocks is a multiple
    // of the number of tiles
    blocks = (1 + (blocks - 1) / num_tiles) * num_tiles;
    auto bs = loop_trip_count / blocks;
    auto bs_1 = bs + 1;
    auto rem = loop_trip_count % blocks;

    auto block = bb.declare(generic_uint(), "blck");
    if (rem > 0) {
        bb.add(for_loop_builder(assignment(block, bs_1 * sg_id), block < bs_1 * rem,
                                add_into(block, bs_1 * num_tiles))
                   .body([&](block_builder &bb) { body(bb, block, bs_1); })
                   .attribute(opencl_unroll_hint(1))
                   .get_product());
    }

    auto sg_id_1 = (std::move(sg_id) + rem % num_tiles) % num_tiles;
    bb.add(for_loop_builder(assignment(block, bs_1 * rem + bs * std::move(sg_id_1)),
                            block < loop_trip_count, add_into(block, bs * num_tiles))
               .body([&](block_builder &bb) { body(bb, block, bs); })
               .attribute(opencl_unroll_hint(1))
               .get_product());
}

void tile_loop_uniformly_dynamic(block_builder &bb, expr loop_trip_count, unsigned block_size,
                                 unsigned num_tiles, var sg_id,
                                 uniform_loop_body_builder const &body) {
    auto blocks =
        bb.declare_assign(generic_uint(), "blocks", 1 + (loop_trip_count - 1) / block_size);
    bb.assign(blocks, (1 + (blocks - 1) / num_tiles) * num_tiles);
    auto bs = bb.declare_assign(generic_uint(), "bs", loop_trip_count / blocks);
    auto bs_1 = bb.declare_assign(generic_uint(), "bs_1", bs + 1);
    auto rem = bb.declare_assign(generic_uint(), "rem", loop_trip_count % blocks);

    auto block = bb.declare(generic_uint(), "blck");
    bb.add(for_loop_builder(assignment(block, bs_1 * sg_id), block < bs_1 * rem,
                            add_into(block, bs_1 * num_tiles))
               .body([&](block_builder &bb) { body(bb, block, bs_1); })
               .attribute(opencl_unroll_hint(1))
               .get_product());

    auto sg_id_1 = (std::move(sg_id) + rem % num_tiles) % num_tiles;
    bb.add(for_loop_builder(assignment(block, bs_1 * rem + bs * std::move(sg_id_1)),
                            block < std::move(loop_trip_count), add_into(block, bs * num_tiles))
               .body([&](block_builder &bb) { body(bb, block, bs); })
               .attribute(opencl_unroll_hint(1))
               .get_product());
}

block_accessor_regular::block_accessor_regular(expr block, int Kb, expr offset)
    : block_(std::move(block)), Kb_(Kb), offset_(std::move(offset)) {}
auto block_accessor_regular::get(int m_block, int k) const -> expr {
    const auto i = k + m_block * Kb_;
    if (offset_) {
        return block_[offset_ + i];
    }
    return block_[i];
}

block_accessor_vector::block_accessor_vector(expr block) : block_(std::move(block)) {}
auto block_accessor_vector::get(int m_block, int k) const -> expr { return block_[m_block].s(k); }

int matrix_block_description::first_block_with_check(std::int32_t subgroup_size) const {
    int fb = 0;
    dispatch_constant_dynamic(
        M, [&](std::int64_t m) { fb = m / subgroup_size; }, [](expr const &) {});
    return fb;
}

bool matrix_block_description::is_unit_stride(int mode) const {
    bool is_unit = false;
    dispatch_constant_dynamic(
        stride[mode], [&](std::int64_t s) { is_unit = s == 1; }, [](expr const &) {});
    return is_unit;
}

expr matrix_block_description::condition(int m_block, std::int32_t subgroup_size) const {
    return get_sub_group_local_id() + m_block * subgroup_size < M;
}

auto read_matrix_block_regular(block_builder &bb, matrix_block_description const &d, int M_mode,
                               core_config const &core_cfg, char const *block_name)
    -> std::unique_ptr<block_accessor> {
    assert(M_mode == 0 || M_mode == 1);

    const int m_blocks = 1 + (d.Mb - 1) / core_cfg.subgroup_size;
    auto const scalar_ty = to_clir_builtin_ty(d.ty);
    auto block = bb.declare(array_of(clir::data_type(scalar_ty), m_blocks * d.Kb), block_name);

    const int first_m_block_with_check = d.first_block_with_check(core_cfg.subgroup_size);
    const bool enable_sub_group_reads =
        core_cfg.block_read_write_supported && d.is_unit_stride(M_mode);
    for (int k = 0; k < d.Kb; ++k) {
        for (int m_block = 0; m_block < m_blocks; ++m_block) {
            auto const store = [&](expr rhs) {
                bb.assign(block[k + m_block * d.Kb], std::move(rhs));
            };
            if (enable_sub_group_reads && m_block < first_m_block_with_check) {
                store(sub_group_block_read_helper(d.pointer, scalar_ty, d.as));
            } else {
                auto rhs = d.pointer[d.stride[M_mode] *
                                     (get_sub_group_local_id() + m_block * core_cfg.subgroup_size)];
                if (m_block >= first_m_block_with_check) {
                    rhs = ternary_conditional(d.condition(m_block, core_cfg.subgroup_size),
                                              std::move(rhs), 0);
                }
                store(std::move(rhs));
            }
        }
        bb.add(add_into(d.pointer, d.stride[1 - M_mode]));
    }
    return std::make_unique<block_accessor_regular>(std::move(block), d.Kb);
}

auto read_matrix_block_vector(block_builder &bb, matrix_block_description const &d, int M_mode,
                              core_config const &core_cfg, char const *block_name)
    -> std::unique_ptr<block_accessor> {
    assert(M_mode == 0 || M_mode == 1);

    const int m_blocks = 1 + (d.Mb - 1) / core_cfg.subgroup_size;
    const auto dt = clir::data_type(to_clir_builtin_ty(d.ty), d.Kb);
    auto block = bb.declare(array_of(dt, m_blocks), block_name);

    int first_m_block_with_check = d.first_block_with_check(core_cfg.subgroup_size);
    for (int m_block = 0; m_block < m_blocks; ++m_block) {
        auto rhs = vload_helper(d.Kb, 0,
                                d.pointer + d.stride[M_mode] * (get_sub_group_local_id() +
                                                                m_block * core_cfg.subgroup_size));
        if (!bool(rhs)) {
            throw internal_compiler_error();
        }
        if (m_block >= first_m_block_with_check) {
            rhs = ternary_conditional(d.condition(m_block, core_cfg.subgroup_size), rhs,
                                      init_vector(dt, {0}));
        }
        bb.assign(block[m_block], std::move(rhs));
    }
    bb.add(add_into(d.pointer, d.Kb * d.stride[1 - M_mode]));

    return std::make_unique<block_accessor_vector>(std::move(block));
}

auto read_matrix_block(block_builder &bb, matrix_block_description const &d, int M_mode,
                       core_config const &core_cfg, char const *block_name)
    -> std::unique_ptr<block_accessor> {
    assert(M_mode == 0 || M_mode == 1);

    if (d.is_unit_stride(1 - M_mode) &&
        (d.Kb == 2 || d.Kb == 3 || d.Kb == 4 || d.Kb == 8 || d.Kb == 16)) {
        return read_matrix_block_vector(bb, d, M_mode, core_cfg, block_name);
    }
    return read_matrix_block_regular(bb, d, M_mode, core_cfg, block_name);
}

void write_matrix_block(block_builder &bb, block_accessor const &block,
                        matrix_block_description const &d, bool is_atomic, expr alpha, expr beta,
                        core_config const &core_cfg) {
    const int m_blocks = 1 + (d.Mb - 1) / core_cfg.subgroup_size;

    const int first_m_block_with_check = d.first_block_with_check(core_cfg.subgroup_size);
    for (int k = 0; k < d.Kb; ++k) {
        for (int m_block = 0; m_block < m_blocks; ++m_block) {
            const auto write = [&](block_builder &bb) {
                store_helper(bb, is_atomic,
                             d.pointer + d.stride[0] * (get_sub_group_local_id() +
                                                        m_block * core_cfg.subgroup_size),
                             d.ty, d.as, alpha * block.get(m_block, k), beta);
            };
            if (m_block >= first_m_block_with_check) {
                bb.add(if_selection_builder(d.condition(m_block, core_cfg.subgroup_size))
                           .then(write)
                           .get_product());
            } else {
                write(bb);
            }
        }
        bb.add(add_into(d.pointer, d.stride[1]));
    }
}

} // namespace tinytc
