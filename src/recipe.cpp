// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "recipe.hpp"
#include "compiler_context.hpp"
#include "error.hpp"
#include "node/type.hpp"
#include "node/visit.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"
#include "util/overloaded.hpp"

#include <algorithm>
#include <complex>
#include <cstring>

namespace tinytc {
template <typename T> bool is_argument_zero(std::size_t arg_size, const void *arg_value) {
    T v;
    memcpy(&v, arg_value, std::min(sizeof(v), arg_size));
    return v == T(0);
}

auto is_argument_zero(tinytc_type_t ty, std::size_t arg_size, const void *arg_value) -> bool {
    return visit(
        overloaded{
            [&](i8_type &) { return is_argument_zero<std::int8_t>(arg_size, arg_value); },
            [&](i16_type &) { return is_argument_zero<std::int16_t>(arg_size, arg_value); },
            [&](i32_type &) { return is_argument_zero<std::int32_t>(arg_size, arg_value); },
            [&](i64_type &) { return is_argument_zero<std::int64_t>(arg_size, arg_value); },
            [&](index_type &ty) {
                const auto idx_width = ty.context()->index_bit_width();
                if (idx_width == 64) {
                    return is_argument_zero<std::int64_t>(arg_size, arg_value);
                } else if (idx_width == 32) {
                    return is_argument_zero<std::int32_t>(arg_size, arg_value);
                }
                throw status::not_implemented;
            },
            [&](bf16_type &) { return is_argument_zero<bfloat16>(arg_size, arg_value); },
            [&](f16_type &) { return is_argument_zero<half>(arg_size, arg_value); },
            [&](f32_type &) { return is_argument_zero<float>(arg_size, arg_value); },
            [&](f64_type &) { return is_argument_zero<double>(arg_size, arg_value); },
            [&](c32_type &) { return is_argument_zero<std::complex<float>>(arg_size, arg_value); },
            [&](c64_type &) { return is_argument_zero<std::complex<double>>(arg_size, arg_value); },
            [](auto &) {
                throw status::ir_expected_number;
                return false;
            }},
        *ty);
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
