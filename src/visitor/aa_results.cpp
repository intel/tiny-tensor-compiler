// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "visitor/aa_results.hpp"
#include "node/value_node.hpp"

#include <utility>

namespace tinytc {

aa_results::aa_results(std::unordered_map<value_node *, value_node *> alias)
    : alias_(std::move(alias)) {}
value_node *aa_results::root(value_node &a) {
    auto root = &a;
    if (alias_.find(root) != alias_.end()) {
        root = alias_[root];
    }
    return root;
}
bool aa_results::alias(value_node &a, value_node &b) { return root(a) == root(b); }

} // namespace tinytc
