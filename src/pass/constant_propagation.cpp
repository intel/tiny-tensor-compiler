// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/constant_propagation.hpp"
#include "node/function_node.hpp"
#include "node/inst_node.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "pass/constant_folding.hpp"
#include "support/ilist.hpp"
#include "support/ilist_base.hpp"
#include "support/visit.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <variant>

namespace tinytc {

void constant_propagation_pass::run_on_function(function_node &fn) { run_on_region(fn.body()); }

void constant_propagation_pass::run_on_region(region_node &reg) {
    for (auto it = reg.begin(); it != reg.end(); ++it) {
        for (auto &subreg : it->child_regions()) {
            run_on_region(subreg);
        }

        const auto update_uses = [&it](tinytc_value_t with) {
            if (it->num_results() != 1) {
                throw status::internal_compiler_error;
            }
            auto r = it->result_begin();
            auto u = r->use_begin();
            while (r->has_uses()) {
                u->set(with);
                u = r->use_begin();
            }
            if (r->has_uses()) {
                throw status::internal_compiler_error;
            }
        };

        fold_result fr = visit(constant_folding{unsafe_fp_math_}, *it);
        std::visit(overloaded{[&](tinytc_value_t val) {
                                  if (val) {
                                      update_uses(val);
                                  }
                              },
                              [&](inst &new_constant) {
                                  if (new_constant) {
                                      if (new_constant->num_results() != 1) {
                                          throw status::internal_compiler_error;
                                      }
                                      update_uses(&*new_constant->result_begin());
                                      // insert new constant
                                      it = reg.insts().insert(it, new_constant.release());
                                      // skip over constant
                                      ++it;
                                  }
                              }},
                   fr);
    }
}

void constant_propagation_pass::set_opt_flag(tinytc::optflag flag, bool enabled) {
    if (flag == tinytc::optflag::unsafe_fp_math) {
        unsafe_fp_math_ = enabled;
    }
}

} // namespace tinytc
