// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/insert_lifetime_stop.hpp"
#include "analysis/aa_results.hpp"
#include "analysis/alias.hpp"
#include "node/data_type_node.hpp"
#include "node/inst_node.hpp"
#include "node/value_node.hpp"
#include "tinytc/types.h"
#include "util/casting.hpp"
#include "util/ilist.hpp"
#include "util/ilist_base.hpp"
#include "util/iterator.hpp"

#include <memory>
#include <vector>

namespace tinytc {

auto insert_lifetime_stop_pass::run_on_region(region_node &reg, aa_results const &aa)
    -> std::unordered_set<value_node const *> {
    if (reg.empty()) {
        return {};
    }

    auto allocas = std::vector<tinytc_value_t>{};
    for (auto &i : reg) {
        if (auto alloca = dyn_cast<alloca_inst>(&i); alloca != nullptr) {
            allocas.emplace_back(&alloca->result(0));
        }
    }

    auto rgn_ops = std::unordered_set<value_node const *>{};
    auto prev_it = reg.end();
    while (prev_it != reg.begin()) {
        auto &i = *(--prev_it);
        for (auto &subreg : i.child_regions()) {
            rgn_ops.merge(run_on_region(subreg, aa));
        }
        for (auto &v : i.operands()) {
            if (isa<memref_data_type>(*v.ty())) {
                rgn_ops.insert(aa.root(v));
            }
        }
        for (auto &v : i.results()) {
            if (isa<memref_data_type>(*v.ty())) {
                rgn_ops.insert(aa.root(v));
            }
        }

        auto alloca_it = allocas.begin();
        while (alloca_it != allocas.end()) {
            if (rgn_ops.contains(*alloca_it)) {
                prev_it = reg.insts().insert_after(
                    prev_it, std::make_unique<lifetime_stop_inst>(*alloca_it).release());
                --prev_it;
                alloca_it = allocas.erase(alloca_it);
            } else {
                ++alloca_it;
            }
        }
    }
    return rgn_ops;
}

void insert_lifetime_stop_pass::run_on_function(function_node &fn) {
    auto aa = alias_analysis{}.run_on_function(fn);
    run_on_region(fn.body(), aa);
}

} // namespace tinytc
