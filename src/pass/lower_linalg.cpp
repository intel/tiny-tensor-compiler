// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/lower_linalg.hpp"
#include "codegen_tools.hpp"
#include "device_info.hpp"
#include "error.hpp"
#include "gemm_tools.hpp"
#include "node/data_type_node.hpp"
#include "node/function_node.hpp"
#include "node/inst_node.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "scalar_type.hpp"
#include "support/casting.hpp"
#include "support/ilist.hpp"
#include "support/ilist_base.hpp"
#include "support/visit.hpp"
#include "support/walk.hpp"
#include "tiling.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace tinytc {

void gemm_microkernel(region_builder &bb, transpose tA, transpose tB, bool atomic, value alpha,
                      value A, value B, value beta, value C, value K, value m_block,
                      std::int64_t m_block_size, bool m_check, value n_block,
                      std::int64_t n_block_size, bool n_check, data_type a_ty, data_type b_ty,
                      data_type c_ty, location const &loc) {
    auto ctx = m_block->context();
    auto index_ty = scalar_data_type::get(ctx, scalar_type::index);

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
    auto coopmatrix_c_ty = get_coopmatrix(c_ty, m_block_size, n_block_size, matrix_use::acc, loc);
    auto const compute_c = [&](region_builder &bb, std::int32_t k_block_size, value K0, value K1,
                               value c_init) -> value {
        auto c_step = bb.add(make_constant(k_block_size, index_ty, loc));
        auto return_values = bb.for_loop(
            K0, K1, c_step, {c_init}, index_ty, [&](region_builder &bb, array_view<value> p) {
                const auto k = p[0];

                value pos_a[2] = {m_block, k};
                if (tA == transpose::T) {
                    std::swap(pos_a[0], pos_a[1]);
                }
                auto coopmatrix_a_ty =
                    get_coopmatrix(a_ty, m_block_size, k_block_size, matrix_use::a, loc);
                auto a = bb.add(make_cooperative_matrix_load(tA, check_a, A, pos_a[0], pos_a[1],
                                                             coopmatrix_a_ty));

                value pos_b[2] = {k, n_block};
                if (tB == transpose::T) {
                    std::swap(pos_b[0], pos_b[1]);
                }
                auto coopmatrix_b_ty =
                    get_coopmatrix(b_ty, k_block_size, n_block_size, matrix_use::b, loc);
                auto b = bb.add(make_cooperative_matrix_load(tB, check_b, B, pos_b[0], pos_b[1],
                                                             coopmatrix_b_ty));
                auto c_next =
                    bb.add(make_cooperative_matrix_mul_add(a, b, p[1], coopmatrix_c_ty, loc));
                bb.add(make_yield(c_next, loc));
            });
        return return_values[0];
    };

    auto c_init = bb.add(make_constant_zero(coopmatrix_c_ty, loc));

    auto k_block_size = max_K_unrolling;

    const auto const_K = get_int_constant(K);
    if (const_K) {
        k_block_size = compute_k_block_size(*const_K);
    }

    auto c_zero = bb.add(make_constant_zero(index_ty, loc));
    auto c_k_block_size = bb.add(make_constant(k_block_size, index_ty, loc));
    auto tmp = instant_constant_fold_add(bb, make_arith(arithmetic::div, K, c_k_block_size, loc));
    auto K0 = instant_constant_fold_add(bb, make_arith(arithmetic::mul, tmp, c_k_block_size, loc));
    c_init = compute_c(bb, k_block_size, c_zero, K0, c_init);
    auto needs_remainder = instant_constant_fold_add(bb, make_cmp(cmp_condition::lt, K0, K, loc));
    auto r = get_bool_constant(needs_remainder);
    if (r) {
        if (*r != 0) {
            c_init = compute_c(bb, 1, K0, K, c_init);
        }
    } else {
        auto remainder = bb.if_condition(
            needs_remainder,
            [&](region_builder &bb) {
                auto c_next = compute_c(bb, 1, K0, K, c_init);
                bb.add(make_yield(c_next, loc));
            },
            {coopmatrix_c_ty}, loc);
        c_init = remainder[0];
    }

    auto alpha_ab = mixed_precision_coopmatrix_scale(bb, alpha, c_init, loc);
    if (atomic) {
        auto flag = get_atomic_store_flag(beta);
        if (!flag) {
            throw compilation_error(loc, status::ir_invalid_beta);
        }
        bb.add(make_cooperative_matrix_store(check_c, *flag, alpha_ab, C, m_block, n_block, loc));
    } else {
        auto c_load = bb.add(make_cooperative_matrix_load(transpose::N, check_c, C, m_block,
                                                          n_block, coopmatrix_c_ty));
        auto beta_c = mixed_precision_coopmatrix_scale(bb, beta, c_load, loc);
        auto alpha_ab_plus_beta_c = bb.add(make_arith(arithmetic::add, alpha_ab, beta_c, loc));
        bb.add(make_cooperative_matrix_store(check_c, store_flag::regular, alpha_ab_plus_beta_c, C,
                                             m_block, n_block, loc));
    }
}

