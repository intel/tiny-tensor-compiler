// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CFG_20240919_HPP
#define CFG_20240919_HPP

#include "node/inst_node.hpp"
#include "node/region_node.hpp"
#include "support/util.hpp"

#include <algorithm>
#include <queue>
#include <unordered_map>
#include <utility>
#include <vector>

namespace tinytc {

class control_flow_graph {
  public:
    inline void add_node(inst_node *a, region_kind kind_max) {
        adj_[a] = adjacency_list{};
        adj_[a].kind_max = kind_max;
    }
    inline void add_edge(inst_node *a, inst_node *b) {
        adj_[a].succ.push_back(b);
        adj_[b].pred.push_back(a);
    }
    void insert_before(inst_node *before_inst, inst_node *new_inst);

    auto node_queue() const -> std::queue<inst_node *>;

    inline auto kind_max(inst_node *a) -> region_kind { return adj_[a].kind_max; }

    inline auto pred_begin(inst_node *a) { return adj_[a].pred.begin(); }
    inline auto pred_end(inst_node *a) { return adj_[a].pred.end(); }
    inline auto
    predecessors(inst_node *a) -> iterator_range_wrapper<std::vector<inst_node *>::iterator> {
        return {pred_begin(a), pred_end(a)};
    }

    inline auto succ_begin(inst_node *a) { return adj_[a].succ.begin(); }
    inline auto succ_end(inst_node *a) { return adj_[a].succ.end(); }
    inline auto
    successors(inst_node *a) -> iterator_range_wrapper<std::vector<inst_node *>::iterator> {
        return {succ_begin(a), succ_end(a)};
    }

  private:
    struct adjacency_list {
        region_kind kind_max = region_kind::mixed;
        std::vector<inst_node *> pred;
        std::vector<inst_node *> succ;
    };
    std::unordered_map<inst_node *, adjacency_list> adj_;
};

auto get_control_flow_graph(region_node &reg) -> control_flow_graph;

} // namespace tinytc

#endif // CFG_20240919_HPP
