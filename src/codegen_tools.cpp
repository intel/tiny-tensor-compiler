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

short bits(scalar_type ty) { return size(ty) * 8; }
expr constant(scalar_type ty, std::int64_t value) { return expr(value, bits(ty)); }
expr constant(scalar_type ty, double value) {
    if (is_complex_type(ty)) {
        const auto ety = element_type(ty);
        return init_vector(to_clir_ty(ty), {constant(ety, value), constant(ety, 0.0)});
    }
    return expr(value, bits(ty));
}
expr multiply(scalar_type ty_a, scalar_type ty_b, expr a, expr b) {
    if (is_complex_type(ty_a) && is_complex_type(ty_b)) {
        return a * b.s(0) + init_vector(to_clir_ty(ty_a), {-a.s(1), a.s(0)}) * b.s(1);
    }
    return a * b;
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

struct block_rw_config {
    builtin_type cast_type;
    expr (*sub_group_block_read)(expr);
    expr (*sub_group_block_write)(expr, expr);
    expr (*as_type)(expr);
};

auto get_block_rw_config(scalar_type ty) {
    switch (ty) {
    case scalar_type::i16:
        return block_rw_config{builtin_type::ushort_t, &intel_sub_group_block_read_us,
                               &intel_sub_group_block_write_us, &as_short};
    case scalar_type::i32:
        return block_rw_config{builtin_type::uint_t, &intel_sub_group_block_read_ui,
                               &intel_sub_group_block_write_ui, &as_int};
    case scalar_type::f32:
        return block_rw_config{builtin_type::uint_t, &intel_sub_group_block_read_ui,
                               &intel_sub_group_block_write_ui, &as_float};
    case scalar_type::i64:
        return block_rw_config{builtin_type::ulong_t, &intel_sub_group_block_read_ul,
                               &intel_sub_group_block_write_ul, &as_long};
    case scalar_type::f64:
        return block_rw_config{builtin_type::ulong_t, &intel_sub_group_block_read_ul,
                               &intel_sub_group_block_write_ul, &as_double};
    default:
        break;
    }
    return block_rw_config{builtin_type::void_t, nullptr, nullptr, nullptr};
}

expr sub_group_block_read_helper(expr pointer, scalar_type ty, address_space as) {
    const auto cfg = get_block_rw_config(ty);
    if (cfg.sub_group_block_read == nullptr) {
        return pointer[get_sub_group_local_id()];
    }
    pointer = cast(pointer_to(clir::data_type(cfg.cast_type, as)), std::move(pointer));
    auto inst = (*cfg.sub_group_block_read)(std::move(pointer));
    return (*cfg.as_type)(std::move(inst));
}
expr sub_group_block_write_helper(expr pointer, expr data, scalar_type ty, address_space as) {
    const auto cfg = get_block_rw_config(ty);
    if (cfg.sub_group_block_write == nullptr) {
        return pointer[get_sub_group_local_id()] = std::move(data);
    }
    pointer = cast(pointer_to(clir::data_type(cfg.cast_type, as)), std::move(pointer));
    data = (*cfg.as_type)(std::move(data));
    return (*cfg.sub_group_block_write)(std::move(pointer), std::move(data));
}

void store_helper(block_builder &bb, bool is_atomic, expr dst, scalar_type ty, address_space as,
                  expr value, scalar_type beta_ty, expr beta) {
    if (is_atomic) {
        atomic_store_helper(bb, std::move(dst), ty, as, std::move(value), beta_ty, std::move(beta));
    } else {
        const auto c_scaled = multiply(ty, beta_ty, dereference(dst), beta);
        bb.assign(dereference(dst), std::move(value) + std::move(c_scaled));
    }
}

void atomic_store_helper(block_builder &bb, expr dst, scalar_type ty, address_space as, expr value,
                         scalar_type beta_ty, expr beta) {
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

block_accessor_regular::block_accessor_regular(expr block, int Kb)
    : block_(std::move(block)), offset_{clir::expr{nullptr}}, Kb_(Kb) {}
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
    auto block = bb.declare(array_of(to_clir_ty(d.ty), m_blocks * d.Kb), block_name);

    const int first_m_block_with_check = d.first_block_with_check(core_cfg.subgroup_size);
    const bool enable_sub_group_reads =
        core_cfg.block_read_write_supported && d.is_unit_stride(M_mode);
    for (int k = 0; k < d.Kb; ++k) {
        for (int m_block = 0; m_block < m_blocks; ++m_block) {
            auto const store = [&](expr rhs) {
                bb.assign(block[k + m_block * d.Kb], std::move(rhs));
            };
            if (enable_sub_group_reads && m_block < first_m_block_with_check) {
                store(sub_group_block_read_helper(d.pointer + m_block * core_cfg.subgroup_size,
                                                  d.ty, d.as));
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
    const auto dt = to_clir_ty(d.ty, d.Kb);
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

    if (d.is_unit_stride(1 - M_mode) && !is_complex_type(d.ty) &&
        (d.Kb == 2 || d.Kb == 3 || d.Kb == 4 || d.Kb == 8 || d.Kb == 16)) {
        return read_matrix_block_vector(bb, d, M_mode, core_cfg, block_name);
    }
    return read_matrix_block_regular(bb, d, M_mode, core_cfg, block_name);
}

void write_matrix_block(block_builder &bb, block_accessor const &block,
                        matrix_block_description const &d, bool is_atomic, scalar_type beta_ty,
                        expr beta, core_config const &core_cfg) {
    const int m_blocks = 1 + (d.Mb - 1) / core_cfg.subgroup_size;

    const int first_m_block_with_check = d.first_block_with_check(core_cfg.subgroup_size);
    for (int k = 0; k < d.Kb; ++k) {
        for (int m_block = 0; m_block < m_blocks; ++m_block) {
            const auto write = [&](block_builder &bb) {
                store_helper(bb, is_atomic,
                             d.pointer + d.stride[0] * (get_sub_group_local_id() +
                                                        m_block * core_cfg.subgroup_size),
                             d.ty, d.as, block.get(m_block, k), beta_ty, beta);
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
