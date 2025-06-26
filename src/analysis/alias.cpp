// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "analysis/alias.hpp"
#include "error.hpp"
#include "node/data_type_node.hpp"
#include "node/inst_node.hpp"
#include "node/inst_view.hpp"
#include "node/value_node.hpp"
#include "node/visit.hpp"
#include "support/walk.hpp"
#include "tinytc/types.hpp"
#include "util/casting.hpp"

#include <functional>
#include <unordered_map>
#include <utility>

namespace tinytc {

class alias_analysis_visitor {
  public:
    void operator()(inst_view);
    void operator()(alloca_inst a);
    void operator()(expand_inst e);
    void operator()(fuse_inst f);
    void operator()(subview_inst s);

    auto get_result() && -> aa_results { return aa_results(std::move(alias_), std::move(allocs_)); }

  private:
    std::unordered_map<const_tinytc_value_t, aa_results::allocation> allocs_;
    std::unordered_map<const_tinytc_value_t, const_tinytc_value_t> alias_;
};

void alias_analysis_visitor::operator()(inst_view) {}
void alias_analysis_visitor::operator()(alloca_inst a) {
    if (a.stack_ptr() >= 0) {
        auto t = dyn_cast<memref_data_type>(a.result().ty());
        if (t == nullptr) {
            throw compilation_error(a.loc(), status::ir_expected_memref);
        }
        allocs_[&a.result()] =
            aa_results::allocation{a.stack_ptr(), a.stack_ptr() + t->size_in_bytes()};
    }
}
void alias_analysis_visitor::operator()(expand_inst e) {
    const_tinytc_value_t source = &e.operand();
    while (alias_.find(source) != alias_.end()) {
        source = alias_[source];
    }
    alias_[&e.result()] = source;
}
void alias_analysis_visitor::operator()(fuse_inst f) {
    const_tinytc_value_t source = &f.operand();
    while (alias_.find(source) != alias_.end()) {
        source = alias_[source];
    }
    alias_[&f.result()] = source;
}

void alias_analysis_visitor::operator()(subview_inst s) {
    const_tinytc_value_t source = &s.operand();
    while (alias_.find(source) != alias_.end()) {
        source = alias_[source];
    }
    alias_[&s.result()] = source;
}

auto alias_analysis::run_on_function(tinytc_func &fn) -> aa_results {
    auto visitor = alias_analysis_visitor{};

    walk<walk_order::pre_order>(fn, [&visitor](tinytc_inst &i) { visit(visitor, i); });

    return std::move(visitor).get_result();
}

} // namespace tinytc
