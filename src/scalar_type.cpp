// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "scalar_type.hpp"
#include "error.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

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

auto acc_type(scalar_type ty) -> scalar_type {
    switch (ty) {
    case scalar_type::i8:
        return scalar_type::i32;
    case scalar_type::bf16:
    case scalar_type::f16:
        return scalar_type::f32;
    default:
        return ty;
    }
}

auto component_count(scalar_type ty) -> vector_size {
    switch (ty) {
    case scalar_type::c32:
    case scalar_type::c64:
        return vector_size::v2;
    default:
        break;
    }
    return vector_size::v1;
}
auto component_type(scalar_type ty) -> scalar_type {
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

auto promotable(scalar_type a_ty, scalar_type b_ty) -> bool {
    if (a_ty == b_ty) {
        return true;
    }
    const auto a_cc = static_cast<int>(component_count(a_ty));
    const auto b_cc = static_cast<int>(component_count(b_ty));
    const auto a_ct = component_type(a_ty);
    const auto b_ct = component_type(b_ty);
    return (is_integer_type(a_ct) || !is_integer_type(b_ct)) &&
           (size(a_ct) < size(b_ct) || a_ct == b_ct) && a_cc <= b_cc;
}

auto promote(scalar_type a_ty, scalar_type b_ty) -> std::optional<scalar_type> {
    if (promotable(a_ty, b_ty)) {
        return b_ty;
    } else if (promotable(b_ty, a_ty)) {
        return a_ty;
    }
    return std::nullopt;
}

auto promote_or_throw(scalar_type a_ty, scalar_type b_ty, location const &loc) -> scalar_type {
    auto res = promote(a_ty, b_ty);
    if (res) {
        return *res;
    }
    throw compilation_error(loc, status::ir_forbidden_promotion);
}

auto alignment(scalar_type ty, vector_size count) -> std::int32_t {
    const std::int32_t scale = count == vector_size::v3 ? 4 : static_cast<std::int32_t>(count);
    return scale * size(ty);
}

auto is_cast_allowed(scalar_type from_ty, scalar_type to_ty) -> bool {
    return !is_complex_type(from_ty) || is_complex_type(to_ty);
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
