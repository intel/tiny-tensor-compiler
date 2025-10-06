// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/dead_code_elimination.hpp"
#include "error.hpp"
#include "node/func.hpp"
#include "node/inst.hpp"
#include "node/inst_view.hpp"
#include "node/region.hpp"
#include "node/value.hpp"
#include "node/visit.hpp"
#include "tinytc/types.hpp"
#include "util/ilist.hpp"
#include "util/ilist_base.hpp"

#include <cstdint>
#include <iterator>
#include <string>
#include <variant>

namespace tinytc {

enum is_dead_t : int {
    not_dead = -2,
    dead = -1,
    merge_region = 0,
    // ... value can be 1, 2, 3, ... to indicate which region should be merged
};

class dead_code_analysis {
  public:
    auto operator()(inst_view in) -> int;
    auto operator()(if_inst in) -> int;
    auto operator()(for_inst in) -> int;
};

auto dead_code_analysis::operator()(inst_view in) -> int {
    /* Instruction have side effects if either of the following is true
     *
     * - More than one child region (if, for, foreach, parallel, ...)
     * - Instruction does not have results (barrier, GEMM, GER, ...)
     * - Atomic update
     *
     */
    const bool has_side_effects = in.get().num_child_regions() > 0 || in.get().num_results() == 0 ||
                                  isa<atomic_update_inst>(in.get()) ||
                                  isa<cooperative_matrix_atomic_update_inst>(in.get());

    bool any_result_has_uses = false;
    for (auto &res : in.get().results()) {
        any_result_has_uses = any_result_has_uses || res.has_uses();
    }

    return !has_side_effects && !any_result_has_uses ? dead : not_dead;
}

auto dead_code_analysis::operator()(if_inst in) -> int {
    constant_inst cond_const = dyn_cast<constant_inst>(in.condition().defining_inst());
    if (cond_const && std::holds_alternative<bool>(cond_const.value())) {
        const bool cond = std::get<bool>(cond_const.value());
        if (in.is_otherwise_empty() && cond == false) {
            // Remove entire if
            return dead;
        } else if (cond == true) {
            // Merge then branch
            return merge_region + 0;
        } else {
            // Merge otherwise branch
            return merge_region + 1;
        }
    }

    return not_dead;
}

auto dead_code_analysis::operator()(for_inst in) -> int {
    constant_inst from_const = dyn_cast<constant_inst>(in.from().defining_inst());
    constant_inst to_const = dyn_cast<constant_inst>(in.to().defining_inst());
    if (in.get().num_results() == 0 && from_const && to_const) {
        // For-instruction is dead if from >= to
        const bool from_ge_to =
            std::holds_alternative<std::int64_t>(from_const.value()) &&
            std::holds_alternative<std::int64_t>(to_const.value()) &&
            std::get<std::int64_t>(from_const.value()) >= std::get<std::int64_t>(to_const.value());
        return from_ge_to ? dead : not_dead;
    }
    return not_dead;
}

void dead_code_elimination_pass::run_on_function(tinytc_func &fn) { run_on_region(fn.body()); }

void dead_code_elimination_pass::run_on_region(tinytc_region &reg) {
    auto prev_it = reg.end();
    while (prev_it != reg.begin()) {
        auto it = --prev_it;
        auto state = visit(dead_code_analysis{}, *it);
        if (state == dead) {
            // Instruction is dead so we can erase it
            prev_it = reg.insts().erase(it);
        } else if (state >= merge_region) {
            // Instruction always takes the same branch, so we can merge the branch into the parent
            // region and delete other branches
            auto &merge_reg = it->child_region(state);
            auto merge_it = merge_reg.end();
            auto insert_it = prev_it;
            // Merge instructions in reverse order
            while (merge_it != merge_reg.begin()) {
                --merge_it;
                auto instr = merge_it.get();
                if (isa<yield_inst>(*instr)) {
                    // If we encounter a yield instruction we have to update the uses of the
                    // instruction's results with the operands of the yield instruction
                    if (it->num_results() != instr->num_operands()) {
                        throw compilation_error(it->loc(), status::ir_yield_mismatch);
                    }
                    for (std::int32_t r_no = 0; r_no < it->num_results(); ++r_no) {
                        auto &r = it->result(r_no);
                        auto &op = instr->op(r_no);
                        auto u = r.use_begin();
                        while (r.has_uses()) {
                            u->set(&op);
                            u = r.use_begin();
                        }
                    }
                    merge_it = merge_reg.insts().erase(merge_it);
                } else {
                    // Otherwise we move the instruction from the branch to the parent region
                    merge_it = merge_reg.insts().unlink(merge_it);
                    insert_it = reg.insts().insert(insert_it, instr);
                }
            }
            // Check whether a result still has a use
            for (auto &r : it->results()) {
                if (r.has_uses()) {
                    throw compilation_error(it->loc(), status::internal_compiler_error,
                                            "Error in dead code elimination: Trying to delete a "
                                            "result that still has uses");
                }
            }
            // Erase instruction
            // Note that prev_it is set such that we run dead code analysis on all instructions that
            // were just merged
            prev_it = reg.insts().erase(it);
        } else {
            // Run on subgregions for non-dead, non-merge instructions
            for (auto &subreg : it->child_regions()) {
                run_on_region(subreg);
            }
        }
    }
}

} // namespace tinytc
