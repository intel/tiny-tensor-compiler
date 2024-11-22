// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/lower_foreach.hpp"
#include "codegen_tools.hpp"
#include "device_info.hpp"
#include "node/function_node.hpp"
#include "node/inst_node.hpp"
#include "node/region_node.hpp"
#include "pass/clone.hpp"
#include "support/visit.hpp"
#include "support/walk.hpp"
#include "tiling.hpp"

namespace tinytc {

template <typename F>
void make_loop0(region_builder &bb, value from, value to, value sg_id, int sgs, int num_tiles,
                F &&make_body, location const &loc) {
    auto ity = from->ty();
    auto ctx = compiler_context{sg_id->context(), true};
    auto bool_ty = get_boolean(ctx);
    auto i32_ty = get_scalar(ctx, scalar_type::i32);
    auto sg_lid_i32 = bb.add(make_builtin(builtin::subgroup_local_id, i32_ty, loc));
    auto sg_lid = bb.add(make_cast(sg_lid_i32, ity, loc));
    auto size = bb.add(make_arith(arithmetic::sub, to, from, ity, loc));
    auto work_item_offset = bb.add(make_arith(arithmetic::add, from, sg_lid, ity, loc));
    tile_loop_by_sgs_new(
        bb, size, sgs, num_tiles, sg_id,
        [&](region_builder &bb, value block, bool is_remainder, value trip_count) {
            auto loop_var0 = bb.add(make_arith(arithmetic::add, block, work_item_offset, ity, loc));
            if (is_remainder) {
                auto cond = bb.add(make_cmp(cmp_condition::lt, sg_lid, trip_count, bool_ty, loc));
                bb.if_condition(cond, [&](region_builder &bb) { make_body(bb, loop_var0); }, loc);
            } else {
                make_body(bb, loop_var0);
            }
        });
}

class foreach_generator {
  public:
    foreach_generator(local_tiling tiling, core_config core_cfg)
        : tiling_{std::move(tiling)}, core_cfg_{std::move(core_cfg)} {}
    auto operator()(inst_node &) -> inst { return inst{}; }
    auto operator()(foreach_inst &in) -> inst;

  private:
    local_tiling tiling_ = {};
    core_config core_cfg_ = {};
};

auto foreach_generator::operator()(foreach_inst &in) -> inst {
    auto parallel = make_parallel(in.loc());
    tinytc_region_t body = &parallel->child_region(0);
    auto bb = region_builder{body};

    auto i32_ty = scalar_data_type::get(in.context(), scalar_type::i32);
    auto sg_id = bb.add(make_builtin(builtin::subgroup_id, i32_ty, in.loc()));

    auto cloner = inst_cloner{};
    auto loop_vars = in.loop_vars().begin();
    auto from = in.from().begin();
    auto to = in.to().begin();
    auto ity = (*from).ty();

    if (in.dim() > 1) {
        auto const make_inner_loop_nest = [&](region_builder &bb, value from1, value to1) {
            tinytc_region_t current_region = bb.get_region().get();
            for (std::int64_t i = in.dim() - 1; i > 1; --i) {
                auto for_i = std::make_unique<for_inst>(ity, &from[i], &to[i], nullptr,
                                                        array_view<tinytc_value_t>{},
                                                        array_view<tinytc_data_type_t>{}, in.loc());
                cloner.set_subs(&loop_vars[i], &for_i->loop_var());
                tinytc_region_t next_region = &for_i->body();
                current_region->insts().push_back(for_i.release());
                current_region = next_region;
            }
            region_builder{current_region}.for_loop(
                ity, from1, to1,
                [&](region_builder &bb, value loop_var1) {
                    cloner.set_subs(&loop_vars[1], loop_var1.get());
                    cloner.clone_region(in.body(), *bb.get_region());
                },
                in.loc());
        };

        auto c_m_tiles = bb.add(make_constant(tiling_.m_tiles(), sg_id->ty(), in.loc()));
        auto sg_id1 = bb.add(make_arith(arithmetic::div, sg_id, c_m_tiles, sg_id->ty(), in.loc()));
        auto sg_id0 = bb.add(make_arith(arithmetic::rem, sg_id, c_m_tiles, sg_id->ty(), in.loc()));

        auto size1 = bb.add(make_arith(arithmetic::sub, &to[1], &from[1], ity, in.loc()));
        tile_loop_uniformly_new(
            bb, size1, core_cfg_.subgroup_size, tiling_.n_tiles(), sg_id1,
            [&](region_builder &bb, value block, value trip_count1) {
                auto from1 = bb.add(make_arith(arithmetic::add, &from[1], block, ity, in.loc()));
                auto to1 = bb.add(make_arith(arithmetic::add, from1, trip_count1, ity, in.loc()));
                make_loop0(
                    bb, &from[0], &to[0], sg_id0, core_cfg_.subgroup_size, tiling_.m_tiles(),
                    [&](region_builder &bb, value loop_var0) {
                        cloner.set_subs(&loop_vars[0], loop_var0.get());
                        make_inner_loop_nest(bb, from1, to1);
                    },
                    in.loc());
            });
    } else if (in.dim() == 1) {
        make_loop0(
            bb, &from[0], &to[0], sg_id, core_cfg_.subgroup_size,
            tiling_.m_tiles() * tiling_.n_tiles(),
            [&](region_builder &bb, value loop_var0) {
                cloner.set_subs(&loop_vars[0], loop_var0.get());
                cloner.clone_region(in.body(), *bb.get_region());
            },
            in.loc());
    }

    return parallel;
}

lower_foreach_pass::lower_foreach_pass(::tinytc_core_info const *info) : info_(std::move(info)) {
    if (info_ == nullptr) {
        throw std::invalid_argument("info must not be nullptr");
    }
}

void lower_foreach_pass::run_on_function(function_node &fn) {
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
            auto lowered_inst = visit(foreach_generator{tiling, core_cfg}, *it);
            if (lowered_inst) {
                it = reg.insts().erase(it);
                it = reg.insts().insert(it, lowered_inst.release());
            }
        }
    });
}

} // namespace tinytc
