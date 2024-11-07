// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "spv/converter.hpp"
#include "compiler_context.hpp"
#include "error.hpp"
#include "node/data_type_node.hpp"
#include "node/function_node.hpp"
#include "node/inst_node.hpp"
#include "node/program_node.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "scalar_type.hpp"
#include "spv/enums.hpp"
#include "spv/instructions.hpp"
#include "spv/opencl.std.hpp"
#include "spv/uniquifier.hpp"
#include "spv/visit.hpp"
#include "support/casting.hpp"
#include "support/ilist_base.hpp"
#include "support/util.hpp"
#include "support/visit.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <array>
#include <complex>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace tinytc::spv {

auto convert_prog_to_spirv(tinytc_prog const &p,
                           tinytc_core_info const &info) -> std::unique_ptr<mod> {
    auto m = std::make_unique<mod>();

    auto conv = inst_converter{p.context(), *m};

    conv.unique().capability(Capability::Addresses);
    conv.unique().capability(Capability::Kernel);
    conv.unique().capability(Capability::SubgroupDispatch);

    m->add_to<OpMemoryModel>(section::memory_model, AddressingModel::Physical64,
                             MemoryModel::OpenCL);

    for (auto const &fn : p) {
        try {
            conv.run_on_function(fn, info.get_core_config(fn.subgroup_size()));
        } catch (std::out_of_range const &e) {
            throw compilation_error(fn.loc(), status::unsupported_subgroup_size);
        }
    }

    // Add missing capabilites
    for (std::int32_t s = 0; s < num_module_sections; ++s) {
        for (auto const &i : m->insts(enum_cast<section>(s))) {
            visit(overloaded{[&]<spv_inst_with_required_capabilities I>(I const &) {
                                 for (auto const &cap : I::required_capabilities) {
                                     conv.unique().capability(cap);
                                 }
                             },
                             [&](auto const &) {}},
                  i);
        }
    }

    return m;
}

inst_converter::inst_converter(tinytc_compiler_context_t ctx, mod &m)
    : ctx_(ctx), mod_(&m), unique_(ctx, m) {}

auto inst_converter::get_scalar_type(value_node const &v) -> scalar_type {
    auto st = dyn_cast<scalar_data_type>(v.ty());
    if (!st) {
        throw compilation_error(v.loc(), status::ir_expected_scalar);
    }
    return st->ty();
}
auto inst_converter::get_coopmatrix_type(value_node const &v) -> scalar_type {
    auto ct = dyn_cast<coopmatrix_data_type>(v.ty());
    if (!ct) {
        throw compilation_error(v.loc(), status::ir_expected_coopmatrix);
    }
    return ct->component_ty();
}

auto inst_converter::load_builtin(BuiltIn b) -> spv_inst * {
    auto builtin = unique_.builtin_var(b);
    if (auto it = std::find(builtins_used_by_function_.begin(), builtins_used_by_function_.end(),
                            builtin);
        it == builtins_used_by_function_.end()) {
        builtins_used_by_function_.push_back(builtin);
    }
    return mod_->add<OpLoad>(unique_.builtin_pointee_ty(b), builtin, MemoryAccess::Aligned,
                             unique_.builtin_alignment(b));
}

auto inst_converter::declare(value_node const &v, spv_inst *in) { vals_[&v] = in; }
auto inst_converter::val(value_node const &v) -> spv_inst * {
    if (auto it = vals_.find(&v); it != vals_.end()) {
        return it->second;
    }
    throw compilation_error(v.loc(), status::spirv_undefined_value);
}
auto inst_converter::multi_declare(value_node const &v, std::vector<spv_inst *> insts) {
    multi_vals_[&v] = std::move(insts);
}
auto inst_converter::multi_val(value_node const &v) -> std::vector<spv_inst *> & {
    if (auto it = multi_vals_.find(&v); it != multi_vals_.end()) {
        return it->second;
    }
    throw compilation_error(v.loc(), status::spirv_undefined_value);
}

void inst_converter::operator()(inst_node const &in) {
    // @todo
    throw compilation_error(in.loc(), status::not_implemented);
}