class linalg_generator {
  public:
    linalg_generator(local_tiling tiling, core_config core_cfg)
        : tiling_{std::move(tiling)}, core_cfg_{std::move(core_cfg)} {}
    auto operator()(inst_node &) -> inst { return inst{}; }
    auto operator()(axpby_inst &in) -> inst;
    auto operator()(ger_inst &in) -> inst;
    auto operator()(gemm_inst &in) -> inst;
    auto operator()(gemv_inst &in) -> inst;
    auto operator()(hadamard_inst &in) -> inst;
    auto operator()(sum_inst &in) -> inst;

  private:
    auto get_memref_type(value_node const &v) const -> const memref_data_type *;

    local_tiling tiling_ = {};
    core_config core_cfg_ = {};
};

auto linalg_generator::get_memref_type(value_node const &v) const -> const memref_data_type * {
    auto t = dyn_cast<memref_data_type>(v.ty());
    if (t == nullptr) {
        throw compilation_error(v.loc(), status::ir_expected_memref);
    }
    return t;
}

auto linalg_generator::operator()(axpby_inst &in) -> inst {
    auto parallel = make_parallel(in.loc());
    tinytc_region_t body = &parallel->child_region(0);
    auto bb = region_builder{body};

    auto ctx = compiler_context{in.alpha().context(), true};
    auto index_ty = get_scalar(ctx, scalar_type::index);
    auto i32_ty = get_scalar(ctx, scalar_type::i32);

    auto bt = get_memref_type(in.B());

    auto sgid = bb.add(make_subgroup_id(ctx, in.loc()));

    auto const inner_loop = [&](region_builder &bb, value Ab, value Bb, value trip_count,
                                int num_tiles, value sgid) {
        tile_loop_by_sgs_standard(bb, trip_count, core_cfg_.subgroup_size, num_tiles, sgid,
                                  [&](region_builder &bb, value mm) {
                                      auto a = bb.add(make_load(Ab, {mm}, in.loc()));
                                      blas_update(bb, in.atomic(), &in.alpha(), a, &in.beta(), Bb,
                                                  {mm}, in.loc());
                                  });
    };

    if (bt->dim() == 0) {
        auto m = bb.add(make_subgroup_local_id(ctx, in.loc()));
        auto c0 = bb.add(make_constant(0, i32_ty));
        auto cond0 = bb.add(make_cmp(cmp_condition::eq, sgid, c0));
        auto cond1 = bb.add(make_cmp(cmp_condition::eq, m, c0));
        auto cond = bb.add(make_arith(arithmetic::and_, cond0, cond1));
        bb.if_condition(cond, [&](region_builder &bb) {
            auto a = bb.add(make_load(&in.A(), {}, in.loc()));
            blas_update(bb, in.atomic(), &in.alpha(), a, &in.beta(), &in.B(), {}, in.loc());
        });
    } else if (bt->dim() == 1) {
        auto c_shape0 = instant_constant_fold_add(bb, make_size(&in.B(), 0, in.loc()));
        inner_loop(bb, &in.A(), &in.B(), c_shape0, tiling_.m_tiles() * tiling_.n_tiles(), sgid);
    } else if (bt->dim() == 2) {
        auto c_m_tiles = bb.add(make_constant(tiling_.m_tiles(), i32_ty, in.loc()));
        auto sg_n = bb.add(make_arith(arithmetic::div, sgid, c_m_tiles, in.loc()));
        auto sg_m = bb.add(make_arith(arithmetic::rem, sgid, c_m_tiles, in.loc()));

        auto c_shape0 = instant_constant_fold_add(bb, make_size(&in.B(), 0, in.loc()));
        auto c_shape1 = instant_constant_fold_add(bb, make_size(&in.B(), 1, in.loc()));
        tile_loop_uniformly_new(
            bb, c_shape1, core_cfg_.subgroup_size, tiling_.n_tiles(), sg_n,
            [&](region_builder &bb, value block, value trip_count) {
                auto zero = bb.add(make_constant(0, index_ty));
                bb.for_loop(zero, trip_count, index_ty, [&](region_builder &bb, value n) {
                    auto nn = bb.add(make_arith(arithmetic::add, block, n, in.loc()));
                    auto static_offset_list = std::array<std::int64_t, 2u>{0, dynamic};
                    auto static_size_list = std::array<std::int64_t, 2u>{dynamic, 0};
                    auto Bb = bb.add(make_subview(&in.B(), static_offset_list, static_size_list,
                                                  {nn}, {c_shape0}, in.loc()));
                    if (in.tA() == transpose::T) {
                        std::swap(static_offset_list[0], static_offset_list[1]);
                        std::swap(static_size_list[0], static_size_list[1]);
                    }
                    auto Ab = bb.add(make_subview(&in.A(), static_offset_list, static_size_list,
                                                  {nn}, {c_shape0}, in.loc()));
                    inner_loop(bb, Ab, Bb, c_shape0, tiling_.m_tiles(), sg_m);
                });
            });
    }

    return parallel;
}

