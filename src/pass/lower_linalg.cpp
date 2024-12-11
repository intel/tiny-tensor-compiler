// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/lower_linalg.hpp"
#include "codegen_tools.hpp"
#include "device_info.hpp"
#include "error.hpp"
#include "gemm_tools.hpp"
#include "matrix_ext_info.hpp"
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
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

namespace tinytc {

void gemm_microkernel(region_builder &bb, transpose tA, transpose tB, bool atomic, value alpha,
                      value A, value B, value beta, value C, value K, value m_block,
                      std::int32_t m_block_size, bool m_check, value n_block,
                      std::int32_t n_block_size, bool n_check,
                      array_view<std::int32_t> K_block_sizes, data_type a_ty, data_type b_ty,
                      data_type c_ty, location const &loc) {
    auto ctx = m_block->context();
    auto bool_ty = boolean_data_type::get(ctx);
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

    const auto c_acc_ty = [&c_ty, &loc]() {
        auto ct = dyn_cast<scalar_data_type>(c_ty);
        if (ct == nullptr) {
            throw compilation_error(loc, status::internal_compiler_error);
        }
        if (ct->ty() == scalar_type::bf16 || ct->ty() == scalar_type::f16) {
            return scalar_data_type::get(c_ty->context(), scalar_type::f32);
        }
        return c_ty;
    }();

    auto coopmatrix_c_ty = get_coopmatrix(c_ty, m_block_size, n_block_size, matrix_use::acc, loc);
    auto coopmatrix_c_acc_ty =
        get_coopmatrix(c_acc_ty, m_block_size, n_block_size, matrix_use::acc, loc);
    auto const compute_c = [&](region_builder &bb, std::int32_t k_block_size, value K0, value K1,
                               value c_acc, bool check_k = false) -> value {
        auto c_step = bb.add(make_constant(k_block_size, index_ty, loc));
        auto return_values = bb.for_loop(
            index_ty, K0, K1, c_step, {c_acc}, {c_acc->ty()},
            [&](region_builder &bb, array_view<value> p) {
                const auto k = p[0];

                value pos_a[2] = {m_block, k};
                if (tA == transpose::T) {
                    std::swap(pos_a[0], pos_a[1]);
                }
                auto coopmatrix_a_ty =
                    get_coopmatrix(a_ty, m_block_size, k_block_size, matrix_use::a, loc);
                const auto my_check_a = check_k ? add_check(check_a, checked_flag::cols) : check_a;
                auto a = bb.add(make_cooperative_matrix_load(tA, my_check_a, A, pos_a[0], pos_a[1],
                                                             coopmatrix_a_ty));

                value pos_b[2] = {k, n_block};
                if (tB == transpose::T) {
                    std::swap(pos_b[0], pos_b[1]);
                }
                auto coopmatrix_b_ty =
                    get_coopmatrix(b_ty, k_block_size, n_block_size, matrix_use::b, loc);
                const auto my_check_b = check_k ? add_check(check_b, checked_flag::rows) : check_b;
                auto b = bb.add(make_cooperative_matrix_load(tB, my_check_b, B, pos_b[0], pos_b[1],
                                                             coopmatrix_b_ty));
                auto c_next =
                    bb.add(make_cooperative_matrix_mul_add(a, b, p[1], coopmatrix_c_acc_ty, loc));
                bb.add(make_yield(c_next, loc));
            });
        return return_values[0];
    };

    auto c_acc = bb.add(make_constant_zero(coopmatrix_c_acc_ty, loc));

    auto k_block_size = K_block_sizes.back();

    const auto const_K = get_int_constant(K);
    if (const_K) {
        k_block_size = choose_k_block_size(K_block_sizes, *const_K);
    }

    auto c_zero = bb.add(make_constant_zero(index_ty, loc));
    auto c_k_block_size = bb.add(make_constant(k_block_size, index_ty, loc));
    auto tmp = instant_constant_fold_add(
        bb, make_arith(arithmetic::div, K, c_k_block_size, index_ty, loc));
    auto K0 = instant_constant_fold_add(
        bb, make_arith(arithmetic::mul, tmp, c_k_block_size, index_ty, loc));
    c_acc = compute_c(bb, k_block_size, c_zero, K0, c_acc);
    auto needs_remainder =
        instant_constant_fold_add(bb, make_cmp(cmp_condition::lt, K0, K, bool_ty, loc));
    auto r = get_bool_constant(needs_remainder);
    if (r) {
        if (*r != 0) {
            const auto K_block_size = K_block_sizes.front();
            c_acc = compute_c(bb, K_block_size, K0, K, c_acc, K_block_size > 1);
        }
    } else {
        auto remainder = bb.ifelse(
            needs_remainder,
            [&](region_builder &bb) {
                const auto K_block_size = K_block_sizes.front();
                auto c_next = compute_c(bb, K_block_size, K0, K, c_acc, K_block_size > 1);
                bb.add(make_yield(c_next, loc));
            },
            [&](region_builder &bb) { bb.add(make_yield(c_acc, loc)); }, {coopmatrix_c_acc_ty},
            loc);
        c_acc = remainder[0];
    }

    if (coopmatrix_c_ty != coopmatrix_c_acc_ty) {
        c_acc = bb.add(make_cast(c_acc, coopmatrix_c_ty, loc));
    }
    auto alpha_ab = mixed_precision_coopmatrix_scale(bb, alpha, c_acc, loc);
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
        auto alpha_ab_plus_beta_c =
            bb.add(make_arith(arithmetic::add, alpha_ab, beta_c, alpha_ab->ty(), loc));
        bb.add(make_cooperative_matrix_store(check_c, store_flag::regular, alpha_ab_plus_beta_c, C,
                                             m_block, n_block, loc));
    }
}

