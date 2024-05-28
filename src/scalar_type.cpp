// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "scalar_type.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <cstddef>

namespace tinytc {

bool is_floating_type(scalar_type ty) {
    switch (ty) {
    case scalar_type::f32:
    case scalar_type::f64:
        return true;
    default:
        break;
    }
    return false;
}

clir::data_type to_clir_ty(scalar_type ty, clir::address_space as, clir::type_qualifier q) {
    return to_clir_ty(ty, 1, as, q);
}

clir::data_type to_clir_ty(scalar_type ty, short size, clir::address_space as,
                           clir::type_qualifier q) {
    const auto base_type = [](scalar_type ty) {
        switch (ty) {
        case scalar_type::i1:
            return clir::builtin_type::bool_t;
        case scalar_type::i8:
            return clir::builtin_type::char_t;
        case scalar_type::i16:
            return clir::builtin_type::short_t;
        case scalar_type::i32:
            return clir::builtin_type::int_t;
        case scalar_type::i64:
            return clir::builtin_type::long_t;
        case scalar_type::index:
            return clir::builtin_type::long_t;
        case scalar_type::f32:
            return clir::builtin_type::float_t;
        case scalar_type::f64:
            return clir::builtin_type::double_t;
        }
        return clir::builtin_type::void_t;
    };
    if (size == 1) {
        return clir::data_type(base_type(ty), as, q);
    }
    return clir::data_type(base_type(ty), size, as, q);
}

clir::data_type to_clir_atomic_ty(scalar_type ty, clir::address_space as, clir::type_qualifier q) {
    auto const base_type = [](scalar_type ty) {
        switch (ty) {
        case scalar_type::i32:
            return clir::builtin_type::atomic_int_t;
        case scalar_type::i64:
            return clir::builtin_type::atomic_long_t;
        case scalar_type::index:
            return clir::builtin_type::atomic_long_t;
        case scalar_type::f32:
            return clir::builtin_type::atomic_float_t;
        case scalar_type::f64:
            return clir::builtin_type::atomic_double_t;
        default:
            break;
        }
        return clir::builtin_type::void_t;
    };
    return clir::data_type(base_type(ty), as, q);
}

} // namespace tinytc

char const *tinytc_scalar_type_to_string(tinytc_scalar_type_t ty) {
    switch (ty) {
    case tinytc_scalar_type_i1:
        return "i1";
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
    case tinytc_scalar_type_f32:
        return "f32";
    case tinytc_scalar_type_f64:
        return "f64";
    }
    return "unknown";
}
size_t tinytc_scalar_type_size(tinytc_scalar_type_t ty) {
    switch (ty) {
    case tinytc_scalar_type_i1:
    case tinytc_scalar_type_i8:
        return 1;
    case tinytc_scalar_type_i16:
        return 2;
    case tinytc_scalar_type_i32:
    case tinytc_scalar_type_f32:
        return 4;
    case tinytc_scalar_type_i64:
    case tinytc_scalar_type_index:
    case tinytc_scalar_type_f64:
        return 8;
    }
    return 0;
}
