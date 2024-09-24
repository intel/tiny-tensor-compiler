// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "error.hpp"
#include "location.hpp"
#include "node/function_node.hpp"
#include "node/region_node.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace tinytc;

extern "C" {

tinytc_status_t tinytc_function_create(tinytc_func_t *fun, char const *name, uint32_t arg_list_size,
                                       tinytc_value_t *arg_list, tinytc_region_t body,
                                       const tinytc_location_t *loc) {
    if (fun == nullptr || (arg_list_size > 0 && arg_list == nullptr) || body == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto arg_vec = std::vector<value>();
        arg_vec.reserve(arg_list_size);
        for (uint32_t i = 0; i < arg_list_size; ++i) {
            arg_vec.emplace_back(value(arg_list[i], true));
        }
        *fun = std::make_unique<function_node>(std::string(name), std::move(arg_vec), body,
                                               get_optional(loc))
                   .release();
    });
}

tinytc_status_t tinytc_function_set_work_group_size(tinytc_func_t fun, int32_t x, int32_t y) {
    return exception_to_status_code([&] { fun->work_group_size({x, y}); });
}

tinytc_status_t tinytc_function_set_subgroup_size(tinytc_func_t fun, int32_t sgs) {
    return exception_to_status_code([&] { fun->subgroup_size(sgs); });
}

void tinytc_func_destroy(tinytc_func_t obj) { delete obj; }
}
