// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "error.hpp"
#include "location.hpp"
#include "node/function_node.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"

#include <cstdint>
#include <memory>
#include <string>

using namespace tinytc;

extern "C" {

tinytc_status_t tinytc_func_create(tinytc_func_t *fun, uint32_t name_length, char const *name,
                                   uint32_t num_params, const tinytc_data_type_t *param_type_list,
                                   tinytc_data_type_t ty, const tinytc_location_t *loc) {
    if (fun == nullptr || (num_params > 0 && param_type_list == nullptr) || ty == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *fun = std::make_unique<function_node>(std::string(name, name_length),
                                               array_view(param_type_list, num_params), ty,
                                               get_optional(loc))
                   .release();
    });
}

tinytc_status_t tinytc_func_set_parameter_attr(tinytc_func_t fun, int32_t arg_no, attr a) {
    if (fun == nullptr || arg_no < 0 || arg_no >= fun->num_params()) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { fun->param_attr(arg_no, a); });
}

tinytc_status_t tinytc_func_set_attr(tinytc_func_t fun, tinytc_attr_t a) {
    if (fun == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { fun->attr(a); });
}

tinytc_status_t tinytc_func_get_body(tinytc_func_t fun, tinytc_region_t *body) {
    if (fun == nullptr || body == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *body = &fun->body(); });
}

void tinytc_func_destroy(tinytc_func_t obj) { delete obj; }
}
