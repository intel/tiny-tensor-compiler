// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "visitor/check_ir.hpp"
#include "error.hpp"

#include <clir/handle.hpp>
#include <clir/visit.hpp>

#include <functional>
#include <utility>
#include <vector>

using clir::visit;

namespace tinytc {

/* Stmt nodes */
void ir_checker::operator()(inst_node &in) {
    bool ok = in.kind() != inst_kind::collective || !inside_spmd_region_;
    if (!ok) {
        throw compilation_error(in.loc(), status::ir_collective_called_from_spmd);
    }
}
void ir_checker::operator()(for_inst &p) { return visit(*this, *p.body()); }
void ir_checker::operator()(foreach_inst &p) {
    this->operator()(static_cast<inst_node &>(p));
    inside_spmd_region_ = true;
    visit(*this, *p.body());
    inside_spmd_region_ = false;
}
void ir_checker::operator()(if_inst &in) {
    visit(*this, *in.then());
    if (in.otherwise()) {
        visit(*this, *in.otherwise());
    }
}

/* Region nodes */
void ir_checker::operator()(rgn &b) {
    for (auto &s : b.insts()) {
        visit(*this, *s);
    }
}

/* Function nodes */
void ir_checker::operator()(prototype &) {}
void ir_checker::operator()(function &fn) {
    visit(*this, *fn.prototype());
    visit(*this, *fn.body());
}

/* Program nodes */
void ir_checker::operator()(program &p) {
    for (auto &s : p.declarations()) {
        visit(*this, *s);
    }
}

} // namespace tinytc
