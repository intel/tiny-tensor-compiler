// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/lower_coopmatrix.hpp"
#include "device_info.hpp"
#include "error.hpp"
#include "node/function_node.hpp"
#include "node/inst_node.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "support/ilist.hpp"
#include "support/ilist_base.hpp"
#include "support/visit.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <stdexcept>
#include <utility>

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
bool coopmatrix_code_generator::operator()(cooperative_matrix_apply_inst &) { return false; }

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
