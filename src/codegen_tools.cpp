// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "codegen_tools.hpp"
#include "compiler_context.hpp"
#include "error.hpp"
#include "node/func.hpp"
#include "node/inst.hpp"
#include "node/region.hpp"
#include "node/type.hpp"
#include "node/value.hpp"
#include "node/visit.hpp"
#include "pass/constant_folding.hpp"
#include "scalar_type.hpp"
#include "tinytc/types.h"
#include "util/casting.hpp"
#include "util/ilist_base.hpp"
#include "util/overloaded.hpp"

#include <array>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>

namespace tinytc {

auto get_core_config_and_tiling(tinytc_func const &fn, const_tinytc_core_info_t info)
    -> std::pair<core_config, local_tiling> {
    const auto get_core_config = [&]() -> core_config {
        try {
            return info->get_core_config(fn.subgroup_size());
        } catch (std::out_of_range const &e) {
            throw compilation_error(fn.loc(), status::unsupported_subgroup_size);
        }
    };
    core_config core_cfg = get_core_config();
    const auto wgs = fn.work_group_size();
    local_tiling tiling = {wgs[0] / core_cfg.subgroup_size, wgs[1]};
    return {core_cfg, tiling};
}

void tile_loop_by_sgs(region_builder &bb, tinytc_value_t loop_trip_count, int sgs, int num_tiles,
                      tinytc_value_t sg_id, sgs_loop_body_builder const &body,
                      attr for_attributes) {
    auto ity = loop_trip_count->ty();
    auto bool_ty = boolean_data_type::get(ity->context());
    auto c_sgs = bb.create<constant_inst>(sgs, ity);
    auto c_sgs_tiles = bb.create<constant_inst>(sgs * num_tiles, ity);
    auto c0 = bb.create<constant_inst>(0, ity);
    auto c_tiles_1 = bb.create<constant_inst>(num_tiles - 1, ity);

    auto blocks = instant_constant_fold_add(bb, create<div_inst>(loop_trip_count, c_sgs, ity));
    auto rem = instant_constant_fold_add(bb, create<rem_inst>(loop_trip_count, c_sgs, ity));

    auto sg_id_cast = instant_constant_fold_add(bb, create<cast_inst>(sg_id, ity));
    auto is_blocks_gt_0 =
        instant_constant_fold_add(bb, create<greater_than_inst>(blocks, c0, bool_ty));
    bb.if_condition(is_blocks_gt_0, [&](region_builder &bb) {
        auto block_start = instant_constant_fold_add(bb, create<mul_inst>(c_sgs, sg_id_cast, ity));
        auto block_end = instant_constant_fold_add(bb, create<mul_inst>(c_sgs, blocks, ity));
        bb.for_loop(
            std::move(block_start), std::move(block_end), c_sgs_tiles,
            [&](region_builder &bb, tinytc_value_t block) { body(bb, block, false, c_sgs); },
            for_attributes);
    });

    auto condition0 = instant_constant_fold_add(bb, create<greater_than_inst>(rem, c0, bool_ty));
    bb.if_condition(condition0, [&](region_builder &bb) {
        auto condition1 =
            instant_constant_fold_add(bb, create<equal_inst>(sg_id_cast, c_tiles_1, bool_ty));
        bb.if_condition(condition1, [&](region_builder &bb) {
            auto block = instant_constant_fold_add(bb, create<mul_inst>(blocks, c_sgs, ity));
            body(bb, block, true, rem);
        });
    });
}

void tile_loop_uniformly(region_builder &bb, tinytc_value_t loop_trip_count, int block_size,
                         int num_tiles, tinytc_value_t sg_id, uniform_loop_body_builder const &body,
                         attr for_attributes) {
    auto ity = loop_trip_count->ty();
    auto bool_ty = boolean_data_type::get(ity->context());
    auto c0 = bb.create<constant_inst>(0, ity);
    auto c1 = bb.create<constant_inst>(1, ity);
    auto c_tiles = bb.create<constant_inst>(num_tiles, ity);

    // Here we compute
    // blocks = ceil(loop_trip_count / block_size) = 1 + (loop_trip_count - 1) / block_size
    // blocks = ceil(blocks / num_tiles) * num_tiles = (1 + (blocks - 1) / num_tiles) *
    // num_tiles
    auto c_block_size = bb.create<constant_inst>(block_size, ity);
    auto blocks0 = instant_constant_fold_add(bb, create<sub_inst>(loop_trip_count, c1, ity));
    auto blocks1 = instant_constant_fold_add(bb, create<div_inst>(blocks0, c_block_size, ity));
    auto blocks2 = instant_constant_fold_add(bb, create<div_inst>(blocks1, c_tiles, ity));
    auto blocks3 = instant_constant_fold_add(bb, create<add_inst>(c1, blocks2, ity));
    auto blocks = instant_constant_fold_add(bb, create<mul_inst>(blocks3, c_tiles, ity));

    auto bs = instant_constant_fold_add(bb, create<div_inst>(loop_trip_count, blocks, ity));
    auto bs_1 = instant_constant_fold_add(bb, create<add_inst>(bs, c1, ity));
    auto rem = instant_constant_fold_add(bb, create<rem_inst>(loop_trip_count, blocks, ity));

    auto sg_id_cast = instant_constant_fold_add(bb, create<cast_inst>(sg_id, ity));
    // The following if makes it easy to eliminate the remainder handler in optimization if rem
    // == 0 is known at compile time. Without the if, we would need to prove that block_start_1
    // is non-negative to eliminate the for-loop.
    auto is_rem_gt_0 = instant_constant_fold_add(bb, create<greater_than_inst>(rem, c0, bool_ty));
    bb.if_condition(is_rem_gt_0, [&](region_builder &bb) {
        auto block_start_1 = instant_constant_fold_add(bb, create<mul_inst>(bs_1, sg_id_cast, ity));
        auto block_end_1 = instant_constant_fold_add(bb, create<mul_inst>(bs_1, rem, ity));
        auto step_1 = instant_constant_fold_add(bb, create<mul_inst>(bs_1, c_tiles, ity));
        bb.for_loop(
            std::move(block_start_1), std::move(block_end_1), std::move(step_1),
            [&](region_builder &bb, tinytc_value_t block) { body(bb, block, bs_1); },
            for_attributes);
    });

    auto tmp0 = instant_constant_fold_add(bb, create<rem_inst>(rem, c_tiles, ity));
    auto tmp1 = instant_constant_fold_add(bb, create<add_inst>(sg_id_cast, tmp0, ity));
    auto sg_id_1 = instant_constant_fold_add(bb, create<rem_inst>(tmp1, c_tiles, ity));
    auto tmp2 = instant_constant_fold_add(bb, create<mul_inst>(bs, sg_id_1, ity));
    auto tmp3 = instant_constant_fold_add(bb, create<mul_inst>(bs_1, rem, ity));
    auto block_start = instant_constant_fold_add(bb, create<add_inst>(tmp3, tmp2, ity));
    auto step = instant_constant_fold_add(bb, create<mul_inst>(bs, c_tiles, ity));
    bb.for_loop(
        std::move(block_start), loop_trip_count, std::move(step),
        [&](region_builder &bb, tinytc_value_t block) { body(bb, block, bs); }, for_attributes);
}

auto promote_binop_operands(region_builder &bb, scalar_type result_ty, tinytc_value_t a,
                            tinytc_value_t b, location const &loc)
    -> std::pair<tinytc_value_t, tinytc_value_t> {
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
            a = bb.create<cast_inst>(a, promoted_ty, loc);
        }
        if (bt->ty() != result_ty) {
            b = bb.create<cast_inst>(b, promoted_ty, loc);
        }
    }
    return {a, b};
}

