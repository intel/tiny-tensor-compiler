// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "visitor/alias_analysis.hpp"
#include "error.hpp"
#include "node/data_type_node.hpp"
#include "tinytc/tinytc.hpp"

#include <clir/visit.hpp>

#include <vector>

using clir::visit;

namespace tinytc {

/* Stmt nodes */
void alias_analyser::operator()(inst_node &) {}
void alias_analyser::operator()(alloca_inst &a) {
    auto t = dynamic_cast<memref_data_type *>(a.result()->ty().get());
    if (t == nullptr) {
        throw compilation_error(a.loc(), status::ir_expected_memref);
    }
    allocs_[a.result().get()] =
        aa_results::allocation{a.stack_ptr(), a.stack_ptr() + t->size_in_bytes()};
}
void alias_analyser::operator()(loop_inst &p) { visit(*this, *p.body()); }
void alias_analyser::operator()(expand_inst &e) {
    value_node *source = e.operand().get();
    while (alias_.find(source) != alias_.end()) {
        source = alias_[source];
    }
    alias_[e.result().get()] = source;
}
void alias_analyser::operator()(fuse_inst &f) {
    value_node *source = f.operand().get();
    while (alias_.find(source) != alias_.end()) {
        source = alias_[source];
    }
    alias_[f.result().get()] = source;
}

void alias_analyser::operator()(if_inst &in) {
    visit(*this, *in.then());
    if (in.otherwise()) {
        visit(*this, *in.otherwise());
    }
}

void alias_analyser::operator()(subview_inst &s) {
    value_node *source = s.operand().get();
    while (alias_.find(source) != alias_.end()) {
        source = alias_[source];
    }
    alias_[s.result().get()] = source;
}

/* Region nodes */
void alias_analyser::operator()(rgn &b) {
    for (auto &s : b.insts()) {
        visit(*this, *s);
    }
}

/* Function nodes */
void alias_analyser::operator()(prototype &) {}
void alias_analyser::operator()(function &fn) {
    alias_.clear();
    visit(*this, *fn.prototype());
    visit(*this, *fn.body());
}

aa_results alias_analyser::get_result() const { return aa_results(alias_, allocs_); }

} // namespace tinytc
