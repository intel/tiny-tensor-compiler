// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "number.hpp"
#include "compiler_context.hpp"
#include "error.hpp"
#include "node/type.hpp"
#include "node/visit.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"
#include "util/casting.hpp"
#include "util/overloaded.hpp"

#include <cstddef>

namespace tinytc {

auto acc_type(tinytc_type_t ty) -> tinytc_type_t {
    return visit(
        overloaded{[](i8_type &ty) -> tinytc_type_t { return i32_type::get(ty.context()); },
                   [](bf16_type &ty) -> tinytc_type_t { return f32_type::get(ty.context()); },
                   [](f16_type &ty) -> tinytc_type_t { return f32_type::get(ty.context()); },
                   [](tinytc_type &ty) -> tinytc_type_t { return &ty; }},
        *ty);
}

auto component_count(tinytc_type_t ty) -> vector_size {
    return isa<complex_type>(*ty) ? vector_size::v2 : vector_size::v1;
}
auto component_type(tinytc_type_t ty) -> tinytc_type_t {
    if (isa<c32_type>(*ty)) {
        return f32_type::get(ty->context());
    } else if (isa<c64_type>(*ty)) {
        return f64_type::get(ty->context());
    } else if (isa<number_type>(*ty)) {
        return ty;
    }
    // only call component type for number types
    throw status::ir_expected_number;
}

auto promotable(tinytc_type_t a_ty, tinytc_type_t b_ty) -> bool {
    if (a_ty == b_ty) {
        return true;
    }
    const auto a_cc = static_cast<int>(component_count(a_ty));
    const auto b_cc = static_cast<int>(component_count(b_ty));
    const auto a_ct = component_type(a_ty);
    const auto b_ct = component_type(b_ty);
    return (isa<integer_type>(*a_ct) || !isa<integer_type>(*b_ct)) &&
           (size(a_ct) < size(b_ct) || a_ct == b_ct) && a_cc <= b_cc;
}

auto promote(tinytc_type_t a_ty, tinytc_type_t b_ty) -> tinytc_type_t {
    if (promotable(a_ty, b_ty)) {
        return b_ty;
    } else if (promotable(b_ty, a_ty)) {
        return a_ty;
    }
    return nullptr;
}

auto promote_or_throw(tinytc_type_t a_ty, tinytc_type_t b_ty, location const &loc)
    -> tinytc_type_t {
    if (auto res = promote(a_ty, b_ty); res) {
        return res;
    }
    throw compilation_error(loc, status::ir_forbidden_promotion);
}

auto alignment(tinytc_type_t ty, vector_size count) -> std::int32_t {
    const std::int32_t scale = count == vector_size::v3 ? 4 : static_cast<std::int32_t>(count);
    return scale * size(ty);
}

auto is_cast_allowed(tinytc_type_t from_ty, tinytc_type_t to_ty) -> bool {
    return isa<number_type>(*from_ty) && isa<number_type>(*to_ty) &&
           (!isa<complex_type>(*from_ty) || isa<complex_type>(*to_ty));
}

auto size(tinytc_type_t ty) -> std::size_t {
    return visit(overloaded{[](i8_type &) -> std::size_t { return 1; },  //
                            [](i16_type &) -> std::size_t { return 2; }, //
                            [](i32_type &) -> std::size_t { return 4; }, //
                            [](i64_type &) -> std::size_t { return 8; }, //
                            [](index_type &ty) -> std::size_t {
                                return ty.context()->index_bit_width() / 8;
                            },                                            //
                            [](bf16_type &) -> std::size_t { return 2; }, //
                            [](f16_type &) -> std::size_t { return 2; },  //
                            [](f32_type &) -> std::size_t { return 4; },  //
                            [](f64_type &) -> std::size_t { return 8; },  //
                            [](c32_type &) -> std::size_t { return 8; },  //
                            [](c64_type &) -> std::size_t { return 16; }, //
                            [](tinytc_type &) -> std::size_t {
                                // only call size for number types
                                throw status::ir_expected_number;
                            }},
                 *ty);
}

} // namespace tinytc

