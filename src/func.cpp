// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "error.hpp"
#include "location.hpp"
#include "node/function_node.hpp"
#include "tinytc/tinytc.h"

#include <memory>
#include <utility>

using namespace tinytc;

extern "C" {

tinytc_status_t tinytc_function_prototype_create(tinytc_func_t *fun, char const *name,
                                                 uint32_t arg_list_size, tinytc_value_t *arg_list,
                                                 const tinytc_location_t *loc) {
    if (fun == nullptr || (arg_list_size > 0 && arg_list == nullptr)) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto arg_vec = std::vector<value>();
        arg_vec.reserve(arg_list_size);
        for (uint32_t i = 0; i < arg_list_size; ++i) {
            arg_vec.emplace_back(value(arg_list[i], true));
        }
        *fun = std::make_unique<prototype>(std::string(name), std::move(arg_vec), get_optional(loc))
                   .release();
    });
}

tinytc_status_t tinytc_function_create(tinytc_func_t *fun, tinytc_func_t prototype,
                                       tinytc_region_t body, const tinytc_location_t *loc) {
    if (fun == nullptr || prototype == nullptr || body == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        CHECK(tinytc_func_retain(prototype));
        CHECK(tinytc_region_retain(body));
        *fun = std::make_unique<function>(prototype, body, get_optional(loc)).release();
    });
}

tinytc_status_t tinytc_function_set_work_group_size(tinytc_func_t fun, uint32_t x, uint32_t y) {
    function *f = dynamic_cast<function *>(fun);
    if (f == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { f->work_group_size({x, y}); });
}

tinytc_status_t tinytc_function_set_subgroup_size(tinytc_func_t fun, uint32_t sgs) {
    function *f = dynamic_cast<function *>(fun);
    if (f == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { f->subgroup_size(sgs); });
}

tinytc_status_t tinytc_func_release(tinytc_func_t fun) {
    if (fun == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto ref_count = fun->dec_ref();
        if (ref_count == 0) {
            delete fun;
        }
    });
}

tinytc_status_t tinytc_func_retain(tinytc_func_t fun) {
    if (fun == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { fun->inc_ref(); });
}
}
