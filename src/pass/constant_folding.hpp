// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CONSTANT_FOLDING_HELPER_20241011_HPP
#define CONSTANT_FOLDING_HELPER_20241011_HPP

#include "error.hpp"
#include "node/data_type_node.hpp"
#include "node/inst_view.hpp"
#include "node/value_node.hpp"
#include "scalar_type.hpp"
#include "support/fp_util.hpp" // IWYU pragma: keep
#include "tinytc/builder.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"
#include "util/casting.hpp"

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdint>
#include <cstdlib>
#include <type_traits>
#include <utility>
#include <variant>

namespace tinytc {

using fold_result = std::variant<tinytc_value_t, inst>;

class constant_folding {
  public:
    constant_folding(bool unsafe_fp_math);

    auto operator()(inst_view) -> fold_result;
    auto operator()(arith_inst) -> fold_result;
    auto operator()(arith_unary_inst) -> fold_result;
    auto operator()(cooperative_matrix_scale_inst) -> fold_result;
    auto operator()(cast_inst) -> fold_result;
    auto operator()(compare_inst) -> fold_result;
    auto operator()(math_unary_inst) -> fold_result;
    auto operator()(size_inst in) -> fold_result;
    auto operator()(subgroup_broadcast_inst in) -> fold_result;

  private:
    auto get_memref_type(tinytc_value const &v) const -> const memref_data_type *;

    bool unsafe_fp_math_;
};

struct compute_unary_op {
    arithmetic_unary operation;
    data_type ty;
    location const &loc;

    auto operator()(bool a) -> fold_result {
        bool val = false;
        switch (operation) {
        case arithmetic_unary::not_:
            val = !a;
            break;
        default:
            throw compilation_error(loc, status::ir_boolean_unsupported);
        }
        return create<constant_inst>(val, ty, loc);
    }

    template <typename T>
    requires(std::is_integral_v<T> && !std::is_same_v<T, bool>)
    auto operator()(T a) -> fold_result {
        T val = 0;
        switch (operation) {
        case arithmetic_unary::abs:
            val = a < 0 ? -a : a;
            break;
        case arithmetic_unary::neg:
            val = -a;
            break;
        case arithmetic_unary::not_:
            val = ~a;
            break;
        default:
            throw compilation_error(loc, status::ir_int_unsupported);
        }
        return create<constant_inst>(val, ty, loc);
    }

    template <typename T>
    requires(is_floating_point_or_lp_float_v<T>)
    auto operator()(T a) -> fold_result {
        T val = 0;
        switch (operation) {
        case arithmetic_unary::abs:
            val = a < T{0} ? -a : a;
            break;
        case arithmetic_unary::neg:
            val = -a;
            break;
        default:
            throw compilation_error(loc, status::ir_fp_unsupported);
        }
        return create<constant_inst>(val, ty, loc);
    }

    template <typename T, typename U>
    requires(is_complex_v<T>)
    auto operator()(U const &A) -> fold_result {
        const auto neg_conj = [&](T const &a) {
            T val = {};
            switch (operation) {
            case arithmetic_unary::neg:
                val = -a;
                break;
            case arithmetic_unary::conj:
                val = std::conj(a);
                break;
            default:
                return inst{nullptr};
            }
            return create<constant_inst>(val, ty, loc);
        };
        const auto abs_im_re = [&](T const &a) -> inst {
            typename T::value_type val = {};
            switch (operation) {
            case arithmetic_unary::abs:
                val = std::abs(a);
                break;
            case arithmetic_unary::im:
                val = std::imag(a);
                break;
            case arithmetic_unary::re:
                val = std::real(a);
                break;
            default:
                return inst{nullptr};
            }
            scalar_data_type *sty = dyn_cast<scalar_data_type>(ty);
            if (!sty) {
                throw compilation_error(loc, status::ir_expected_scalar);
            }
            auto cst_ty = scalar_data_type::get(sty->context(), component_type(sty->ty()));
            return create<constant_inst>(val, cst_ty, loc);
        };

        const auto a = static_cast<T>(A);
        auto result = neg_conj(a);
        if (result) {
            return result;
        }
        result = abs_im_re(a);
        if (result) {
            return result;
        }
        throw compilation_error(loc, status::ir_complex_unsupported);
    }
};

struct compute_binary_op {
    arithmetic operation;
    data_type ty;
    location const &loc;

    auto operator()(bool a, bool b) -> fold_result {
        bool val = false;
        switch (operation) {
        case arithmetic::and_:
            val = a && b;
            break;
        case arithmetic::or_:
            val = a || b;
            break;
        case arithmetic::xor_:
            val = a != b;
            break;
        default:
            throw compilation_error(loc, status::ir_boolean_unsupported);
        }
        return create<constant_inst>(val, ty, loc);
    }