class linalg_generator {
  public:
    linalg_generator(local_tiling const &tiling, core_config const &core_cfg, region_node &reg,
                     tinytc_inst_iterator_t ip)
        : tiling_{tiling}, core_cfg_{core_cfg}, bb_{&reg, ip} {}
    inline void operator()(inst_node &in) {
        throw compilation_error(in.loc(), status::not_implemented);
    }
    void operator()(axpby_inst &in);
    void operator()(ger_inst &in);
    void operator()(gemm_inst &in);
    void operator()(gemv_inst &in);
    void operator()(hadamard_inst &in);
    void operator()(sum_inst &in);

    inline auto insertion_point() -> tinytc_inst_iterator_t { return bb_.get_insertion_point(); }

  private:
    auto get_memref_type(value_node const &v) const -> const memref_data_type *;

    local_tiling const &tiling_;
    core_config const &core_cfg_;
    region_builder bb_;
};

auto linalg_generator::get_memref_type(value_node const &v) const -> const memref_data_type * {
    auto t = dyn_cast<memref_data_type>(v.ty());
    if (t == nullptr) {
        throw compilation_error(v.loc(), status::ir_expected_memref);
    }
    return t;
}

void linalg_generator::operator()(axpby_inst &in) {
    auto ctx = compiler_context{in.alpha().context(), true};
    auto bool_ty = get_boolean(ctx);
    auto index_ty = get_scalar(ctx, scalar_type::index);

    auto at = get_memref_type(in.A());
    auto bt = get_memref_type(in.B());
    if (bt->dim() == 0) {
        auto parallel = make_parallel(in.loc());
        tinytc_region_t body = &parallel->child_region(0);
        auto bb = region_builder{body};

        auto i32_ty = get_scalar(ctx, scalar_type::i32);
        auto sg_id = bb.add(make_builtin(builtin::subgroup_id, i32_ty, in.loc()));
        auto sg_lid = bb.add(make_builtin(builtin::subgroup_local_id, i32_ty, in.loc()));
        auto c0 = bb.add(make_constant(0, i32_ty));
        auto cond0 = bb.add(make_cmp(cmp_condition::eq, sg_id, c0, bool_ty, in.loc()));
        auto cond1 = bb.add(make_cmp(cmp_condition::eq, sg_lid, c0, bool_ty, in.loc()));
        auto cond = bb.add(make_arith(arithmetic::and_, cond0, cond1, cond0->ty()));
        bb.if_condition(cond, [&](region_builder &bb) {
            auto a = bb.add(make_load(&in.A(), {}, at->element_data_ty(), in.loc()));
            blas_update(bb, in.atomic(), &in.alpha(), a, &in.beta(), &in.B(), {}, in.loc());
        });

        bb_.add(std::move(parallel));
    } else if (bt->dim() == 1) {
        auto c0 = bb_.add(make_constant(0, index_ty, in.loc()));
        auto c_shape0 = bb_.add(make_size(&in.B(), 0, index_ty, in.loc()));
        bb_.foreach_loop(
            index_ty, {c0.get()}, {c_shape0.get()},
            [&](region_builder &bb, auto loop_vars) {
                auto a =
                    bb.add(make_load(&in.A(), {loop_vars[0]}, at->element_data_ty(), in.loc()));
                blas_update(bb, in.atomic(), &in.alpha(), a, &in.beta(), &in.B(), {loop_vars[0]},
                            in.loc());
            },
            in.loc());
    } else if (bt->dim() == 2) {
        auto c0 = bb_.add(make_constant(0, index_ty, in.loc()));
        auto c_shape0 = bb_.add(make_size(&in.B(), 0, index_ty, in.loc()));
        auto c_shape1 = bb_.add(make_size(&in.B(), 1, index_ty, in.loc()));
        bb_.foreach_loop(
            index_ty, {c0.get(), c0.get()}, {c_shape0.get(), c_shape1.get()},
            [&](region_builder &bb, auto loop_vars) {
                auto a_idx = std::array<value, 2u>{loop_vars[0], loop_vars[1]};
                if (in.tA() == transpose::T) {
                    std::swap(a_idx[0], a_idx[1]);
                }
                auto a = bb.add(make_load(&in.A(), a_idx, at->element_data_ty(), in.loc()));
                blas_update(bb, in.atomic(), &in.alpha(), a, &in.beta(), &in.B(),
                            {loop_vars[0], loop_vars[1]}, in.loc());
            },
            in.loc());
    }
}

