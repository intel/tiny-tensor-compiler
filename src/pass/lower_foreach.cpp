// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/lower_foreach.hpp"
#include "codegen_tools.hpp"
#include "device_info.hpp"
#include "error.hpp"
#include "node/func.hpp"
#include "node/inst.hpp"
#include "node/inst_view.hpp"
#include "node/region.hpp"
#include "node/type.hpp"
#include "node/value.hpp"
#include "node/visit.hpp"
#include "pass/clone.hpp"
#include "support/walk.hpp"
#include "tiling.hpp"
#include "tinytc/builder.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"
#include "util/ilist.hpp"
#include "util/ilist_base.hpp"
#include "util/iterator.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace tinytc {

template <typename F>
void make_loop0(region_builder &bb, tinytc_value_t from, tinytc_value_t to, tinytc_value_t sg_id,
                int sgs, int num_tiles, F &&make_body, location const &loc) {
    auto ity = from->ty();
    auto ctx = sg_id->context();
    auto bool_ty = get<boolean_type>(ctx);
    auto i32_ty = get<number_type>(ctx, scalar_type::i32);
    auto sg_lid_i32 = bb.create<subgroup_local_id_inst>(i32_ty, loc);
    auto sg_lid = bb.create<cast_inst>(sg_lid_i32, ity, loc);
    auto size = instant_constant_fold_add(bb, create<sub_inst>(to, from, ity, loc));
    auto work_item_offset = bb.create<add_inst>(from, sg_lid, ity, loc);
    tile_loop_by_sgs(
        bb, size, sgs, num_tiles, sg_id,
        [&](region_builder &bb, tinytc_value_t block, bool is_remainder,
            tinytc_value_t trip_count) {
            auto loop_var0 = bb.create<add_inst>(block, work_item_offset, ity, loc);
            if (is_remainder) {
                auto cond = bb.create<less_than_inst>(sg_lid, trip_count, bool_ty, loc);
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
    auto operator()(inst_view) -> inst { return inst{}; }
    auto operator()(foreach_inst in) -> inst;

  private:
    local_tiling tiling_ = {};
    core_config core_cfg_ = {};
};

auto foreach_generator::operator()(foreach_inst in) -> inst {
    const int block_size0 = core_cfg_.subgroup_size;

    auto parallel = create<parallel_inst>(in.loc());
    tinytc_region_t body = &parallel->child_region(0);
    auto bb = region_builder{body};

    auto i32_ty = number_type::get(in.get().context(), scalar_type::i32);

    auto cloner = inst_cloner{};
    auto loop_vars = in.loop_vars().begin();
    auto from = in.from().begin();
    auto to = in.to().begin();
    auto ity = (*from).ty();

    if (in.dim() > 1) {
        auto const make_inner_loop_nest = [&](region_builder &bb, tinytc_value_t from1,
                                              tinytc_value_t to1) {
            tinytc_region_t current_region = bb.get_region();
            for (std::int64_t i = in.dim() - 1; i > 1; --i) {
                auto for_i =
                    inst{for_inst::create(&from[i], &to[i], nullptr, array_view<tinytc_value_t>{},
                                          array_view<tinytc_type_t>{}, in.loc())};
                auto for_i_view = for_inst(for_i.get());
                cloner.set_subs(&loop_vars[i], &for_i_view.loop_var());
                tinytc_region_t next_region = &for_i_view.body();
                current_region->insts().push_back(for_i.release());
                current_region = next_region;
            }
            region_builder{current_region}.for_loop(
                from1, to1,
                [&](region_builder &bb, tinytc_value_t loop_var1) {
                    cloner.set_subs(&loop_vars[1], loop_var1);
                    cloner.clone_region(in.body(), *bb.get_region());
                },
                nullptr, in.loc());
        };

        auto sg_id0 = bb.create<subgroup_id_inst>(comp3::x, i32_ty, in.loc());
        auto sg_id1 = bb.create<subgroup_id_inst>(comp3::y, i32_ty, in.loc());

        auto size1 = bb.create<sub_inst>(&to[1], &from[1], ity, in.loc());
        tile_loop_uniformly(
            bb, size1, core_cfg_.subgroup_size, tiling_.n_tiles(), sg_id1,
            [&](region_builder &bb, tinytc_value_t block, tinytc_value_t trip_count1) {
                auto from1 = bb.create<add_inst>(&from[1], block, ity, in.loc());
                auto to1 = bb.create<add_inst>(from1, trip_count1, ity, in.loc());
                make_loop0(
                    bb, &from[0], &to[0], sg_id0, block_size0, tiling_.m_tiles(),
                    [&](region_builder &bb, tinytc_value_t loop_var0) {
                        cloner.set_subs(&loop_vars[0], loop_var0);
                        make_inner_loop_nest(bb, from1, to1);
                    },
                    in.loc());
            });
    } else if (in.dim() == 1) {
        auto sg_id = bb.create<subgroup_linear_id_inst>(i32_ty, in.loc());
        make_loop0(
            bb, &from[0], &to[0], sg_id, block_size0, tiling_.m_tiles() * tiling_.n_tiles(),
            [&](region_builder &bb, tinytc_value_t loop_var0) {
                cloner.set_subs(&loop_vars[0], loop_var0);
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

void lower_foreach_pass::run_on_function(tinytc_func &fn) {
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

    walk<walk_order::post_order>(fn, [&](tinytc_region &reg) {
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
