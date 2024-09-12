// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "analysis/aa_results.hpp"
#include "node/value_node.hpp"

#include <utility>

namespace tinytc {

aa_results::aa_results(std::unordered_map<value_node const *, value_node const *> alias,
                       std::unordered_map<value_node const *, allocation> allocs)
    : alias_(std::move(alias)), allocs_(std::move(allocs)) {}

auto aa_results::root(value_node const &a) const -> value_node const * {
    auto root = &a;
    if (auto it = alias_.find(root); it != alias_.end()) {
        root = it->second;
    }
    return root;
}
bool aa_results::alias(value_node const &a, value_node const &b) const {
    auto ra = root(a);
    auto rb = root(b);
    if (ra == rb) {
        return true;
    }
    auto stack_a = allocs_.find(ra);
    auto stack_b = allocs_.find(rb);
    if (stack_a != allocs_.end() && stack_b != allocs_.end()) {
        return stack_a->second.stop > stack_b->second.start &&
               stack_b->second.stop > stack_a->second.start;
    }
    return false;
}

} // namespace tinytc
