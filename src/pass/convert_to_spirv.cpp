// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/convert_to_spirv.hpp"
#include "compiler_context.hpp"
#include "error.hpp"
#include "node/data_type_node.hpp"
#include "node/function_node.hpp"
#include "node/inst_node.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "scalar_type.hpp"
#include "spv/enums.hpp"
#include "spv/instructions.hpp"
#include "spv/opencl.std.hpp"
#include "spv/uniquifier.hpp"
#include "support/casting.hpp"
#include "support/fnv1a.hpp"
#include "support/ilist_base.hpp"
#include "support/util.hpp"
#include "support/visit.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <algorithm>
#include <array>
#include <complex>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace tinytc {

class spirv_converter {
  public:
    inline spirv_converter(::tinytc_core_info const *info, spv::mod &mod,
                           tinytc_compiler_context_t ctx)
        : info_(info), mod_(&mod), ctx_(ctx), unique_(ctx, mod) {}

    // Instruction nodes
    void operator()(inst_node const &in);
    void operator()(arith_inst const &in);
    void operator()(arith_unary_inst const &in);
    void operator()(barrier_inst const &in);
    void operator()(cast_inst const &in);
    void operator()(compare_inst const &in);
    void operator()(constant_inst const &in);
    void operator()(group_id_inst const &in);
    void operator()(group_size_inst const &in);
    void operator()(num_subgroups_inst const &in);
    void operator()(subgroup_id_inst const &in);
    void operator()(subgroup_local_id_inst const &in);
    void operator()(subgroup_size_inst const &in);

    void run_on_program(program_node const &p);

  private:
    auto get_scalar_type(value_node const &v) -> scalar_type;
    auto get_coopmatrix_type(value_node const &v) -> scalar_type;
    auto declare(value_node const &v, spv::spv_inst *in);
    auto val(value_node const &v) -> spv::spv_inst *;
    auto multi_declare(value_node const &v, std::vector<spv::spv_inst *> insts);
    auto multi_val(value_node const &v) -> std::vector<spv::spv_inst *> &;
    auto declare_function_type(std::vector<spv::spv_inst *> params) -> spv::spv_inst *;
    void run_on_region(region_node const &fn);
    void run_on_function(function_node const &fn);

    ::tinytc_core_info const *info_;
    spv::mod *mod_;
    tinytc_compiler_context_t ctx_;
    spv::uniquifier unique_;
    std::unordered_map<const_tinytc_value_t, spv::spv_inst *> vals_;
    std::unordered_map<const_tinytc_value_t, std::vector<spv::spv_inst *>> multi_vals_;
    std::unordered_multimap<std::uint64_t, spv::OpTypeFunction *> function_tys_;
    core_config core_cfg_ = {};
};

auto spirv_converter::get_scalar_type(value_node const &v) -> scalar_type {
    auto st = dyn_cast<scalar_data_type>(v.ty());
    if (!st) {
        throw compilation_error(v.loc(), status::ir_expected_scalar);
    }
    return st->ty();
}
auto spirv_converter::get_coopmatrix_type(value_node const &v) -> scalar_type {
    auto ct = dyn_cast<coopmatrix_data_type>(v.ty());
    if (!ct) {
        throw compilation_error(v.loc(), status::ir_expected_coopmatrix);
    }
    return ct->component_ty();
}

auto spirv_converter::declare(value_node const &v, spv::spv_inst *in) { vals_[&v] = in; }
auto spirv_converter::val(value_node const &v) -> spv::spv_inst * {
    if (auto it = vals_.find(&v); it != vals_.end()) {
        return it->second;
    }
    throw compilation_error(v.loc(), status::spirv_undefined_value);
}
auto spirv_converter::multi_declare(value_node const &v, std::vector<spv::spv_inst *> insts) {
    multi_vals_[&v] = std::move(insts);
}
auto spirv_converter::multi_val(value_node const &v) -> std::vector<spv::spv_inst *> & {
    if (auto it = multi_vals_.find(&v); it != multi_vals_.end()) {
        return it->second;
    }
    throw compilation_error(v.loc(), status::spirv_undefined_value);
}
auto spirv_converter::declare_function_type(std::vector<spv::spv_inst *> params)
    -> spv::spv_inst * {
    auto map_key = fnv1a0();
    for (auto const &p : params) {
        map_key = fnv1a_step(map_key, p);
    }
    auto range = function_tys_.equal_range(map_key);
    for (auto it = range.first; it != range.second; ++it) {
        if (std::equal(params.begin(), params.end(), it->second->op1().begin(),
                       it->second->op1().end())) {
            return it->second;
        }
    }
    auto void_ty = unique_.spv_ty(*void_data_type::get(ctx_));
    return function_tys_
        .emplace(map_key, mod_->add_to<spv::OpTypeFunction>(spv::section::type_const_var, void_ty,
                                                            std::move(params)))
        ->second;
}

