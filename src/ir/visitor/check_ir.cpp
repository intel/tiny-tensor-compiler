// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "ir/visitor/check_ir.hpp"
#include "tinytc/ir/func.hpp"

#include <clir/handle.hpp>
#include <clir/visit.hpp>

#include <functional>
#include <utility>
#include <vector>

using clir::visit;

namespace tinytc {

ir_checker::ir_checker(error_reporter_function reporter) : reporter_(std::move(reporter)) {}

/* Stmt nodes */
bool ir_checker::operator()(inst_node &in) {
    bool ok = in.kind() != inst_kind::collective || !inside_spmd_region_;
    if (!ok) {
        reporter_(in.loc(), "Collective instruction must not be called from SPMD region");
    }
    return ok;
}
bool ir_checker::operator()(for_inst &p) { return visit(*this, *p.body()); }
bool ir_checker::operator()(foreach_inst &p) {
    bool ok = this->operator()(static_cast<inst_node &>(p));
    inside_spmd_region_ = true;
    ok = ok && visit(*this, *p.body());
    inside_spmd_region_ = false;
    return ok;
}
bool ir_checker::operator()(if_inst &in) {
    bool ok = visit(*this, *in.then());
    if (in.otherwise()) {
        ok = ok && visit(*this, *in.otherwise());
    }
    return ok;
}

/* Region nodes */
bool ir_checker::operator()(rgn &b) {
    bool ok = true;
    for (auto &s : b.insts()) {
        ok = ok && visit(*this, *s);
    }
    return ok;
}

/* Function nodes */
bool ir_checker::operator()(prototype &) { return true; }
bool ir_checker::operator()(function &fn) {
    bool ok = true;
    ok = ok && visit(*this, *fn.prototype());
    ok = ok && visit(*this, *fn.body());
    return ok;
}

/* Program nodes */
bool ir_checker::operator()(program &p) {
    bool ok = true;
    for (auto &s : p.declarations()) {
        ok = ok && visit(*this, *s);
    }
    return ok;
}

} // namespace tinytc