    template <typename T>
    requires(std::is_integral_v<T> && !std::is_same_v<T, bool>)
    auto operator()(T a, T b) -> fold_result {
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
            val = a << b;
            break;
        case arithmetic::shr:
            val = a >> b;
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
        case arithmetic::min:
            val = std::min(a, b);
            break;
        case arithmetic::max:
            val = std::max(a, b);
            break;
        }
        return create<constant_inst>(val, ty, loc);
    }

    template <typename T, typename U>
    requires(!std::is_integral_v<T>)
    auto operator()(U const &A, U const &B) -> fold_result {
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
            if constexpr (is_complex_v<T>) {
                throw compilation_error(loc, status::ir_complex_unsupported);
            } else {
                val = std::fmod(a, b);
            }
            break;
        case arithmetic::min:
            if constexpr (is_complex_v<T>) {
                throw compilation_error(loc, status::ir_complex_unsupported);
            } else {
                val = std::min(a, b);
            }
            break;
        case arithmetic::max:
            if constexpr (is_complex_v<T>) {
                throw compilation_error(loc, status::ir_complex_unsupported);
            } else {
                val = std::max(a, b);
            }
            break;
        default:
            if constexpr (is_complex_v<T>) {
                throw compilation_error(loc, status::ir_complex_unsupported);
            }
            throw compilation_error(loc, status::ir_fp_unsupported);
            break;
        }
        return create<constant_inst>(val, ty, loc);
    }
};

struct compute_binop_identities {
    bool unsafe_fp_math;
    arithmetic operation;
    tinytc_value &operand;
    bool is_second_operand;
    location const &loc;

    auto operator()(bool a) -> fold_result {
        switch (operation) {
        case arithmetic::and_:
            if (!a) {
                return create<constant_inst>(false, operand.ty(), loc);
            }
            break;
        case arithmetic::or_:
        case arithmetic::xor_:
            if (!a) {
                return &operand;
            }
            break;
        default:
            break;
        }
        return tinytc_value_t{};
    }

    template <typename T>
    requires(std::is_integral_v<T> && !std::is_same_v<T, bool>)
    auto operator()(T a) -> fold_result {
        switch (operation) {
        case arithmetic::add:
            if (a == T{0}) { // operand + 0 or 0 + operand
                return &operand;
            }
            break;
        case arithmetic::sub:
            if (a == T{0} && !is_second_operand) { // operand - 0
                return &operand;
            }
            break;
        case arithmetic::mul:
            if (a == T{0}) { // operand * 0 or 0 * operand
                return create<constant_inst>(T{0}, operand.ty(), loc);
            } else if (a == T{1}) { // operand * 1 or 1 * operand
                return &operand;
            }
            break;
        case arithmetic::div:
            if (a == T{1} && !is_second_operand) { // operand / 1
                return &operand;
            }
            break;
        case arithmetic::rem:
            if (a == T{1} && !is_second_operand) { // operand % 1
                return create<constant_inst>(T{0}, operand.ty(), loc);
            }
            break;
        case arithmetic::shl:
        case arithmetic::shr:
            if (a == T{0}) {
                if (is_second_operand) { // 0 << operand
                    return create<constant_inst>(T{0}, operand.ty(), loc);
                } else { // operand << 0
                    return &operand;
                }
            }
            break;
        case arithmetic::and_:
            if (a == T{0}) {
                return create<constant_inst>(T{0}, operand.ty(), loc);
            }
            break;
        case arithmetic::or_:
        case arithmetic::xor_:
            if (a == T{0}) {
                return &operand;
            }
            break;
        default:
            break;
        }
        return tinytc_value_t{};
    }

    template <typename T, typename U>
    requires(!std::is_integral_v<T>)
    auto operator()(U const &A) -> fold_result {
        const auto a = static_cast<T>(A);
        switch (operation) {
        case arithmetic::add:
            if (a == T{0}) { // operand + 0 or 0 + operand
                return &operand;
            }
            break;
        case arithmetic::sub:
            if (a == T{0} && !is_second_operand) { // operand - 0
                return &operand;
            }
            break;
        case arithmetic::mul:
            if (unsafe_fp_math && a == T{0}) { // operand * 0 or 0 * operand
                return create<constant_inst>(T{0}, operand.ty(), loc);
            } else if (a == T{1}) { // operand * 1 or 1 * operand
                return &operand;
            }
            break;
        case arithmetic::div:
            if (a == T{1} && !is_second_operand) { // operand / 1
                return &operand;
            }
            break;
        default:
            break;
        }
        return tinytc_value_t{};
    }
};