void inst_converter::operator()(arith_inst const &in) {
    auto const make_boolean = [&](arithmetic op, spv_inst *ty, spv_inst *a,
                                  spv_inst *b) -> spv_inst * {
        switch (op) {
        case arithmetic::and_:
            return mod_->add<OpLogicalAnd>(ty, a, b);
        case arithmetic::or_:
            return mod_->add<OpLogicalOr>(ty, a, b);
        case arithmetic::xor_:
            return mod_->add<OpLogicalNotEqual>(ty, a, b);
        default:
            break;
        }
        throw compilation_error(in.loc(), status::ir_boolean_unsupported);
    };
    auto const make_int = [&](arithmetic op, spv_inst *ty, spv_inst *a, spv_inst *b) -> spv_inst * {
        switch (op) {
        case arithmetic::add:
            return mod_->add<OpIAdd>(ty, a, b);
        case arithmetic::sub:
            return mod_->add<OpISub>(ty, a, b);
        case arithmetic::mul:
            return mod_->add<OpIMul>(ty, a, b);
        case arithmetic::div:
            return mod_->add<OpSDiv>(ty, a, b);
        case arithmetic::rem:
            return mod_->add<OpSRem>(ty, a, b);
        case arithmetic::shl:
            return mod_->add<OpShiftLeftLogical>(ty, a, b);
        case arithmetic::shr:
            return mod_->add<OpShiftRightArithmetic>(ty, a, b);
        case arithmetic::and_:
            return mod_->add<OpBitwiseAnd>(ty, a, b);
        case arithmetic::or_:
            return mod_->add<OpBitwiseOr>(ty, a, b);
        case arithmetic::xor_:
            return mod_->add<OpBitwiseXor>(ty, a, b);
        }
        throw compilation_error(in.loc(), status::internal_compiler_error);
    };
    auto const make_float_complex = [&](arithmetic op, spv_inst *ty, spv_inst *a,
                                        spv_inst *b) -> spv_inst * {
        switch (op) {
        case arithmetic::add:
            return mod_->add<OpFAdd>(ty, a, b);
        case arithmetic::sub:
            return mod_->add<OpFSub>(ty, a, b);
        case arithmetic::mul:
            return mod_->add<OpFMul>(ty, a, b);
        case arithmetic::div:
            return mod_->add<OpFDiv>(ty, a, b);
        case arithmetic::rem:
            return mod_->add<OpFRem>(ty, a, b);
        default:
            break;
        }
        throw compilation_error(in.loc(), status::ir_fp_unsupported);
    };
    auto const make = [&](scalar_type sty, arithmetic op, spv_inst *ty, spv_inst *a,
                          spv_inst *b) -> spv_inst * {
        switch (sty) {
        case scalar_type::i8:
        case scalar_type::i16:
        case scalar_type::i32:
        case scalar_type::i64:
        case scalar_type::index:
            return make_int(op, ty, a, b);
        case scalar_type::f32:
        case scalar_type::f64:
        case scalar_type::c32:
        case scalar_type::c64:
            return make_float_complex(op, ty, a, b);
        }
        throw compilation_error(in.loc(), status::internal_compiler_error);
    };

    auto ty = unique_.spv_ty(in.result(0).ty());

    if (isa<boolean_data_type>(*in.result(0).ty())) {
        auto av = val(in.a());
        auto bv = val(in.b());
        declare(in.result(0), make_boolean(in.operation(), ty, av, bv));
    } else if (auto st = dyn_cast<scalar_data_type>(in.result(0).ty()); st) {
        auto av = val(in.a());
        auto bv = val(in.b());
        declare(in.result(0), make(st->ty(), in.operation(), ty, av, bv));
    } else if (auto ct = dyn_cast<coopmatrix_data_type>(in.result(0).ty()); ct) {
        auto const length = ct->length(core_cfg_.subgroup_size);
        auto insts = std::vector<spv_inst *>{};
        insts.reserve(length);

        auto &av = multi_val(in.a());
        auto &bv = multi_val(in.b());
        for (std::int64_t i = 0; i < length; ++i) {
            insts.emplace_back(make(ct->component_ty(), in.operation(), ty, av[i], bv[i]));
        }

        multi_declare(in.result(0), std::move(insts));
    } else {
        throw compilation_error(in.loc(), status::ir_expected_coopmatrix_or_scalar);
    }
}

