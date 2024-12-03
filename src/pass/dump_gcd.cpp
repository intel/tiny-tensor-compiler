// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/dump_gcd.hpp"
#include "analysis/gcd.hpp"
#include "node/inst_node.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "pass/dump_ir.hpp"
#include "support/util.hpp"
#include "support/visit.hpp"
#include "support/walk.hpp"

#include <functional>
#include <optional>
#include <ostream>
#include <string_view>
#include <vector>

namespace tinytc {

dump_gcd_pass::dump_gcd_pass(std::ostream &os) : os_(&os) {}

void dump_gcd_pass::run_on_function(function_node const &fn) {
    auto dump_ir = dump_ir_pass(*os_, 0);
    dump_ir.init_slot_tracker(fn);
    auto gcd = gcd_analysis{}.run_on_function(fn);

    *os_ << "GCD in @" << fn.name() << std::endl;
    walk<walk_order::pre_order>(fn, [&](inst_node const &i) {
        if (i.num_results() > 0 || i.num_child_regions() > 0) {
            *os_ << "> ";
            visit(dump_ir, i);
            *os_ << std::endl;
            auto const dump_gcd = [&](value_node const &v) {
                auto g = gcd.get_if(v);
                if (g) {
                    *os_ << "  gcd(";
                    dump_ir.dump_val(v);
                    *os_ << ") = " << *g << std::endl;
                }
            };
            for (auto &res : i.results()) {
                dump_gcd(res);
            }
            for (auto &reg : i.child_regions()) {
                for (auto &p : reg.params()) {
                    dump_gcd(p);
                }
            }
        }
    });
    *os_ << std::endl;
}

} // namespace tinytc
