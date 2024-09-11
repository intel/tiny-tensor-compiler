// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/alias_analysis.hpp"
#include "error.hpp"
#include "node/data_type_node.hpp"
#include "support/casting.hpp"
#include "support/visit.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <vector>

namespace tinytc {

/* Stmt nodes */
void alias_analyser::operator()(inst_node const &) {}
void alias_analyser::operator()(alloca_inst const &a) {
    auto t = dyn_cast<memref_data_type>(a.result()->ty().get());
    if (t == nullptr) {
        throw compilation_error(a.loc(), status::ir_expected_memref);
    }
    allocs_[a.result().get()] =
        aa_results::allocation{a.stack_ptr(), a.stack_ptr() + t->size_in_bytes()};
}
void alias_analyser::operator()(loop_inst const &p) { visit(*this, *p.body()); }
void alias_analyser::operator()(expand_inst const &e) {
    value_node const *source = e.operand().get();
    while (alias_.find(source) != alias_.end()) {
        source = alias_[source];
    }
    alias_[e.result().get()] = source;
}
void alias_analyser::operator()(fuse_inst const &f) {
    value_node const *source = f.operand().get();
    while (alias_.find(source) != alias_.end()) {
        source = alias_[source];
    }
    alias_[f.result().get()] = source;
}

void alias_analyser::operator()(if_inst const &in) {
    visit(*this, *in.then());
    if (in.otherwise()) {
        visit(*this, *in.otherwise());
    }
}

void alias_analyser::operator()(parallel_inst const &p) { visit(*this, *p.body()); }

void alias_analyser::operator()(subview_inst const &s) {
    value_node const *source = s.operand().get();
    while (alias_.find(source) != alias_.end()) {
        source = alias_[source];
    }
    alias_[s.result().get()] = source;
}

/* Region nodes */
void alias_analyser::operator()(rgn const &b) {
    for (auto &s : b.insts()) {
        visit(*this, *s);
    }
}

/* Function nodes */
void alias_analyser::operator()(prototype const &) {}
void alias_analyser::operator()(function const &fn) {
    alias_.clear();
    visit(*this, *fn.prototype());
    visit(*this, *fn.body());
}

aa_results alias_analyser::get_result() const { return aa_results(alias_, allocs_); }

} // namespace tinytc
