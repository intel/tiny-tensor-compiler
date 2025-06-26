// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/insert_lifetime_stop.hpp"
#include "analysis/aa_results.hpp"
#include "analysis/alias.hpp"
#include "node/data_type_node.hpp"
#include "node/inst_node.hpp"
#include "node/inst_view.hpp"
#include "node/value_node.hpp"
#include "tinytc/types.h"
#include "util/casting.hpp"
#include "util/ilist.hpp"
#include "util/ilist_base.hpp"
#include "util/iterator.hpp"

#include <iterator>
#include <vector>

namespace tinytc {

auto insert_lifetime_stop_pass::run_on_region(tinytc_region &reg, aa_results const &aa)
    -> std::unordered_set<const_tinytc_value_t> {
    if (reg.empty()) {
        return {};
    }

    auto allocas = std::vector<tinytc_value_t>{};
    for (auto &i : reg) {
        if (auto alloca = dyn_cast<alloca_inst>(&i); alloca) {
            allocas.emplace_back(&alloca.result());
        }
    }

    auto rgn_ops = std::unordered_set<const_tinytc_value_t>{};
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
                prev_it =
                    reg.insts().insert_after(prev_it, lifetime_stop_inst::create(*alloca_it, {}));
                --prev_it;
                alloca_it = allocas.erase(alloca_it);
            } else {
                ++alloca_it;
            }
        }
    }
    return rgn_ops;
}

void insert_lifetime_stop_pass::run_on_function(tinytc_func &fn) {
    auto aa = alias_analysis{}.run_on_function(fn);
    run_on_region(fn.body(), aa);
}

} // namespace tinytc