struct compute_compare {
    cmp_condition cond;
    data_type ty;
    location const &loc;

    template <typename T>
    requires(std::is_integral_v<T> || is_floating_point_or_lp_float_v<T>)
    auto operator()(T a, T b) -> fold_result {
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
        return create<constant_inst>(val, ty, loc);
    }

    template <typename T, typename F>
    auto operator()(std::complex<F> const &A, std::complex<F> const &B) -> fold_result {
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
        return create<constant_inst>(val, ty, loc);
    }
};

template <typename T, typename U> struct value_cast_impl {
    auto operator()(U const &u) { return static_cast<T>(u); }
};

template <typename F, typename U> struct value_cast_impl<std::complex<F>, U> {
    auto operator()(U const &u) { return std::complex<F>{static_cast<F>(u), static_cast<F>(0)}; }
};

template <typename F1, typename F2> struct value_cast_impl<std::complex<F1>, std::complex<F2>> {
    auto operator()(std::complex<F2> const &u) { return static_cast<std::complex<F1>>(u); }
};

template <typename U> struct value_cast_impl<bool, U> {
    auto operator()(U const &u) { return u != U{}; }
};

template <typename F> struct value_cast_impl<bool, std::complex<F>> {
    auto operator()(std::complex<F> const &) -> bool { throw status::ir_forbidden_cast; }
};

template <typename T, typename F> struct value_cast_impl<T, std::complex<F>> {
    auto operator()(std::complex<F> const &) -> T { throw status::ir_forbidden_cast; }
};

template <typename T, typename U> auto value_cast(U const &u) { return value_cast_impl<T, U>{}(u); }

template <typename T>
auto compute_cast(scalar_data_type *to_ty, T A, location const &loc) -> fold_result {
    switch (to_ty->ty()) {
    case scalar_type::i8:
        return create<constant_inst>(value_cast<std::int8_t>(A), to_ty, loc);
    case scalar_type::i16:
        return create<constant_inst>(value_cast<std::int16_t>(A), to_ty, loc);
    case scalar_type::i32:
        return create<constant_inst>(value_cast<std::int32_t>(A), to_ty, loc);
    case scalar_type::i64:
        return create<constant_inst>(value_cast<std::int64_t>(A), to_ty, loc);
    case scalar_type::index:
        return create<constant_inst>(value_cast<host_index_type>(A), to_ty, loc);
    case scalar_type::bf16:
        return create<constant_inst>(value_cast<bfloat16>(A), to_ty, loc);
    case scalar_type::f16:
        return create<constant_inst>(value_cast<half>(A), to_ty, loc);
    case scalar_type::f32:
        return create<constant_inst>(value_cast<float>(A), to_ty, loc);
    case scalar_type::f64:
        return create<constant_inst>(value_cast<double>(A), to_ty, loc);
    case scalar_type::c32:
        return create<constant_inst>(value_cast<std::complex<float>>(A), to_ty, loc);
    case scalar_type::c64:
        return create<constant_inst>(value_cast<std::complex<double>>(A), to_ty, loc);
    };
    return {};
};

struct compute_math_unary_op {
    math_unary operation;
    data_type ty;
    location const &loc;

    template <typename T>
    requires(std::is_integral_v<T>)
    auto operator()(T) -> fold_result {
        throw compilation_error(loc, status::ir_int_unsupported);
    }

    template <typename T>
    requires(is_floating_point_or_lp_float_v<T>)
    auto operator()(T a) -> fold_result {
        T val = {};
        switch (operation) {
        case math_unary::cos:
        case math_unary::native_cos:
            val = std::cos(a);
            break;
        case math_unary::sin:
        case math_unary::native_sin:
            val = std::sin(a);
            break;
        case math_unary::exp:
        case math_unary::native_exp:
            val = std::exp(a);
            break;
        case math_unary::exp2:
        case math_unary::native_exp2:
            val = std::exp2(a);
            break;
        default:
            throw compilation_error(loc, status::ir_fp_unsupported);
        }
        return create<constant_inst>(val, ty, loc);
    }

    template <typename T, typename U>
    requires(is_complex_v<T>)
    auto operator()(U const &a) -> fold_result {
        T val = {};
        switch (operation) {
        case math_unary::exp:
        case math_unary::native_exp:
            val = std::exp(a);
            break;
        case math_unary::exp2:
        case math_unary::native_exp2:
            val = std::pow(T{std::complex<double>{2.0, 0.0}}, a);
            break;
        default:
            throw compilation_error(loc, status::ir_complex_unsupported);
        }
        return create<constant_inst>(val, ty, loc);
    }
};

} // namespace tinytc

#endif // CONSTANT_FOLDING_HELPER_20241011_HPP
