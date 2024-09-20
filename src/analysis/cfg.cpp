// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "analysis/cfg.hpp"
#include "node/inst_node.hpp"
#include "support/casting.hpp"
#include "support/visit.hpp"

namespace tinytc {

auto control_flow_graph::node_queue() const -> std::queue<inst_node *> {
    auto q = std::queue<inst_node *>{};
    for (auto &[key, neighbors] : adj_) {
        q.push(key);
    }
    return q;
}

auto get_control_flow_graph(region_node &topreg) -> control_flow_graph {
    auto cfg = control_flow_graph{};

    const auto add_region = [&cfg](region_node &reg,
                                   auto &add_region_ref) -> std::pair<inst_node *, inst_node *> {
        if (reg.empty()) {
            return {};
        }

        auto start = reg.begin()->get();
        cfg.add_node(start);

        auto pred_nodes = std::queue<inst_node *>{};
        pred_nodes.push(start);

        for (auto it = reg.begin() + 1; it != reg.end(); ++it) {
            inst_node *node = it->get();
            cfg.add_node(node);

            for (; !pred_nodes.empty(); pred_nodes.pop()) {
                cfg.add_edge(pred_nodes.front(), node);
            }

            if ((*it)->num_child_regions() > 0) {
                for (auto &subreg : (*it)->child_regions()) {
                    auto [substart, subexit] = add_region_ref(*subreg, add_region_ref);
                    cfg.add_edge(node, substart);
                    if (isa<loop_inst>(**it)) {
                        cfg.add_edge(subexit, node);
                        pred_nodes.push(node);
                    } else {
                        pred_nodes.push(subexit);
                    }
                }
            } else {
                pred_nodes.push(node);
            }
        }

        // every region must have exactly one exit node and the exit node must be last
        // @todo: NOT guaranteed for parallel_inst and function yet!
        if (pred_nodes.size() != 1) {
            throw internal_compiler_error{};
        }

        return std::make_pair(std::move(start), std::move(pred_nodes.front()));
    };

    add_region(topreg, add_region);

    return cfg;
}

} // namespace tinytc
