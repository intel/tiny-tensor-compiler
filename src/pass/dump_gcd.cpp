// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/dump_gcd.hpp"
#include "analysis/gcd.hpp"
#include "device_info.hpp"
#include "node/inst_node.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "node/visit.hpp"
#include "pass/dump_ir.hpp"
#include "support/walk.hpp"
#include "util/iterator.hpp"

#include <functional>
#include <iterator>
#include <optional>
#include <ostream>
#include <string_view>
#include <vector>

namespace tinytc {

dump_gcd_pass::dump_gcd_pass(std::ostream &os, ::tinytc_core_info const *info)
    : os_(&os), info_{info} {}

void dump_gcd_pass::run_on_function(tinytc_func &fn) {
    auto dump_ir = dump_ir_pass(*os_, 0);
    dump_ir.init_slot_tracker(fn);
    auto gcd = gcd_analysis{info_->alignment()}.run_on_function(fn);

    auto const dump_range = [&](auto begin, auto end) {
        *os_ << "[";
        for (auto it = begin; it != end; ++it) {
            if (it != begin) {
                *os_ << ",";
            }
            *os_ << *it;
        }
        *os_ << "]";
    };
    auto const dump_gcd = [&](tinytc_value const &v) {
        auto g = gcd.get_if(v);
        if (g) {
            *os_ << "  gcd(";
            dump_ir.dump_val(v);
            *os_ << ") = " << *g << std::endl;
        }
        auto mi = gcd.get_memref_if(v);
        if (mi) {
            *os_ << "  offset_gcd(";
            dump_ir.dump_val(v);
            *os_ << ") = " << mi->offset_gcd() << std::endl;
            *os_ << "  shape_gcd(";
            dump_ir.dump_val(v);
            *os_ << ") = ";
            dump_range(mi->shape_gcd_begin(), mi->shape_gcd_end());
            *os_ << std::endl << "  stride_gcd(";
            dump_ir.dump_val(v);
            *os_ << ") = ";
            dump_range(mi->stride_gcd_begin(), mi->stride_gcd_end());
            *os_ << std::endl;
        }
    };

    *os_ << "GCD in @" << fn.name() << std::endl;
    for (auto &p : fn.params()) {
        dump_gcd(p);
    }
    walk<walk_order::pre_order>(fn, [&](tinytc_inst &i) {
        if (i.num_results() > 0 || i.num_child_regions() > 0) {
            *os_ << "> ";
            visit(dump_ir, i);
            *os_ << std::endl;
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
