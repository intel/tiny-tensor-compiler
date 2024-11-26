// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "scalar_type.hpp"
#include "support/util.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <algorithm>
#include <cstddef>

namespace tinytc {

bool is_floating_type(scalar_type ty) {
    switch (ty) {
    case scalar_type::bf16:
    case scalar_type::f16:
    case scalar_type::f32:
    case scalar_type::f64:
        return true;
    default:
        break;
    }
    return false;
}

bool is_complex_type(scalar_type ty) {
    switch (ty) {
    case scalar_type::c32:
    case scalar_type::c64:
        return true;
    default:
        break;
    }
    return false;
}

bool is_integer_type(scalar_type ty) {
    switch (ty) {
    case scalar_type::i8:
    case scalar_type::i16:
    case scalar_type::i32:
    case scalar_type::i64:
    case scalar_type::index:
        return true;
    default:
        break;
    }
    return false;
}

scalar_type element_type(scalar_type ty) {
    switch (ty) {
    case scalar_type::c32:
        return scalar_type::f32;
    case scalar_type::c64:
        return scalar_type::f64;
    default:
        break;
    }
    return ty;
}

scalar_type compatible_type(scalar_type a_ty, scalar_type b_ty) {
    int max = std::max(static_cast<int>(a_ty), static_cast<int>(b_ty));
    return enum_cast<scalar_type>(max);
}

std::int32_t alignment(scalar_type ty, component_count count) {
    const std::int32_t scale = count == component_count::v3 ? 4 : static_cast<std::int32_t>(count);
    return scale * tinytc_scalar_type_size(static_cast<tinytc_scalar_type_t>(ty));
}

} // namespace tinytc

char const *tinytc_scalar_type_to_string(tinytc_scalar_type_t ty) {
    switch (ty) {
    case tinytc_scalar_type_i8:
        return "i8";
    case tinytc_scalar_type_i16:
        return "i16";
    case tinytc_scalar_type_i32:
        return "i32";
    case tinytc_scalar_type_i64:
        return "i64";
    case tinytc_scalar_type_index:
        return "index";
    case tinytc_scalar_type_bf16:
        return "bf16";
    case tinytc_scalar_type_f16:
        return "f16";
    case tinytc_scalar_type_f32:
        return "f32";
    case tinytc_scalar_type_f64:
        return "f64";
    case tinytc_scalar_type_c32:
        return "c32";
    case tinytc_scalar_type_c64:
        return "c64";
    }
    return "unknown";
}
size_t tinytc_scalar_type_size(tinytc_scalar_type_t ty) {
    switch (ty) {
    case tinytc_scalar_type_i8:
        return 1;
    case tinytc_scalar_type_i16:
    case tinytc_scalar_type_bf16:
    case tinytc_scalar_type_f16:
        return 2;
    case tinytc_scalar_type_i32:
    case tinytc_scalar_type_f32:
        return 4;
    case tinytc_scalar_type_i64:
    case tinytc_scalar_type_index:
    case tinytc_scalar_type_f64:
    case tinytc_scalar_type_c32:
        return 8;
    case tinytc_scalar_type_c64:
        return 16;
    }
    return 0;
}
