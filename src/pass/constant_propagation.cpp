// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/constant_propagation.hpp"
#include "error.hpp"
#include "node/function_node.hpp"
#include "node/inst_node.hpp"
#include "node/region_node.hpp"
#include "pass/constant_propagation_helper.hpp"
#include "scalar_type.hpp"
#include "support/casting.hpp"
#include "support/visit.hpp"
#include "support/walk.hpp"
#include "tinytc/tinytc.hpp"

#include <cmath>
#include <complex>
#include <variant>

namespace tinytc {

template <typename F> class unary_op_dispatcher {
  private:
    scalar_type switch_ty;
    F computer;

  public:
    unary_op_dispatcher(scalar_type sw_ty, F &&f)
        : switch_ty{sw_ty}, computer{std::forward<F>(f)} {}

    auto operator()(std::int64_t const &A) -> inst {
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
    auto operator()(double const &A) -> inst {
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
    auto operator()(std::complex<double> const &A) -> inst {
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

    auto operator()(std::int64_t const &A, std::int64_t const &B) -> inst {
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
    auto operator()(double const &A, double const &B) -> inst {
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
    auto operator()(std::complex<double> const &A, std::complex<double> const &B) -> inst {
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
    template <typename T, typename U> auto operator()(T const &, U const &) -> inst {
        throw compilation_error(computer.loc, status::ir_scalar_mismatch);
    }
};

class constant_evaluator {
  public:
    auto operator()(inst_node &) -> inst;
    auto operator()(arith_inst &) -> inst;
    auto operator()(arith_unary_inst &) -> inst;
    auto operator()(cast_inst &) -> inst;
    auto operator()(compare_inst &) -> inst;
    auto operator()(size_inst &in) -> inst;

  private:
    auto get_memref_type(value_node const &v) const -> const memref_data_type *;
};

auto constant_evaluator::get_memref_type(value_node const &v) const -> const memref_data_type * {
    auto t = dyn_cast<memref_data_type>(v.ty());
    if (t == nullptr) {
        throw compilation_error(v.loc(), status::ir_expected_memref);
    }
    return t;
}

auto constant_evaluator::operator()(inst_node &) -> inst { return {}; }

auto constant_evaluator::operator()(arith_inst &in) -> inst {
    auto &op_a = in.a();
    auto &op_b = in.b();

    constant_inst *a_const = dyn_cast<constant_inst>(op_a.defining_inst());
    constant_inst *b_const = dyn_cast<constant_inst>(op_b.defining_inst());
    if (a_const == nullptr || b_const == nullptr) {
        return inst{};
    }

    auto at = dyn_cast<scalar_data_type>(op_a.ty());
    if (at == nullptr) {
        throw compilation_error(op_a.loc(), status::ir_expected_scalar);
    }

    auto computer = compute_binary_op{in.operation(), op_a.ty(), in.loc()};
    auto dispatcher = binary_op_dispatcher{at->ty(), std::move(computer)};
    return std::visit(std::move(dispatcher), a_const->value(), b_const->value());
}

auto constant_evaluator::operator()(arith_unary_inst &in) -> inst {
    auto &op_a = in.a();

    constant_inst *a_const = dyn_cast<constant_inst>(op_a.defining_inst());
    if (a_const == nullptr) {
        return inst{};
    }

    auto at = dyn_cast<scalar_data_type>(op_a.ty());
    if (at == nullptr) {
        throw compilation_error(op_a.loc(), status::ir_expected_scalar);
    }

    auto computer = compute_unary_op{in.operation(), op_a.ty(), in.loc()};
    auto dispatcher = unary_op_dispatcher{at->ty(), std::move(computer)};
    return std::visit(std::move(dispatcher), a_const->value());
}

auto constant_evaluator::operator()(cast_inst &in) -> inst {
    auto &op_a = in.a();

    constant_inst *a_const = dyn_cast<constant_inst>(op_a.defining_inst());
    if (a_const == nullptr) {
        return inst{};
    }

    auto rt = dyn_cast<scalar_data_type>(in.result(0).ty());
    if (rt == nullptr) {
        throw compilation_error(in.result(0).loc(), status::ir_expected_scalar);
    }

    return std::visit(overloaded{[&](auto A) -> inst { return compute_cast(rt, A, in.loc()); }},
                      a_const->value());
}

auto constant_evaluator::operator()(compare_inst &in) -> inst {
    auto &op_a = in.a();
    auto &op_b = in.b();

    constant_inst *a_const = dyn_cast<constant_inst>(op_a.defining_inst());
    constant_inst *b_const = dyn_cast<constant_inst>(op_b.defining_inst());
    if (a_const == nullptr || b_const == nullptr) {
        return inst{};
    }

    auto at = dyn_cast<scalar_data_type>(op_a.ty());
    if (at == nullptr) {
        throw compilation_error(op_a.loc(), status::ir_expected_scalar);
    }

    auto computer = compute_compare{in.cond(), in.result(0).ty(), in.loc()};
    auto dispatcher = binary_op_dispatcher{at->ty(), std::move(computer)};
    return std::visit(std::move(dispatcher), a_const->value(), b_const->value());
}

auto constant_evaluator::operator()(size_inst &in) -> inst {
    auto ct = get_memref_type(in.operand());

    auto mode_size = ct->shape(in.mode());
    if (!is_dynamic_value(mode_size)) {
        return make_constant(
            mode_size, scalar_data_type::get(in.operand().context(), scalar_type::index), in.loc());
    }

    return inst{};
}

void constant_propagation_pass::run_on_function(function_node &fn) {
    // @todo: Use worklist instead of pre-order?
    walk<walk_order::pre_order>(fn, [&](region_node &reg) {
        for (auto it = reg.begin(); it != reg.end(); ++it) {
            auto known_constant = visit(constant_evaluator{}, *it);
            if (known_constant) {
                // update uses
                if (it->num_results() != known_constant->num_results()) {
                    throw status::internal_compiler_error;
                }
                auto r_old = it->result_begin();
                auto r_new = known_constant->result_begin();
                for (; r_old != it->result_end() && r_new != known_constant->result_end();
                     ++r_old, ++r_new) {
                    r_new->name(r_old->name());
                    auto u = r_old->use_begin();
                    while (r_old->has_uses()) {
                        u->set(&*r_new);
                        u = r_old->use_begin();
                    }
                    if (r_old->has_uses()) {
                        throw status::internal_compiler_error;
                    }
                }
                // delete old instruction
                it = reg.insts().erase(it);
                // insert new instruction
                it = reg.insts().insert(it, known_constant.release());
            }
        }
    });
}

} // namespace tinytc
