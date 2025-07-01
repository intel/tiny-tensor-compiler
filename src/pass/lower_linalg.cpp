// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/lower_linalg.hpp"
#include "codegen_tools.hpp"
#include "device_info.hpp"
#include "error.hpp"
#include "gemm_tools.hpp"
#include "matrix_ext_info.hpp"
#include "node/inst.hpp"
#include "node/inst_view.hpp"
#include "node/region.hpp"
#include "node/type.hpp"
#include "node/value.hpp"
#include "node/visit.hpp"
#include "scalar_type.hpp"
#include "support/walk.hpp"
#include "tiling.hpp"
#include "tinytc/builder.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"
#include "util/casting.hpp"
#include "util/ilist.hpp"
#include "util/ilist_base.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

namespace tinytc {

void gemm_microkernel(region_builder &bb, transpose tA, transpose tB, bool atomic,
                      tinytc_value_t alpha, tinytc_value_t A, tinytc_value_t B, tinytc_value_t beta,
                      tinytc_value_t C, tinytc_value_t K, tinytc_value_t m_block,
                      std::int32_t m_block_size, std::int32_t num_m_blocks, bool m_check,
                      tinytc_value_t n_block, std::int32_t n_block_size, std::int32_t num_n_blocks,
                      bool n_check, array_view<std::int32_t> K_block_sizes, tinytc_type_t a_ty,
                      tinytc_type_t b_ty, tinytc_type_t c_ty, attr for_attributes,
                      location const &loc) {
    auto ctx = m_block->context();
    auto bool_ty = boolean_type::get(ctx);
    auto index_ty = index_type::get(ctx);

    const auto check_a = m_check ? checked_flag::rows : checked_flag::none;
    const auto check_b = n_check ? checked_flag::cols : checked_flag::none;
    const auto check_c = [&] {
        if (m_check && n_check) {
            return checked_flag::both;
        } else if (m_check) {
            return checked_flag::rows;
        } else if (n_check) {
            return checked_flag::cols;
        }
        return checked_flag::none;
    }();
    auto c_m_block_size = bb.create<constant_inst>(m_block_size, index_ty, loc);
    auto c_n_block_size = bb.create<constant_inst>(n_block_size, index_ty, loc);

    const auto c_acc_ty = [&c_ty, &loc]() {
        if (!isa<number_type>(*c_ty)) {
            throw compilation_error(loc, status::ir_expected_number);
        }
        return acc_type(c_ty);
    }();

    auto coopmatrix_c_ty = get<coopmatrix_type>(c_ty, m_block_size, n_block_size, matrix_use::acc);
    auto coopmatrix_c_acc_ty =
        get<coopmatrix_type>(c_acc_ty, m_block_size, n_block_size, matrix_use::acc);
    auto const compute_c_step = [&](region_builder &bb, std::int32_t k_block_size, tinytc_value_t k,
                                    array_view<tinytc_value_t> const &c_acc,
                                    array_view<tinytc_type_t> const &c_acc_tys,
                                    bool check_k = false) {
        tinytc_value_t pos_a[2] = {m_block, k};
        int amode = 0;
        if (tA == transpose::T) {
            std::swap(pos_a[0], pos_a[1]);
            amode = 1 - amode;
        }
        auto coopmatrix_a_ty =
            get<coopmatrix_type>(a_ty, m_block_size, k_block_size, matrix_use::a);
        const auto my_check_a = check_k ? add_check(check_a, checked_flag::cols) : check_a;
        auto a = std::vector<tinytc_value_t>{};
        a.reserve(num_m_blocks);
        for (std::int32_t i = 0; i < num_m_blocks; ++i) {
            a.emplace_back(bb.create<cooperative_matrix_load_inst>(tA, my_check_a, A, pos_a[0],
                                                                   pos_a[1], coopmatrix_a_ty));
            if (i + 1 < num_m_blocks) {
                pos_a[amode] = bb.create<add_inst>(pos_a[amode], c_m_block_size, index_ty, loc);
            }
        }

        tinytc_value_t pos_b[2] = {k, n_block};
        int bmode = 1;
        if (tB == transpose::T) {
            std::swap(pos_b[0], pos_b[1]);
            bmode = 1 - bmode;
        }
        auto coopmatrix_b_ty =
            get<coopmatrix_type>(b_ty, k_block_size, n_block_size, matrix_use::b);
        const auto my_check_b = check_k ? add_check(check_b, checked_flag::rows) : check_b;
        auto b = std::vector<tinytc_value_t>{};
        b.reserve(num_n_blocks);
        for (std::int32_t i = 0; i < num_n_blocks; ++i) {
            b.emplace_back(bb.create<cooperative_matrix_load_inst>(tB, my_check_b, B, pos_b[0],
                                                                   pos_b[1], coopmatrix_b_ty));
            if (i + 1 < num_n_blocks) {
                pos_b[bmode] = bb.create<add_inst>(pos_b[bmode], c_n_block_size, index_ty, loc);
            }
        }

        auto c_next = std::vector<tinytc_value_t>{};
        c_next.reserve(num_m_blocks * num_n_blocks);
        for (std::int32_t n = 0; n < num_n_blocks; ++n) {
            for (std::int32_t m = 0; m < num_m_blocks; ++m) {
                c_next.emplace_back(bb.create<cooperative_matrix_mul_add_inst>(
                    a[m], b[n], c_acc[m + n * num_m_blocks], c_acc_tys[m + n * num_m_blocks], loc));
            }
        }
        return c_next;
    };
    auto const compute_c = [&](region_builder &bb, std::int32_t k_block_size, tinytc_value_t K0,
                               tinytc_value_t K1, std::vector<tinytc_value_t> const &c_acc,
                               std::vector<tinytc_type_t> const &c_acc_tys,
                               bool check_k = false) -> std::vector<tinytc_value_t> {
        auto c_step = bb.create<constant_inst>(k_block_size, index_ty, loc);
        auto return_values = bb.for_loop(
            K0, K1, c_step, c_acc, c_acc_tys,
            [&](region_builder &bb, array_view<tinytc_value_t> p) {
                const auto k = p[0];
                auto c_acc_iter = array_view<tinytc_value_t>(p.begin() + 1, p.end());
                auto c_next = compute_c_step(bb, k_block_size, k, c_acc_iter, c_acc_tys, check_k);
                bb.create<yield_inst>(c_next, loc);
            },
            for_attributes);
        return return_values;
    };

    auto c_acc = std::vector<tinytc_value_t>{};
    c_acc.reserve(num_m_blocks * num_n_blocks);
    for (std::int32_t i = 0; i < num_m_blocks * num_n_blocks; ++i) {
        c_acc.emplace_back(bb.constant_zero(coopmatrix_c_acc_ty, loc));
    }
    auto c_acc_tys = std::vector<tinytc_type_t>(c_acc.size());
    for (auto &ty : c_acc_tys) {
        ty = coopmatrix_c_acc_ty;
    }
    auto c_tys = std::vector<tinytc_type_t>(c_acc.size());
    for (auto &ty : c_tys) {
        ty = coopmatrix_c_ty;
    }

    auto k_block_size = K_block_sizes.back();

    const auto const_K = get_int_constant(K);
    if (const_K) {
        k_block_size = choose_k_block_size(K_block_sizes, *const_K);
    }

    auto c_zero = bb.constant_zero(index_ty, loc);
    auto c_k_block_size = bb.create<constant_inst>(k_block_size, index_ty, loc);
    auto tmp = instant_constant_fold_add(bb, create<div_inst>(K, c_k_block_size, index_ty, loc));
    auto K0 = instant_constant_fold_add(bb, create<mul_inst>(tmp, c_k_block_size, index_ty, loc));
    auto needs_remainder =
        instant_constant_fold_add(bb, create<less_than_inst>(K0, K, bool_ty, loc));
    auto r = get_bool_constant(needs_remainder);
    if (r) {
        if (*r != 0) {
            c_acc = compute_c(bb, k_block_size, c_zero, K0, c_acc, c_acc_tys);
            const auto K_block_size = K_block_sizes.front();
            c_acc = compute_c(bb, K_block_size, K0, K, c_acc, c_acc_tys, K_block_size > 1);
        } else {
            c_acc = compute_c(bb, k_block_size, c_zero, K0, c_acc, c_acc_tys);
        }
    } else {
        c_acc = compute_c(bb, k_block_size, c_zero, K0, c_acc, c_acc_tys);
        auto remainder = bb.ifelse(
            needs_remainder,
            [&](region_builder &bb) {
                const auto K_block_size = K_block_sizes.front();
                auto c_next =
                    compute_c(bb, K_block_size, K0, K, c_acc, c_acc_tys, K_block_size > 1);
                bb.create<yield_inst>(c_next, loc);
            },
            [&](region_builder &bb) { bb.create<yield_inst>(c_acc, loc); }, c_acc_tys, loc);
        c_acc = std::move(remainder);
    }

    for (auto &a : c_acc) {
        a = mixed_precision_coopmatrix_scale(bb, alpha, a, loc);
    }

    const bool needs_final_cast = coopmatrix_c_ty != coopmatrix_c_acc_ty;
    if (atomic) {
        auto flag = get_atomic_store_flag(beta);
        if (!flag) {
            throw compilation_error(loc, status::ir_invalid_beta);
        }
        for (std::int32_t n = 0; n < num_n_blocks; ++n) {
            auto pos1_offset = bb.create<constant_inst>(n * n_block_size, index_ty, loc);
            auto pos1 = bb.create<add_inst>(n_block, pos1_offset, index_ty, loc);
            for (std::int32_t m = 0; m < num_m_blocks; ++m) {
                auto pos0_offset = bb.create<constant_inst>(m * m_block_size, index_ty, loc);
                auto pos0 = bb.create<add_inst>(m_block, pos0_offset, index_ty, loc);
                auto alpha_ab_mn = c_acc[m + n * num_m_blocks];
                if (needs_final_cast) {
                    alpha_ab_mn = bb.create<cast_inst>(alpha_ab_mn, coopmatrix_c_ty, loc);
                }
                bb.create<cooperative_matrix_store_inst>(transpose::N, check_c, *flag, alpha_ab_mn,
                                                         C, pos0, pos1, loc);
            }
        }
    } else {
        for (std::int32_t n = 0; n < num_n_blocks; ++n) {
            auto pos1_offset = bb.create<constant_inst>(n * n_block_size, index_ty, loc);
            auto pos1 = bb.create<add_inst>(n_block, pos1_offset, index_ty, loc);
            for (std::int32_t m = 0; m < num_m_blocks; ++m) {
                auto pos0_offset = bb.create<constant_inst>(m * m_block_size, index_ty, loc);
                auto pos0 = bb.create<add_inst>(m_block, pos0_offset, index_ty, loc);
                auto c_load = bb.create<cooperative_matrix_load_inst>(transpose::N, check_c, C,
                                                                      pos0, pos1, coopmatrix_c_ty);
                auto &alpha_ab_mn = c_acc[m + n * num_m_blocks];
                auto alpha_ab_plus_beta_c = [&] {
                    if (needs_final_cast) {
                        auto c_load_acc = bb.create<cast_inst>(c_load, coopmatrix_c_acc_ty, loc);
                        auto beta_c = mixed_precision_coopmatrix_scale(bb, beta, c_load_acc, loc);
                        auto alpha_ab_plus_beta_c =
                            bb.create<add_inst>(alpha_ab_mn, beta_c, alpha_ab_mn->ty(), loc);
                        return bb.create<cast_inst>(alpha_ab_plus_beta_c, coopmatrix_c_ty, loc);
                    } else {
                        auto beta_c = mixed_precision_coopmatrix_scale(bb, beta, c_load, loc);
                        return bb.create<add_inst>(alpha_ab_mn, beta_c, alpha_ab_mn->ty(), loc);
                    }
                }();
                bb.create<cooperative_matrix_store_inst>(transpose::N, check_c, store_flag::regular,
                                                         alpha_ab_plus_beta_c, C, pos0, pos1, loc);
            }
        }
    }
}

class linalg_generator {
  public:
    linalg_generator(local_tiling const &tiling, core_config const &core_cfg, tinytc_region &reg,
                     tinytc_inst_iterator_t ip)
        : tiling_{tiling}, core_cfg_{core_cfg}, bb_{&reg, ip} {}
    inline void operator()(inst_view in) {
        throw compilation_error(in.loc(), status::not_implemented);
    }
    void operator()(axpby_inst in);
    void operator()(cumsum_inst in);
    void operator()(gemm_inst in);
    void operator()(gemv_inst in);
    void operator()(ger_inst in);
    void operator()(hadamard_inst in);
    void operator()(sum_inst in);

