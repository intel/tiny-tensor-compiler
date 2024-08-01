// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "visitor/slot_tracker.hpp"
#include "tinytc/tinytc.hpp"

#include <clir/visit.hpp>

#include <utility>
#include <vector>

using clir::visit;

namespace tinytc {

void slot_tracker::set_slot(value_node const &v) {
    if (!v.has_name()) {
        slot_map_[&v] = slot_++;
    }
}

/* Stmt nodes */
void slot_tracker::operator()(inst_node const &in) {
    for (auto const &result : in.results()) {
        set_slot(*result);
    }
}
void slot_tracker::operator()(loop_inst const &p) {
    set_slot(*p.loop_var());
    return visit(*this, *p.body());
}

void slot_tracker::operator()(if_inst const &in) {
    visit(*this, *in.then());
    if (in.otherwise()) {
        visit(*this, *in.otherwise());
    }
}

void slot_tracker::operator()(parallel_inst const &p) { return visit(*this, *p.body()); }

/* Region nodes */
void slot_tracker::operator()(rgn const &b) {
    for (auto const &s : b.insts()) {
        visit(*this, *s);
    }
}

/* Function nodes */
void slot_tracker::operator()(prototype const &p) {
    for (auto const &arg : p.args()) {
        set_slot(*arg);
    }
}

void slot_tracker::operator()(function const &fn) {
    slot_ = 0;
    visit(*this, *fn.prototype());
    visit(*this, *fn.body());
}

/* Program nodes */
void slot_tracker::operator()(program const &p) {
    for (auto const &s : p.declarations()) {
        visit(*this, *s);
    }
}

auto slot_tracker::get_slot(value_node const &v) -> std::int64_t {
    auto it = slot_map_.find(&v);
    return it != slot_map_.end() ? it->second : -1;
}

} // namespace tinytc