auto linalg_generator::operator()(ger_inst &in) -> inst {
    auto parallel = make_parallel(in.loc());
    tinytc_region_t body = &parallel->child_region(0);
    auto bb = region_builder{body};

    auto ctx = compiler_context{in.alpha().context(), true};
    auto i32_ty = get_scalar(ctx, scalar_type::i32);
    auto index_ty = get_scalar(ctx, scalar_type::index);

    auto sgid = bb.add(make_subgroup_id(ctx, in.loc()));
    auto c_m_tiles = bb.add(make_constant(tiling_.m_tiles(), i32_ty, in.loc()));
    auto sg_n = bb.add(make_arith(arithmetic::div, sgid, c_m_tiles, in.loc()));
    auto sg_m = bb.add(make_arith(arithmetic::rem, sgid, c_m_tiles, in.loc()));

    auto c_shape0 = instant_constant_fold_add(bb, make_size(&in.C(), 0, in.loc()));
    auto c_shape1 = instant_constant_fold_add(bb, make_size(&in.C(), 1, in.loc()));
    tile_loop_uniformly_new(
        bb, c_shape1, core_cfg_.subgroup_size, tiling_.n_tiles(), sg_n,
        [&](region_builder &bb, value block, value trip_count) {
            auto zero = bb.add(make_constant(0, index_ty));
            bb.for_loop(zero, trip_count, index_ty, [&](region_builder &bb, value n) {
                auto nn = bb.add(make_arith(arithmetic::add, block, n, in.loc()));
                auto b = bb.add(make_load(&in.B(), {nn}, in.loc()));
                tile_loop_by_sgs_standard(bb, c_shape0, core_cfg_.subgroup_size, tiling_.m_tiles(),
                                          sg_m, [&](region_builder &bb, value mm) {
                                              auto a = bb.add(make_load(&in.A(), {mm}, in.loc()));
                                              auto ab = mixed_precision_arithmetic(
                                                  bb, arithmetic::mul, a, b, in.loc());
                                              blas_update(bb, in.atomic(), &in.alpha(), ab,
                                                          &in.beta(), &in.C(), {mm, nn}, in.loc());
                                          });
            });
        });

    return parallel;
}