void linalg_generator::operator()(ger_inst &in) {
    auto index_ty = scalar_data_type::get(in.alpha().context(), scalar_type::index);
    auto c0 = bb_.add(make_constant(0, index_ty, in.loc()));
    auto c_shape0 = bb_.add(make_size(&in.C(), 0, index_ty, in.loc()));
    auto c_shape1 = bb_.add(make_size(&in.C(), 1, index_ty, in.loc()));
    bb_.foreach_loop(
        index_ty, {c0.get(), c0.get()}, {c_shape0.get(), c_shape1.get()},
        [&](region_builder &bb, auto loop_vars) {
            auto at = get_memref_type(in.A());
            auto bt = get_memref_type(in.B());
            auto a = bb.add(make_load(&in.A(), {loop_vars[0]}, at->element_data_ty(), in.loc()));
            auto b = bb.add(make_load(&in.B(), {loop_vars[1]}, bt->element_data_ty(), in.loc()));
            auto ab = mixed_precision_arithmetic(bb, arithmetic::mul, a, b, in.loc());
            blas_update(bb, in.atomic(), &in.alpha(), ab, &in.beta(), &in.C(),
                        {loop_vars[0], loop_vars[1]}, in.loc());
        },
        in.loc());
}

void linalg_generator::operator()(gemm_inst &in) {
    auto parallel = make_parallel(in.loc());
    tinytc_region_t body = &parallel->child_region(0);
    auto bb = region_builder{body};

    auto at = get_memref_type(in.A());
    auto bt = get_memref_type(in.B());
    auto ct = get_memref_type(in.C());

    auto ctx = compiler_context{in.alpha().context(), true};
    auto i32_ty = get_scalar(ctx, scalar_type::i32);
    auto index_ty = get_scalar(ctx, scalar_type::index);

    auto sgid = bb.add(make_builtin(builtin::subgroup_id, i32_ty, in.loc()));
    auto c_m_tiles = bb.add(make_constant(tiling_.m_tiles(), i32_ty, in.loc()));
    auto sg_n = bb.add(make_arith(arithmetic::div, sgid, c_m_tiles, i32_ty, in.loc()));
    auto sg_m = bb.add(make_arith(arithmetic::rem, sgid, c_m_tiles, i32_ty, in.loc()));

    auto [max_rows, max_cols] = max_register_block_gemm(
        size(ct->element_ty()), core_cfg_.subgroup_size, core_cfg_.register_space,
        is_complex_type(ct->element_ty()) ? 2 : 1);

    auto c_shape0 = instant_constant_fold_add(bb, make_size(&in.C(), 0, index_ty, in.loc()));
    auto c_shape1 = instant_constant_fold_add(bb, make_size(&in.C(), 1, index_ty, in.loc()));
    auto K = instant_constant_fold_add(
        bb, make_size(&in.A(), in.tA() == transpose::T ? 0 : 1, index_ty, in.loc()));

    auto const_shape0 = get_int_constant(c_shape0);
    auto const_shape1 = get_int_constant(c_shape1);

    const auto [block_size0, block_size1, do_tile_uniformly, K_block_sizes] =
        [&]() -> std::tuple<std::int32_t, std::int32_t, bool, std::vector<std::int32_t>> {
        if (auto ext_type = core_cfg_.matrix->get_precision(at->element_ty(), bt->element_ty(),
                                                            ct->element_ty());
            ext_type) {
            const auto M_bs = ext_type->M_block_sizes();
            const auto block_size0 =
                const_shape0 ? choose_block_size(M_bs, tiling_.m_tiles(), *const_shape0) : M_bs[0];
            const auto N_bs = ext_type->N_block_sizes(block_size0);
            const auto block_size1 =
                const_shape1 ? choose_block_size(N_bs, tiling_.n_tiles(), *const_shape1) : N_bs[0];
            const auto K_bs = ext_type->K_block_sizes(block_size0, block_size1);

            return std::make_tuple(block_size0, block_size1, false, K_bs);
        }

        const auto block_size0 = const_shape0
                                     ? compute_m_block_size(core_cfg_.subgroup_size, max_rows,
                                                            tiling_.m_tiles(), *const_shape0)
                                     : max_rows;
        const auto block_size1 = max_cols;

        return std::make_tuple(block_size0, block_size1, const_shape1.has_value(),
                               std::vector<std::int32_t>(standard_K_block_sizes.begin(),
                                                         standard_K_block_sizes.end()));
    }();

    if (do_tile_uniformly) {
        tile_loop_uniformly(
            bb, c_shape1, block_size1, tiling_.n_tiles(), sg_n,
            [&](region_builder &bb, value n_block, value trip_count) {
                auto const_trip_count = get_int_constant(trip_count);
                if (!const_trip_count) {
                    throw compilation_error(in.loc(), status::internal_compiler_error);
                }
                tile_loop_by_sgs(bb, c_shape0, block_size0, tiling_.m_tiles(), sg_m,
                                 [&](region_builder &bb, value m_block, bool m_check, value) {
                                     gemm_microkernel(
                                         bb, in.tA(), in.tB(), in.atomic(), &in.alpha(), &in.A(),
                                         &in.B(), &in.beta(), &in.C(), K, m_block, block_size0,
                                         m_check, n_block, *const_trip_count, false, K_block_sizes,
                                         at->element_data_ty(), bt->element_data_ty(),
                                         ct->element_data_ty(), in.loc());
                                 });
            });
    } else {
        tile_loop_by_sgs(bb, c_shape1, block_size1, tiling_.n_tiles(), sg_n,
                         [&](region_builder &bb, value n_block, bool n_check, value) {
                             tile_loop_by_sgs(
                                 bb, c_shape0, block_size0, tiling_.m_tiles(), sg_m,
                                 [&](region_builder &bb, value m_block, bool m_check, value) {
                                     gemm_microkernel(bb, in.tA(), in.tB(), in.atomic(),
                                                      &in.alpha(), &in.A(), &in.B(), &in.beta(),
                                                      &in.C(), K, m_block, block_size0, m_check,
                                                      n_block, block_size1, n_check, K_block_sizes,
                                                      at->element_data_ty(), bt->element_data_ty(),
                                                      ct->element_data_ty(), in.loc());
                                 });
                         });
    }

    bb_.add(std::move(parallel));
}