void inst_converter::operator()(arith_unary_inst const &in) {
    auto const make_boolean = [&](arithmetic_unary op, spv_inst *ty, spv_inst *a) -> spv_inst * {
        switch (op) {
        case arithmetic_unary::not_:
            return mod_->add<OpLogicalNot>(ty, a);
        default:
            break;
        }
        throw compilation_error(in.loc(), status::ir_boolean_unsupported);
    };
    auto const make_int = [&](arithmetic_unary op, spv_inst *ty, spv_inst *a) -> spv_inst * {
        switch (op) {
        case arithmetic_unary::abs:
            return mod_->add<OpExtInst>(ty, unique_.opencl_ext(),
                                        static_cast<std::int32_t>(OpenCLEntrypoint::s_abs),
                                        std::vector<IdRef>{a});
        case arithmetic_unary::neg:
            return mod_->add<OpSNegate>(ty, a);
        case arithmetic_unary::not_:
            return mod_->add<OpNot>(ty, a);
        default:
            break;
        }
        throw compilation_error(in.loc(), status::internal_compiler_error);
    };
    auto const make_float = [&](arithmetic_unary op, spv_inst *ty, spv_inst *a) -> spv_inst * {
        switch (op) {
        case arithmetic_unary::abs:
            return mod_->add<OpExtInst>(ty, unique_.opencl_ext(),
                                        static_cast<std::int32_t>(OpenCLEntrypoint::fabs),
                                        std::vector<IdRef>{a});
        case arithmetic_unary::neg:
            return mod_->add<OpFNegate>(ty, a);
        default:
            break;
        }
        throw compilation_error(in.loc(), status::internal_compiler_error);
    };
    auto const make_complex = [&](arithmetic_unary op, scalar_type sty, spv_inst *ty,
                                  spv_inst *a) -> spv_inst * {
        switch (op) {
        case arithmetic_unary::abs: {
            auto spv_a_ty = unique_.spv_ty(scalar_data_type::get(ctx_, sty));
            auto a2 = mod_->add<OpFMul>(spv_a_ty, a, a);
            auto a2_0 = mod_->add<OpCompositeExtract>(ty, a2, std::vector<LiteralInteger>{0});
            auto a2_1 = mod_->add<OpCompositeExtract>(ty, a2, std::vector<LiteralInteger>{1});
            auto a2_0p1 = mod_->add<OpFAdd>(ty, a2_0, a2_1);
            return mod_->add<OpExtInst>(ty, unique_.opencl_ext(),
                                        static_cast<std::int32_t>(OpenCLEntrypoint::sqrt),
                                        std::vector<IdRef>{a2_0p1});
        }
        case arithmetic_unary::neg:
            return mod_->add<OpFNegate>(ty, a);
        case arithmetic_unary::conj: {
            auto spv_float_ty = unique_.spv_ty(scalar_data_type::get(ctx_, element_type(sty)));
            auto a_im =
                mod_->add<OpCompositeExtract>(spv_float_ty, a, std::vector<LiteralInteger>{1});
            auto neg_a_im = mod_->add<OpFNegate>(spv_float_ty, a_im);
            return mod_->add<OpCompositeInsert>(ty, neg_a_im, a, std::vector<LiteralInteger>{1});
        }
        case arithmetic_unary::im:
            return mod_->add<OpCompositeExtract>(ty, a, std::vector<LiteralInteger>{1});
        case arithmetic_unary::re:
            return mod_->add<OpCompositeExtract>(ty, a, std::vector<LiteralInteger>{0});
        default:
            break;
        }
        throw compilation_error(in.loc(), status::internal_compiler_error);
    };
    auto const make = [&](scalar_type sty, arithmetic_unary op, spv_inst *ty,
                          spv_inst *a) -> spv_inst * {
        switch (sty) {
        case scalar_type::i8:
        case scalar_type::i16:
        case scalar_type::i32:
        case scalar_type::i64:
        case scalar_type::index:
            return make_int(op, ty, a);
        case scalar_type::f32:
        case scalar_type::f64:
            return make_float(op, ty, a);
        case scalar_type::c32:
        case scalar_type::c64: {
            return make_complex(op, sty, ty, a);
        }
        }
        throw compilation_error(in.loc(), status::internal_compiler_error);
    };

    auto ty = unique_.spv_ty(in.result(0).ty());
    if (isa<boolean_data_type>(*in.a().ty())) {
        auto av = val(in.a());
        declare(in.result(0), make_boolean(in.operation(), ty, av));
    } else if (auto st = dyn_cast<scalar_data_type>(in.a().ty()); st) {
        auto av = val(in.a());
        declare(in.result(0), make(st->ty(), in.operation(), ty, av));
    } else if (auto ct = dyn_cast<coopmatrix_data_type>(in.a().ty()); ct) {
        auto const length = ct->length(core_cfg_.subgroup_size);
        auto insts = std::vector<spv_inst *>{};
        insts.reserve(length);

        auto &av = multi_val(in.a());
        for (std::int64_t i = 0; i < length; ++i) {
            insts.emplace_back(make(ct->component_ty(), in.operation(), ty, av[i]));
        }

        multi_declare(in.result(0), std::move(insts));
    } else {
        throw compilation_error(in.loc(), status::ir_expected_coopmatrix_or_scalar);
    }
}

