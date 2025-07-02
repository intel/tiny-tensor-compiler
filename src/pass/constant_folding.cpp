// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/constant_folding.hpp"
#include "error.hpp"
#include "node/inst.hpp"
#include "node/type.hpp"
#include "node/value.hpp"
#include "node/visit.hpp"
#include "number_dispatch.hpp"
#include "tinytc/tinytc.hpp"
#include "util/casting.hpp"
#include "util/overloaded.hpp"
#include "util/visit.hpp"

#include <complex>
#include <cstdint>
#include <utility>
#include <variant>

namespace tinytc {

template <typename F> class op_dispatcher {
  private:
    tinytc_type_t dispatch_ty;
    F computer;

  public:
    op_dispatcher(tinytc_type_t ty, F &&f) : dispatch_ty{ty}, computer{std::forward<F>(f)} {}

    template <typename... T>
    requires(std::is_same_v<T, std::int64_t> && ...)
    auto operator()(T &&...ops) -> fold_result {
        return dispatch_int_to_native(dispatch_ty, [&]<typename U>() {
            return computer.template operator()<U>(std::forward<T>(ops)...);
        });
    }
    template <typename... T>
    requires(std::is_same_v<T, double> && ...)
    auto operator()(T &&...ops) -> fold_result {
        return dispatch_float_to_native(dispatch_ty, [&]<typename U>() {
            return computer.template operator()<U>(std::forward<T>(ops)...);
        });
    }
    template <typename... T>
    requires(std::is_same_v<T, std::complex<double>> && ...)
    auto operator()(T &&...ops) -> fold_result {
        return dispatch_complex_to_native(dispatch_ty, [&]<typename U>() {
            return computer.template operator()<U>(std::forward<T>(ops)...);
        });
    }
    template <typename... T> auto operator()(T &&...) -> fold_result {
        throw compilation_error(computer.loc, status::ir_number_mismatch);
    }
};

constant_folding::constant_folding(bool unsafe_fp_math) : unsafe_fp_math_(unsafe_fp_math) {}

auto constant_folding::get_memref_type(tinytc_value const &v) const -> const memref_type * {
    auto t = dyn_cast<memref_type>(v.ty());
    if (t == nullptr) {
        throw compilation_error(v.loc(), status::ir_expected_memref);
    }
    return t;
}

auto constant_folding::operator()(inst_view) -> fold_result { return tinytc_value_t{}; }

auto constant_folding::operator()(arith_inst in) -> fold_result {
    auto &op_a = in.a();
    auto &op_b = in.b();

    constant_inst a_const = dyn_cast<constant_inst>(op_a.defining_inst());
    constant_inst b_const = dyn_cast<constant_inst>(op_b.defining_inst());

    if (isa<boolean_type>(*op_a.ty())) {
        if ((a_const && !std::holds_alternative<bool>(a_const.value())) ||
            (b_const && !std::holds_alternative<bool>(b_const.value()))) {
            throw compilation_error(in.loc(), status::internal_compiler_error);
        }
        if (a_const && b_const) {
            return compute_binary_op{in.get().type_id(), op_a.ty(), in.loc()}(
                std::get<bool>(a_const.value()), std::get<bool>(b_const.value()));
        } else if (a_const) {
            return compute_binop_identities{unsafe_fp_math_, in.get().type_id(), op_b, true,
                                            in.loc()}(std::get<bool>(a_const.value()));
        } else if (b_const) {
            return compute_binop_identities{unsafe_fp_math_, in.get().type_id(), op_a, false,
                                            in.loc()}(std::get<bool>(b_const.value()));
        }
        return tinytc_value_t{};
    }

    auto at = dyn_cast<number_type>(op_a.ty());
    if (at == nullptr) {
        // Arithmetic on coopmatrix is component-wise and if a coopmatrix is constant, then all
        // elements have the same value. Thus, constant folding on coopmatrix types is identical to
        // constant folding on scalar types.
        auto ct = dyn_cast<coopmatrix_type>(op_a.ty());
        if (ct == nullptr) {
            throw compilation_error(op_a.loc(), status::ir_expected_coopmatrix_number_or_boolean);
        }
        at = dyn_cast<number_type>(ct->component_ty());
    }

    if (a_const && b_const) {
        auto computer = compute_binary_op{in.get().type_id(), op_a.ty(), in.loc()};
        auto dispatcher = op_dispatcher{at, std::move(computer)};
        return std::visit(std::move(dispatcher), a_const.value(), b_const.value());
    } else if (a_const) {
        auto computer =
            compute_binop_identities{unsafe_fp_math_, in.get().type_id(), op_b, true, in.loc()};
        auto dispatcher = op_dispatcher{at, std::move(computer)};
        return std::visit(std::move(dispatcher), a_const.value());
    } else if (b_const) {
        auto computer =
            compute_binop_identities{unsafe_fp_math_, in.get().type_id(), op_a, false, in.loc()};
        auto dispatcher = op_dispatcher{at, std::move(computer)};
        return std::visit(std::move(dispatcher), b_const.value());
    }
    return tinytc_value_t{};
}

auto constant_folding::operator()(arith_unary_inst in) -> fold_result {
    auto &op_a = in.a();

    constant_inst a_const = dyn_cast<constant_inst>(op_a.defining_inst());
    if (!a_const) {
        return tinytc_value_t{};
    }

    if (isa<boolean_type>(*op_a.ty())) {
        if (!std::holds_alternative<bool>(a_const.value())) {
            throw compilation_error(in.loc(), status::internal_compiler_error);
        }
        return compute_unary_op{in.get().type_id(), op_a.ty(),
                                in.loc()}(std::get<bool>(a_const.value()));
    }

    auto at = dyn_cast<number_type>(op_a.ty());
    if (at == nullptr) {
        // Arithmetic on coopmatrix is component-wise and if a coopmatrix is constant, then all
        // elements have the same value. Thus, constant folding on coopmatrix types is identical to
        // constant folding on scalar types.
        auto ct = dyn_cast<coopmatrix_type>(op_a.ty());
        if (ct == nullptr) {
            throw compilation_error(op_a.loc(), status::ir_expected_coopmatrix_or_number);
        }
        at = dyn_cast<number_type>(ct->component_ty());
    }

    auto computer = compute_unary_op{in.get().type_id(), op_a.ty(), in.loc()};
    auto dispatcher = op_dispatcher{at, std::move(computer)};
    return std::visit(std::move(dispatcher), a_const.value());
}

auto constant_folding::operator()(cast_inst in) -> fold_result {
    auto &op_a = in.a();

    constant_inst a_const = dyn_cast<constant_inst>(op_a.defining_inst());
    if (!a_const) {
        return tinytc_value_t{};
    }

    auto rt = dyn_cast<number_type>(in.result().ty());
    if (rt == nullptr) {
        // Cast on coopmatrix is component-wise and if a coopmatrix is constant, then all
        // elements have the same value. Thus, constant folding on coopmatrix types is identical to
        // constant folding on scalar types.
        auto ct = dyn_cast<coopmatrix_type>(in.result().ty());
        if (ct == nullptr) {
            throw compilation_error(in.result().loc(), status::ir_expected_coopmatrix_or_number);
        }
        rt = dyn_cast<number_type>(ct->component_ty());
    }

    return std::visit(
        overloaded{[&](auto A) -> fold_result { return compute_cast(rt, A, in.loc()); }},
        a_const.value());
}

auto constant_folding::operator()(compare_inst in) -> fold_result {
    auto &op_a = in.a();
    auto &op_b = in.b();

    constant_inst a_const = dyn_cast<constant_inst>(op_a.defining_inst());
    constant_inst b_const = dyn_cast<constant_inst>(op_b.defining_inst());
    if (!a_const || !b_const) {
        return tinytc_value_t{};
    }

    auto at = dyn_cast<number_type>(op_a.ty());
    if (at == nullptr) {
        throw compilation_error(op_a.loc(), status::ir_expected_number);
    }

    auto computer = compute_compare{in.get().type_id(), in.result().ty(), in.loc()};
    auto dispatcher = op_dispatcher{at, std::move(computer)};
    return std::visit(std::move(dispatcher), a_const.value(), b_const.value());
}

auto constant_folding::operator()(cooperative_matrix_scale_inst in) -> fold_result {
    auto &op_a = in.a();
    auto &op_b = in.b();

    constant_inst a_const = dyn_cast<constant_inst>(op_a.defining_inst());

    auto at = dyn_cast<number_type>(op_a.ty());
    if (at == nullptr) {
        throw compilation_error(op_a.loc(), status::ir_expected_number);
    }

    if (a_const) {
        auto computer = compute_binop_identities{unsafe_fp_math_, IK::IK_mul, op_b, true, in.loc()};
        auto dispatcher = op_dispatcher{at, std::move(computer)};
        return std::visit(std::move(dispatcher), a_const.value());
    }
    return tinytc_value_t{};
}

auto constant_folding::operator()(math_unary_inst in) -> fold_result {
    auto &op_a = in.a();

    constant_inst a_const = dyn_cast<constant_inst>(op_a.defining_inst());
    if (!a_const) {
        return tinytc_value_t{};
    }

    auto at = dyn_cast<number_type>(op_a.ty());
    if (at == nullptr) {
        return tinytc_value_t{};
    }

    auto computer = compute_math_unary_op{in.get().type_id(), op_a.ty(), in.loc()};
    auto dispatcher = op_dispatcher{at, std::move(computer)};
    return std::visit(std::move(dispatcher), a_const.value());
}

auto constant_folding::operator()(size_inst in) -> fold_result {
    auto mode_size =
        visit(overloaded{[&](group_type const &g) -> std::int64_t { return g.size(); },
                         [&](memref_type const &m) -> std::int64_t { return m.shape(in.mode()); },
                         [&](auto const &) -> std::int64_t {
                             throw compilation_error(in.loc(), status::ir_expected_memref_or_group);
                         }},
              *in.operand().ty());

    if (!is_dynamic_value(mode_size)) {
        return create<constant_inst>(mode_size, index_type::get(in.operand().context()), in.loc());
    }
    return tinytc_value_t{};
}

auto constant_folding::operator()(subgroup_broadcast_inst in) -> fold_result {
    auto &op_a = in.a();

    constant_inst a_const = dyn_cast<constant_inst>(op_a.defining_inst());
    if (a_const) {
        return &a_const.result();
    }
    return tinytc_value_t{};
}

} // namespace tinytc
