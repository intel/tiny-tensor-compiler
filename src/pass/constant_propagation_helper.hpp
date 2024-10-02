// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CONSTANT_PROPAGATION_HELPER_20241002_HPP
#define CONSTANT_PROPAGATION_HELPER_20241002_HPP

#include "scalar_type.hpp"
#include "tinytc/tinytc.hpp"

#include <concepts>
#include <type_traits>

namespace tinytc {

struct compute_unary_op {
    arithmetic_unary operation;
    data_type ty;
    location const &loc;

    template <typename T>
    requires(std::is_integral_v<T>)
    auto operator()(T a) {
        T val = 0;
        switch (operation) {
        case arithmetic_unary::neg:
            val = -a;
            break;
        case arithmetic_unary::not_:
            if constexpr (std::is_same_v<T, bool>) {
                val = !a;
            } else {
                val = ~a;
            }
            break;
        }
        return make_constant(val, ty, loc);
    }

    template <typename T, typename U>
    requires(!std::is_integral_v<T>)
    auto operator()(U const &A) -> inst {
        const auto a = static_cast<T>(A);
        T val = {};
        switch (operation) {
        case arithmetic_unary::neg:
            val = -a;
            break;
        default:
            if constexpr (!std::is_floating_point_v<T>) {
                throw compilation_error(loc, status::ir_complex_unsupported);
            }
            throw compilation_error(loc, status::ir_fp_unsupported);
            break;
        }
        return make_constant(val, ty, loc);
    }
};

struct compute_binary_op {
    arithmetic operation;
    data_type ty;
    location const &loc;

    template <typename T>
    requires(std::is_integral_v<T>)
    auto operator()(T a, T b) {
        T val = 0;
        switch (operation) {
        case arithmetic::add:
            val = a + b;
            break;
        case arithmetic::sub:
            val = a - b;
            break;
        case arithmetic::mul:
            val = a * b;
            break;
        case arithmetic::div:
            val = a / b;
            break;
        case arithmetic::rem:
            val = a % b;
            break;
        case arithmetic::shl:
            if constexpr (std::is_same_v<T, bool>) {
                throw compilation_error(loc, status::ir_i1_unsupported);
            } else {
                val = a << b;
            }
            break;
        case arithmetic::shr:
            if constexpr (std::is_same_v<T, bool>) {
                throw compilation_error(loc, status::ir_i1_unsupported);
            } else {
                val = a >> b;
            }
            break;
        case arithmetic::and_:
            val = a & b;
            break;
        case arithmetic::or_:
            val = a | b;
            break;
        case arithmetic::xor_:
            val = a ^ b;
            break;
        }
        return make_constant(val, ty, loc);
    }

    template <typename T, typename U>
    requires(!std::is_integral_v<T>)
    auto operator()(U const &A, U const &B) -> inst {
        const auto a = static_cast<T>(A);
        const auto b = static_cast<T>(B);
        T val = {};
        switch (operation) {
        case arithmetic::add:
            val = a + b;
            break;
        case arithmetic::sub:
            val = a - b;
            break;
        case arithmetic::mul:
            val = a * b;
            break;
        case arithmetic::div:
            val = a / b;
            break;
        case arithmetic::rem:
            if constexpr (!std::is_floating_point_v<T>) {
                throw compilation_error(loc, status::ir_complex_unsupported);
            } else {
                val = std::fmod(a, b);
            }
            break;
        default:
            if constexpr (!std::is_floating_point_v<T>) {
                throw compilation_error(loc, status::ir_complex_unsupported);
            }
            throw compilation_error(loc, status::ir_fp_unsupported);
            break;
        }
        return make_constant(val, ty, loc);
    }
};

struct compute_compare {
    cmp_condition cond;
    data_type ty;
    location const &loc;

    template <typename T>
    requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
    auto operator()(T a, T b) {
        bool val = false;
        switch (cond) {
        case cmp_condition::eq:
            val = (a == b);
            break;
        case cmp_condition::ne:
            val = (a != b);
            break;
        case cmp_condition::gt:
            val = (a > b);
            break;
        case cmp_condition::ge:
            val = (a >= b);
            break;
        case cmp_condition::lt:
            val = (a < b);
            break;
        case cmp_condition::le:
            val = (a <= b);
            break;
        };
        return make_constant(val, ty, loc);
    }

    template <typename T, typename F>
    auto operator()(std::complex<F> const &A, std::complex<F> const &B) {
        const auto a = static_cast<T>(A);
        const auto b = static_cast<T>(B);
        bool val = false;
        switch (cond) {
        case cmp_condition::eq:
            val = (a == b);
            break;
        case cmp_condition::ne:
            val = (a != b);
            break;
        default:
            throw compilation_error(loc, status::ir_complex_unsupported);
            break;
        };
        return make_constant(val, ty, loc);
    }
};

template <typename T, typename U> struct value_cast_impl {
    auto operator()(U const &u) { return static_cast<T>(u); }
};

template <typename U> struct value_cast_impl<bool, U> {
    auto operator()(U const &u) { return u != U{}; }
};

template <typename F> struct value_cast_impl<bool, std::complex<F>> {
    auto operator()(std::complex<F> const &u) { return u != std::complex<F>{}; }
};

template <typename T, typename F> struct value_cast_impl<T, std::complex<F>> {
    auto operator()(std::complex<F> const &u) { return static_cast<T>(u.real()); }
};

template <typename T, typename U> auto value_cast(U const &u) { return value_cast_impl<T, U>{}(u); }

template <typename T> auto compute_cast(scalar_data_type *to_ty, T A, location const &loc) -> inst {
    switch (to_ty->ty()) {
    case scalar_type::i1:
        return make_constant(value_cast<bool>(A), to_ty, loc);
    case scalar_type::i8:
        return make_constant(value_cast<std::int8_t>(A), to_ty, loc);
    case scalar_type::i16:
        return make_constant(value_cast<std::int16_t>(A), to_ty, loc);
    case scalar_type::i32:
        return make_constant(value_cast<std::int32_t>(A), to_ty, loc);
    case scalar_type::i64:
        return make_constant(value_cast<std::int64_t>(A), to_ty, loc);
    case scalar_type::index:
        return make_constant(value_cast<host_index_type>(A), to_ty, loc);
    case scalar_type::f32:
        return make_constant(value_cast<float>(A), to_ty, loc);
    case scalar_type::f64:
        return make_constant(value_cast<double>(A), to_ty, loc);
    case scalar_type::c32:
        return make_constant(value_cast<std::complex<float>>(A), to_ty, loc);
    case scalar_type::c64:
        return make_constant(value_cast<std::complex<double>>(A), to_ty, loc);
    };
    return {};
};

} // namespace tinytc

#endif // CONSTANT_PROPAGATION_HELPER_20241002_HPP
