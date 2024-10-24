// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/constant_folding.hpp"
#include "error.hpp"
#include "node/data_type_node.hpp"
#include "node/inst_node.hpp"
#include "node/value_node.hpp"
#include "scalar_type.hpp"
#include "support/casting.hpp"
#include "support/visit.hpp"
#include "tinytc/tinytc.hpp"

#include <complex>
#include <cstdint>
#include <utility>
#include <variant>

namespace tinytc {

template <typename F> class unary_op_dispatcher {
  private:
    scalar_type switch_ty;
    F computer;

  public:
    unary_op_dispatcher(scalar_type sw_ty, F &&f)
        : switch_ty{sw_ty}, computer{std::forward<F>(f)} {}

    auto operator()(std::int64_t const &A) -> fold_result {
        switch (switch_ty) {
        case scalar_type::i1:
            return computer.template operator()<bool>(A);
        case scalar_type::i8:
            return computer.template operator()<std::int8_t>(A);
        case scalar_type::i16:
            return computer.template operator()<std::int16_t>(A);
        case scalar_type::i32:
            return computer.template operator()<std::int32_t>(A);
        case scalar_type::i64:
            return computer.template operator()<std::int64_t>(A);
        case scalar_type::index:
            return computer.template operator()<host_index_type>(A);
        default:
            throw compilation_error(computer.loc, status::ir_scalar_mismatch);
            break;
        };
    }
    auto operator()(double const &A) -> fold_result {
        switch (switch_ty) {
        case scalar_type::f32:
            return computer.template operator()<float>(A);
        case scalar_type::f64:
            return computer.template operator()<double>(A);
        default:
            throw compilation_error(computer.loc, status::ir_scalar_mismatch);
            break;
        }
    }
    auto operator()(std::complex<double> const &A) -> fold_result {
        switch (switch_ty) {
        case scalar_type::c32:
            return computer.template operator()<std::complex<float>>(A);
        case scalar_type::c64:
            return computer.template operator()<std::complex<double>>(A);
        default:
            throw compilation_error(computer.loc, status::ir_scalar_mismatch);
            break;
        }
    }
};

template <typename F> class binary_op_dispatcher {
  private:
    scalar_type switch_ty;
    F computer;

  public:
    binary_op_dispatcher(scalar_type sw_ty, F &&f)
        : switch_ty{sw_ty}, computer{std::forward<F>(f)} {}

    auto operator()(std::int64_t const &A, std::int64_t const &B) -> fold_result {
        switch (switch_ty) {
        case scalar_type::i1:
            return computer.template operator()<bool>(A, B);
        case scalar_type::i8:
            return computer.template operator()<std::int8_t>(A, B);
        case scalar_type::i16:
            return computer.template operator()<std::int16_t>(A, B);
        case scalar_type::i32:
            return computer.template operator()<std::int32_t>(A, B);
        case scalar_type::i64:
            return computer.template operator()<std::int64_t>(A, B);
        case scalar_type::index:
            return computer.template operator()<host_index_type>(A, B);
        default:
            throw compilation_error(computer.loc, status::ir_scalar_mismatch);
            break;
        };
    }
    auto operator()(double const &A, double const &B) -> fold_result {
        switch (switch_ty) {
        case scalar_type::f32:
            return computer.template operator()<float>(A, B);
        case scalar_type::f64:
            return computer.template operator()<double>(A, B);
        default:
            throw compilation_error(computer.loc, status::ir_scalar_mismatch);
            break;
        }
    }
    auto operator()(std::complex<double> const &A, std::complex<double> const &B) -> fold_result {
        switch (switch_ty) {
        case scalar_type::c32:
            return computer.template operator()<std::complex<float>>(A, B);
        case scalar_type::c64:
            return computer.template operator()<std::complex<double>>(A, B);
        default:
            throw compilation_error(computer.loc, status::ir_scalar_mismatch);
            break;
        }
    }
    template <typename T, typename U> auto operator()(T const &, U const &) -> fold_result {
        throw compilation_error(computer.loc, status::ir_scalar_mismatch);
    }
};

constant_folding::constant_folding(bool unsafe_fp_math) : unsafe_fp_math_(unsafe_fp_math) {}

auto constant_folding::get_memref_type(value_node const &v) const -> const memref_data_type * {
    auto t = dyn_cast<memref_data_type>(v.ty());
    if (t == nullptr) {
        throw compilation_error(v.loc(), status::ir_expected_memref);
    }
    return t;
}

auto constant_folding::operator()(inst_node &) -> fold_result { return tinytc_value_t{}; }

auto constant_folding::operator()(arith_inst &in) -> fold_result {
    auto &op_a = in.a();
    auto &op_b = in.b();

    constant_inst *a_const = dyn_cast<constant_inst>(op_a.defining_inst());
    constant_inst *b_const = dyn_cast<constant_inst>(op_b.defining_inst());

    auto at = dyn_cast<scalar_data_type>(op_a.ty());
    if (at == nullptr) {
        // Arithmetic on coopmatrix is component-wise and if a coopmatrix is constant, then all
        // elements have the same value. Thus, constant folding on coopmatrix types is identical to
        // constant folding on scalar types.
        auto ct = dyn_cast<coopmatrix_data_type>(op_a.ty());
        if (ct == nullptr) {
            throw compilation_error(op_a.loc(), status::ir_expected_coopmatrix_or_scalar);
        }
        at = dyn_cast<scalar_data_type>(ct->ty());
    }

    if (a_const != nullptr && b_const != nullptr) {
        auto computer = compute_binary_op{in.operation(), op_a.ty(), in.loc()};
        auto dispatcher = binary_op_dispatcher{at->ty(), std::move(computer)};
        return std::visit(std::move(dispatcher), a_const->value(), b_const->value());
    } else if (a_const != nullptr) {
        auto computer =
            compute_binop_identities{unsafe_fp_math_, in.operation(), op_b, true, in.loc()};
        auto dispatcher = unary_op_dispatcher{at->ty(), std::move(computer)};
        return std::visit(std::move(dispatcher), a_const->value());
    } else if (b_const != nullptr) {
        auto computer =
            compute_binop_identities{unsafe_fp_math_, in.operation(), op_a, false, in.loc()};
        auto dispatcher = unary_op_dispatcher{at->ty(), std::move(computer)};
        return std::visit(std::move(dispatcher), b_const->value());
    }
    return tinytc_value_t{};
}

auto constant_folding::operator()(arith_unary_inst &in) -> fold_result {
    auto &op_a = in.a();

    constant_inst *a_const = dyn_cast<constant_inst>(op_a.defining_inst());
    if (a_const == nullptr) {
        return tinytc_value_t{};
    }

    auto at = dyn_cast<scalar_data_type>(op_a.ty());
    if (at == nullptr) {
        // Arithmetic on coopmatrix is component-wise and if a coopmatrix is constant, then all
        // elements have the same value. Thus, constant folding on coopmatrix types is identical to
        // constant folding on scalar types.
        auto ct = dyn_cast<coopmatrix_data_type>(op_a.ty());
        if (ct == nullptr) {
            throw compilation_error(op_a.loc(), status::ir_expected_coopmatrix_or_scalar);
        }
        at = dyn_cast<scalar_data_type>(ct->ty());
    }

    auto computer = compute_unary_op{in.operation(), op_a.ty(), in.loc()};
    auto dispatcher = unary_op_dispatcher{at->ty(), std::move(computer)};
    return std::visit(std::move(dispatcher), a_const->value());
}

auto constant_folding::operator()(cast_inst &in) -> fold_result {
    auto &op_a = in.a();

    constant_inst *a_const = dyn_cast<constant_inst>(op_a.defining_inst());
    if (a_const == nullptr) {
        return tinytc_value_t{};
    }

    auto rt = dyn_cast<scalar_data_type>(in.result(0).ty());
    if (rt == nullptr) {
        // Cast on coopmatrix is component-wise and if a coopmatrix is constant, then all
        // elements have the same value. Thus, constant folding on coopmatrix types is identical to
        // constant folding on scalar types.
        auto ct = dyn_cast<coopmatrix_data_type>(in.result(0).ty());
        if (ct == nullptr) {
            throw compilation_error(in.result(0).loc(), status::ir_expected_coopmatrix_or_scalar);
        }
        rt = dyn_cast<scalar_data_type>(ct->ty());
    }

    return std::visit(
        overloaded{[&](auto A) -> fold_result { return compute_cast(rt, A, in.loc()); }},
        a_const->value());
}

auto constant_folding::operator()(compare_inst &in) -> fold_result {
    auto &op_a = in.a();
    auto &op_b = in.b();

    constant_inst *a_const = dyn_cast<constant_inst>(op_a.defining_inst());
    constant_inst *b_const = dyn_cast<constant_inst>(op_b.defining_inst());
    if (a_const == nullptr || b_const == nullptr) {
        return tinytc_value_t{};
    }

    auto at = dyn_cast<scalar_data_type>(op_a.ty());
    if (at == nullptr) {
        throw compilation_error(op_a.loc(), status::ir_expected_scalar);
    }

    auto computer = compute_compare{in.cond(), in.result(0).ty(), in.loc()};
    auto dispatcher = binary_op_dispatcher{at->ty(), std::move(computer)};
    return std::visit(std::move(dispatcher), a_const->value(), b_const->value());
}

auto constant_folding::operator()(cooperative_matrix_scale_inst &in) -> fold_result {
    auto &op_a = in.a();
    auto &op_b = in.b();

    constant_inst *a_const = dyn_cast<constant_inst>(op_a.defining_inst());

    auto at = dyn_cast<scalar_data_type>(op_a.ty());
    if (at == nullptr) {
        throw compilation_error(op_a.loc(), status::ir_expected_scalar);
    }

    if (a_const != nullptr) {
        auto computer =
            compute_binop_identities{unsafe_fp_math_, arithmetic::mul, op_b, true, in.loc()};
        auto dispatcher = unary_op_dispatcher{at->ty(), std::move(computer)};
        return std::visit(std::move(dispatcher), a_const->value());
    }
    return tinytc_value_t{};
}

auto constant_folding::operator()(size_inst &in) -> fold_result {
    auto ct = get_memref_type(in.operand());

    auto mode_size = ct->shape(in.mode());
    if (!is_dynamic_value(mode_size)) {
        return make_constant(
            mode_size, scalar_data_type::get(in.operand().context(), scalar_type::index), in.loc());
    }

    return tinytc_value_t{};
}

} // namespace tinytc