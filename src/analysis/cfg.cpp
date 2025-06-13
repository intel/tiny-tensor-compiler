// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "analysis/cfg.hpp"
#include "node/inst_node.hpp"
#include "util/casting.hpp"
#include "util/ilist_base.hpp"

#include <algorithm>
#include <utility>

namespace tinytc {

void control_flow_graph::insert_before(inst_node *before_inst, inst_node *new_inst) {
    add_node(new_inst, adj_[before_inst].kind_max);
    adj_[new_inst].pred = std::move(adj_[before_inst].pred);
    add_edge(new_inst, before_inst);
}

auto control_flow_graph::node_queue() const -> std::queue<inst_node *> {
    auto q = std::queue<inst_node *>{};
    for (auto &[key, neighbors] : adj_) {
        q.push(key);
    }
    return q;
}

auto get_control_flow_graph(region_node &topreg) -> control_flow_graph {
    auto cfg = control_flow_graph{};

    const auto add_region =
        [&cfg](region_node &reg, region_kind kind_max,
               auto &add_region_ref) -> std::pair<inst_node *, std::queue<inst_node *>> {
        if (reg.empty()) {
            return {};
        }

        auto pred_nodes = std::queue<inst_node *>{};
        const auto visit_inst = [&](inst_node *node) {
            bool empty_child_regions = true;
            if (node->num_child_regions() > 0) {
                for (auto &subreg : node->child_regions()) {
                    auto [substart, subexits] =
                        add_region_ref(subreg, std::max(kind_max, subreg.kind()), add_region_ref);
                    if (substart != nullptr && !subexits.empty()) {
                        empty_child_regions = false;
                        cfg.add_edge(node, substart);
                        if (isa<loop_inst>(*node)) {
                            for (; !subexits.empty(); subexits.pop()) {
                                cfg.add_edge(subexits.front(), node);
                            }
                            pred_nodes.push(node);
                        } else {
                            for (; !subexits.empty(); subexits.pop()) {
                                pred_nodes.push(subexits.front());
                            }
                        }
                    }
                }
            }
            if (empty_child_regions) {
                pred_nodes.push(node);
            }
        };

        auto start = reg.begin().get();
        cfg.add_node(start, kind_max);
        visit_inst(start);

        for (auto it = ++reg.begin(); it != reg.end(); ++it) {
            inst_node *node = it.get();
            cfg.add_node(node, kind_max);

            for (; !pred_nodes.empty(); pred_nodes.pop()) {
                cfg.add_edge(pred_nodes.front(), node);
            }

            visit_inst(node);
        }

        return std::make_pair(std::move(start), std::move(pred_nodes));
    };

    add_region(topreg, topreg.kind(), add_region);

    return cfg;
}

} // namespace tinytc