auto mixed_precision_coopmatrix_scale(region_builder &bb, tinytc_value_t a, tinytc_value_t b,
                                      location const &loc) -> tinytc_value_t {
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
        a = bb.create<cast_inst>(a, bt->ty(), loc);
    }
    return bb.create<cooperative_matrix_scale_inst>(a, b, bt, loc);
}

auto get_atomic_store_flag(tinytc_value_t beta) -> std::optional<store_flag> {
    constant_inst beta_cst = dyn_cast<constant_inst>(beta->defining_inst());
    if (beta_cst) {
        if (beta_cst.is_zero()) {
            return store_flag::atomic;
        } else if (beta_cst.is_identity()) {
            return store_flag::atomic_add;
        }
    }
    return std::nullopt;
}
void blas_update(region_builder &bb, bool atomic, tinytc_value_t alpha, tinytc_value_t ab,
                 tinytc_value_t beta, tinytc_value_t C, array_view<tinytc_value_t> index_list,
                 location const &loc) {
    memref_data_type *ct = dyn_cast<memref_data_type>(C->ty());
    if (ct == nullptr) {
        throw compilation_error(loc, {C}, status::ir_expected_scalar);
    }
    auto alpha_ab = mixed_precision_arithmetic<mul_inst>(bb, ct->element_ty(), alpha, ab, loc);
    if (atomic) {
        auto flag = get_atomic_store_flag(beta);
        if (!flag) {
            throw compilation_error(loc, status::ir_invalid_beta);
        }
        bb.create<store_inst>(*flag, alpha_ab, C, index_list, loc);
    } else {
        auto c = bb.create<load_inst>(C, index_list, ct->element_data_ty(), loc);
        auto beta_c = mixed_precision_arithmetic<mul_inst>(bb, ct->element_ty(), beta, c, loc);
        auto alpha_ab_plus_beta_c =
            mixed_precision_arithmetic<add_inst>(bb, ct->element_ty(), alpha_ab, beta_c, loc);
        bb.create<store_inst>(store_flag::regular, alpha_ab_plus_beta_c, C, index_list, loc);
    }
}

