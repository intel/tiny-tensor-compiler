// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "analysis/stack.hpp"
#include "error.hpp"
#include "node/inst_view.hpp"
#include "node/type.hpp"
#include "node/value.hpp"
#include "support/walk.hpp"
#include "tinytc/types.hpp"
#include "util/casting.hpp"

#include <algorithm>
#include <functional>

namespace tinytc {

auto stack_high_water_mark::run_on_function(tinytc_func &fn) -> std::int64_t {
    std::int64_t high_water_mark = 0;

    walk<walk_order::pre_order>(fn, [&high_water_mark](tinytc_inst &i) {
        if (auto a = dyn_cast<alloca_inst>(&i); a) {
            auto t = dyn_cast<memref_data_type>(a.result().ty());
            if (t == nullptr) {
                throw compilation_error(a.loc(), status::ir_expected_memref);
            }
            high_water_mark = std::max(high_water_mark, a.stack_ptr() + t->size_in_bytes());
        }
    });

    return high_water_mark;
}

} // namespace tinytc