    inline auto insertion_point() -> tinytc_inst_iterator_t { return bb_.get_insertion_point(); }

  private:
    auto get_memref_type(tinytc_value const &v) const -> const memref_type *;

    local_tiling const &tiling_;
    core_config const &core_cfg_;
    region_builder bb_;
};

auto linalg_generator::get_memref_type(tinytc_value const &v) const -> const memref_type * {
    auto t = dyn_cast<memref_type>(v.ty());
    if (t == nullptr) {
        throw compilation_error(v.loc(), status::ir_expected_memref);
    }
    return t;
}

void linalg_generator::operator()(axpby_inst in) {
    auto ctx = in.alpha().context();
    auto bool_ty = get<boolean_type>(ctx);
    auto index_ty = get<index_type>(ctx);

    auto at = get_memref_type(in.A());
    auto bt = get_memref_type(in.B());
    if (bt->dim() == 0) {
        auto parallel = create<parallel_inst>(in.loc());
        tinytc_region_t body = &parallel->child_region(0);
        auto bb = region_builder{body};

        auto i32_ty = get<i32_type>(ctx);
        auto sg_id = bb.create<subgroup_linear_id_inst>(i32_ty, in.loc());
        auto sg_lid = bb.create<subgroup_local_id_inst>(i32_ty, in.loc());
        auto c0 = bb.create<constant_inst>(0, i32_ty);
        auto cond0 = bb.create<equal_inst>(sg_id, c0, bool_ty, in.loc());
        auto cond1 = bb.create<equal_inst>(sg_lid, c0, bool_ty, in.loc());
        auto cond = bb.create<and_inst>(cond0, cond1, cond0->ty());
        bb.if_condition(cond, [&](region_builder &bb) {
            auto a = bb.create<load_inst>(&in.A(), array_view<tinytc_value_t>{}, at->element_ty(),
                                          in.loc());
            blas_update(bb, in.atomic(), &in.alpha(), a, &in.beta(), &in.B(), {}, in.loc());
        });

        bb_.add(std::move(parallel));
    } else if (bt->dim() == 1) {
        auto c0 = bb_.constant_zero(index_ty, in.loc());
        auto c_shape0 =
            instant_constant_fold_add(bb_, create<size_inst>(0, &in.B(), index_ty, in.loc()));
        bb_.foreach_loop(
            {c0}, {c_shape0},
            [&](region_builder &bb, auto loop_vars) {
                auto a = bb.create<load_inst>(&in.A(), array_view{loop_vars[0]}, at->element_ty(),
                                              in.loc());
                blas_update(bb, in.atomic(), &in.alpha(), a, &in.beta(), &in.B(), {loop_vars[0]},
                            in.loc());
            },
            in.loc());
    } else if (bt->dim() == 2) {
        auto c0 = bb_.constant_zero(index_ty, in.loc());
        auto c_shape0 =
            instant_constant_fold_add(bb_, create<size_inst>(0, &in.B(), index_ty, in.loc()));
        auto c_shape1 =
            instant_constant_fold_add(bb_, create<size_inst>(1, &in.B(), index_ty, in.loc()));
        bb_.foreach_loop(
            {c0, c0}, {c_shape0, c_shape1},
            [&](region_builder &bb, auto loop_vars) {
                auto a_idx = std::array<tinytc_value_t, 2u>{loop_vars[0], loop_vars[1]};
                if (in.tA() == transpose::T) {
                    std::swap(a_idx[0], a_idx[1]);
                }
                auto a = bb.create<load_inst>(&in.A(), a_idx, at->element_ty(), in.loc());
                blas_update(bb, in.atomic(), &in.alpha(), a, &in.beta(), &in.B(),
                            {loop_vars[0], loop_vars[1]}, in.loc());
            },
            in.loc());
    }
}

void linalg_generator::operator()(cumsum_inst in) {
    auto at = get_memref_type(in.A());
    auto bt = get_memref_type(in.B());

    const auto num_tiles = tiling_.m_tiles() * tiling_.n_tiles();
    auto ctx = in.alpha().context();
    auto bool_ty = get<boolean_type>(ctx);
    auto i32_ty = get<i32_type>(ctx);
    auto index_ty = get<index_type>(ctx);
    const auto &loc = in.loc();

    auto const scan_loop_1d = [&](region_builder &bb, work_group_inclusive_scan &scan,
                                  tinytc_value_t a_sub, tinytc_value_t b_sub) {
        auto c_sgs = bb.create<constant_inst>(scan.subgroup_size(), i32_ty, loc);
        auto sglid = bb.create<subgroup_local_id_inst>(i32_ty, loc);
        auto from_index = [&]() -> tinytc_value_t {
            if (scan.num_tiles() > 1) {
                auto sgid = bb.create<subgroup_linear_id_inst>(i32_ty, loc);
                auto from0 = bb.create<mul_inst>(sgid, c_sgs, i32_ty, loc);
                auto from1 = bb.create<add_inst>(from0, sglid, i32_ty, loc);
                return bb.create<cast_inst>(from1, index_ty, loc);
            } else {
                return bb.create<cast_inst>(sglid, index_ty, loc);
            }
        }();

        auto c_step =
            bb.create<constant_inst>(scan.subgroup_size() * scan.num_tiles(), index_ty, loc);

        auto c_1 = bb.constant_one(index_ty, loc);
        auto shape0 = instant_constant_fold_add(bb, create<size_inst>(0, a_sub, index_ty, loc));
        auto tr0 = instant_constant_fold_add(bb, create<sub_inst>(shape0, c_1, index_ty, loc));
        auto tr1 = instant_constant_fold_add(bb, create<div_inst>(tr0, c_step, index_ty, loc));
        auto tr2 = instant_constant_fold_add(bb, create<add_inst>(tr1, c_1, index_ty, loc));
        auto trip_count =
            instant_constant_fold_add(bb, create<mul_inst>(tr2, c_step, index_ty, loc));

        auto c_init = bb.constant_zero(bt->element_ty(), loc);
        auto a_scan = bb.for_loop(
            from_index, trip_count, c_step, {c_init}, {bt->element_ty()},
            [&](region_builder &bb, array_view<tinytc_value_t> args) {
                auto is_in_bounds = bb.create<less_than_inst>(args[0], shape0, bool_ty, loc);
                auto a = bb.ifelse(
                    is_in_bounds,
                    [&](region_builder &bb) {
                        auto a =
                            bb.create<load_inst>(a_sub, array_view{args[0]}, at->element_ty(), loc);
                        if (at->element_ty() != bt->element_ty()) {
                            a = bb.create<cast_inst>(a, bt->element_ty(), loc);
                        }
                        bb.create<yield_inst>(array_view{a}, loc);
                    },
                    [&](region_builder &bb) { bb.create<yield_inst>(array_view{c_init}, loc); },
                    {bt->element_ty()}, loc);
                auto [a_scan, next_prefix] = scan.make(bb, a[0], true, loc);
                a_scan = bb.create<add_inst>(args[1], a_scan, bt->element_ty(), loc);
                next_prefix = bb.create<add_inst>(args[1], next_prefix, bt->element_ty(), loc);
                bb.if_condition(
                    is_in_bounds,
                    [&](region_builder &bb) {
                        blas_update(bb, in.atomic(), &in.alpha(), a_scan, &in.beta(), b_sub,
                                    {args[0]}, loc);
                    },
                    loc);
                bb.create<yield_inst>(array_view{next_prefix}, loc);
            });
    };

    if (bt->dim() == 1) {
        auto parallel = create<parallel_inst>(loc);
        tinytc_region_t body = &parallel->child_region(0);
        auto bb = region_builder{body};

        auto scan = work_group_inclusive_scan(num_tiles, core_cfg_.subgroup_size, bt->element_ty());
        scan.setup(bb_, loc);

        scan_loop_1d(bb, scan, &in.A(), &in.B());

        bb_.add(std::move(parallel));
        scan.teardown(bb_);
    } else if (bt->dim() >= 2 && in.mode() == 0) {
        auto scan = work_group_inclusive_scan(1, core_cfg_.subgroup_size, bt->element_ty());
        scan.setup(bb_, loc);

        auto parallel = create<parallel_inst>(loc);

        auto c_zero = bb_.constant_zero(index_ty, loc);
        tinytc_region_t parent_region = &parallel->child_region(0);
        auto offsets = std::vector<tinytc_value_t>(bt->dim() - 1, nullptr);
        for (std::int64_t i = bt->dim() - 1; i > 1; --i) {
            auto bb = region_builder{parent_region};
            auto shape_i = bb.create<size_inst>(i, &in.B(), index_ty, loc);
            auto for_i =
                inst{for_inst::create(c_zero, shape_i, nullptr, array_view<tinytc_value_t>{},
                                      array_view<tinytc_type_t>{}, loc)};
            auto for_i_view = for_inst(for_i.get());
            offsets[i - 1] = &for_i_view.body().param(0);
            parent_region = &for_i_view.body();
            bb.add(std::move(for_i));
        }

        auto bb = region_builder{parent_region};
        auto sgid = bb.create<subgroup_linear_id_inst>(i32_ty, loc);
        auto sgid_index = bb.create<cast_inst>(sgid, index_ty, loc);

        auto shape0 = bb.create<size_inst>(0, &in.B(), index_ty, loc);
        auto shape1 = bb.create<size_inst>(1, &in.B(), index_ty, loc);
        auto c_num_tiles = bb.create<constant_inst>(num_tiles, index_ty, loc);
        bb.for_loop(
            sgid_index, shape1, c_num_tiles,
            [&](region_builder &bb, array_view<tinytc_value_t> args) {
                auto static_offset = std::vector<std::int64_t>(bt->dim(), dynamic);
                auto static_size = std::vector<std::int64_t>(bt->dim(), 0);
                static_offset[0] = 0;
                static_size[0] = dynamic;
                auto a_sub_ty = get<memref_type>(at->element_ty(), array_view{dynamic},
                                                 array_view{at->stride(0)}, at->addrspace());
                auto b_sub_ty = get<memref_type>(bt->element_ty(), array_view{dynamic},
                                                 array_view{bt->stride(0)}, bt->addrspace());
                offsets[0] = args[0];
                auto a_sub = bb.create<subview_inst>(static_offset, static_size, &in.A(), offsets,
                                                     array_view{shape0}, a_sub_ty, loc);
                auto b_sub = bb.create<subview_inst>(static_offset, static_size, &in.B(), offsets,
                                                     array_view{shape0}, b_sub_ty, loc);
                scan_loop_1d(bb, scan, a_sub, b_sub);
            });

        bb_.add(std::move(parallel));
        scan.teardown(bb_);
    } else if (bt->dim() >= 2) {
        auto c_zero = bb_.constant_zero(index_ty, loc);
        auto lb = std::vector<tinytc_value_t>(bt->dim() - 1, c_zero);
        auto ub = std::vector<tinytc_value_t>{};
        ub.reserve(bt->dim() - 1);
        for (std::int64_t i = 0; i < bt->dim(); ++i) {
            if (i != in.mode()) {
                ub.emplace_back(
                    instant_constant_fold_add(bb_, create<size_inst>(i, &in.B(), index_ty, loc)));
            }
        }

        auto J = bb_.create<size_inst>(in.mode(), &in.B(), index_ty, loc);
        bb_.foreach_loop(
            lb, ub,
            [&](region_builder &bb, auto loop_vars) {
                auto static_offset = std::vector<std::int64_t>(bt->dim(), dynamic);
                auto static_size = std::vector<std::int64_t>(bt->dim(), 0);
                static_offset[in.mode()] = 0;
                static_size[in.mode()] = dynamic;
                auto a_sub_ty =
                    get<memref_type>(at->element_ty(), array_view{dynamic},
                                     array_view{at->stride(in.mode())}, at->addrspace());
                auto a_sub = bb.create<subview_inst>(static_offset, static_size, &in.A(), loop_vars,
                                                     array_view{J}, a_sub_ty, loc);
                auto b_sub_ty =
                    get<memref_type>(bt->element_ty(), array_view{dynamic},
                                     array_view{bt->stride(in.mode())}, bt->addrspace());
                auto b_sub = bb.create<subview_inst>(static_offset, static_size, &in.B(), loop_vars,
                                                     array_view{J}, b_sub_ty, loc);

                auto c_init = bb.constant_zero(bt->element_ty());
                auto acc = bb.for_loop(c_zero, J, {}, {c_init}, {bt->element_ty()},
                                       [&](region_builder &bb, array_view<tinytc_value_t> args) {
                                           auto a = bb.create<load_inst>(a_sub, array_view{args[0]},
                                                                         at->element_ty(), loc);
                                           auto prefix = mixed_precision_arithmetic<add_inst>(
                                               bb, bt->element_ty(), args[1], a, loc);
                                           blas_update(bb, in.atomic(), &in.alpha(), prefix,
                                                       &in.beta(), b_sub, {args[0]}, loc);
                                           bb.create<yield_inst>(array_view{prefix}, loc);
                                       });
            },
            loc);
    }
}

void linalg_generator::operator()(gemm_inst in) {
    auto parallel = create<parallel_inst>(in.loc());
    tinytc_region_t body = &parallel->child_region(0);
    auto bb = region_builder{body};

    auto at = get_memref_type(in.A());
    auto bt = get_memref_type(in.B());
    auto ct = get_memref_type(in.C());

    auto ctx = in.alpha().context();
    auto i32_ty = get<i32_type>(ctx);
    auto index_ty = get<index_type>(ctx);

    auto sg_m = bb.create<subgroup_id_inst>(comp3::x, i32_ty, in.loc());
    auto sg_n = bb.create<subgroup_id_inst>(comp3::y, i32_ty, in.loc());

    auto [max_rows, max_cols] = max_register_block_gemm(
        size(at->element_ty()), size(bt->element_ty()), size(acc_type(ct->element_ty())),
        core_cfg_.subgroup_size, core_cfg_.register_space,
        isa<complex_type>(*ct->element_ty()) ? 2 : 1);

    auto c_shape0 =
        instant_constant_fold_add(bb, create<size_inst>(0, &in.C(), index_ty, in.loc()));
    auto c_shape1 =
        instant_constant_fold_add(bb, create<size_inst>(1, &in.C(), index_ty, in.loc()));
    auto K = instant_constant_fold_add(
        bb, create<size_inst>(in.tA() == transpose::T ? 0 : 1, &in.A(), index_ty, in.loc()));

    auto const_shape0 = get_int_constant(c_shape0);
    auto const_shape1 = get_int_constant(c_shape1);

    const auto [block_size0, num_blocks0, block_size1, num_blocks1, do_tile_uniformly,
                K_block_sizes] =
        [&]() -> std::tuple<std::int32_t, std::int32_t, std::int32_t, std::int32_t, bool,
                            std::vector<std::int32_t>> {
        if (auto ext_type = core_cfg_.matrix->get_precision(at->element_ty()->type_id(),
                                                            bt->element_ty()->type_id(),
                                                            ct->element_ty()->type_id());
            ext_type) {
            const auto M_bs = ext_type->M_block_sizes();
            // @todo Think about what do if we have multiple sizes for M
            const auto block_size0 = M_bs.back();
            const auto shape0 = const_shape0 ? *const_shape0 : max_rows;
            const auto num_blocks0 =
                choose_block_size_multiple(block_size0, max_rows, tiling_.m_tiles(), shape0);

            // @todo Think about what do for multiple N sizes
            const auto N_bs = ext_type->N_block_sizes(block_size0);
            const auto block_size1 = N_bs.back();
            const auto shape1 = const_shape1 ? *const_shape1 : max_cols;
            const auto num_blocks1 =
                choose_block_size_multiple(block_size1, max_cols, tiling_.n_tiles(), shape1);
            const auto K_bs = ext_type->K_block_sizes(block_size0, block_size1);

            return std::make_tuple(block_size0, num_blocks0, block_size1, num_blocks1, false, K_bs);
        }

        const auto block_size0 = core_cfg_.subgroup_size;
        const auto shape0 = const_shape0 ? *const_shape0 : max_rows;
        const auto num_blocks0 =
            choose_block_size_multiple(block_size0, max_rows, tiling_.m_tiles(), shape0);
        const auto block_size1 = max_cols;
        const auto num_blocks1 = 1;

        return std::make_tuple(block_size0, num_blocks0, block_size1, num_blocks1,
                               const_shape1.has_value(),
                               std::vector<std::int32_t>(standard_K_block_sizes.begin(),
                                                         standard_K_block_sizes.end()));
    }();

    if (do_tile_uniformly) {
        tile_loop_uniformly(
            bb, c_shape1, block_size1 * num_blocks1, tiling_.n_tiles(), sg_n,
            [&](region_builder &bb, tinytc_value_t n_block, tinytc_value_t trip_count) {
                auto const_trip_count = get_int_constant(trip_count);
                if (!const_trip_count) {
                    throw compilation_error(in.loc(), status::internal_compiler_error);
                }
                tile_loop_by_sgs(
                    bb, c_shape0, block_size0, tiling_.m_tiles(), sg_m,
                    [&](region_builder &bb, tinytc_value_t m_block, bool m_check, tinytc_value_t) {
                        gemm_microkernel(bb, in.tA(), in.tB(), in.atomic(), &in.alpha(), &in.A(),
                                         &in.B(), &in.beta(), &in.C(), K, m_block, block_size0,
                                         num_blocks0, m_check, n_block, *const_trip_count,
                                         num_blocks1, false, K_block_sizes, at->element_ty(),
                                         bt->element_ty(), ct->element_ty(), nullptr, in.loc());
                    });
            });
    } else {
        auto no_unroll = get_dictionary_attr_with_sorted(
            ctx, named_attr{get_string_attr(ctx, "unroll"), get_boolean_attr(ctx, false)});
        tile_loop_by_sgs(
            bb, c_shape1, block_size1 * num_blocks1, tiling_.n_tiles(), sg_n,
            [&](region_builder &bb, tinytc_value_t n_block, bool n_check, tinytc_value_t) {
                tile_loop_by_sgs(
                    bb, c_shape0, block_size0 * num_blocks0, tiling_.m_tiles(), sg_m,
                    [&](region_builder &bb, tinytc_value_t m_block, bool m_check, tinytc_value_t) {
                        gemm_microkernel(bb, in.tA(), in.tB(), in.atomic(), &in.alpha(), &in.A(),
                                         &in.B(), &in.beta(), &in.C(), K, m_block, block_size0,
                                         num_blocks0, m_check, n_block, block_size1, num_blocks1,
                                         n_check, K_block_sizes, at->element_ty(), bt->element_ty(),
                                         ct->element_ty(), no_unroll, in.loc());
                    },
                    no_unroll);
            },
            no_unroll);
    }

    bb_.add(std::move(parallel));
}

void linalg_generator::operator()(gemv_inst in) {
    auto index_ty = index_type::get(in.alpha().context());
    auto c0 = bb_.constant_zero(index_ty, in.loc());
    auto c_shape0 =
        instant_constant_fold_add(bb_, create<size_inst>(0, &in.C(), index_ty, in.loc()));
    auto ct = get_memref_type(in.C());
    bb_.foreach_loop(
        {c0}, {c_shape0},
        [&](region_builder &bb, auto loop_vars) {
            auto c_init = bb.constant_zero(ct->element_ty());
            auto K =
                bb.create<size_inst>(in.tA() == transpose::T ? 0 : 1, &in.A(), index_ty, in.loc());
            auto c_acc = bb.for_loop(
                c0, K, {}, {c_init}, {ct->element_ty()},
                [&](region_builder &bb, array_view<tinytc_value_t> p) {
                    auto a_idx = std::array<tinytc_value_t, 2u>{loop_vars[0], p[0]};
                    if (in.tA() == transpose::T) {
                        std::swap(a_idx[0], a_idx[1]);
                    }
                    auto at = get_memref_type(in.A());
                    auto bt = get_memref_type(in.B());
                    auto a = bb.create<load_inst>(&in.A(), a_idx, at->element_ty(), in.loc());
                    auto b =
                        bb.create<load_inst>(&in.B(), array_view{p[0]}, bt->element_ty(), in.loc());
                    auto ab =
                        mixed_precision_arithmetic<mul_inst>(bb, ct->element_ty(), a, b, in.loc());
                    auto ab_c = mixed_precision_arithmetic<add_inst>(bb, ct->element_ty(), p[1], ab,
                                                                     in.loc());
                    bb.create<yield_inst>(array_view{ab_c}, in.loc());
                });
            blas_update(bb, in.atomic(), &in.alpha(), c_acc[0], &in.beta(), &in.C(), {loop_vars[0]},
                        in.loc());
        },
        in.loc());
}

void linalg_generator::operator()(ger_inst in) {
    auto index_ty = index_type::get(in.alpha().context());
    auto c0 = bb_.constant_zero(index_ty, in.loc());
    auto c_shape0 =
        instant_constant_fold_add(bb_, create<size_inst>(0, &in.C(), index_ty, in.loc()));
    auto c_shape1 =
        instant_constant_fold_add(bb_, create<size_inst>(1, &in.C(), index_ty, in.loc()));
    bb_.foreach_loop(
        {c0, c0}, {c_shape0, c_shape1},
        [&](region_builder &bb, auto loop_vars) {
            auto at = get_memref_type(in.A());
            auto bt = get_memref_type(in.B());
            auto ct = get_memref_type(in.C());
            auto a =
                bb.create<load_inst>(&in.A(), array_view{loop_vars[0]}, at->element_ty(), in.loc());
            auto b =
                bb.create<load_inst>(&in.B(), array_view{loop_vars[1]}, bt->element_ty(), in.loc());
            auto ab = mixed_precision_arithmetic<mul_inst>(bb, ct->element_ty(), a, b, in.loc());
            blas_update(bb, in.atomic(), &in.alpha(), ab, &in.beta(), &in.C(),
                        {loop_vars[0], loop_vars[1]}, in.loc());
        },
        in.loc());
}

void linalg_generator::operator()(hadamard_inst in) {
    auto index_ty = index_type::get(in.alpha().context());
    auto at = get_memref_type(in.A());
    auto bt = get_memref_type(in.B());
    auto ct = get_memref_type(in.C());

    auto lb = std::vector<tinytc_value_t>(ct->dim());
    auto ub = std::vector<tinytc_value_t>(ct->dim());

    auto c0 = bb_.constant_zero(index_ty, in.loc());
    for (std::int64_t i = 0; i < ct->dim(); ++i) {
        lb[i] = c0;
        ub[i] = instant_constant_fold_add(bb_, create<size_inst>(i, &in.C(), index_ty, in.loc()));
    }

    bb_.foreach_loop(
        lb, ub,
        [&](region_builder &bb, auto loop_vars) {
            auto a = bb.create<load_inst>(&in.A(), loop_vars, at->element_ty(), in.loc());
            auto b = bb.create<load_inst>(&in.B(), loop_vars, bt->element_ty(), in.loc());
            auto ab = mixed_precision_arithmetic<mul_inst>(bb, ct->element_ty(), a, b, in.loc());
            blas_update(bb, in.atomic(), &in.alpha(), ab, &in.beta(), &in.C(), loop_vars, in.loc());
        },
        in.loc());
}

void linalg_generator::operator()(sum_inst in) {
    auto at = get_memref_type(in.A());
    auto bt = get_memref_type(in.B());

    auto ctx = in.alpha().context();
    auto bool_ty = get<boolean_type>(ctx);
    auto i32_ty = get<i32_type>(ctx);
    auto index_ty = get<index_type>(ctx);

    if (bt->dim() == 0) {
        const auto num_tiles = tiling_.m_tiles() * tiling_.n_tiles();
        auto reducer = work_group_reduce(num_tiles, core_cfg_.subgroup_size, bt->element_ty());
        reducer.setup(bb_, in.loc());

        auto parallel = create<parallel_inst>(in.loc());
        tinytc_region_t body = &parallel->child_region(0);
        auto bb = region_builder{body};

        auto c_sgs = bb.create<constant_inst>(core_cfg_.subgroup_size, i32_ty, in.loc());
        auto sgid = bb.create<subgroup_linear_id_inst>(i32_ty, in.loc());
        auto m = bb.create<subgroup_local_id_inst>(i32_ty, in.loc());
        auto from0 = bb.create<mul_inst>(sgid, c_sgs, i32_ty, in.loc());
        auto from1 = bb.create<add_inst>(from0, m, i32_ty, in.loc());
        auto from_index = bb.create<cast_inst>(from1, index_ty, in.loc());

        auto c_trip_count =
            instant_constant_fold_add(bb, create<size_inst>(0, &in.A(), index_ty, in.loc()));
        auto c_step =
            bb.create<constant_inst>(core_cfg_.subgroup_size * num_tiles, index_ty, in.loc());
        auto c_init = bb.constant_zero(bt->element_ty(), in.loc());

        auto acc = bb.for_loop(from_index, c_trip_count, c_step, {c_init}, {bt->element_ty()},
                               [&](region_builder &bb, array_view<tinytc_value_t> args) {
                                   auto a = bb.create<load_inst>(&in.A(), array_view{args[0]},
                                                                 at->element_ty(), in.loc());
                                   auto sum = mixed_precision_arithmetic<add_inst>(
                                       bb, bt->element_ty(), args[1], a, in.loc());
                                   bb.create<yield_inst>(array_view{sum}, in.loc());
                               });
        auto acc_reduced = reducer.make(bb, acc[0], in.loc());

        auto c_zero = bb.constant_zero(i32_ty, in.loc());
        auto is_first_work_item = bb.create<equal_inst>(from1, c_zero, bool_ty, in.loc());
        bb.if_condition(
            is_first_work_item,
            [&](region_builder &bb) {
                blas_update(bb, in.atomic(), &in.alpha(), acc_reduced, &in.beta(), &in.B(), {},
                            in.loc());
            },
            in.loc());

        bb_.add(std::move(parallel));
        reducer.teardown(bb_);
    } else if (bt->dim() == 1) {
        auto c0 = bb_.constant_zero(index_ty, in.loc());
        auto c_shape0 =
            instant_constant_fold_add(bb_, create<size_inst>(0, &in.B(), index_ty, in.loc()));
        bb_.foreach_loop(
            array_view<tinytc_value_t>{c0}, array_view<tinytc_value_t>{c_shape0},
            [&](region_builder &bb, auto loop_vars) {
                auto K = bb.create<size_inst>(in.tA() == transpose::T ? 0 : 1, &in.A(), index_ty,
                                              in.loc());
                auto c_init = bb.constant_zero(bt->element_ty());
                auto acc = bb.for_loop(
                    c0, K, {}, {c_init}, {bt->element_ty()},
                    [&](region_builder &bb, array_view<tinytc_value_t> args) {
                        auto index_list = std::array<tinytc_value_t, 2u>{loop_vars[0], args[0]};
                        if (in.tA() == transpose::T) {
                            std::swap(index_list[0], index_list[1]);
                        }
                        auto a =
                            bb.create<load_inst>(&in.A(), index_list, at->element_ty(), in.loc());
                        auto sum = mixed_precision_arithmetic<add_inst>(bb, bt->element_ty(),
                                                                        args[1], a, in.loc());
                        bb.create<yield_inst>(array_view{sum}, in.loc());
                    });
                blas_update(bb, in.atomic(), &in.alpha(), acc[0], &in.beta(), &in.B(),
                            {loop_vars[0]}, in.loc());
            },
            in.loc());
    }
}

lower_linalg_pass::lower_linalg_pass(::tinytc_core_info const *info) : info_(std::move(info)) {
    if (info_ == nullptr) {
        throw std::invalid_argument("info must not be nullptr");
    }
}

void lower_linalg_pass::run_on_function(tinytc_func &fn) {
    auto [core_cfg, tiling] = get_core_config_and_tiling(fn, info_);

    walk<walk_order::post_order>(fn, [&](tinytc_region &reg) {
        auto it = reg.begin();
        while (it != reg.end()) {
            if (isa<blas_a2_inst>(*it) || isa<blas_a3_inst>(*it)) {
                auto gen = linalg_generator{tiling, core_cfg, reg, it.get()};
                visit(gen, *it);
                it = reg.insts().erase(gen.insertion_point());
            } else {
                ++it;
            }
        }
    });
}

} // namespace tinytc
