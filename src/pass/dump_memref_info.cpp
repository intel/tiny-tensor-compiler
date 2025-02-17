// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/dump_memref_info.hpp"
#include "analysis/memref.hpp"
#include "device_info.hpp"
#include "node/inst_node.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "pass/dump_ir.hpp"
#include "support/util.hpp"
#include "support/visit.hpp"
#include "support/walk.hpp"

#include <functional>
#include <ostream>
#include <string_view>
#include <vector>

namespace tinytc {

dump_memref_info_pass::dump_memref_info_pass(std::ostream &os, ::tinytc_core_info const *info)
    : os_(&os), info_{info} {}

void dump_memref_info_pass::run_on_function(function_node const &fn) {
    auto dump_ir = dump_ir_pass(*os_, 0);
    dump_ir.init_slot_tracker(fn);
    auto mr = memref_analysis{info_->alignment()}.run_on_function(fn);

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
    auto const dump_memref_info = [&](value_node const &v) {
        auto m = mr.get_if(v);
        if (m) {
            *os_ << "  alignment(";
            dump_ir.dump_val(v);
            *os_ << ") = " << m->alignment() << std::endl;
            *os_ << "  shape_gcd(";
            dump_ir.dump_val(v);
            *os_ << ") = ";
            dump_range(m->shape_gcd_begin(), m->shape_gcd_end());
            *os_ << std::endl << "  stride_gcd(";
            dump_ir.dump_val(v);
            *os_ << ") = ";
            dump_range(m->stride_gcd_begin(), m->stride_gcd_end());
            *os_ << std::endl;
        }
    };

    *os_ << "Memref info @" << fn.name() << std::endl;
    for (auto &p : fn.params()) {
        dump_memref_info(p);
    }
    walk<walk_order::pre_order>(fn, [&](inst_node const &i) {
        if (i.num_results() > 0 || i.num_child_regions() > 0) {
            *os_ << "> ";
            visit(dump_ir, i);
            *os_ << std::endl;
            for (auto &res : i.results()) {
                dump_memref_info(res);
            }
            for (auto &reg : i.child_regions()) {
                for (auto &p : reg.params()) {
                    dump_memref_info(p);
                }
            }
        }
    });
    *os_ << std::endl;
}

} // namespace tinytc
