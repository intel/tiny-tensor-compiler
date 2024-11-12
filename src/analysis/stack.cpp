// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "analysis/stack.hpp"
#include "node/data_type_node.hpp"
#include "node/inst_node.hpp"
#include "support/casting.hpp"
#include "support/walk.hpp"

#include <algorithm>

namespace tinytc {

auto stack_high_water_mark::run_on_function(function_node const &fn) -> std::int64_t {
    std::int64_t high_water_mark = 0;

    walk<walk_order::pre_order>(fn, [&high_water_mark](inst_node const &i) {
        if (auto *a = dyn_cast<const alloca_inst>(&i); a) {
            auto t = dyn_cast<const memref_data_type>(a->result(0).ty());
            if (t == nullptr) {
                throw compilation_error(a->loc(), status::ir_expected_memref);
            }
            high_water_mark = std::max(high_water_mark, a->stack_ptr() + t->size_in_bytes());
        }
    });

    return high_water_mark;
}

} // namespace tinytc
