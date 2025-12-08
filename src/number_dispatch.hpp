// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause
//
#ifndef NUMBER_DISPATCH_20250702_HPP
#define NUMBER_DISPATCH_20250702_HPP

#include "compiler_context.hpp"
#include "node/type.hpp"
#include "node/visit.hpp"
#include "util/casting.hpp"
#include "util/overloaded.hpp"

#include <type_traits>

namespace tinytc {

template <typename F, typename... T>
auto dispatch_int_to_native(tinytc_type_t ty, F &&f, T &&...args) {
    using RetT =
        std::common_type_t<decltype(f.template operator()<std::int8_t>(std::declval<T &&>()...)),
                           decltype(f.template operator()<std::int16_t>(std::declval<T &&>()...)),
                           decltype(f.template operator()<std::int32_t>(std::declval<T &&>()...)),
                           decltype(f.template operator()<std::int64_t>(std::declval<T &&>()...))>;
    return visit(
        overloaded{[&](i8_type &) -> RetT {
                       return f.template operator()<std::int8_t>(std::forward<T>(args)...);
                   },
                   [&](i16_type &) -> RetT {
                       return f.template operator()<std::int16_t>(std::forward<T>(args)...);
                   },
                   [&](i32_type &) -> RetT {
                       return f.template operator()<std::int32_t>(std::forward<T>(args)...);
                   },
                   [&](i64_type &) -> RetT {
                       return f.template operator()<std::int64_t>(std::forward<T>(args)...);
                   },
                   [&](index_type &ty) -> RetT {
                       const auto idx_width = ty.context()->index_bit_width();
                       if (idx_width == 64) {
                           return f.template operator()<std::int64_t>(std::forward<T>(args)...);
                       } else if (idx_width == 32) {
                           return f.template operator()<std::int32_t>(std::forward<T>(args)...);
                       }
                       throw status::not_implemented;
                   },
                   [](tinytc_type &) -> RetT { throw status::ir_expected_int; }},
        *ty);
}

template <typename F, typename... T>
auto dispatch_float_to_native(tinytc_type_t ty, F &&f, T &&...args) {
    using RetT =
        std::common_type_t<decltype(f.template operator()<bfloat16>(std::declval<T &&>()...)),
                           decltype(f.template operator()<half>(std::declval<T &&>()...)),
                           decltype(f.template operator()<float>(std::declval<T &&>()...)),
                           decltype(f.template operator()<double>(std::declval<T &&>()...))>;
    return visit(overloaded{[&](bf16_type &) -> RetT {
                                return f.template operator()<bfloat16>(std::forward<T>(args)...);
                            },
                            [&](f16_type &) -> RetT {
                                return f.template operator()<half>(std::forward<T>(args)...);
                            },
                            [&](f32_type &) -> RetT {
                                return f.template operator()<float>(std::forward<T>(args)...);
                            },
                            [&](f64_type &) -> RetT {
                                return f.template operator()<double>(std::forward<T>(args)...);
                            },
                            [](tinytc_type &) -> RetT { throw status::ir_expected_float; }},
                 *ty);
}

template <typename F, typename... T>
auto dispatch_complex_to_native(tinytc_type_t ty, F &&f, T &&...args) {
    using RetT = std::common_type_t<
        decltype(f.template operator()<std::complex<float>>(std::declval<T &&>()...)),
        decltype(f.template operator()<std::complex<double>>(std::declval<T &&>()...))>;
    return visit(
        overloaded{[&](c32_type &) -> RetT {
                       return f.template operator()<std::complex<float>>(std::forward<T>(args)...);
                   },
                   [&](c64_type &) -> RetT {
                       return f.template operator()<std::complex<double>>(std::forward<T>(args)...);
                   },
                   [](tinytc_type &) -> RetT { throw status::ir_expected_complex; }},
        *ty);
}

template <typename F, typename... T>
auto dispatch_number_to_native(tinytc_type_t ty, F &&f, T &&...args) {
    if (isa<integer_type>(*ty)) {
        return dispatch_int_to_native(ty, std::forward<F>(f), std::forward<T>(args)...);
    } else if (isa<float_type>(*ty)) {
        return dispatch_float_to_native(ty, std::forward<F>(f), std::forward<T>(args)...);
    } else if (isa<complex_type>(*ty)) {
        return dispatch_complex_to_native(ty, std::forward<F>(f), std::forward<T>(args)...);
    }
    throw status::ir_expected_number;
}

} // namespace tinytc

#endif // NUMBER_DISPATCH_20250702_HPP
