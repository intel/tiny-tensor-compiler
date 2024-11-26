// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/check_ir.hpp"
#include "error.hpp"
#include "node/value_node.hpp"
#include "support/casting.hpp"
#include "support/ilist_base.hpp"
#include "support/visit.hpp"
#include "support/walk.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <cstdint>
#include <functional>

namespace tinytc {

void check_ir_pass::check_yield(region_node const &reg, inst_node const &in,
                                status yield_missing_status) {
    auto last_inst = --reg.end();
    if (last_inst == reg.end()) {
        throw compilation_error(reg.loc(), yield_missing_status);
    }
    auto yield = dyn_cast<const yield_inst>(last_inst.get());
    if (!yield) {
        throw compilation_error(reg.loc(), yield_missing_status);
    }
    if (yield->num_operands() != in.num_results()) {
        throw compilation_error(yield->loc(), status::ir_yield_mismatch);
    }
    for (std::int64_t i = 0; i < in.num_results(); ++i) {
        if (yield->op(i).ty() != in.result(i).ty()) {
            throw compilation_error(yield->loc(), {&yield->op(i)}, status::ir_yield_mismatch);
        }
    }
}

void check_ir_pass::operator()(inst_node const &) {}
void check_ir_pass::operator()(for_inst const &in) {
    if (in.num_results() > 0) {
        check_yield(in.body(), in);
    }
}
void check_ir_pass::operator()(if_inst const &in) {
    if (in.num_results() > 0) {
        check_yield(in.then(), in);
        check_yield(in.otherwise(), in, status::ir_yield_in_else_branch_missing);
    }
}

void check_ir_pass::run_on_function(function_node &fn) {
    walk(fn, [this](inst_node const &i, walk_stage const &stage) {
        const bool child_region_is_spmd_region =
            i.num_child_regions() > 0 && i.child_region(0).kind() == region_kind::spmd;

        if (stage.is_before_all_regions()) {
            if (i.kind() == inst_execution_kind::collective && inside_spmd_region_) {
                throw compilation_error(i.loc(), status::ir_collective_called_from_spmd);
            } else if (i.kind() == inst_execution_kind::spmd && !inside_spmd_region_) {
                throw compilation_error(i.loc(), status::ir_spmd_called_from_collective);
            }

            if (child_region_is_spmd_region) {
                inside_spmd_region_ = true;
            }
        }

        if (child_region_is_spmd_region && stage.is_after_all_regions()) {
            inside_spmd_region_ = false;
        }

        visit(*this, i);
    });
}

} // namespace tinytc
