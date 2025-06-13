// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/lower_coopmatrix.hpp"
#include "codegen_tools.hpp"
#include "coopmatrix_layout.hpp"
#include "device_info.hpp"
#include "error.hpp"
#include "node/data_type_node.hpp"
#include "node/function_node.hpp"
#include "node/inst_node.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "pass/clone.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"
#include "util/casting.hpp"
#include "util/ilist.hpp"
#include "util/ilist_base.hpp"
#include "util/visit.hpp"

#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

namespace tinytc {

class coopmatrix_code_generator {
  public:
    coopmatrix_code_generator(core_config core_cfg, region_node &reg)
        : core_cfg_{std::move(core_cfg)}, bb_{&reg} {}
    // Returns true if instruction was replaced
    bool operator()(inst_node &in);
    bool operator()(cooperative_matrix_apply_inst &in);

    void run_on_region(region_node &reg);

  private:
    auto needs_coopmatrix_vector_impl(inst_node &in);

    core_config core_cfg_;
    region_builder bb_;
};

bool coopmatrix_code_generator::operator()(inst_node &) { return false; }

bool coopmatrix_code_generator::operator()(cooperative_matrix_apply_inst &in) {
    if (in.body().empty()) {
        throw compilation_error(in.loc(), status::ir_must_have_yield);
    }

    auto bool_ty = boolean_data_type::get(in.context());
    auto i32_ty = scalar_data_type::get(in.context(), scalar_type::i32);

    auto cloner = inst_cloner{};

    auto ct = get_coopmatrix_type(in.a());
    auto cl = get_layout(core_cfg_, ct);

    auto p = bb_.add(make_builtin(builtin::subgroup_local_id, i32_ty, in.loc()));
    auto i = p;
    auto j0 = value{nullptr};
    if (cl.rows < core_cfg_.subgroup_size) {
        auto cI = bb_.add(make_constant(cl.rows, i32_ty, in.loc()));
        i = bb_.add(make_arith(arithmetic::rem, p, cI, i32_ty, in.loc()));
        j0 = bb_.add(make_arith(arithmetic::div, p, cI, i32_ty, in.loc()));
    }
    const auto col_inc_factor = core_cfg_.subgroup_size / cl.rows;

    auto copy = value{&in.a()};
    for (std::int64_t v = 0; v < cl.length; ++v) {
        const auto k1 = v % cl.blocks1;
        const auto u = v / cl.blocks1 % cl.cols;
        const auto k2 = v / (cl.blocks1 * cl.cols);

        auto row = i;
        const auto block_offset = k1 * cl.rows + k2 * cl.rows * cl.blocks1;
        if (block_offset) {
            auto cblock_offset = bb_.add(make_constant(block_offset, i32_ty, in.loc()));
            row = bb_.add(make_arith(arithmetic::add, i, cblock_offset, i32_ty, in.loc()));
        }
        auto j1 = bb_.add(make_constant(u * col_inc_factor, i32_ty, in.loc()));
        auto col = j0 ? bb_.add(make_arith(arithmetic::add, j0, j1, i32_ty, in.loc())) : j1;
        auto val = bb_.add(make_cooperative_matrix_extract(&in.a(), v, ct->ty(), in.loc()));

        cloner.set_subs(&in.row(), row);
        cloner.set_subs(&in.col(), col);
        cloner.set_subs(&in.val(), val);

        auto modified_val = value{};
        if ((u + 1) * col_inc_factor > cl.shape1) {
            auto cshape1 = bb_.add(make_constant(cl.shape1, i32_ty, in.loc()));
            auto cond = bb_.add(make_cmp(cmp_condition::lt, col, cshape1, bool_ty, in.loc()));
            modified_val = bb_.ifelse(
                                  cond,
                                  [&](region_builder &bb) {
                                      cloner.clone_region(in.body(), *bb.get_region());
                                  },
                                  [&](region_builder &bb) {
                                      auto c0 = bb.add(make_constant_zero(ct->ty(), in.loc()));
                                      bb.add(make_yield(c0));
                                  },
                                  {ct->ty()}, in.loc())
                               .front();
        } else {
            cloner.clone_region(in.body(), *bb_.get_region());

            auto last_inst = --bb_.get_region()->end();
            if (last_inst != bb_.get_region()->end() && isa<yield_inst>(*last_inst)) {
                auto yi = dyn_cast<yield_inst>(last_inst.get());
                if (yi->num_operands() != 1) {
                    throw compilation_error(in.loc(), status::ir_yield_mismatch);
                }
                modified_val = &yi->op(0);
                bb_.get_region()->insts().erase(last_inst);
            } else {
                throw compilation_error(in.loc(), status::ir_must_have_yield);
            }
        }
        copy = bb_.add(
            make_cooperative_matrix_insert(modified_val, copy, v, in.result(0).ty(), in.loc()));
    }
    for (auto &r : in.results()) {
        auto u = r.use_begin();
        while (r.has_uses()) {
            u->set(copy);
            u = r.use_begin();
        }
    }
    return true;
}

void coopmatrix_code_generator::run_on_region(region_node &reg) {
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

void lower_coopmatrix_pass::run_on_function(function_node &fn) {
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
