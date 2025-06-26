// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CFG_20240919_HPP
#define CFG_20240919_HPP

#include "node/inst_node.hpp"
#include "node/region_node.hpp"
#include "util/iterator.hpp"

#include <queue>
#include <unordered_map>
#include <vector>

namespace tinytc {

class control_flow_graph {
  public:
    inline void add_node(tinytc_inst_t a, region_kind kind_max) {
        adj_[a] = adjacency_list{};
        adj_[a].kind_max = kind_max;
    }
    inline void add_edge(tinytc_inst_t a, tinytc_inst_t b) {
        adj_[a].succ.push_back(b);
        adj_[b].pred.push_back(a);
    }
    void insert_before(tinytc_inst_t before_inst, tinytc_inst_t new_inst);

    auto node_queue() const -> std::queue<tinytc_inst_t>;

    inline auto kind_max(tinytc_inst_t a) -> region_kind { return adj_[a].kind_max; }

    inline auto pred_begin(tinytc_inst_t a) { return adj_[a].pred.begin(); }
    inline auto pred_end(tinytc_inst_t a) { return adj_[a].pred.end(); }
    inline auto predecessors(tinytc_inst_t a)
        -> iterator_range_wrapper<std::vector<tinytc_inst_t>::iterator> {
        return {pred_begin(a), pred_end(a)};
    }

    inline auto succ_begin(tinytc_inst_t a) { return adj_[a].succ.begin(); }
    inline auto succ_end(tinytc_inst_t a) { return adj_[a].succ.end(); }
    inline auto successors(tinytc_inst_t a)
        -> iterator_range_wrapper<std::vector<tinytc_inst_t>::iterator> {
        return {succ_begin(a), succ_end(a)};
    }

  private:
    struct adjacency_list {
        region_kind kind_max = region_kind::mixed;
        std::vector<tinytc_inst_t> pred;
        std::vector<tinytc_inst_t> succ;
    };
    std::unordered_map<tinytc_inst_t, adjacency_list> adj_;
};

auto get_control_flow_graph(tinytc_region &reg) -> control_flow_graph;

} // namespace tinytc

#endif // CFG_20240919_HPP