void inst_converter::operator()(barrier_inst const &in) {
    std::int32_t fence = 0;
    if (in.has_fence(address_space::global)) {
        fence = fence | static_cast<std::int32_t>(MemorySemantics::CrossWorkgroupMemory) |
                static_cast<std::int32_t>(MemorySemantics::SequentiallyConsistent);
    }
    if (in.has_fence(address_space::local)) {
        fence = fence | static_cast<std::int32_t>(MemorySemantics::WorkgroupMemory) |
                static_cast<std::int32_t>(MemorySemantics::SequentiallyConsistent);
    }
    auto scope = unique_.i32_constant(static_cast<std::int32_t>(Scope::Workgroup));
    auto memory_semantics = unique_.i32_constant(fence);
    mod_->add<OpControlBarrier>(scope, scope, memory_semantics);
}

void inst_converter::operator()(cast_inst const &in) {
    auto const cast_from_int = [&](scalar_type to_ty, spv_inst *spv_to_ty,
                                   spv_inst *a) -> spv_inst * {
        switch (to_ty) {
        case scalar_type::i8:
        case scalar_type::i16:
        case scalar_type::i32:
        case scalar_type::i64:
        case scalar_type::index:
            return mod_->add<OpSConvert>(spv_to_ty, a);
        case scalar_type::f32:
        case scalar_type::f64:
            return mod_->add<OpConvertSToF>(spv_to_ty, a);
        case scalar_type::c32:
        case scalar_type::c64: {
            auto spv_float_ty = unique_.spv_ty(scalar_data_type::get(ctx_, element_type(to_ty)));
            auto re = mod_->add<OpConvertSToF>(spv_float_ty, a);
            return mod_->add<OpCompositeInsert>(spv_to_ty, re, unique_.null_constant(spv_to_ty),
                                                std::vector<LiteralInteger>{0});
        }
        }
        throw compilation_error(in.loc(), status::ir_forbidden_cast);
    };
    auto const cast_from_float = [&](scalar_type to_ty, spv_inst *spv_to_ty,
                                     spv_inst *a) -> spv_inst * {
        switch (to_ty) {
        case scalar_type::i8:
        case scalar_type::i16:
        case scalar_type::i32:
        case scalar_type::i64:
        case scalar_type::index:
            return mod_->add<OpConvertFToS>(spv_to_ty, a);
        case scalar_type::f32:
        case scalar_type::f64:
            return mod_->add<OpFConvert>(spv_to_ty, a);
        case scalar_type::c32:
        case scalar_type::c64: {
            auto spv_float_ty = unique_.spv_ty(scalar_data_type::get(ctx_, element_type(to_ty)));
            auto re = mod_->add<OpFConvert>(spv_float_ty, a);
            return mod_->add<OpCompositeInsert>(spv_to_ty, re, unique_.null_constant(spv_to_ty),
                                                std::vector<LiteralInteger>{0});
        }
        }
        throw compilation_error(in.loc(), status::ir_forbidden_cast);
    };
    auto const cast_from_complex = [&](scalar_type to_ty, spv_inst *spv_to_ty,
                                       spv_inst *a) -> spv_inst * {
        switch (to_ty) {
        case scalar_type::c32:
        case scalar_type::c64:
            return mod_->add<OpFConvert>(spv_to_ty, a);
        default:
            throw compilation_error(in.loc(), status::ir_forbidden_cast);
        }
    };
    auto const make = [&](scalar_type to_ty, scalar_type a_ty, spv_inst *spv_to_ty,
                          spv_inst *a) -> spv_inst * {
        if (a_ty == to_ty) {
            return mod_->add<OpCopyObject>(spv_to_ty, a);
        }
        switch (a_ty) {
        case scalar_type::i8:
        case scalar_type::i16:
        case scalar_type::i32:
        case scalar_type::i64:
        case scalar_type::index:
            return cast_from_int(to_ty, spv_to_ty, a);
        case scalar_type::f32:
        case scalar_type::f64:
            return cast_from_float(to_ty, spv_to_ty, a);
        case scalar_type::c32:
        case scalar_type::c64: {
            return cast_from_complex(to_ty, spv_to_ty, a);
        }
        }
        throw compilation_error(in.loc(), status::internal_compiler_error);
    };

    auto spv_to_ty = unique_.spv_ty(in.result(0).ty());

    if (auto st = dyn_cast<scalar_data_type>(in.result(0).ty()); st) {
        auto av = val(in.a());
        auto a_ty = get_scalar_type(in.a());
        declare(in.result(0), make(st->ty(), a_ty, spv_to_ty, av));
    } else if (auto ct = dyn_cast<coopmatrix_data_type>(in.result(0).ty()); ct) {
        auto const length = ct->length(core_cfg_.subgroup_size);
        auto insts = std::vector<spv_inst *>{};
        insts.reserve(length);

        auto &av = multi_val(in.a());
        auto a_ty = get_coopmatrix_type(in.a());
        for (std::int64_t i = 0; i < length; ++i) {
            insts.emplace_back(make(ct->component_ty(), a_ty, spv_to_ty, av[i]));
        }

        multi_declare(in.result(0), std::move(insts));
    } else {
        throw compilation_error(in.loc(), status::ir_expected_coopmatrix_or_scalar);
    }
}