void linalg_generator::operator()(gemv_inst &in) {
    auto index_ty = scalar_data_type::get(in.alpha().context(), scalar_type::index);
    auto c0 = bb_.add(make_constant(0, index_ty, in.loc()));
    auto c_shape0 = bb_.add(make_size(&in.C(), 0, index_ty, in.loc()));
    auto ct = get_memref_type(in.C());
    bb_.foreach_loop(
        index_ty, {c0.get()}, {c_shape0.get()},
        [&](region_builder &bb, auto loop_vars) {
            auto c_init = bb.add(make_constant_zero(ct->element_data_ty()));
            auto K =
                bb.add(make_size(&in.A(), in.tA() == transpose::T ? 0 : 1, index_ty, in.loc()));
            auto c_acc = bb.for_loop(
                index_ty, c0, K, {}, {c_init}, {ct->element_data_ty()},
                [&](region_builder &bb, array_view<value> p) {
                    auto a_idx = std::array<value, 2u>{loop_vars[0], p[0]};
                    if (in.tA() == transpose::T) {
                        std::swap(a_idx[0], a_idx[1]);
                    }
                    auto at = get_memref_type(in.A());
                    auto bt = get_memref_type(in.B());
                    auto a = bb.add(make_load(&in.A(), a_idx, at->element_data_ty(), in.loc()));
                    auto b = bb.add(make_load(&in.B(), {p[0]}, bt->element_data_ty(), in.loc()));
                    auto ab = mixed_precision_arithmetic(bb, arithmetic::mul, a, b, in.loc());
                    auto ab_c = mixed_precision_arithmetic(bb, arithmetic::add, p[1], ab, in.loc());
                    bb.add(make_yield({ab_c}, in.loc()));
                });
            blas_update(bb, in.atomic(), &in.alpha(), c_acc[0], &in.beta(), &in.C(), {loop_vars[0]},
                        in.loc());
        },
        in.loc());
}

