// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "codegen_tools.hpp"
#include "compiler_context.hpp"
#include "error.hpp"
#include "node/data_type_node.hpp"
#include "node/inst_node.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "pass/constant_folding.hpp"
#include "scalar_type.hpp"
#include "support/casting.hpp"
#include "support/ilist_base.hpp"
#include "support/visit.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/types.h"

#include <type_traits>
#include <utility>
#include <variant>

namespace tinytc {

void tile_loop_by_sgs(region_builder &bb, value loop_trip_count, int sgs, int num_tiles,
                      value sg_id, sgs_loop_body_builder const &body, std::int32_t unroll_factor) {
    auto ity = loop_trip_count->ty();
    auto bool_ty = boolean_data_type::get(ity->context());
    auto c_sgs = bb.add(make_constant(sgs, ity));
    auto c_sgs_tiles = bb.add(make_constant(sgs * num_tiles, ity));
    auto c0 = bb.add(make_constant(0, ity));
    auto c_tiles_1 = bb.add(make_constant(num_tiles - 1, ity));

    auto blocks =
        instant_constant_fold_add(bb, make_arith(arithmetic::div, loop_trip_count, c_sgs, ity));
    auto rem =
        instant_constant_fold_add(bb, make_arith(arithmetic::rem, loop_trip_count, c_sgs, ity));

    auto sg_id_cast = instant_constant_fold_add(bb, make_cast(sg_id, ity));
    auto is_blocks_gt_0 =
        instant_constant_fold_add(bb, make_cmp(cmp_condition::gt, blocks, c0, bool_ty));
    bb.if_condition(is_blocks_gt_0, [&](region_builder &bb) {
        auto block_start =
            instant_constant_fold_add(bb, make_arith(arithmetic::mul, c_sgs, sg_id_cast, ity));
        auto block_end =
            instant_constant_fold_add(bb, make_arith(arithmetic::mul, c_sgs, blocks, ity));
        bb.for_loop(ity, std::move(block_start), std::move(block_end), c_sgs_tiles,
                    [&](region_builder &bb, value block) { body(bb, block, false, c_sgs); });
        auto it = bb.get_insertion_point();
        prev(it);
        //CHECK_STATUS(tinytc_inst_set_loop_unroll_factor(it, unroll_factor));
    });

    auto condition0 = instant_constant_fold_add(bb, make_cmp(cmp_condition::gt, rem, c0, bool_ty));
    bb.if_condition(condition0, [&](region_builder &bb) {
        auto condition1 = instant_constant_fold_add(
            bb, make_cmp(cmp_condition::eq, sg_id_cast, c_tiles_1, bool_ty));
        bb.if_condition(condition1, [&](region_builder &bb) {
            auto block =
                instant_constant_fold_add(bb, make_arith(arithmetic::mul, blocks, c_sgs, ity));
            body(bb, block, true, rem);
        });
    });
}

void tile_loop_uniformly(region_builder &bb, value loop_trip_count, int block_size, int num_tiles,
                         value sg_id, uniform_loop_body_builder const &body,
                         std::int32_t unroll_factor) {
    auto ity = loop_trip_count->ty();
    auto bool_ty = boolean_data_type::get(ity->context());
    auto c0 = bb.add(make_constant(0, ity));
    auto c1 = bb.add(make_constant(1, ity));
    auto c_tiles = bb.add(make_constant(num_tiles, ity));

    // Here we compute
    // blocks = ceil(loop_trip_count / block_size) = 1 + (loop_trip_count - 1) / block_size
    // blocks = ceil(blocks / num_tiles) * num_tiles = (1 + (blocks - 1) / num_tiles) *
    // num_tiles
    auto c_block_size = bb.add(make_constant(block_size, ity));
    auto blocks0 =
        instant_constant_fold_add(bb, make_arith(arithmetic::sub, loop_trip_count, c1, ity));
    auto blocks1 =
        instant_constant_fold_add(bb, make_arith(arithmetic::div, blocks0, c_block_size, ity));
    auto blocks2 =
        instant_constant_fold_add(bb, make_arith(arithmetic::div, blocks1, c_tiles, ity));
    auto blocks3 = instant_constant_fold_add(bb, make_arith(arithmetic::add, c1, blocks2, ity));
    auto blocks = instant_constant_fold_add(bb, make_arith(arithmetic::mul, blocks3, c_tiles, ity));

    auto bs =
        instant_constant_fold_add(bb, make_arith(arithmetic::div, loop_trip_count, blocks, ity));
    auto bs_1 = instant_constant_fold_add(bb, make_arith(arithmetic::add, bs, c1, ity));
    auto rem =
        instant_constant_fold_add(bb, make_arith(arithmetic::rem, loop_trip_count, blocks, ity));

    auto sg_id_cast = instant_constant_fold_add(bb, make_cast(sg_id, ity));
    // The following if makes it easy to eliminate the remainder handler in optimization if rem
    // == 0 is known at compile time. Without the if, we would need to prove that block_start_1
    // is non-negative to eliminate the for-loop.
    auto is_rem_gt_0 = instant_constant_fold_add(bb, make_cmp(cmp_condition::gt, rem, c0, bool_ty));
    bb.if_condition(is_rem_gt_0, [&](region_builder &bb) {
        auto block_start_1 =
            instant_constant_fold_add(bb, make_arith(arithmetic::mul, bs_1, sg_id_cast, ity));
        auto block_end_1 =
            instant_constant_fold_add(bb, make_arith(arithmetic::mul, bs_1, rem, ity));
        auto step_1 =
            instant_constant_fold_add(bb, make_arith(arithmetic::mul, bs_1, c_tiles, ity));
        bb.for_loop(ity, std::move(block_start_1), std::move(block_end_1), std::move(step_1),
                    [&](region_builder &bb, value block) { body(bb, block, bs_1); });
        auto it = bb.get_insertion_point();
        prev(it);
        //CHECK_STATUS(tinytc_inst_set_loop_unroll_factor(it, unroll_factor));
    });

    auto tmp0 = instant_constant_fold_add(bb, make_arith(arithmetic::rem, rem, c_tiles, ity));
    auto tmp1 = instant_constant_fold_add(bb, make_arith(arithmetic::add, sg_id_cast, tmp0, ity));
    auto sg_id_1 = instant_constant_fold_add(bb, make_arith(arithmetic::rem, tmp1, c_tiles, ity));
    auto tmp2 = instant_constant_fold_add(bb, make_arith(arithmetic::mul, bs, sg_id_1, ity));
    auto tmp3 = instant_constant_fold_add(bb, make_arith(arithmetic::mul, bs_1, rem, ity));
    auto block_start = instant_constant_fold_add(bb, make_arith(arithmetic::add, tmp3, tmp2, ity));
    auto step = instant_constant_fold_add(bb, make_arith(arithmetic::mul, bs, c_tiles, ity));
    bb.for_loop(ity, std::move(block_start), loop_trip_count, std::move(step),
                [&](region_builder &bb, value block) { body(bb, block, bs); });
    auto it = bb.get_insertion_point();
    prev(it);
    //CHECK_STATUS(tinytc_inst_set_loop_unroll_factor(it, unroll_factor));
}

auto mixed_precision_arithmetic(region_builder &bb, scalar_type result_ty, arithmetic operation,
                                value a, value b, location const &loc) -> value {
    scalar_data_type *at = dyn_cast<scalar_data_type>(a->ty());
    scalar_data_type *bt = dyn_cast<scalar_data_type>(b->ty());
    if (at == nullptr || bt == nullptr) {
        throw compilation_error(loc, status::ir_expected_scalar);
    }
    if (at->ty() != result_ty || bt->ty() != result_ty) {
        if (!promotable(at->ty(), result_ty) || !promotable(bt->ty(), result_ty)) {
            throw compilation_error(loc, status::ir_forbidden_promotion);
        }
        auto promoted_ty = scalar_data_type::get(at->context(), result_ty);

        if (at->ty() != result_ty) {
            a = bb.add(make_cast(a, promoted_ty, loc));
        }
        if (bt->ty() != result_ty) {
            b = bb.add(make_cast(b, promoted_ty, loc));
        }
    }
    return bb.add(
        make_arith(operation, a, b, scalar_data_type::get(at->context(), result_ty), loc));
}
auto mixed_precision_coopmatrix_scale(region_builder &bb, value a, value b,
                                      location const &loc) -> value {
    scalar_data_type *at = dyn_cast<scalar_data_type>(a->ty());
    if (at == nullptr) {
        throw compilation_error(loc, status::ir_expected_scalar);
    }
    coopmatrix_data_type *bt = dyn_cast<coopmatrix_data_type>(b->ty());
    if (bt == nullptr) {
        throw compilation_error(loc, status::ir_expected_coopmatrix);
    }
    const auto a_ty = at->ty();
    const auto b_ty = bt->component_ty();
    if (a_ty != b_ty) {
        if (!promotable(a_ty, b_ty)) {
            throw compilation_error(loc, status::ir_forbidden_promotion);
        }
        a = bb.add(make_cast(a, bt->ty(), loc));
    }
    return bb.add(make_cooperative_matrix_scale(a, b, bt, loc));
}

auto get_atomic_store_flag(value beta) -> std::optional<store_flag> {
    constant_inst *beta_cst = dyn_cast<constant_inst>(beta->defining_inst());
    if (beta_cst) {
        if (beta_cst->is_zero()) {
            return store_flag::atomic;
        } else if (beta_cst->is_identity()) {
            return store_flag::atomic_add;
        }
    }
    return std::nullopt;
}
void blas_update(region_builder &bb, bool atomic, value alpha, value ab, value beta, value C,
                 array_view<value> index_list, location const &loc) {
    memref_data_type *ct = dyn_cast<memref_data_type>(C->ty());
    if (ct == nullptr) {
        throw compilation_error(loc, {C.get()}, status::ir_expected_scalar);
    }
    auto alpha_ab =
        mixed_precision_arithmetic(bb, ct->element_ty(), arithmetic::mul, alpha, ab, loc);
    if (atomic) {
        auto flag = get_atomic_store_flag(beta);
        if (!flag) {
            throw compilation_error(loc, status::ir_invalid_beta);
        }
        bb.add(make_store(*flag, alpha_ab, C, index_list, loc));
    } else {
        auto c = bb.add(make_load(C, index_list, ct->element_data_ty(), loc));
        auto beta_c =
            mixed_precision_arithmetic(bb, ct->element_ty(), arithmetic::mul, beta, c, loc);
        auto alpha_ab_plus_beta_c = mixed_precision_arithmetic(
            bb, ct->element_ty(), arithmetic::add, alpha_ab, beta_c, loc);
        bb.add(make_store(store_flag::regular, alpha_ab_plus_beta_c, C, index_list, loc));
    }
}

auto instant_constant_fold_add(region_builder &bb, inst i) -> value {
    auto ctx = i->context();
    if (!ctx) {
        throw compilation_error(i->loc(), status::internal_compiler_error);
    }

    auto fold = visit(constant_folding{ctx->opt_flag(optflag::unsafe_fp_math)}, *i);
    auto val = std::visit(overloaded{[](tinytc_value_t &v) -> tinytc_value_t { return v; },
                                     [&bb](inst &j) -> tinytc_value_t {
                                         if (j) {
                                             return bb.add(std::move(j));
                                         }
                                         return nullptr;
                                     }},
                          fold);
    if (val) {
        return val;
    }
    return bb.add(std::move(i));
}

auto get_bool_constant(tinytc_value_t val) -> std::optional<bool> {
    if (auto i = val->defining_inst(); i) {
        if (auto *ci = dyn_cast<constant_inst>(i); ci) {
            if (std::holds_alternative<bool>(ci->value())) {
                return std::get<bool>(ci->value());
            }
        }
    }
    return std::nullopt;
}

auto get_int_constant(const_tinytc_value_t val) -> std::optional<std::int64_t> {
    if (auto i = val->defining_inst(); i) {
        if (auto *ci = dyn_cast<const constant_inst>(i); ci) {
            if (std::holds_alternative<std::int64_t>(ci->value())) {
                return std::get<std::int64_t>(ci->value());
            }
        }
    }
    return std::nullopt;
}

auto get_int_constant(tinytc_value const &val) -> std::optional<std::int64_t> {
    return get_int_constant(&val);
}

auto get_coopmatrix_type(tinytc_value const &v) -> coopmatrix_data_type const * {
    auto ct = dyn_cast<coopmatrix_data_type>(v.ty());
    if (!ct) {
        throw compilation_error(v.loc(), status::ir_expected_coopmatrix);
    }
    return ct;
}
auto get_memref_type(tinytc_value const &v) -> memref_data_type const * {
    auto mt = dyn_cast<memref_data_type>(v.ty());
    if (!mt) {
        throw compilation_error(v.loc(), status::ir_expected_memref);
    }
    return mt;
}
auto get_scalar_type(tinytc_value const &v) -> scalar_type {
    auto st = dyn_cast<scalar_data_type>(v.ty());
    if (!st) {
        throw compilation_error(v.loc(), status::ir_expected_scalar);
    }
    return st->ty();
}
auto get_yield(location const &loc, tinytc_region const &reg) -> yield_inst const * {
    const yield_inst *y = nullptr;
    if (auto it = reg.end(); --it != reg.end()) {
        y = dyn_cast<const yield_inst>(it.get());
    }
    if (!y) {
        throw compilation_error(loc, status::ir_must_have_yield);
    }
    return y;
}

auto add_check(checked_flag flag, checked_flag new_flag) -> checked_flag {
    return checked_flag{std::underlying_type_t<checked_flag>(flag) |
                        std::underlying_type_t<checked_flag>(new_flag)};
}

} // namespace tinytc
