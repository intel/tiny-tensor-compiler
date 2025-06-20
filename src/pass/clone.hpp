// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CLONE_20241118_HPP
#define CLONE_20241118_HPP

#include "node/inst_node.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"

#include <unordered_map>
#include <vector>

namespace tinytc {

class inst_cloner {
  public:
    void reset_subs();
    void set_subs(tinytc_value_t in_val, tinytc_value_t out_val);
    auto subs(tinytc_value_t val) -> tinytc_value_t;

    auto clone_instruction(inst_node &in) -> inst;
    void clone_region(region_node &source, region_node &target);

  private:
    template <typename T> auto subs_value_range(T &&range) {
        auto vec = std::vector<tinytc_value_t>();
        vec.reserve(range.size());
        for (auto &r : range) {
            vec.emplace_back(subs(&r));
        }
        return vec;
    }

    std::unordered_map<tinytc_value_t, tinytc_value_t> subs_map_;
};

} // namespace tinytc

#endif // CLONE_20241118_HPP
