// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "analysis/alias.hpp"
#include "error.hpp"
#include "node/data_type_node.hpp"
#include "node/inst_node.hpp"
#include "node/value_node.hpp"
#include "support/casting.hpp"
#include "support/visit.hpp"
#include "support/walk.hpp"
#include "tinytc/types.hpp"

#include <functional>
#include <unordered_map>
#include <utility>

namespace tinytc {

class alias_analysis_visitor {
  public:
    void operator()(inst_node const &);
    void operator()(alloca_inst const &a);
    void operator()(expand_inst const &e);
    void operator()(fuse_inst const &f);
    void operator()(subview_inst const &s);

    auto get_result() && -> aa_results { return aa_results(std::move(alias_), std::move(allocs_)); }

  private:
    std::unordered_map<value_node const *, aa_results::allocation> allocs_;
    std::unordered_map<value_node const *, value_node const *> alias_;
};

void alias_analysis_visitor::operator()(inst_node const &) {}
void alias_analysis_visitor::operator()(alloca_inst const &a) {
    if (a.stack_ptr() >= 0) {
        auto t = dyn_cast<memref_data_type>(a.result()->ty());
        if (t == nullptr) {
            throw compilation_error(a.loc(), status::ir_expected_memref);
        }
        allocs_[a.result()] =
            aa_results::allocation{a.stack_ptr(), a.stack_ptr() + t->size_in_bytes()};
    }
}
void alias_analysis_visitor::operator()(expand_inst const &e) {
    value_node const *source = &e.operand();
    while (alias_.find(source) != alias_.end()) {
        source = alias_[source];
    }
    alias_[e.result()] = source;
}
void alias_analysis_visitor::operator()(fuse_inst const &f) {
    value_node const *source = &f.operand();
    while (alias_.find(source) != alias_.end()) {
        source = alias_[source];
    }
    alias_[f.result()] = source;
}

void alias_analysis_visitor::operator()(subview_inst const &s) {
    value_node const *source = &s.operand();
    while (alias_.find(source) != alias_.end()) {
        source = alias_[source];
    }
    alias_[s.result()] = source;
}

auto alias_analysis::run_on_function(function_node &fn) -> aa_results {
    auto visitor = alias_analysis_visitor{};

    walk<walk_order::pre_order>(fn, [&visitor](inst_node &i) { visit(visitor, i); });

    return std::move(visitor).get_result();
}

} // namespace tinytc
