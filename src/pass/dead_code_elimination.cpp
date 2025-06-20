// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/dead_code_elimination.hpp"
#include "node/function_node.hpp"
#include "node/inst_node.hpp"
#include "node/inst_view.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "node/visit.hpp"
#include "util/ilist.hpp"
#include "util/ilist_base.hpp"

#include <cstdint>
#include <iterator>
#include <variant>

namespace tinytc {

class dead_code_analysis {
  public:
    auto operator()(inst_view in) -> bool;
    auto operator()(if_inst in) -> bool;
    auto operator()(for_inst in) -> bool;
};

auto dead_code_analysis::operator()(inst_view in) -> bool {
    /* Instruction have side effects if either of the following is true
     *
     * - More than one child region (if, for, foreach, parallel, ...)
     * - Instruction does not have results (barrier, GEMM, GER, ...)
     *
     */
    const bool has_side_effects = in.get().num_child_regions() > 0 || in.get().num_results() == 0;

    bool any_result_has_uses = false;
    for (auto &res : in.get().results()) {
        any_result_has_uses = any_result_has_uses || res.has_uses();
    }

    return !has_side_effects && !any_result_has_uses;
}

auto dead_code_analysis::operator()(if_inst in) -> bool {
    constant_inst cond_const = dyn_cast<constant_inst>(in.condition().defining_inst());
    if (in.get().num_results() == 0 && cond_const) {
        // If-instruction is dead if condition is constant and false
        return std::holds_alternative<bool>(cond_const.value()) &&
               std::get<bool>(cond_const.value()) == false;
    }

    return false;
}

auto dead_code_analysis::operator()(for_inst in) -> bool {
    constant_inst from_const = dyn_cast<constant_inst>(in.from().defining_inst());
    constant_inst to_const = dyn_cast<constant_inst>(in.to().defining_inst());
    if (in.get().num_results() == 0 && from_const && to_const) {
        // For-instruction is dead if from >= to
        return std::holds_alternative<std::int64_t>(from_const.value()) &&
               std::holds_alternative<std::int64_t>(to_const.value()) &&
               std::get<std::int64_t>(from_const.value()) >=
                   std::get<std::int64_t>(to_const.value());
    }
    return false;
}

void dead_code_elimination_pass::run_on_function(function_node &fn) { run_on_region(fn.body()); }

void dead_code_elimination_pass::run_on_region(region_node &reg) {
    auto prev_it = reg.end();
    while (prev_it != reg.begin()) {
        auto it = --prev_it;
        auto is_dead = visit(dead_code_analysis{}, *it);
        if (is_dead) {
            prev_it = reg.insts().erase(it);
        } else {
            for (auto &subreg : it->child_regions()) {
                run_on_region(subreg);
            }
        }
    }
}

} // namespace tinytc
