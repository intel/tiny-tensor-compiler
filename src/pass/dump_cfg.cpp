// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/dump_cfg.hpp"
#include "analysis/cfg.hpp"
#include "pass/dump_ir.hpp"
#include "util/iterator.hpp"

#include <cstdint>
#include <ostream>
#include <queue>
#include <string_view>
#include <vector>

namespace tinytc {

dump_cfg_pass::dump_cfg_pass(std::ostream &os) : os_(&os) {}

void dump_cfg_pass::run_on_function(function_node &fn) {
    auto dump_ir = dump_ir_pass(*os_, 0);

    *os_ << "digraph " << fn.name() << " {" << std::endl;

    auto cfg = get_control_flow_graph(fn.body());
    auto q = cfg.node_queue();
    for (; !q.empty(); q.pop()) {
        auto &node = q.front();

        *os_ << reinterpret_cast<uintptr_t>(node) << " [label=\"";
        dump_ir.run_on_instruction(*node);
        *os_ << "\"]" << std::endl;

        for (auto &neigh : cfg.successors(node)) {
            *os_ << reinterpret_cast<uintptr_t>(node) << " -> "
                 << reinterpret_cast<uintptr_t>(neigh) << std::endl;
        }
    }

    *os_ << "}" << std::endl;
}

} // namespace tinytc
