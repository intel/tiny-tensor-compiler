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
    case scalar_type::i1:
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
        case scalar_type::c32:
            return clir::builtin_type::float_t;
        case scalar_type::f64:
        case scalar_type::c64:
            return clir::builtin_type::double_t;
        }
        return clir::builtin_type::void_t;
    };
    const auto components = [](scalar_type ty) -> short {
        switch (ty) {
        case scalar_type::i1:
        case scalar_type::i8:
        case scalar_type::i16:
        case scalar_type::i32:
        case scalar_type::i64:
        case scalar_type::index:
        case scalar_type::f32:
        case scalar_type::f64:
            return 1;
        case scalar_type::c32:
        case scalar_type::c64:
            return 2;
        }
        return 0;
    };
    size *= components(ty);
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

clir::address_space to_clir_address_space(address_space as) {
    switch (as) {
    case address_space::global:
        return clir::address_space::global_t;
    case address_space::local:
        return clir::address_space::local_t;
    }
    return clir::address_space::global_t;
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
    case tinytc_scalar_type_c32:
        return "c32";
    case tinytc_scalar_type_c64:
        return "c64";
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
    case tinytc_scalar_type_c32:
        return 8;
    case tinytc_scalar_type_c64:
        return 16;
    }
    return 0;
}
