// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/slot_tracker.hpp"
#include "node/inst_node.hpp"
#include "support/walk.hpp"
#include "tinytc/tinytc.hpp"

#include <utility>

namespace tinytc {

void slot_tracker::set_slot(value_node const &v) {
    if (!v.has_name()) {
        slot_map_[&v] = slot_++;
    }
}

void slot_tracker::run_on_function(function_node &fn) {
    slot_ = 0;
    for (auto const &arg : fn.params()) {
        set_slot(*arg);
    }
    walk<walk_order::pre_order>(fn, [this](inst_node const &i) {
        for (auto const &result : i.results()) {
            set_slot(*result);
        }
    });
}

auto slot_tracker::get_slot(value_node const &v) -> std::int64_t {
    auto it = slot_map_.find(&v);
    return it != slot_map_.end() ? it->second : -1;
}

} // namespace tinytc