void inst_converter::operator()(compare_inst const &in) {
    auto const compare_int = [&](cmp_condition cond, spv_inst *spv_to_ty, spv_inst *a,
                                 spv_inst *b) -> spv_inst * {
        switch (cond) {
        case cmp_condition::eq:
            return mod_->add<OpIEqual>(spv_to_ty, a, b);
        case cmp_condition::ne:
            return mod_->add<OpINotEqual>(spv_to_ty, a, b);
        case cmp_condition::gt:
            return mod_->add<OpSGreaterThan>(spv_to_ty, a, b);
        case cmp_condition::ge:
            return mod_->add<OpSGreaterThanEqual>(spv_to_ty, a, b);
        case cmp_condition::lt:
            return mod_->add<OpSLessThan>(spv_to_ty, a, b);
        case cmp_condition::le:
            return mod_->add<OpSLessThanEqual>(spv_to_ty, a, b);
        }
        throw compilation_error(in.loc(), status::internal_compiler_error);
    };
    auto const compare_float = [&](cmp_condition cond, spv_inst *spv_to_ty, spv_inst *a,
                                   spv_inst *b) -> spv_inst * {
        switch (cond) {
        case cmp_condition::eq:
            return mod_->add<OpFOrdEqual>(spv_to_ty, a, b);
        case cmp_condition::ne:
            return mod_->add<OpFUnordNotEqual>(spv_to_ty, a, b);
        case cmp_condition::gt:
            return mod_->add<OpFOrdGreaterThan>(spv_to_ty, a, b);
        case cmp_condition::ge:
            return mod_->add<OpFOrdGreaterThanEqual>(spv_to_ty, a, b);
        case cmp_condition::lt:
            return mod_->add<OpFOrdLessThan>(spv_to_ty, a, b);
        case cmp_condition::le:
            return mod_->add<OpFOrdLessThanEqual>(spv_to_ty, a, b);
        }
        throw compilation_error(in.loc(), status::internal_compiler_error);
    };
    auto const compare_complex = [&](cmp_condition cond, spv_inst *spv_to_ty, spv_inst *a,
                                     spv_inst *b) -> spv_inst * {
        switch (cond) {
        case cmp_condition::eq: {
            auto components_equal = mod_->add<OpFOrdEqual>(unique_.bool2_ty(), a, b);
            return mod_->add<OpAll>(spv_to_ty, components_equal);
        }
        case cmp_condition::ne: {
            auto components_not_equal = mod_->add<OpFUnordNotEqual>(unique_.bool2_ty(), a, b);
            return mod_->add<OpAll>(spv_to_ty, components_not_equal);
        }
        default:
            throw compilation_error(in.loc(), status::ir_complex_unsupported);
        }
    };
    auto const make = [&](scalar_type a_ty, cmp_condition cond, spv_inst *spv_to_ty, spv_inst *a,
                          spv_inst *b) -> spv_inst * {
        switch (a_ty) {
        case scalar_type::i8:
        case scalar_type::i16:
        case scalar_type::i32:
        case scalar_type::i64:
        case scalar_type::index:
            return compare_int(cond, spv_to_ty, a, b);
        case scalar_type::f32:
        case scalar_type::f64:
            return compare_float(cond, spv_to_ty, a, b);
        case scalar_type::c32:
        case scalar_type::c64:
            return compare_complex(cond, spv_to_ty, a, b);
        }
        throw compilation_error(in.loc(), status::internal_compiler_error);
    };

    auto spv_to_ty = unique_.spv_ty(in.result(0).ty());
    auto av = val(in.a());
    auto bv = val(in.b());
    auto a_ty = get_scalar_type(in.a());
    declare(in.result(0), make(a_ty, in.cond(), spv_to_ty, av, bv));
}