void spirv_converter::operator()(inst_node const &in) {
    // @todo
    throw compilation_error(in.loc(), status::not_implemented);
}

void spirv_converter::operator()(arith_inst const &in) {
    auto const make_boolean = [&](arithmetic op, spv::spv_inst *ty, spv::spv_inst *a,
                                  spv::spv_inst *b) -> spv::spv_inst * {
        switch (op) {
        case arithmetic::and_:
            return mod_->add<spv::OpLogicalAnd>(ty, a, b);
        case arithmetic::or_:
            return mod_->add<spv::OpLogicalOr>(ty, a, b);
        case arithmetic::xor_:
            return mod_->add<spv::OpLogicalNotEqual>(ty, a, b);
        default:
            break;
        }
        throw compilation_error(in.loc(), status::ir_boolean_unsupported);
    };
    auto const make_int = [&](arithmetic op, spv::spv_inst *ty, spv::spv_inst *a,
                              spv::spv_inst *b) -> spv::spv_inst * {
        switch (op) {
        case arithmetic::add:
            return mod_->add<spv::OpIAdd>(ty, a, b);
        case arithmetic::sub:
            return mod_->add<spv::OpISub>(ty, a, b);
        case arithmetic::mul:
            return mod_->add<spv::OpIMul>(ty, a, b);
        case arithmetic::div:
            return mod_->add<spv::OpSDiv>(ty, a, b);
        case arithmetic::rem:
            return mod_->add<spv::OpSRem>(ty, a, b);
        case arithmetic::shl:
            return mod_->add<spv::OpShiftLeftLogical>(ty, a, b);
        case arithmetic::shr:
            return mod_->add<spv::OpShiftRightArithmetic>(ty, a, b);
        case arithmetic::and_:
            return mod_->add<spv::OpBitwiseAnd>(ty, a, b);
        case arithmetic::or_:
            return mod_->add<spv::OpBitwiseOr>(ty, a, b);
        case arithmetic::xor_:
            return mod_->add<spv::OpBitwiseXor>(ty, a, b);
        }
        throw compilation_error(in.loc(), status::internal_compiler_error);
    };
    auto const make_float_complex = [&](arithmetic op, spv::spv_inst *ty, spv::spv_inst *a,
                                        spv::spv_inst *b) -> spv::spv_inst * {
        switch (op) {
        case arithmetic::add:
            return mod_->add<spv::OpFAdd>(ty, a, b);
        case arithmetic::sub:
            return mod_->add<spv::OpFSub>(ty, a, b);
        case arithmetic::mul:
            return mod_->add<spv::OpFMul>(ty, a, b);
        case arithmetic::div:
            return mod_->add<spv::OpFDiv>(ty, a, b);
        case arithmetic::rem:
            return mod_->add<spv::OpFRem>(ty, a, b);
        default:
            break;
        }
        throw compilation_error(in.loc(), status::ir_fp_unsupported);
    };
    auto const make = [&](scalar_type sty, arithmetic op, spv::spv_inst *ty, spv::spv_inst *a,
                          spv::spv_inst *b) -> spv::spv_inst * {
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

    auto ty = unique_.spv_ty(*in.result(0).ty());

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
        auto insts = std::vector<spv::spv_inst *>{};
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

void spirv_converter::operator()(arith_unary_inst const &in) {
    auto const make_boolean = [&](arithmetic_unary op, spv::spv_inst *ty,
                                  spv::spv_inst *a) -> spv::spv_inst * {
        switch (op) {
        case arithmetic_unary::not_:
            return mod_->add<spv::OpLogicalNot>(ty, a);
        default:
            break;
        }
        throw compilation_error(in.loc(), status::ir_boolean_unsupported);
    };
    auto const make_int = [&](arithmetic_unary op, spv::spv_inst *ty,
                              spv::spv_inst *a) -> spv::spv_inst * {
        switch (op) {
        case arithmetic_unary::abs:
            return mod_->add<spv::OpExtInst>(
                ty, unique_.opencl_ext(), static_cast<std::int32_t>(spv::OpenCLEntrypoint::s_abs),
                std::vector<spv::IdRef>{a});
        case arithmetic_unary::neg:
            return mod_->add<spv::OpSNegate>(ty, a);
        case arithmetic_unary::not_:
            return mod_->add<spv::OpNot>(ty, a);
        default:
            break;
        }
        throw compilation_error(in.loc(), status::internal_compiler_error);
    };
    auto const make_float = [&](arithmetic_unary op, spv::spv_inst *ty,
                                spv::spv_inst *a) -> spv::spv_inst * {
        switch (op) {
        case arithmetic_unary::abs:
            return mod_->add<spv::OpExtInst>(ty, unique_.opencl_ext(),
                                             static_cast<std::int32_t>(spv::OpenCLEntrypoint::fabs),
                                             std::vector<spv::IdRef>{a});
        case arithmetic_unary::neg:
            return mod_->add<spv::OpFNegate>(ty, a);
        default:
            break;
        }
        throw compilation_error(in.loc(), status::internal_compiler_error);
    };
    auto const make_complex = [&](arithmetic_unary op, scalar_type sty, spv::spv_inst *ty,
                                  spv::spv_inst *a) -> spv::spv_inst * {
        switch (op) {
        case arithmetic_unary::abs: {
            auto spv_a_ty = unique_.spv_ty(*scalar_data_type::get(ctx_, sty));
            auto a2 = mod_->add<spv::OpFMul>(spv_a_ty, a, a);
            auto a2_0 =
                mod_->add<spv::OpCompositeExtract>(ty, a2, std::vector<spv::LiteralInteger>{0});
            auto a2_1 =
                mod_->add<spv::OpCompositeExtract>(ty, a2, std::vector<spv::LiteralInteger>{1});
            auto a2_0p1 = mod_->add<spv::OpFAdd>(ty, a2_0, a2_1);
            return mod_->add<spv::OpExtInst>(ty, unique_.opencl_ext(),
                                             static_cast<std::int32_t>(spv::OpenCLEntrypoint::sqrt),
                                             std::vector<spv::IdRef>{a2_0p1});
        }
        case arithmetic_unary::neg:
            return mod_->add<spv::OpFNegate>(ty, a);
        case arithmetic_unary::conj: {
            auto spv_float_ty = unique_.spv_ty(*scalar_data_type::get(ctx_, element_type(sty)));
            auto a_im = mod_->add<spv::OpCompositeExtract>(spv_float_ty, a,
                                                           std::vector<spv::LiteralInteger>{1});
            auto neg_a_im = mod_->add<spv::OpFNegate>(spv_float_ty, a_im);
            return mod_->add<spv::OpCompositeInsert>(ty, neg_a_im, a,
                                                     std::vector<spv::LiteralInteger>{1});
        }
        case arithmetic_unary::im:
            return mod_->add<spv::OpCompositeExtract>(ty, a, std::vector<spv::LiteralInteger>{1});
        case arithmetic_unary::re:
            return mod_->add<spv::OpCompositeExtract>(ty, a, std::vector<spv::LiteralInteger>{0});
        default:
            break;
        }
        throw compilation_error(in.loc(), status::internal_compiler_error);
    };
    auto const make = [&](scalar_type sty, arithmetic_unary op, spv::spv_inst *ty,
                          spv::spv_inst *a) -> spv::spv_inst * {
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

    auto ty = unique_.spv_ty(*in.result(0).ty());
    if (isa<boolean_data_type>(*in.a().ty())) {
        auto av = val(in.a());
        declare(in.result(0), make_boolean(in.operation(), ty, av));
    } else if (auto st = dyn_cast<scalar_data_type>(in.a().ty()); st) {
        auto av = val(in.a());
        declare(in.result(0), make(st->ty(), in.operation(), ty, av));
    } else if (auto ct = dyn_cast<coopmatrix_data_type>(in.a().ty()); ct) {
        auto const length = ct->length(core_cfg_.subgroup_size);
        auto insts = std::vector<spv::spv_inst *>{};
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

void spirv_converter::operator()(barrier_inst const &in) {
    std::int32_t fence = 0;
    if (in.has_fence(address_space::global)) {
        fence = fence | static_cast<std::int32_t>(spv::MemorySemantics::CrossWorkgroupMemory) |
                static_cast<std::int32_t>(spv::MemorySemantics::SequentiallyConsistent);
    }
    if (in.has_fence(address_space::local)) {
        fence = fence | static_cast<std::int32_t>(spv::MemorySemantics::WorkgroupMemory) |
                static_cast<std::int32_t>(spv::MemorySemantics::SequentiallyConsistent);
    }
    auto scope = unique_.i32_constant(static_cast<std::int32_t>(spv::Scope::Workgroup));
    auto memory_semantics = unique_.i32_constant(fence);
    mod_->add<spv::OpControlBarrier>(scope, scope, memory_semantics);
}

void spirv_converter::operator()(cast_inst const &in) {
    auto const cast_from_int = [&](scalar_type to_ty, spv::spv_inst *spv_to_ty,
                                   spv::spv_inst *a) -> spv::spv_inst * {
        switch (to_ty) {
        case scalar_type::i8:
        case scalar_type::i16:
        case scalar_type::i32:
        case scalar_type::i64:
        case scalar_type::index:
            return mod_->add<spv::OpSConvert>(spv_to_ty, a);
        case scalar_type::f32:
        case scalar_type::f64:
            return mod_->add<spv::OpConvertSToF>(spv_to_ty, a);
        case scalar_type::c32:
        case scalar_type::c64: {
            auto spv_float_ty = unique_.spv_ty(*scalar_data_type::get(ctx_, element_type(to_ty)));
            auto re = mod_->add<spv::OpConvertSToF>(spv_float_ty, a);
            return mod_->add<spv::OpCompositeInsert>(spv_to_ty, re,
                                                     unique_.null_constant(spv_to_ty),
                                                     std::vector<spv::LiteralInteger>{0});
        }
        }
        throw compilation_error(in.loc(), status::ir_forbidden_cast);
    };
    auto const cast_from_float = [&](scalar_type to_ty, spv::spv_inst *spv_to_ty,
                                     spv::spv_inst *a) -> spv::spv_inst * {
        switch (to_ty) {
        case scalar_type::i8:
        case scalar_type::i16:
        case scalar_type::i32:
        case scalar_type::i64:
        case scalar_type::index:
            return mod_->add<spv::OpConvertFToS>(spv_to_ty, a);
        case scalar_type::f32:
        case scalar_type::f64:
            return mod_->add<spv::OpFConvert>(spv_to_ty, a);
        case scalar_type::c32:
        case scalar_type::c64: {
            auto spv_float_ty = unique_.spv_ty(*scalar_data_type::get(ctx_, element_type(to_ty)));
            auto re = mod_->add<spv::OpFConvert>(spv_float_ty, a);
            return mod_->add<spv::OpCompositeInsert>(spv_to_ty, re,
                                                     unique_.null_constant(spv_to_ty),
                                                     std::vector<spv::LiteralInteger>{0});
        }
        }
        throw compilation_error(in.loc(), status::ir_forbidden_cast);
    };
    auto const cast_from_complex = [&](scalar_type to_ty, spv::spv_inst *spv_to_ty,
                                       spv::spv_inst *a) -> spv::spv_inst * {
        switch (to_ty) {
        case scalar_type::c32:
        case scalar_type::c64:
            return mod_->add<spv::OpFConvert>(spv_to_ty, a);
        default:
            throw compilation_error(in.loc(), status::ir_forbidden_cast);
        }
    };
    auto const make = [&](scalar_type to_ty, scalar_type a_ty, spv::spv_inst *spv_to_ty,
                          spv::spv_inst *a) -> spv::spv_inst * {
        if (a_ty == to_ty) {
            return mod_->add<spv::OpCopyObject>(spv_to_ty, a);
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

    auto spv_to_ty = unique_.spv_ty(*in.result(0).ty());

    if (auto st = dyn_cast<scalar_data_type>(in.result(0).ty()); st) {
        auto av = val(in.a());
        auto a_ty = get_scalar_type(in.a());
        declare(in.result(0), make(st->ty(), a_ty, spv_to_ty, av));
    } else if (auto ct = dyn_cast<coopmatrix_data_type>(in.result(0).ty()); ct) {
        auto const length = ct->length(core_cfg_.subgroup_size);
        auto insts = std::vector<spv::spv_inst *>{};
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

void spirv_converter::operator()(compare_inst const &in) {
    auto const compare_int = [&](cmp_condition cond, spv::spv_inst *spv_to_ty, spv::spv_inst *a,
                                 spv::spv_inst *b) -> spv::spv_inst * {
        switch (cond) {
        case cmp_condition::eq:
            return mod_->add<spv::OpIEqual>(spv_to_ty, a, b);
        case cmp_condition::ne:
            return mod_->add<spv::OpINotEqual>(spv_to_ty, a, b);
        case cmp_condition::gt:
            return mod_->add<spv::OpSGreaterThan>(spv_to_ty, a, b);
        case cmp_condition::ge:
            return mod_->add<spv::OpSGreaterThanEqual>(spv_to_ty, a, b);
        case cmp_condition::lt:
            return mod_->add<spv::OpSLessThan>(spv_to_ty, a, b);
        case cmp_condition::le:
            return mod_->add<spv::OpSLessThanEqual>(spv_to_ty, a, b);
        }
        throw compilation_error(in.loc(), status::internal_compiler_error);
    };
    auto const compare_float = [&](cmp_condition cond, spv::spv_inst *spv_to_ty, spv::spv_inst *a,
                                   spv::spv_inst *b) -> spv::spv_inst * {
        switch (cond) {
        case cmp_condition::eq:
            return mod_->add<spv::OpFOrdEqual>(spv_to_ty, a, b);
        case cmp_condition::ne:
            return mod_->add<spv::OpFUnordNotEqual>(spv_to_ty, a, b);
        case cmp_condition::gt:
            return mod_->add<spv::OpFOrdGreaterThan>(spv_to_ty, a, b);
        case cmp_condition::ge:
            return mod_->add<spv::OpFOrdGreaterThanEqual>(spv_to_ty, a, b);
        case cmp_condition::lt:
            return mod_->add<spv::OpFOrdLessThan>(spv_to_ty, a, b);
        case cmp_condition::le:
            return mod_->add<spv::OpFOrdLessThanEqual>(spv_to_ty, a, b);
        }
        throw compilation_error(in.loc(), status::internal_compiler_error);
    };
    auto const compare_complex = [&](cmp_condition cond, spv::spv_inst *spv_to_ty, spv::spv_inst *a,
                                     spv::spv_inst *b) -> spv::spv_inst * {
        switch (cond) {
        case cmp_condition::eq: {
            auto components_equal = mod_->add<spv::OpFOrdEqual>(unique_.bool2_ty(), a, b);
            return mod_->add<spv::OpAll>(spv_to_ty, components_equal);
        }
        case cmp_condition::ne: {
            auto components_not_equal = mod_->add<spv::OpFUnordNotEqual>(unique_.bool2_ty(), a, b);
            return mod_->add<spv::OpAll>(spv_to_ty, components_not_equal);
        }
        default:
            throw compilation_error(in.loc(), status::ir_complex_unsupported);
        }
    };
    auto const make = [&](scalar_type a_ty, cmp_condition cond, spv::spv_inst *spv_to_ty,
                          spv::spv_inst *a, spv::spv_inst *b) -> spv::spv_inst * {
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

    auto spv_to_ty = unique_.spv_ty(*in.result(0).ty());
    auto av = val(in.a());
    auto bv = val(in.b());
    auto a_ty = get_scalar_type(in.a());
    declare(in.result(0), make(a_ty, in.cond(), spv_to_ty, av, bv));
}

void spirv_converter::operator()(constant_inst const &in) {
    auto const make = [&](scalar_type sty, spv::spv_inst *spv_ty,
                          constant_inst::value_type const &val) -> spv::spv_inst * {
        auto const add_constant = [this, &spv_ty](auto val) -> spv::spv_inst * {
            return mod_->add_to<spv::OpConstant>(spv::section::type_const_var, spv_ty, val);
        };
        auto const add_constant_complex = [this, &spv_ty](spv::spv_inst *spv_float_ty, auto re,
                                                          auto im) -> spv::spv_inst * {
            auto c_re =
                mod_->add_to<spv::OpConstant>(spv::section::type_const_var, spv_float_ty, re);
            auto c_im =
                mod_->add_to<spv::OpConstant>(spv::section::type_const_var, spv_float_ty, im);
            return mod_->add_to<spv::OpConstantComposite>(spv::section::type_const_var, spv_ty,
                                                          std::vector<spv::spv_inst *>{c_re, c_im});
        };
        const auto visitor = overloaded{
            [&](bool) -> spv::spv_inst * { return nullptr; },
            [&](std::int64_t i) -> spv::spv_inst * {
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
            [&](double d) -> spv::spv_inst * {
                switch (sty) {
                case scalar_type::f32:
                    return add_constant(static_cast<float>(d));
                case scalar_type::f64:
                    return add_constant(d);
                default:
                    return nullptr;
                }
            },
            [&](std::complex<double> d) -> spv::spv_inst * {
                switch (sty) {
                case scalar_type::c32: {
                    auto spv_float_ty =
                        unique_.spv_ty(*scalar_data_type::get(ctx_, scalar_type::f32));
                    return add_constant_complex(spv_float_ty, static_cast<float>(d.real()),
                                                static_cast<float>(d.imag()));
                }
                case scalar_type::c64: {
                    auto spv_float_ty =
                        unique_.spv_ty(*scalar_data_type::get(ctx_, scalar_type::f64));
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

    auto spv_ty = unique_.spv_ty(*in.result(0).ty());

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

        multi_declare(in.result(0), std::vector<spv::spv_inst *>(length, cst));
    } else {
        throw compilation_error(in.loc(), status::ir_expected_coopmatrix_or_scalar);
    }
}

void spirv_converter::operator()(group_id_inst const &in) {}
void spirv_converter::operator()(group_size_inst const &in) {}
void spirv_converter::operator()(num_subgroups_inst const &in) {}
void spirv_converter::operator()(subgroup_id_inst const &in) {}
void spirv_converter::operator()(subgroup_local_id_inst const &in) {}
void spirv_converter::operator()(subgroup_size_inst const &in) {}

void spirv_converter::run_on_region(region_node const &reg) {
    mod_->add<spv::OpLabel>();
    for (auto const &i : reg) {
        visit(*this, i);
    }
    mod_->add<spv::OpReturn>();
}

void spirv_converter::run_on_function(function_node const &fn) {
    auto const subgroup_size = fn.subgroup_size();
    try {
        core_cfg_ = info_->get_core_config(subgroup_size);
    } catch (std::out_of_range const &e) {
        throw compilation_error(fn.loc(), status::unsupported_subgroup_size);
    }

    // Function type
    auto fun_ty = declare_function_type([&] {
        auto params = std::vector<spv::spv_inst *>{};
        params.reserve(fn.num_params());
        for (auto const &p : fn.params()) {
            params.push_back(unique_.spv_ty(*p.ty()));
        }
        return params;
    }());

    // Function
    auto void_ty = unique_.spv_ty(*void_data_type::get(ctx_));
    auto fun = mod_->add<spv::OpFunction>(void_ty, spv::FunctionControl::None, fun_ty);
    for (auto const &p : fn.params()) {
        declare(p, mod_->add<spv::OpFunctionParameter>(unique_.spv_ty(*p.ty())));
    }
    run_on_region(fn.body());
    mod_->add<spv::OpFunctionEnd>();

    // Entry point
    mod_->add_to<spv::OpEntryPoint>(spv::section::entry_point, spv::ExecutionModel::Kernel, fun,
                                    std::string{fn.name()}, std::vector<spv::spv_inst *>{});

    // Execution mode
    auto const work_group_size = fn.work_group_size();
    mod_->add_to<spv::OpExecutionMode>(spv::section::execution_mode, fun,
                                       spv::ExecutionMode::LocalSize,
                                       spv::ExecutionModeAttr{std::array<std::int32_t, 3u>{
                                           work_group_size[0], work_group_size[1], 1}});
    mod_->add_to<spv::OpExecutionMode>(spv::section::execution_mode, fun,
                                       spv::ExecutionMode::SubgroupSize,
                                       spv::ExecutionModeAttr{fn.subgroup_size()});
}

void spirv_converter::run_on_program(program_node const &p) {
    unique_.capability(spv::Capability::Addresses);
    unique_.capability(spv::Capability::Kernel);
    unique_.capability(spv::Capability::SubgroupDispatch);

    mod_->add_to<spv::OpMemoryModel>(spv::section::memory_model, spv::AddressingModel::Physical64,
                                     spv::MemoryModel::OpenCL);

    for (auto const &fn : p) {
        run_on_function(fn);
    }
}

convert_to_spirv_pass::convert_to_spirv_pass(::tinytc_core_info const *info)
    : info_(std::move(info)) {
    if (info_ == nullptr) {
        throw status::invalid_arguments;
    }
}

auto convert_to_spirv_pass::run_on_program(program_node const &p) -> std::unique_ptr<spv::mod> {
    auto m = std::make_unique<spv::mod>();

    spirv_converter(info_, *m, p.context()).run_on_program(p);

    return m;
}

} // namespace tinytc
