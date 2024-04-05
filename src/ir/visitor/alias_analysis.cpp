// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/ir/visitor/alias_analysis.hpp"
#include "tinytc/ir/func.hpp"
#include "tinytc/ir/inst.hpp"
#include "tinytc/ir/region.hpp"
#include "tinytc/ir/value.hpp"

#include <clir/handle.hpp>
#include <clir/visit.hpp>

#include <vector>

using clir::visit;

namespace tinytc::ir::internal {

/* Stmt nodes */
void alias_analyser::operator()(inst_node &) {}
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

aa_results alias_analyser::get_result() const { return aa_results(alias_); }

} // namespace tinytc::ir::internal
