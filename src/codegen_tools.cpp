// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "codegen_tools.hpp"
#include "scalar_type.hpp"
#include "util.hpp"

#include <clir/attr_defs.hpp>
#include <clir/builtin_function.hpp>
#include <clir/data_type.hpp>
#include <clir/handle.hpp>
#include <clir/internal/expr_node.hpp>
#include <clir/visit.hpp>

#include <memory>
#include <utility>

using namespace clir;

namespace tinytc {

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

} // namespace tinytc