void inst_converter::operator()(constant_inst const &in) {
    auto const make = [&](scalar_type sty, spv_inst *spv_ty,
                          constant_inst::value_type const &val) -> spv_inst * {
        auto const add_constant = [this, &spv_ty](auto val) -> spv_inst * {
            return mod_->add_to<OpConstant>(section::type_const_var, spv_ty, val);
        };
        auto const add_constant_complex = [this, &spv_ty](spv_inst *spv_float_ty, auto re,
                                                          auto im) -> spv_inst * {
            auto c_re = mod_->add_to<OpConstant>(section::type_const_var, spv_float_ty, re);
            auto c_im = mod_->add_to<OpConstant>(section::type_const_var, spv_float_ty, im);
            return mod_->add_to<OpConstantComposite>(section::type_const_var, spv_ty,
                                                     std::vector<spv_inst *>{c_re, c_im});
        };
        const auto visitor = overloaded{
            [&](bool) -> spv_inst * { return nullptr; },
            [&](std::int64_t i) -> spv_inst * {
                switch (sty) {
                case scalar_type::i8:
                    return add_constant(static_cast<std::int8_t>(i));
                case scalar_type::i16:
                    return add_constant(static_cast<std::int16_t>(i));
                case scalar_type::i32:
                    return add_constant(static_cast<std::int32_t>(i));
                case scalar_type::i64:
                case scalar_type::index:
                    return add_constant(i);
                default:
                    return nullptr;
                }
            },
            [&](double d) -> spv_inst * {
                switch (sty) {
                case scalar_type::f32:
                    return add_constant(static_cast<float>(d));
                case scalar_type::f64:
                    return add_constant(d);
                default:
                    return nullptr;
                }
            },
            [&](std::complex<double> d) -> spv_inst * {
                switch (sty) {
                case scalar_type::c32: {
                    auto spv_float_ty =
                        unique_.spv_ty(scalar_data_type::get(ctx_, scalar_type::f32));
                    return add_constant_complex(spv_float_ty, static_cast<float>(d.real()),
                                                static_cast<float>(d.imag()));
                }
                case scalar_type::c64: {
                    auto spv_float_ty =
                        unique_.spv_ty(scalar_data_type::get(ctx_, scalar_type::f64));
                    return add_constant_complex(spv_float_ty, d.real(), d.imag());
                }
                default:
                    return nullptr;
                }
            },
        };
        auto cst = std::visit(visitor, val);
        if (cst == nullptr) {
            throw compilation_error(in.loc(), status::internal_compiler_error);
        }
        return cst;
    };

    auto spv_ty = unique_.spv_ty(in.result(0).ty());

    if (isa<boolean_data_type>(*in.result(0).ty())) {
        if (!std::holds_alternative<bool>(in.value())) {
            throw compilation_error(in.loc(), status::internal_compiler_error);
        }
        declare(in.result(0), unique_.bool_constant(std::get<bool>(in.value())));
    } else if (auto st = dyn_cast<scalar_data_type>(in.result(0).ty()); st) {
        declare(in.result(0), make(st->ty(), spv_ty, in.value()));
    } else if (auto ct = dyn_cast<coopmatrix_data_type>(in.result(0).ty()); ct) {
        auto const length = ct->length(core_cfg_.subgroup_size);
        auto cst = make(ct->component_ty(), spv_ty, in.value());

        multi_declare(in.result(0), std::vector<spv_inst *>(length, cst));
    } else {
        throw compilation_error(in.loc(), status::ir_expected_coopmatrix_or_scalar);
    }
}

