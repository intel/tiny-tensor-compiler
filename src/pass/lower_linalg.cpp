// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/lower_linalg.hpp"
#include "codegen_tools.hpp"
#include "error.hpp"
#include "node/function_node.hpp"
#include "node/inst_node.hpp"
#include "support/casting.hpp"
#include "support/visit.hpp"
#include "support/walk.hpp"
#include "tiling.hpp"
#include "tinytc/tinytc.hpp"

namespace tinytc {

class linalg_generator {
  public:
    linalg_generator(local_tiling tiling, core_config core_cfg)
        : tiling_{std::move(tiling)}, core_cfg_{std::move(core_cfg)} {}
    auto operator()(inst_node &) -> inst { return inst{}; }
    auto operator()(ger_inst &g) -> inst;

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

auto linalg_generator::operator()(ger_inst &g) -> inst {
    auto parallel = make_parallel(g.loc());
    tinytc_region_t body = &parallel->child_region(0);
    auto bb = region_builder{body};

    auto ctx = compiler_context{g.alpha().context(), true};
    auto i32_ty = get_scalar(ctx, scalar_type::i32);
    auto index_ty = get_scalar(ctx, scalar_type::index);

    auto sgid = bb.add(make_subgroup_id(ctx, g.loc()));
    auto c_m_tiles = bb.add(make_constant(tiling_.m_tiles(), i32_ty, g.loc()));
    auto sg_n = bb.add(make_arith(arithmetic::div, sgid, c_m_tiles, g.loc()));
    auto sg_m = bb.add(make_arith(arithmetic::rem, sgid, c_m_tiles, g.loc()));
    auto m = bb.add(make_subgroup_local_id(ctx, g.loc()));
    auto m_index = bb.add(make_cast(m, index_ty, g.loc()));

    auto c_shape0 = bb.add(make_size(&g.C(), 0, g.loc()));
    auto c_shape1 = bb.add(make_size(&g.C(), 1, g.loc()));
    tile_loop_uniformly_new(
        bb, c_shape1, core_cfg_.subgroup_size, tiling_.n_tiles(), sg_n,
        [&](region_builder &bb, value block, value trip_count) {
            auto zero = bb.add(make_constant(0, index_ty));
            bb.for_loop(zero, trip_count, index_ty, [&](region_builder &bb, value n) {
                auto nn = bb.add(make_arith(arithmetic::add, block, n, g.loc()));
                auto b = bb.add(make_load(&g.B(), {nn}, g.loc()));
                tile_loop_by_sgs_new(
                    bb, c_shape0, core_cfg_.subgroup_size, tiling_.m_tiles(), sg_m,
                    [&](region_builder &bb, value block, bool, value) {
                        auto mm = bb.add(make_arith(arithmetic::add, block, m_index, g.loc()));
                        auto a = bb.add(make_load(&g.A(), {mm}, g.loc()));
                        auto ab = bb.add(make_arith(arithmetic::mul, a, b, g.loc()));
                        bb.add(make_store(ab, &g.C(), {mm, nn}, g.loc()));
                    });
            });
        });

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
