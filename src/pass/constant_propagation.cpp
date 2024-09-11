// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/constant_propagation.hpp"
#include "error.hpp"
#include "node/data_type_node.hpp"
#include "node/value_node.hpp"
#include "support/casting.hpp"
#include "support/visit.hpp"
#include "tinytc/tinytc.hpp"

#include <cmath>

namespace tinytc {

/* Inst nodes */
void constant_propagation::operator()(inst_node &in) {
    for (auto &op : in.operands()) {
        if (op) {
            uintptr_t u = std::bit_cast<uintptr_t>(op.get());
            if (auto kc = known_constants_.find(u); kc != known_constants_.end()) {
                op = kc->second;
            }
        }
    }
}

void constant_propagation::operator()(arith_inst &arith) {
    this->operator()(static_cast<inst_node &>(arith));

    auto const &a = arith.a();
    auto const &b = arith.b();

    auto at = dyn_cast<scalar_data_type>(a->ty().get());
    if (at == nullptr) {
        throw compilation_error(a->loc(), status::ir_expected_scalar);
    }

    if (is_floating_type(at->ty())) {
        auto av = dyn_cast<float_imm>(a.get());
        auto bv = dyn_cast<float_imm>(b.get());
        if (av != nullptr && bv != nullptr) {
            auto const compute = [&arith](auto a, auto b) {
                switch (arith.operation()) {
                case arithmetic::add:
                    return a + b;
                case arithmetic::sub:
                    return a - b;
                case arithmetic::mul:
                    return a * b;
                case arithmetic::div:
                    return a / b;
                case arithmetic::rem:
                    return std::fmod(a, b);
                default:
                    break;
                }
                throw compilation_error(arith.loc(), status::ir_fp_unsupported);
            };

            auto constant_val = value{};
            switch (at->ty()) {
            case scalar_type::f32:
                constant_val = make_imm(
                    compute(static_cast<float>(av->value()), static_cast<float>(bv->value())),
                    scalar_type::f32, arith.loc());
                break;
            case scalar_type::f64:
                constant_val =
                    make_imm(compute(av->value(), bv->value()), scalar_type::f64, arith.loc());
                break;
            default:
                break;
            };
            if (constant_val) {
                uintptr_t u = std::bit_cast<uintptr_t>(arith.result().get());
                known_constants_[u] = std::move(constant_val);
            }
        }
    } else {
        auto av = dyn_cast<int_imm>(a.get());
        auto bv = dyn_cast<int_imm>(b.get());
        if (av != nullptr && bv != nullptr) {
            auto const compute = [&arith](auto a, auto b) {
                switch (arith.operation()) {
                case arithmetic::add:
                    return a + b;
                case arithmetic::sub:
                    return a - b;
                case arithmetic::mul:
                    return a * b;
                case arithmetic::div:
                    return a / b;
                case arithmetic::rem:
                    return a % b;
                case arithmetic::shl:
                    return a << b;
                case arithmetic::shr:
                    return a >> b;
                case arithmetic::and_:
                    return a & b;
                case arithmetic::or_:
                    return a | b;
                case arithmetic::xor_:
                    return a ^ b;
                }
                throw compilation_error(arith.loc(), status::runtime_error);
            };

            auto constant_val = value{};
            switch (at->ty()) {
            case scalar_type::i1: {
                bool const val =
                    compute(static_cast<bool>(av->value()), static_cast<bool>(bv->value()));
                constant_val =
                    make_imm(static_cast<std::int64_t>(val), scalar_type::i1, arith.loc());
                break;
            }
            case scalar_type::i8:
                constant_val = make_imm(compute(static_cast<std::int8_t>(av->value()),
                                                static_cast<std::int8_t>(bv->value())),
                                        arith.loc());
                break;
            case scalar_type::i16:
                constant_val = make_imm(compute(static_cast<std::int16_t>(av->value()),
                                                static_cast<std::int16_t>(bv->value())),
                                        arith.loc());
                break;
            case scalar_type::i32:
                constant_val = make_imm(compute(static_cast<std::int32_t>(av->value()),
                                                static_cast<std::int32_t>(bv->value())),
                                        arith.loc());
                break;
            case scalar_type::i64:
                constant_val =
                    make_imm(compute(av->value(), bv->value()), scalar_type::i64, arith.loc());
                break;
            case scalar_type::index:
                constant_val =
                    make_imm(compute(av->value(), bv->value()), scalar_type::index, arith.loc());
                break;
            default:
                break;
            };
            if (constant_val) {
                uintptr_t u = std::bit_cast<uintptr_t>(arith.result().get());
                known_constants_[u] = std::move(constant_val);
            }
        }
    }
}

void constant_propagation::operator()(parallel_inst &p) { visit(*this, *p.body()); }

/* Region nodes */
void constant_propagation::operator()(rgn &b) {
    for (auto &s : b.insts()) {
        visit(*this, *s);
    }
}

/* Function nodes */
void constant_propagation::operator()(function &fn) { visit(*this, *fn.body()); }

/* Program nodes */
void constant_propagation::operator()(program &p) {
    for (auto &fn : p.functions()) {
        visit(*this, *fn);
    }
}

} // namespace tinytc