auto instant_constant_fold_add(region_builder &bb, inst i) -> tinytc_value_t {
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
        if (auto ci = dyn_cast<constant_inst>(i); ci) {
            if (std::holds_alternative<bool>(ci.value())) {
                return std::get<bool>(ci.value());
            }
        }
    }
    return std::nullopt;
}

auto get_int_constant(const_tinytc_value_t val) -> std::optional<std::int64_t> {
    if (auto i = val->defining_inst(); i) {
        if (auto ci = dyn_cast<constant_inst>(i); ci) {
            if (std::holds_alternative<std::int64_t>(ci.value())) {
                return std::get<std::int64_t>(ci.value());
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
auto get_yield(location const &loc, tinytc_region &reg) -> yield_inst {
    auto y = yield_inst(nullptr);
    if (auto it = reg.end(); --it != reg.end()) {
        y = dyn_cast<yield_inst>(it.get());
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

work_group_op::work_group_op(std::int32_t num_tiles, std::int32_t subgroup_size, tinytc_type_t ty)
    : num_tiles_{num_tiles}, subgroup_size_{subgroup_size}, ty_{ty}, tmp_{nullptr} {}

void work_group_op::setup(region_builder &bb, location const &loc) {
    if (num_tiles_ > 1) {
        auto tmp_ty = get_memref(ty_, {num_tiles_}, {}, address_space::local, loc);
        tmp_ = bb.create<alloca_inst>(tmp_ty, loc);
    }
}

void work_group_op::teardown(region_builder &bb) {
    if (tmp_) {
        bb.add(inst{lifetime_stop_inst::create(tmp_, {})});
    }
}

auto work_group_reduce::make(region_builder &bb, tinytc_value_t a, location const &loc)
    -> tinytc_value_t {
    auto a_reduced = bb.create<subgroup_reduce_add_inst>(a, ty_, loc);

    if (num_tiles_ > 1) {
        auto ctx = compiler_context{a->context(), true};
        auto bool_ty = get_boolean(ctx);
        auto i32_ty = get_scalar(ctx, scalar_type::i32);
        auto index_ty = get_scalar(ctx, scalar_type::index);

        auto sgid = bb.create<subgroup_linear_id_inst>(i32_ty, loc);
        auto sglid = bb.create<subgroup_local_id_inst>(i32_ty, loc);
        auto c_zero = bb.constant_zero(i32_ty, loc);
        auto is_sglid_0 = bb.create<equal_inst>(sglid, c_zero, bool_ty, loc);
        bb.if_condition(
            is_sglid_0,
            [&](region_builder &bb) {
                auto sgid_index = bb.create<cast_inst>(sgid, index_ty, loc);
                bb.create<store_inst>(store_flag::regular, a_reduced, tmp_, array_view{sgid_index},
                                      loc);
            },
            loc);
        bb.create<barrier_inst>(static_cast<tinytc_address_spaces_t>(address_space::local), loc);

        auto is_lid_0 = bb.create<equal_inst>(sgid, c_zero, bool_ty, loc);
        bb.if_condition(
            is_lid_0,
            [&](region_builder &bb) {
                auto c_num_tiles = bb.create<constant_inst>(num_tiles_, i32_ty, loc);
                auto c_sgs = bb.create<constant_inst>(subgroup_size_, i32_ty, loc);
                auto c_init = bb.constant_zero(ty_, loc);
                auto acc =
                    bb.for_loop(sglid, c_num_tiles, c_sgs, {c_init}, {ty_},
                                [&](region_builder &bb, array_view<tinytc_value_t> args) {
                                    auto lv_index = bb.create<cast_inst>(args[0], index_ty, loc);
                                    auto a_sg_reduced =
                                        bb.create<load_inst>(tmp_, array_view{lv_index}, ty_, loc);
                                    auto sum = bb.create<add_inst>(args[1], a_sg_reduced, ty_, loc);
                                    bb.create<yield_inst>(array_view{sum}, loc);
                                });
                a_reduced = bb.create<subgroup_reduce_add_inst>(acc[0], ty_, loc);
                return a_reduced;
            },
            loc);
    }
    return a_reduced;
}

auto work_group_inclusive_scan::make(region_builder &bb, tinytc_value_t a, bool compute_sum,
                                     location const &loc)
    -> std::pair<tinytc_value_t, tinytc_value_t> {
    auto a_scan = bb.create<subgroup_inclusive_scan_add_inst>(a, ty_, loc);

    auto ctx = compiler_context{a->context(), true};
    auto i32_ty = get_scalar(ctx, scalar_type::i32);

    if (num_tiles_ > 1) {
        auto bool_ty = get_boolean(ctx);
        auto index_ty = get_scalar(ctx, scalar_type::index);

        auto sgid = bb.create<subgroup_linear_id_inst>(i32_ty, loc);
        auto sglid = bb.create<subgroup_local_id_inst>(i32_ty, loc);

        auto c_sgs_1 = bb.create<constant_inst>(subgroup_size_ - 1, i32_ty, loc);
        auto is_last_sglid = bb.create<equal_inst>(sglid, c_sgs_1, bool_ty, loc);
        bb.if_condition(
            is_last_sglid,
            [&](region_builder &bb) {
                auto sgid_index = bb.create<cast_inst>(sgid, index_ty, loc);
                bb.create<store_inst>(store_flag::regular, a_scan, tmp_, array_view{sgid_index},
                                      loc);
            },
            loc);
        bb.create<barrier_inst>(static_cast<tinytc_address_spaces_t>(address_space::local), loc);

        auto c_zero = bb.constant_zero(i32_ty, loc);
        a_scan = bb.for_loop(c_zero, sgid, nullptr, {a_scan}, {ty_},
                             [&](region_builder &bb, array_view<tinytc_value_t> args) {
                                 auto lv_index = bb.create<cast_inst>(args[0], index_ty, loc);
                                 auto prefix =
                                     bb.create<load_inst>(tmp_, array_view{lv_index}, ty_, loc);
                                 auto scan = bb.create<add_inst>(args[1], prefix, ty_, loc);
                                 bb.create<yield_inst>(array_view{scan}, loc);
                             })[0];

        if (compute_sum) {
            auto c_num_tiles_1 = bb.create<constant_inst>(num_tiles_ - 1, i32_ty, loc);
            auto c_num_tiles_1_index = bb.create<cast_inst>(c_num_tiles_1, index_ty, loc);
            auto is_last_sgid = bb.create<equal_inst>(sgid, c_num_tiles_1, bool_ty, loc);
            auto is_last_work_item = bb.create<and_inst>(is_last_sglid, is_last_sgid, bool_ty, loc);
            bb.if_condition(
                is_last_work_item,
                [&](region_builder &bb) {
                    bb.create<store_inst>(store_flag::regular, a_scan, tmp_,
                                          array_view{c_num_tiles_1_index}, loc);
                },
                loc);
            bb.create<barrier_inst>(static_cast<tinytc_address_spaces_t>(address_space::local),
                                    loc);
            auto sum = bb.create<load_inst>(tmp_, array_view{c_num_tiles_1_index}, ty_, loc);
            return {a_scan, sum};
        }
    } else if (compute_sum) {
        auto c_sgs_1 = bb.create<constant_inst>(subgroup_size_ - 1, i32_ty, loc);
        auto sum = bb.create<subgroup_broadcast_inst>(a_scan, c_sgs_1, ty_, loc);
        return {a_scan, sum};
    }
    return {a_scan, nullptr};
}

} // namespace tinytc
