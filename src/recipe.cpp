// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "recipe.hpp"
#include "error.hpp"
#include "number_dispatch.hpp"
#include "tinytc/core.h"

#include <algorithm>
#include <complex>
#include <cstring>

namespace tinytc {

auto is_argument_zero(tinytc_type_t ty, std::size_t arg_size, const void *arg_value) -> bool {
    return dispatch_number_to_native(
        ty,
        []<typename T>(std::size_t arg_size, const void *arg_value) {
            T v;
            memcpy(&v, arg_value, std::min(sizeof(v), arg_size));
            return v == T(0);
        },
        arg_size, arg_value);
}

} // namespace tinytc

extern "C" {

tinytc_status_t tinytc_recipe_get_prog(const_tinytc_recipe_t recipe, tinytc_prog_t *prg) {
    if (recipe == nullptr || prg == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return tinytc::exception_to_status_code([&] { *prg = recipe->get_program(); });
}

tinytc_status_t tinytc_recipe_get_binary(const_tinytc_recipe_t recipe, tinytc_binary_t *bin) {
    if (recipe == nullptr || bin == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return tinytc::exception_to_status_code([&] { *bin = recipe->get_binary(); });
}

tinytc_status_t tinytc_recipe_release(tinytc_recipe_t obj) {
    if (obj == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    auto ref_count = obj->dec_ref();
    if (ref_count == 0) {
        delete obj;
    }
    return tinytc_status_success;
}

tinytc_status_t tinytc_recipe_retain(tinytc_recipe_t obj) {
    if (obj == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    obj->inc_ref();
    return tinytc_status_success;
}

tinytc_status_t tinytc_recipe_handler_get_recipe(const_tinytc_recipe_handler_t handler,
                                                 tinytc_recipe_t *recipe) {
    if (handler == nullptr || recipe == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return tinytc::exception_to_status_code([&] { *recipe = handler->get_recipe(); });
}

tinytc_status_t tinytc_recipe_handler_release(tinytc_recipe_handler_t obj) {
    if (obj == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    auto ref_count = obj->dec_ref();
    if (ref_count == 0) {
        delete obj;
    }
    return tinytc_status_success;
}

tinytc_status_t tinytc_recipe_handler_retain(tinytc_recipe_handler_t obj) {
    if (obj == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    obj->inc_ref();
    return tinytc_status_success;
}
}
