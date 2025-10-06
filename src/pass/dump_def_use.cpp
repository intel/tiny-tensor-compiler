// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/dump_def_use.hpp"
#include "node/func.hpp"
#include "node/inst.hpp"
#include "node/region.hpp"
#include "node/value.hpp"
#include "node/visit.hpp"
#include "pass/dump_ir.hpp"
#include "support/walk.hpp"
#include "util/iterator.hpp"

#include <functional>
#include <iterator>
#include <ostream>
#include <string_view>
#include <vector>

namespace tinytc {

dump_def_use_pass::dump_def_use_pass(std::ostream &os) : os_(&os) {}

void dump_def_use_pass::run_on_function(tinytc_func &fn) {
    auto dump_ir = dump_ir_pass(*os_, 0);
    dump_ir.init_slot_tracker(fn);

    *os_ << "Def-use in @" << fn.name() << std::endl;
    walk<walk_order::pre_order>(fn, [&](tinytc_inst &i) {
        if (i.num_results() > 0 || i.num_child_regions() > 0) {
            *os_ << "> ";
            visit(dump_ir, i);
            *os_ << std::endl;
            auto const def_use = [&](tinytc_value const &v) {
                *os_ << "  def ";
                dump_ir.dump_val(v);
                *os_ << std::endl;
                for (auto &u : v.uses()) {
                    *os_ << "    > ";
                    visit(dump_ir, *u.owner());
                    *os_ << std::endl;
                }
            };
            for (auto &res : i.results()) {
                def_use(res);
            }
            for (auto &reg : i.child_regions()) {
                for (auto &p : reg.params()) {
                    def_use(p);
                }
            }
        }
    });
    *os_ << std::endl;
}

} // namespace tinytc
