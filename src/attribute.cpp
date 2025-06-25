// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "error.hpp"
#include "node/attr_node.hpp"
#include "tinytc/builder.h"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"

#include <cstdint>
#include <string_view>

using namespace tinytc;

extern "C" {

tinytc_status_t tinytc_array_attr_get(tinytc_attr_t *attr, tinytc_compiler_context_t ctx,
                                      uint32_t array_size, const tinytc_attr_t *array) {
    if (attr == nullptr || ctx == nullptr || (array_size != 0 && array == nullptr)) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *attr = array_attr::get(ctx, array_view<tinytc_attr_t>(array, array_size)); });
}

tinytc_status_t tinytc_boolean_attr_get(tinytc_attr_t *attr, tinytc_compiler_context_t ctx,
                                        tinytc_bool_t value) {
    if (attr == nullptr || ctx == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *attr = boolean_attr::get(ctx, value); });
}

tinytc_status_t tinytc_dictionary_attr_get(tinytc_attr_t *attr, tinytc_compiler_context_t ctx,
                                           uint32_t items_size, tinytc_named_attr_t *items) {
    TINYTC_CHECK_STATUS(tinytc_dictionary_attr_sort(items_size, items));
    return tinytc_dictionary_attr_get_with_sorted(attr, ctx, items_size, items);
}

tinytc_status_t tinytc_dictionary_attr_get_with_sorted(tinytc_attr_t *attr,
                                                       tinytc_compiler_context_t ctx,
                                                       uint32_t items_size,
                                                       const tinytc_named_attr_t *items) {
    if (attr == nullptr || ctx == nullptr || (items_size != 0 && items == nullptr)) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *attr = dictionary_attr::get(ctx, array_view<tinytc_named_attr_t>(items, items_size));
    });
}

tinytc_status_t tinytc_dictionary_attr_sort(uint32_t items_size, tinytc_named_attr_t *items) {
    return exception_to_status_code(
        [&] { dictionary_attr::sort(mutable_array_view<tinytc_named_attr_t>(items, items_size)); });
}

tinytc_status_t tinytc_integer_attr_get(tinytc_attr_t *attr, tinytc_compiler_context_t ctx,
                                        int64_t value) {
    if (attr == nullptr || ctx == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *attr = integer_attr::get(ctx, value); });
}

tinytc_status_t tinytc_string_attr_get(tinytc_attr_t *attr, tinytc_compiler_context_t ctx,
                                       uint32_t str_length, char const *str) {
    if (attr == nullptr || ctx == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *attr = string_attr::get(ctx, std::string_view(str, str_length)); });
}
}
