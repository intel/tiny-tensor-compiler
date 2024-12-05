// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/dump_matrix_ext.hpp"
#include "analysis/matrix_ext.hpp"
#include "node/data_type_node.hpp"
#include "node/inst_node.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "pass/dump_ir.hpp"
#include "support/casting.hpp"
#include "support/util.hpp"
#include "support/visit.hpp"
#include "support/walk.hpp"

#include <functional>
#include <ostream>
#include <string_view>
#include <vector>

namespace tinytc {

dump_matrix_ext_pass::dump_matrix_ext_pass(std::ostream &os, ::tinytc_core_info const *info)
    : os_(&os), info_{info} {}

void dump_matrix_ext_pass::run_on_function(function_node const &fn) {
    auto dump_ir = dump_ir_pass(*os_, 0);
    dump_ir.init_slot_tracker(fn);
    auto mext = matrix_ext_analysis{}.run_on_function(fn, *info_);

    *os_ << "Matrix extension in @" << fn.name() << std::endl;
    walk<walk_order::pre_order>(fn, [&](inst_node const &i) {
        if (i.num_results() > 0 || i.num_child_regions() > 0) {
            *os_ << "> ";
            visit(dump_ir, i);
            *os_ << std::endl;
            auto const dump_matrix_ext = [&](value_node const &v) {
                if (isa<coopmatrix_data_type>(*v.ty())) {
                    *os_ << "  matrix_ext(";
                    dump_ir.dump_val(v);
                    *os_ << ") = " << mext.get(v) << std::endl;
                }
            };
            for (auto &res : i.results()) {
                dump_matrix_ext(res);
            }
            for (auto &reg : i.child_regions()) {
                for (auto &p : reg.params()) {
                    dump_matrix_ext(p);
                }
            }
        }
    });
    *os_ << std::endl;
}

} // namespace tinytc