void linalg_generator::operator()(hadamard_inst &in) {
    auto index_ty = scalar_data_type::get(in.alpha().context(), scalar_type::index);
    auto c0 = bb_.add(make_constant(0, index_ty, in.loc()));
    auto c_shape0 = bb_.add(make_size(&in.C(), 0, index_ty, in.loc()));
    bb_.foreach_loop(
        index_ty, {c0.get()}, {c_shape0.get()},
        [&](region_builder &bb, auto loop_vars) {
            auto at = get_memref_type(in.A());
            auto bt = get_memref_type(in.B());
            auto a = bb.add(make_load(&in.A(), {loop_vars[0]}, at->element_data_ty(), in.loc()));
            auto b = bb.add(make_load(&in.B(), {loop_vars[0]}, bt->element_data_ty(), in.loc()));
            auto ab = mixed_precision_arithmetic(bb, arithmetic::mul, a, b, in.loc());
            blas_update(bb, in.atomic(), &in.alpha(), ab, &in.beta(), &in.C(), {loop_vars[0]},
                        in.loc());
        },
        in.loc());
}

void linalg_generator::operator()(sum_inst &in) {
    auto parallel = make_parallel(in.loc());
    tinytc_region_t body = &parallel->child_region(0);
    auto bb = region_builder{body};

    auto ctx = compiler_context{in.alpha().context(), true};
    auto bool_ty = get_boolean(ctx);
    auto i32_ty = get_scalar(ctx, scalar_type::i32);
    auto index_ty = get_scalar(ctx, scalar_type::index);

    auto at = get_memref_type(in.A());
    auto bt = get_memref_type(in.B());
    if (bt->dim() == 0) {
        auto c_sgs = bb.add(make_constant(core_cfg_.subgroup_size, i32_ty, in.loc()));
        auto sgid = bb.add(make_builtin(builtin::subgroup_id, i32_ty, in.loc()));
        auto m = bb.add(make_builtin(builtin::subgroup_local_id, i32_ty, in.loc()));
        auto from0 = bb.add(make_arith(arithmetic::mul, sgid, c_sgs, i32_ty, in.loc()));
        auto from1 = bb.add(make_arith(arithmetic::add, from0, m, i32_ty, in.loc()));
        auto from_index = bb.add(make_cast(from1, index_ty, in.loc()));

        auto c_zero = bb.add(make_constant_zero(i32_ty, in.loc()));
        auto is_from_0 = bb.add(make_cmp(cmp_condition::eq, from1, c_zero, bool_ty, in.loc()));

        auto c_trip_count =
            instant_constant_fold_add(bb, make_size(&in.A(), 0, index_ty, in.loc()));
        auto c_step = bb.add(make_constant(
            core_cfg_.subgroup_size * tiling_.m_tiles() * tiling_.n_tiles(), index_ty, in.loc()));
        auto c_init = bb.add(make_constant_zero(bt->element_data_ty(), in.loc()));

        auto acc = bb.for_loop(
            index_ty, from_index, c_trip_count, c_step, {c_init}, {bt->element_data_ty()},
            [&](region_builder &bb, array_view<value> args) {
                auto a = bb.add(make_load(&in.A(), {args[0]}, at->element_data_ty(), in.loc()));
                auto sum = mixed_precision_arithmetic(bb, arithmetic::add, args[1], a, in.loc());
                bb.add(make_yield({sum}, in.loc()));
            });
        auto sum = bb.add(
            make_work_group(work_group_operation::reduce_add, acc[0], acc[0]->ty(), in.loc()));
        bb.if_condition(
            is_from_0,
            [&](region_builder &bb) {
                blas_update(bb, in.atomic(), &in.alpha(), sum, &in.beta(), &in.B(), {}, in.loc());
            },
            in.loc());
    } else if (bt->dim() == 1) {
        auto index_ty = scalar_data_type::get(in.alpha().context(), scalar_type::index);
        auto c0 = bb_.add(make_constant(0, index_ty, in.loc()));
        auto c_shape0 = bb_.add(make_size(&in.B(), 0, index_ty, in.loc()));
        bb_.foreach_loop(
            index_ty, array_view<value>{c0.get()}, array_view<value>{c_shape0.get()},
            [&](region_builder &bb, auto loop_vars) {
                auto K =
                    bb.add(make_size(&in.A(), in.tA() == transpose::T ? 0 : 1, index_ty, in.loc()));
                auto c_init = bb.add(make_constant_zero(bt->element_data_ty()));
                auto acc = bb.for_loop(
                    index_ty, c0, K, {}, {c_init}, {bt->element_data_ty()},
                    [&](region_builder &bb, array_view<value> args) {
                        auto index_list = std::array<value, 2u>{loop_vars[0], args[0]};
                        if (in.tA() == transpose::T) {
                            std::swap(index_list[0], index_list[1]);
                        }
                        auto a =
                            bb.add(make_load(&in.A(), index_list, at->element_data_ty(), in.loc()));
                        auto sum =
                            mixed_precision_arithmetic(bb, arithmetic::add, args[1], a, in.loc());
                        bb.add(make_yield({sum}, in.loc()));
                    });
                blas_update(bb, in.atomic(), &in.alpha(), acc[0], &in.beta(), &in.B(),
                            {loop_vars[0]}, in.loc());
            },
            in.loc());
    }

    bb_.add(std::move(parallel));
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
            if (isa<blas_a2_inst>(*it) || isa<blas_a3_inst>(*it)) {
                auto gen = linalg_generator{tiling, core_cfg, reg, it.get()};
                visit(gen, *it);
                it = reg.insts().erase(gen.insertion_point());
            }
        }
    });
}

} // namespace tinytc