auto linalg_generator::operator()(gemm_inst &in) -> inst {
    auto parallel = make_parallel(in.loc());
    tinytc_region_t body = &parallel->child_region(0);
    auto bb = region_builder{body};

    auto at = get_memref_type(in.A());
    auto bt = get_memref_type(in.B());
    auto ct = get_memref_type(in.C());

    auto ctx = compiler_context{in.alpha().context(), true};
    auto i32_ty = get_scalar(ctx, scalar_type::i32);

    auto sgid = bb.add(make_subgroup_id(ctx, in.loc()));
    auto c_m_tiles = bb.add(make_constant(tiling_.m_tiles(), i32_ty, in.loc()));
    auto sg_n = bb.add(make_arith(arithmetic::div, sgid, c_m_tiles, in.loc()));
    auto sg_m = bb.add(make_arith(arithmetic::rem, sgid, c_m_tiles, in.loc()));

    auto [max_rows, max_cols] = max_register_block_gemm(
        size(ct->element_ty()), core_cfg_.subgroup_size, core_cfg_.register_space,
        is_complex_type(ct->element_ty()) ? 2 : 1);

    auto c_shape0 = instant_constant_fold_add(bb, make_size(&in.C(), 0, in.loc()));
    auto c_shape1 = instant_constant_fold_add(bb, make_size(&in.C(), 1, in.loc()));
    auto K = instant_constant_fold_add(
        bb, make_size(&in.A(), in.tA() == transpose::T ? 0 : 1, in.loc()));

    auto const_shape0 = get_int_constant(c_shape0);
    auto const_shape1 = get_int_constant(c_shape1);

    const auto block_size0 = const_shape0 ? compute_m_block_size(core_cfg_.subgroup_size, max_rows,
                                                                 tiling_.m_tiles(), *const_shape0)
                                          : max_rows;
    const auto block_size1 = max_cols;

    if (const_shape1) {
        tile_loop_uniformly_new(
            bb, c_shape1, block_size1, tiling_.n_tiles(), sg_n,
            [&](region_builder &bb, value n_block, value trip_count) {
                auto const_trip_count = get_int_constant(trip_count);
                if (!const_trip_count) {
                    throw compilation_error(in.loc(), status::internal_compiler_error);
                }
                tile_loop_by_sgs_new(bb, c_shape0, block_size0, tiling_.m_tiles(), sg_m,
                                     [&](region_builder &bb, value m_block, bool m_check, value) {
                                         gemm_microkernel(
                                             bb, in.tA(), in.tB(), in.atomic(), &in.alpha(),
                                             &in.A(), &in.B(), &in.beta(), &in.C(), K, m_block,
                                             block_size0, m_check, n_block, *const_trip_count,
                                             false, at->element_data_ty(), bt->element_data_ty(),
                                             ct->element_data_ty(), in.loc());
                                     });
            });
    } else {
        tile_loop_by_sgs_new(bb, c_shape1, block_size1, tiling_.n_tiles(), sg_n,
                             [&](region_builder &bb, value n_block, bool n_check, value) {
                                 tile_loop_by_sgs_new(
                                     bb, c_shape0, block_size0, tiling_.m_tiles(), sg_m,
                                     [&](region_builder &bb, value m_block, bool m_check, value) {
                                         gemm_microkernel(
                                             bb, in.tA(), in.tB(), in.atomic(), &in.alpha(),
                                             &in.A(), &in.B(), &in.beta(), &in.C(), K, m_block,
                                             block_size0, m_check, n_block, block_size1, n_check,
                                             at->element_data_ty(), bt->element_data_ty(),
                                             ct->element_data_ty(), in.loc());
                                     });
                             });
    }

    return parallel;
}

