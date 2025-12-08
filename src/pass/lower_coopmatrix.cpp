// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/lower_coopmatrix.hpp"
#include "codegen_tools.hpp"
#include "coopmatrix_layout.hpp"
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
#include "tinytc/builder.hpp"
#include "tinytc/core.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"
#include "util/ilist.hpp"
#include "util/ilist_base.hpp"

#include <cstdint>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <vector>

namespace tinytc {

class coopmatrix_code_generator {
  public:
    coopmatrix_code_generator(core_config core_cfg, tinytc_region &reg)
        : core_cfg_{std::move(core_cfg)}, bb_{&reg} {}
    // Returns true if instruction was replaced
    bool operator()(inst_view in);
    bool operator()(cooperative_matrix_apply_inst in);

    void run_on_region(tinytc_region &reg);

  private:
    auto needs_coopmatrix_vector_impl(tinytc_inst &in);

    core_config core_cfg_;
    region_builder bb_;
};

bool coopmatrix_code_generator::operator()(inst_view) { return false; }

bool coopmatrix_code_generator::operator()(cooperative_matrix_apply_inst in) {
    if (in.body().empty()) {
        throw compilation_error(in.loc(), status::ir_must_have_yield);
    }

    auto bool_ty = boolean_type::get(in.get().context());
    auto i32_ty = i32_type::get(in.get().context());

    auto cloner = inst_cloner{};

    auto ct = get_coopmatrix_type(in.a());
    auto cl = get_layout(core_cfg_, ct);

    auto p = bb_.create<subgroup_local_id_inst>(i32_ty, in.loc());
    auto i = p;
    auto j0 = tinytc_value_t{nullptr};
    if (cl.rows < core_cfg_.subgroup_size) {
        auto cI = bb_.create<constant_inst>(cl.rows, i32_ty, in.loc());
        i = bb_.create<rem_inst>(p, cI, i32_ty, in.loc());
        j0 = bb_.create<div_inst>(p, cI, i32_ty, in.loc());
    }
    const auto col_inc_factor = core_cfg_.subgroup_size / cl.rows;

    auto copy = &in.a();
    for (std::int64_t v = 0; v < cl.length; ++v) {
        const auto k1 = v % cl.blocks1;
        const auto u = v / cl.blocks1 % cl.cols;
        const auto k2 = v / (cl.blocks1 * cl.cols);

        auto row = i;
        const auto block_offset = k1 * cl.rows + k2 * cl.rows * cl.blocks1;
        if (block_offset) {
            auto cblock_offset = bb_.create<constant_inst>(block_offset, i32_ty, in.loc());
            row = bb_.create<add_inst>(i, cblock_offset, i32_ty, in.loc());
        }
        auto j1 = bb_.create<constant_inst>(u * col_inc_factor, i32_ty, in.loc());
        auto col = j0 ? bb_.create<add_inst>(j0, j1, i32_ty, in.loc()) : j1;
        auto val =
            bb_.create<cooperative_matrix_extract_inst>(v, &in.a(), ct->component_ty(), in.loc());

        cloner.set_subs(&in.row(), row);
        cloner.set_subs(&in.col(), col);
        cloner.set_subs(&in.val(), val);

        auto modified_val = tinytc_value_t{};
        if ((u + 1) * col_inc_factor > cl.shape1) {
            auto cshape1 = bb_.create<constant_inst>(cl.shape1, i32_ty, in.loc());
            auto cond = bb_.create<less_than_inst>(col, cshape1, bool_ty, in.loc());
            modified_val = bb_.ifelse(
                                  cond,
                                  [&](region_builder &bb) {
                                      cloner.clone_region(in.body(), *bb.get_region());
                                  },
                                  [&](region_builder &bb) {
                                      auto c0 = bb.constant_zero(ct->component_ty(), in.loc());
                                      bb.create<yield_inst>(array_view{c0});
                                  },
                                  {ct->component_ty()}, in.loc())
                               .front();
        } else {
            cloner.clone_region(in.body(), *bb_.get_region());

            auto last_inst = --bb_.get_region()->end();
            if (last_inst != bb_.get_region()->end() && isa<yield_inst>(*last_inst)) {
                auto vals = dyn_cast<yield_inst>(last_inst.get()).yielded_vals();
                if (vals.size() != 1) {
                    throw compilation_error(in.loc(), status::ir_yield_mismatch);
                }
                modified_val = &vals[0];
                bb_.get_region()->insts().erase(last_inst);
            } else {
                throw compilation_error(in.loc(), status::ir_must_have_yield);
            }
        }
        copy = bb_.create<cooperative_matrix_insert_inst>(v, modified_val, copy, in.result().ty(),
                                                          in.loc());
    }
    for (auto &r : in.get().results()) {
        auto u = r.use_begin();
        while (r.has_uses()) {
            u->set(copy);
            u = r.use_begin();
        }
    }
    return true;
}

void coopmatrix_code_generator::run_on_region(tinytc_region &reg) {
    // Move all instructions to a temporary ilist.
    // We later move the instructions back, except those that are lowered remain in old_ilist
    // and are cleaned up at the end of the function.
    auto old_ilist = std::move(reg.insts());

    auto old_reg = bb_.get_region();
    bb_ = region_builder{&reg};

    auto it = old_ilist.begin();
    while (it != old_ilist.end()) {
        bool replaced = visit(*this, *it);
        if (!replaced) {
            auto instr = it.get();
            it = old_ilist.unlink(it);
            reg.insts().push_back(instr);
            for (auto &subreg : instr->child_regions()) {
                run_on_region(subreg);
            }
        } else {
            ++it;
        }
    }
    it = old_ilist.end();
    while (it != old_ilist.begin()) {
        --it;
        for (auto &result : it->results()) {
            if (result.has_uses()) {
                throw compilation_error(result.loc(), status::ir_value_still_has_uses);
            }
        }
        it = old_ilist.erase(it);
    }

    bb_ = region_builder{old_reg};
}

lower_coopmatrix_pass::lower_coopmatrix_pass(::tinytc_core_info const *info)
    : info_(std::move(info)) {
    if (info_ == nullptr) {
        throw std::invalid_argument("info must not be nullptr");
    }
}

void lower_coopmatrix_pass::run_on_function(tinytc_func &fn) {
    auto const subgroup_size = fn.subgroup_size();
    core_config core_cfg = {};
    try {
        core_cfg = info_->get_core_config(subgroup_size);
    } catch (std::out_of_range const &e) {
        throw compilation_error(fn.loc(), status::unsupported_subgroup_size);
    }

    auto gen = coopmatrix_code_generator{core_cfg, fn.body()};
    gen.run_on_region(fn.body());
}

} // namespace tinytc
