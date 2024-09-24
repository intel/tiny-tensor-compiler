// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/check_ir.hpp"
#include "error.hpp"
#include "node/inst_node.hpp"
#include "node/region_node.hpp"
#include "support/walk.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

namespace tinytc {

void check_ir_pass::run_on_function(function_node &fn) {
    walk(fn, [this](inst_node const &i, walk_stage const &stage) {
        const bool child_region_is_spmd_region =
            i.num_child_regions() > 0 && i.child_region(0)->kind() == region_kind::spmd;

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
    });
}

} // namespace tinytc