auto linalg_generator::operator()(gemv_inst &in) -> inst {
    auto parallel = make_parallel(in.loc());
    tinytc_region_t body = &parallel->child_region(0);
    auto bb = region_builder{body};

    auto ct = get_memref_type(in.C());

    auto ctx = compiler_context{in.alpha().context(), true};
    auto index_ty = get_scalar(ctx, scalar_type::index);

    auto sgid = bb.add(make_subgroup_id(ctx, in.loc()));

    auto c_shape0 = instant_constant_fold_add(bb, make_size(&in.C(), 0, in.loc()));
    auto K = instant_constant_fold_add(
        bb, make_size(&in.A(), in.tA() == transpose::T ? 0 : 1, in.loc()));

    tile_loop_by_sgs_standard(
        bb, c_shape0, core_cfg_.subgroup_size, tiling_.m_tiles() * tiling_.n_tiles(), sgid,
        [&](region_builder &bb, value mm) {
            auto c_zero = bb.add(make_constant(0, index_ty));
            auto c_step = bb.add(make_constant(1, index_ty));
            auto c_init = bb.add(make_constant_zero(ct->element_data_ty()));
            auto c_acc = bb.for_loop(
                c_zero, K, c_step, {c_init}, index_ty,
                [&](region_builder &bb, array_view<value> p) {
                    auto a_idx = std::array<value, 2u>{mm, p[0]};
                    if (in.tA() == transpose::T) {
                        std::swap(a_idx[0], a_idx[1]);
                    }
                    auto a = bb.add(make_load(&in.A(), a_idx, in.loc()));
                    auto b = bb.add(make_load(&in.B(), {p[0]}, in.loc()));
                    auto ab = mixed_precision_arithmetic(bb, arithmetic::mul, a, b, in.loc());
                    auto ab_c = mixed_precision_arithmetic(bb, arithmetic::add, p[1], ab, in.loc());
                    bb.add(make_yield({ab_c}, in.loc()));
                });
            blas_update(bb, in.atomic(), &in.alpha(), c_acc[0], &in.beta(), &in.C(), {mm},
                        in.loc());
        });

    return parallel;
}

auto linalg_generator::operator()(hadamard_inst &in) -> inst {
    auto parallel = make_parallel(in.loc());
    tinytc_region_t body = &parallel->child_region(0);
    auto bb = region_builder{body};

    auto ctx = compiler_context{in.alpha().context(), true};
    auto sgid = bb.add(make_subgroup_id(ctx, in.loc()));

    auto c_shape0 = instant_constant_fold_add(bb, make_size(&in.C(), 0, in.loc()));
    tile_loop_by_sgs_standard(
        bb, c_shape0, core_cfg_.subgroup_size, tiling_.m_tiles() * tiling_.n_tiles(), sgid,
        [&](region_builder &bb, value mm) {
            auto a = bb.add(make_load(&in.A(), {mm}, in.loc()));
            auto b = bb.add(make_load(&in.B(), {mm}, in.loc()));
            auto ab = mixed_precision_arithmetic(bb, arithmetic::mul, a, b, in.loc());
            blas_update(bb, in.atomic(), &in.alpha(), ab, &in.beta(), &in.C(), {mm}, in.loc());
        });

    return parallel;
}

