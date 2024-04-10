// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/ir/scalar_type.hpp"

#include <clir/builtin_type.hpp>
#include <clir/data_type.hpp>

namespace tinytc {

char const *to_string(scalar_type ty) {
    switch (ty) {
    case scalar_type::bool_:
        return "bool";
    case scalar_type::index:
        return "index";
    case scalar_type::i8:
        return "i8";
    case scalar_type::i16:
        return "i16";
    case scalar_type::i32:
        return "i32";
    case scalar_type::i64:
        return "i64";
    case scalar_type::u8:
        return "u8";
    case scalar_type::u16:
        return "u16";
    case scalar_type::u32:
        return "u32";
    case scalar_type::u64:
        return "u64";
    case scalar_type::f32:
        return "f32";
    case scalar_type::f64:
        return "f64";
    }
    return "unknown";
}

std::size_t size(scalar_type ty) {
    switch (ty) {
    case scalar_type::bool_:
        return 1;
    case scalar_type::index:
        return 4;
    case scalar_type::i8:
    case scalar_type::u8:
        return 1;
    case scalar_type::i16:
    case scalar_type::u16:
        return 2;
    case scalar_type::i32:
    case scalar_type::u32:
    case scalar_type::f32:
        return 4;
    case scalar_type::i64:
    case scalar_type::u64:
    case scalar_type::f64:
        return 8;
    }
    return 0;
}

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

clir::builtin_type internal::to_clir_builtin_ty(scalar_type ty) {
    switch (ty) {
    case scalar_type::bool_:
        return clir::builtin_type::bool_t;
    case scalar_type::index:
        return clir::builtin_type::uint_t;
    case scalar_type::i8:
        return clir::builtin_type::char_t;
    case scalar_type::i16:
        return clir::builtin_type::short_t;
    case scalar_type::i32:
        return clir::builtin_type::int_t;
    case scalar_type::i64:
        return clir::builtin_type::long_t;
    case scalar_type::u8:
        return clir::builtin_type::uchar_t;
    case scalar_type::u16:
        return clir::builtin_type::ushort_t;
    case scalar_type::u32:
        return clir::builtin_type::uint_t;
    case scalar_type::u64:
        return clir::builtin_type::ulong_t;
    case scalar_type::f32:
        return clir::builtin_type::float_t;
    case scalar_type::f64:
        return clir::builtin_type::double_t;
    }
    return clir::builtin_type::void_t;
}

clir::data_type internal::to_clir_ty(scalar_type ty, clir::address_space as,
                                     clir::type_qualifier q) {
    return clir::data_type(to_clir_builtin_ty(ty), as, q);
}

clir::builtin_type internal::to_clir_atomic_builtin_ty(scalar_type ty) {
    switch (ty) {
    case scalar_type::index:
        return clir::builtin_type::atomic_uint_t;
    case scalar_type::i32:
        return clir::builtin_type::atomic_int_t;
    case scalar_type::i64:
        return clir::builtin_type::atomic_long_t;
    case scalar_type::u32:
        return clir::builtin_type::atomic_uint_t;
    case scalar_type::u64:
        return clir::builtin_type::atomic_ulong_t;
    case scalar_type::f32:
        return clir::builtin_type::atomic_float_t;
    case scalar_type::f64:
        return clir::builtin_type::atomic_double_t;
    default:
        break;
    }
    return clir::builtin_type::void_t;
}

clir::data_type internal::to_clir_atomic_ty(scalar_type ty, clir::address_space as,
                                            clir::type_qualifier q) {
    return clir::data_type(to_clir_atomic_builtin_ty(ty), as, q);
}

} // namespace tinytc
