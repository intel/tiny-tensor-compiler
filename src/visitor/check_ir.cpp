// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "visitor/check_ir.hpp"
#include "error.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <clir/visit.hpp>

#include <vector>

using clir::visit;

namespace tinytc {

/* Stmt nodes */
void ir_checker::operator()(inst_node const &in) {
    bool ok = in.kind() != inst_kind::collective || !inside_spmd_region_;
    if (!ok) {
        throw compilation_error(in.loc(), status::ir_collective_called_from_spmd);
    }
}
void ir_checker::operator()(for_inst const &p) { return visit(*this, *p.body()); }
void ir_checker::operator()(foreach_inst const &p) {
    this->operator()(static_cast<inst_node const &>(p));
    inside_spmd_region_ = true;
    visit(*this, *p.body());
    inside_spmd_region_ = false;
}
void ir_checker::operator()(if_inst const &in) {
    visit(*this, *in.then());
    if (in.otherwise()) {
        visit(*this, *in.otherwise());
    }
}

/* Region nodes */
void ir_checker::operator()(rgn const &b) {
    for (auto const &s : b.insts()) {
        visit(*this, *s);
    }
}

/* Function nodes */
void ir_checker::operator()(prototype const &) {}
void ir_checker::operator()(function const &fn) {
    visit(*this, *fn.prototype());
    visit(*this, *fn.body());
}

/* Program nodes */
void ir_checker::operator()(program const &p) {
    for (auto const &s : p.declarations()) {
        visit(*this, *s);
    }
}

} // namespace tinytc
