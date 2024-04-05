// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/ir/visitor/aa_results.hpp"
#include "tinytc/ir/internal/value_node.hpp"

#include <utility>

namespace tinytc::ir::internal {

aa_results::aa_results(std::unordered_map<internal::value_node *, internal::value_node *> alias)
    : alias_(std::move(alias)) {}
internal::value_node *aa_results::root(internal::value_node &a) {
    auto root = &a;
    if (alias_.find(root) != alias_.end()) {
        root = alias_[root];
    }
    return root;
}
bool aa_results::alias(internal::value_node &a, internal::value_node &b) {
    return root(a) == root(b);
}

} // namespace tinytc::ir::internal