auto linalg_generator::operator()(sum_inst &in) -> inst {
    auto parallel = make_parallel(in.loc());
    tinytc_region_t body = &parallel->child_region(0);
    auto bb = region_builder{body};

    auto ctx = compiler_context{in.alpha().context(), true};
    auto i32_ty = get_scalar(ctx, scalar_type::i32);
    auto index_ty = get_scalar(ctx, scalar_type::index);

    auto bt = get_memref_type(in.B());

    auto sgid = bb.add(make_subgroup_id(ctx, in.loc()));

    if (bt->dim() == 0) {
        auto c_sgs = bb.add(make_constant(core_cfg_.subgroup_size, i32_ty, in.loc()));
        auto sgid = bb.add(make_subgroup_id(ctx, in.loc()));
        auto m = bb.add(make_subgroup_local_id(ctx, in.loc()));
        auto from0 = bb.add(make_arith(arithmetic::mul, sgid, c_sgs, in.loc()));
        auto from1 = bb.add(make_arith(arithmetic::add, from0, m, in.loc()));
        auto from_index = bb.add(make_cast(from1, index_ty, in.loc()));

        auto c_zero = bb.add(make_constant_zero(i32_ty, in.loc()));
        auto is_from_0 = bb.add(make_cmp(cmp_condition::eq, from1, c_zero, in.loc()));

        auto c_trip_count = instant_constant_fold_add(bb, make_size(&in.A(), 0, in.loc()));
        auto c_step = bb.add(make_constant(
            core_cfg_.subgroup_size * tiling_.m_tiles() * tiling_.n_tiles(), index_ty, in.loc()));
        auto c_init = bb.add(make_constant_zero(bt->element_data_ty(), in.loc()));

        auto acc = bb.for_loop(from_index, c_trip_count, c_step, {c_init}, index_ty,
                               [&](region_builder &bb, array_view<value> args) {
                                   auto a = bb.add(make_load(&in.A(), {args[0]}, in.loc()));
                                   auto sum = mixed_precision_arithmetic(bb, arithmetic::add,
                                                                         args[1], a, in.loc());
                                   bb.add(make_yield({sum}, in.loc()));
                               });
        auto sum = bb.add(make_work_group(work_group_operation::reduce_add, acc[0], in.loc()));
        bb.if_condition(
            is_from_0,
            [&](region_builder &bb) {
                blas_update(bb, in.atomic(), &in.alpha(), sum, &in.beta(), &in.B(), {}, in.loc());
            },
            {}, in.loc());
    } else if (bt->dim() == 1) {
        auto c_shape0 = instant_constant_fold_add(bb, make_size(&in.B(), 0, in.loc()));
        auto c_trip_count = instant_constant_fold_add(
            bb, make_size(&in.A(), in.tA() == transpose::T ? 0 : 1, in.loc()));
        tile_loop_by_sgs_standard(
            bb, c_shape0, core_cfg_.subgroup_size, tiling_.m_tiles() * tiling_.n_tiles(), sgid,
            [&](region_builder &bb, value mm) {
                auto from = bb.add(make_constant(0, index_ty));
                auto zero = bb.add(make_constant_zero(bt->element_data_ty()));
                auto acc =
                    bb.for_loop(from, c_trip_count, {}, {zero}, index_ty,
                                [&](region_builder &bb, array_view<value> args) {
                                    auto index_list = std::array<value, 2u>{mm, args[0]};
                                    if (in.tA() == transpose::T) {
                                        std::swap(index_list[0], index_list[1]);
                                    }
                                    auto a = bb.add(make_load(&in.A(), index_list, in.loc()));
                                    auto sum = mixed_precision_arithmetic(bb, arithmetic::add,
                                                                          args[1], a, in.loc());
                                    bb.add(make_yield({sum}, in.loc()));
                                });
                blas_update(bb, in.atomic(), &in.alpha(), acc[0], &in.beta(), &in.B(), {mm},
                            in.loc());
            });
    }
    return parallel;
}

lower_linalg_pass::lower_linalg_pass(::tinytc_core_info const *info) : info_(std::move(info)) {
    if (info_ == nullptr) {
        throw std::invalid_argument("info must not be nullptr");
    }
}

void lower_linalg_pass::run_on_function(function_node &fn) {
    auto const subgroup_size = fn.subgroup_size();
    core_config core_cfg = {};
    try {
        core_cfg = info_->get_core_config(subgroup_size);
    } catch (std::out_of_range const &e) {
        throw compilation_error(fn.loc(), status::unsupported_subgroup_size);
    }
    auto const work_group_size = fn.work_group_size();
    local_tiling tiling = {};
    tiling[0] = work_group_size[0] / subgroup_size;
    tiling[1] = work_group_size[1];

    walk<walk_order::post_order>(fn, [&](region_node &reg) {
        for (auto it = reg.begin(); it != reg.end(); ++it) {
            auto lowered_inst = visit(linalg_generator{tiling, core_cfg}, *it);
            if (lowered_inst) {
                it = reg.insts().erase(it);
                it = reg.insts().insert(it, lowered_inst.release());
            }
        }
    });
}

} // namespace tinytc
