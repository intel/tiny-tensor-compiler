// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "recipe.hpp"
#include "error.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/types.hpp"

#include <algorithm>
#include <cstring>

namespace tinytc {
template <typename T> bool is_argument_zero(std::size_t arg_size, const void *arg_value) {
    T v;
    memcpy(&v, arg_value, std::min(sizeof(v), arg_size));
    return v == T(0);
}

auto is_argument_zero(scalar_type type, std::size_t arg_size, const void *arg_value) -> bool {
    switch (type) {
    case scalar_type::index:
        return is_argument_zero<std::uint32_t>(arg_size, arg_value);
    case scalar_type::i8:
        return is_argument_zero<std::int8_t>(arg_size, arg_value);
    case scalar_type::i16:
        return is_argument_zero<std::int16_t>(arg_size, arg_value);
    case scalar_type::i32:
        return is_argument_zero<std::int32_t>(arg_size, arg_value);
    case scalar_type::i64:
        return is_argument_zero<std::int64_t>(arg_size, arg_value);
    case scalar_type::u8:
        return is_argument_zero<std::uint8_t>(arg_size, arg_value);
    case scalar_type::u16:
        return is_argument_zero<std::uint16_t>(arg_size, arg_value);
    case scalar_type::u32:
        return is_argument_zero<std::uint32_t>(arg_size, arg_value);
    case scalar_type::u64:
        return is_argument_zero<std::uint64_t>(arg_size, arg_value);
    case scalar_type::f32:
        return is_argument_zero<float>(arg_size, arg_value);
    case scalar_type::f64:
        return is_argument_zero<double>(arg_size, arg_value);
    case scalar_type::bool_:
        break;
    };
    throw status::invalid_arguments;
}

} // namespace tinytc

extern "C" {

tinytc_status_t tinytc_recipe_get_prog(const_tinytc_recipe_t recipe, tinytc_prog_t *prg) {
    if (recipe == nullptr || prg == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return tinytc::exception_to_status_code(
        [&] { *prg = tinytc::prog(recipe->get_program()).release(); });
}

tinytc_status_t tinytc_recipe_get_binary(const_tinytc_recipe_t recipe, tinytc_binary_t *bin) {
    if (recipe == nullptr || bin == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return tinytc::exception_to_status_code(
        [&] { *bin = tinytc::binary(recipe->get_binary()).release(); });
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
    return tinytc::exception_to_status_code(
        [&] { *recipe = tinytc::recipe(handler->get_recipe()).release(); });
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