void inst_converter::operator()(group_id_inst const &in) {
    auto gid = load_builtin(BuiltIn::GlobalInvocationId);
    auto index_ty = unique_.spv_ty(scalar_data_type::get(ctx_, scalar_type::index));
    declare(in.result(0),
            mod_->add<OpCompositeExtract>(index_ty, gid, std::vector<LiteralInteger>{2}));
}
void inst_converter::operator()(group_size_inst const &in) {
    auto gs = load_builtin(BuiltIn::GlobalSize);
    auto index_ty = unique_.spv_ty(scalar_data_type::get(ctx_, scalar_type::index));
    declare(in.result(0),
            mod_->add<OpCompositeExtract>(index_ty, gs, std::vector<LiteralInteger>{2}));
}
void inst_converter::operator()(num_subgroups_inst const &in) {
    declare(in.result(0), load_builtin(BuiltIn::NumSubgroups));
}

void inst_converter::operator()(parallel_inst const &in) { run_on_region(in.body()); }

void inst_converter::operator()(subgroup_id_inst const &in) {
    declare(in.result(0), load_builtin(BuiltIn::SubgroupId));
}
void inst_converter::operator()(subgroup_local_id_inst const &in) {
    declare(in.result(0), load_builtin(BuiltIn::SubgroupLocalInvocationId));
}
void inst_converter::operator()(subgroup_size_inst const &in) {
    declare(in.result(0), load_builtin(BuiltIn::SubgroupSize));
}

void inst_converter::operator()(work_group_inst const &in) {
    auto const make = [&](scalar_type sty, work_group_operation operation, spv_inst *spv_ty,
                          spv_inst *operand) -> spv_inst * {
        auto scope = unique_.i32_constant(static_cast<std::int32_t>(Scope::Workgroup));
        if (operation == work_group_operation::reduce_add) {
            switch (sty) {
            case scalar_type::i8:
            case scalar_type::i16:
            case scalar_type::i32:
            case scalar_type::i64:
            case scalar_type::index:
                return mod_->add<OpGroupIAdd>(spv_ty, scope, GroupOperation::Reduce, operand);
            case scalar_type::f32:
            case scalar_type::f64:
            case scalar_type::c32:
            case scalar_type::c64:
                return mod_->add<OpGroupFAdd>(spv_ty, scope, GroupOperation::Reduce, operand);
            }
        }
        throw compilation_error(in.loc(), status::not_implemented);
    };

    auto spv_ty = unique_.spv_ty(in.result(0).ty());
    auto sty = get_scalar_type(in.operand());
    declare(in.result(0), make(sty, in.operation(), spv_ty, val(in.operand())));
}

void inst_converter::run_on_region(region_node const &reg) {
    for (auto const &i : reg) {
        visit(*this, i);
    }
}

void inst_converter::run_on_function(function_node const &fn, core_config const &core_cfg) {
    core_cfg_ = core_cfg;

    // Function type
    auto fun_ty = unique_.spv_function_ty([&] {
        auto params = std::vector<spv_inst *>{};
        params.reserve(fn.num_params());
        for (auto const &p : fn.params()) {
            params.emplace_back(unique_.spv_ty(p.ty()));
        }
        return params;
    }());

    // Function
    auto void_ty = unique_.spv_ty(void_data_type::get(ctx_));
    auto fun = mod_->add<OpFunction>(void_ty, FunctionControl::None, fun_ty);
    for (auto const &p : fn.params()) {
        declare(p, mod_->add<OpFunctionParameter>(unique_.spv_ty(p.ty())));
    }
    mod_->add<OpLabel>();
    run_on_region(fn.body());
    mod_->add<OpReturn>();
    mod_->add<OpFunctionEnd>();

    // Entry point
    mod_->add_to<OpEntryPoint>(section::entry_point, ExecutionModel::Kernel, fun,
                               std::string{fn.name()}, std::move(builtins_used_by_function_));

    // Execution mode
    auto const work_group_size = fn.work_group_size();
    mod_->add_to<OpExecutionMode>(
        section::execution_mode, fun, ExecutionMode::LocalSize,
        ExecutionModeAttr{std::array<std::int32_t, 3u>{work_group_size[0], work_group_size[1], 1}});
    mod_->add_to<OpExecutionMode>(section::execution_mode, fun, ExecutionMode::SubgroupSize,
                                  ExecutionModeAttr{fn.subgroup_size()});
}

} // namespace tinytc::spv

