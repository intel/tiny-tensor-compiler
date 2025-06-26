// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "analysis/aa_results.hpp"
#include "node/value.hpp"

#include <utility>

namespace tinytc {

aa_results::aa_results(std::unordered_map<const_tinytc_value_t, const_tinytc_value_t> alias,
                       std::unordered_map<const_tinytc_value_t, allocation> allocs)
    : alias_(std::move(alias)), allocs_(std::move(allocs)) {}

auto aa_results::root(tinytc_value const &a) const -> const_tinytc_value_t {
    auto root = &a;
    if (auto it = alias_.find(root); it != alias_.end()) {
        root = it->second;
    }
    return root;
}
bool aa_results::alias(tinytc_value const &a, tinytc_value const &b) const {
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
