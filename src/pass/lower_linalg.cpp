// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/lower_linalg.hpp"
#include "codegen_tools.hpp"
#include "device_info.hpp"
#include "error.hpp"
#include "node/data_type_node.hpp"
#include "node/function_node.hpp"
#include "node/inst_node.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "support/casting.hpp"
#include "support/ilist.hpp"
#include "support/ilist_base.hpp"
#include "support/visit.hpp"
#include "support/walk.hpp"
#include "tiling.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <algorithm>
#include <array>
#include <stdexcept>
#include <utility>

namespace tinytc {

class linalg_generator {
  public:
    linalg_generator(local_tiling tiling, core_config core_cfg)
        : tiling_{std::move(tiling)}, core_cfg_{std::move(core_cfg)} {}
    auto operator()(inst_node &) -> inst { return inst{}; }
    auto operator()(axpby_inst &in) -> inst;
    auto operator()(ger_inst &in) -> inst;
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
        auto c_shape0 = bb.add(make_size(&in.B(), 0, in.loc()));
        inner_loop(bb, &in.A(), &in.B(), c_shape0, tiling_.m_tiles() * tiling_.n_tiles(), sgid);
    } else if (bt->dim() == 2) {
        auto c_m_tiles = bb.add(make_constant(tiling_.m_tiles(), i32_ty, in.loc()));
        auto sg_n = bb.add(make_arith(arithmetic::div, sgid, c_m_tiles, in.loc()));
        auto sg_m = bb.add(make_arith(arithmetic::rem, sgid, c_m_tiles, in.loc()));

        auto c_shape0 = bb.add(make_size(&in.B(), 0, in.loc()));
        auto c_shape1 = bb.add(make_size(&in.B(), 1, in.loc()));
        tile_loop_uniformly_new(
            bb, c_shape1, core_cfg_.subgroup_size, tiling_.n_tiles(), sg_n,
            [&](region_builder &bb, value block, value trip_count) {
                auto zero = bb.add(make_constant(0, index_ty));
                bb.for_loop(zero, trip_count, index_ty, [&](region_builder &bb, value n) {
                    auto nn = bb.add(make_arith(arithmetic::add, block, n, in.loc()));
                    auto static_offset_list = std::array<std::int64_t, 2u>{dynamic, 0};
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

    auto c_shape0 = bb.add(make_size(&in.C(), 0, in.loc()));
    auto c_shape1 = bb.add(make_size(&in.C(), 1, in.loc()));
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

auto linalg_generator::operator()(hadamard_inst &in) -> inst {
    auto parallel = make_parallel(in.loc());
    tinytc_region_t body = &parallel->child_region(0);
    auto bb = region_builder{body};

    auto ctx = compiler_context{in.alpha().context(), true};
    auto sgid = bb.add(make_subgroup_id(ctx, in.loc()));

    auto c_shape0 = bb.add(make_size(&in.C(), 0, in.loc()));
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
    auto index_ty = get_scalar(ctx, scalar_type::index);

    auto bt = get_memref_type(in.B());

    auto sgid = bb.add(make_subgroup_id(ctx, in.loc()));

    if (bt->dim() == 0) {
        // @todo
    } else if (bt->dim() == 1) {
        auto c_shape0 = bb.add(make_size(&in.B(), 0, in.loc()));
        auto c_trip_count = bb.add(make_size(&in.A(), in.tA() == transpose::T ? 0 : 1, in.loc()));
        tile_loop_by_sgs_standard(
            bb, c_shape0, core_cfg_.subgroup_size, tiling_.m_tiles() * tiling_.n_tiles(), sgid,
            [&](region_builder &bb, value mm) {
                auto zero = bb.add(make_constant(0, index_ty));
                // @todo need for loop that yields values
                bb.for_loop(zero, c_trip_count, index_ty, [&](region_builder &bb, value n) {
                    auto index_list = std::array<value, 2u>{mm, n};
                    if (in.tA() == transpose::T) {
                        std::swap(index_list[0], index_list[1]);
                    }
                    auto a = bb.add(make_load(&in.A(), index_list, in.loc()));
                    blas_update(bb, in.atomic(), &in.alpha(), a, &in.beta(), &in.B(), {mm},
                                in.loc());
                });
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
