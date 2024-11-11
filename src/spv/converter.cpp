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
#include "support/ilist.hpp"
#include "support/ilist_base.hpp"
#include "support/util.hpp"
#include "support/visit.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <algorithm>
#include <array>
#include <complex>
#include <cstdint>
#include <ranges>
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

    // Add missing capabilites and extensions
    for (std::int32_t s = 0; s < num_module_sections; ++s) {
        for (auto const &i : m->insts(enum_cast<section>(s))) {
            visit(overloaded{[&]<spv_inst_with_required_capabilities I>(I const &in) {
                                 if (isa<OpAtomicFAddEXT>(static_cast<spv_inst const &>(in))) {
                                     // We manage OpAtomicFAddExt manually as the required
                                     // capabilitites depend on the data type
                                     return;
                                 }
                                 for (auto const &cap : I::required_capabilities) {
                                     conv.unique().capability(cap);
                                 }
                             },
                             [&](auto const &) {}},
                  i);
            visit(overloaded{[&]<spv_inst_with_required_extensions I>(I const &) {
                                 for (auto const &ext_name : I::required_extensions) {
                                     conv.unique().extension(ext_name);
                                 }
                             },
                             [&](auto const &) {}},
                  i);
        }
    }

    return m;
}

dope_vector::dope_vector(spv_inst *ty, std::vector<std::int64_t> static_shape,
                         std::vector<std::int64_t> static_stride, spv_inst *offset_ty,
                         std::int64_t static_offset)
    : ty_(ty), static_shape_(std::move(static_shape)), static_stride_(std::move(static_stride)),
      shape_(dim(), nullptr), stride_(dim(), nullptr), offset_ty_(offset_ty),
      static_offset_(static_offset) {
    if (static_shape_.size() != static_stride_.size()) {
        throw status::internal_compiler_error;
    }
}

auto dope_vector::num_dynamic() const -> std::int64_t {
    auto const sum_dynamic = [](std::vector<std::int64_t> const &vec) {
        std::int64_t num_dynamic = 0;
        for (auto &v : vec) {
            if (is_dynamic_value(v)) {
                ++num_dynamic;
            }
        }
        return num_dynamic;
    };
    return sum_dynamic(static_shape_) + sum_dynamic(static_stride_);
}

inst_converter::inst_converter(tinytc_compiler_context_t ctx, mod &m)
    : ctx_(ctx), mod_(&m), unique_(ctx, m) {}

auto inst_converter::get_last_label() -> spv_inst * {
    auto &insts = mod_->insts(section::function);
    auto it = insts.end();
    while (it != insts.begin()) {
        auto in = (--it).get();
        if (isa<OpLabel>(*in)) {
            return in;
        }
    }
    return nullptr;
}

auto inst_converter::get_dope_vector(tinytc_value const &v) -> dope_vector * {
    if (auto it = dope_vec_.find(&v); it != dope_vec_.end()) {
        return &it->second;
    }
    return nullptr;
}

auto inst_converter::get_scalar_type(value_node const &v) const -> scalar_type {
    auto st = dyn_cast<scalar_data_type>(v.ty());
    if (!st) {
        throw compilation_error(v.loc(), status::ir_expected_scalar);
    }
    return st->ty();
}
auto inst_converter::get_coopmatrix_type(value_node const &v) const -> scalar_type {
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

auto inst_converter::make_constant(scalar_type sty, spv_inst *spv_ty,
                                   constant_inst::value_type const &val) -> spv_inst * {
    auto const add_constant_complex = [this, &spv_ty](auto cst) -> spv_inst * {
        auto c_re = unique_.constant(cst.real());
        auto c_im = unique_.constant(cst.imag());
        return mod_->add_to<OpConstantComposite>(section::type_const_var, spv_ty,
                                                 std::vector<spv_inst *>{c_re, c_im});
    };
    const auto visitor = overloaded{
        [&](bool) -> spv_inst * { return nullptr; },
        [&](std::int64_t i) -> spv_inst * {
            switch (sty) {
            case scalar_type::i8:
                return unique_.constant(static_cast<std::int8_t>(i));
            case scalar_type::i16:
                return unique_.constant(static_cast<std::int16_t>(i));
            case scalar_type::i32:
                return unique_.constant(static_cast<std::int32_t>(i));
            case scalar_type::i64:
            case scalar_type::index:
                return unique_.constant(i);
            default:
                return nullptr;
            }
        },
        [&](double d) -> spv_inst * {
            switch (sty) {
            case scalar_type::f32:
                return unique_.constant(static_cast<float>(d));
            case scalar_type::f64:
                return unique_.constant(d);
            default:
                return nullptr;
            }
        },
        [&](std::complex<double> d) -> spv_inst * {
            switch (sty) {
            case scalar_type::c32:
                return add_constant_complex(static_cast<std::complex<float>>(d));
            case scalar_type::c64:
                return add_constant_complex(d);
            default:
                return nullptr;
            }
        },
    };
    return std::visit(visitor, val);
}

auto inst_converter::make_dope_vector(tinytc_value const &v) -> dope_vector * {
    if (dope_vec_.contains(&v)) {
        throw compilation_error(v.loc(), status::internal_compiler_error);
    }

    auto spv_index_ty = unique_.spv_ty(scalar_data_type::get(ctx_, scalar_type::index));
    return ::tinytc::visit(
        overloaded{[&](memref_data_type const &mr) -> dope_vector * {
                       return &(dope_vec_[&v] = dope_vector{spv_index_ty, mr.shape(), mr.stride()});
                   },
                   [&](group_data_type const &g) -> dope_vector * {
                       if (auto mt = dyn_cast<memref_data_type>(g.ty()); mt) {
                           auto pointer_ty =
                               unique_.spv_pointer_ty(StorageClass::CrossWorkgroup, spv_index_ty,
                                                      alignment(scalar_type::i64));
                           return &(dope_vec_[&v] =
                                        dope_vector{pointer_ty, mt->shape(), mt->stride(),
                                                    spv_index_ty, g.offset()});
                       } else {
                           throw compilation_error(v.loc(), status::ir_expected_memref);
                       }
                   },
                   [](auto const &) -> dope_vector * { return nullptr; }},
        *v.ty());
}

void inst_converter::make_store(store_flag flag, scalar_type sty, address_space as,
                                spv_inst *pointer, spv_inst *value) {
    auto const add_fadd_caps = [&] {
        switch (sty) {
        case scalar_type::i8:
        case scalar_type::i16:
        case scalar_type::i32:
        case scalar_type::i64:
        case scalar_type::index:
            break;
        case scalar_type::f32:
        case scalar_type::c32:
            unique_.capability(Capability::AtomicFloat32AddEXT);
            break;
        case scalar_type::f64:
        case scalar_type::c64:
            unique_.capability(Capability::AtomicFloat64AddEXT);
            break;
        }
    };
    auto const split_re_im = [&]() -> std::array<std::array<spv_inst *, 2u>, 2u> {
        auto component_sty = element_type(sty);
        auto float_ty = unique_.spv_ty(scalar_data_type::get(ctx_, component_sty));
        const auto storage_cls = address_space_to_storage_class(as);
        auto pointer_ty = unique_.spv_pointer_ty(storage_cls, float_ty, alignment(component_sty));
        auto c0 = unique_.constant(std::int32_t{0});
        auto c1 = unique_.constant(std::int32_t{1});
        auto re_ptr = mod_->add<OpInBoundsAccessChain>(pointer_ty, pointer, std::vector<IdRef>{c0});
        auto im_ptr = mod_->add<OpInBoundsAccessChain>(pointer_ty, pointer, std::vector<IdRef>{c1});
        auto re_val =
            mod_->add<OpCompositeExtract>(float_ty, value, std::vector<LiteralInteger>{0});
        auto im_val =
            mod_->add<OpCompositeExtract>(float_ty, value, std::vector<LiteralInteger>{1});
        return {{{re_ptr, re_val}, {im_ptr, im_val}}};
    };
    switch (flag) {
    case store_flag::regular:
        mod_->add<OpStore>(pointer, value);
        break;
    case store_flag::atomic: {
        auto scope = unique_.constant(static_cast<std::int32_t>(Scope::Workgroup));
        auto semantics = unique_.constant(static_cast<std::int32_t>(MemorySemantics::Relaxed));
        switch (sty) {
        case scalar_type::c32:
        case scalar_type::c64: {
            auto re_im = split_re_im();
            mod_->add<OpAtomicStore>(re_im[0][0], scope, semantics, re_im[0][1]);
            mod_->add<OpAtomicStore>(re_im[1][0], scope, semantics, re_im[1][1]);
            break;
        }
        default:
            mod_->add<OpAtomicStore>(pointer, scope, semantics, value);
            break;
        }
        break;
    }
    case store_flag::atomic_add: {
        auto result_ty = unique_.spv_ty(scalar_data_type::get(ctx_, sty));
        auto scope = unique_.constant(static_cast<std::int32_t>(Scope::Workgroup));
        auto semantics = unique_.constant(static_cast<std::int32_t>(MemorySemantics::Relaxed));
        switch (sty) {
        case scalar_type::i8:
        case scalar_type::i16:
        case scalar_type::i32:
        case scalar_type::i64:
        case scalar_type::index:
            mod_->add<OpAtomicIAdd>(result_ty, pointer, scope, semantics, value);
            break;
        case scalar_type::f32:
        case scalar_type::f64:
            add_fadd_caps();
            mod_->add<OpAtomicFAddEXT>(result_ty, pointer, scope, semantics, value);
            break;
        case scalar_type::c32:
        case scalar_type::c64: {
            add_fadd_caps();
            auto re_im = split_re_im();
            auto component_sty = element_type(sty);
            auto float_ty = unique_.spv_ty(scalar_data_type::get(ctx_, component_sty));
            mod_->add<OpAtomicFAddEXT>(float_ty, re_im[0][0], scope, semantics, re_im[0][1]);
            mod_->add<OpAtomicFAddEXT>(float_ty, re_im[1][0], scope, semantics, re_im[1][1]);
            break;
        }
        }
        break;
    } break;
    }
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
    auto scope = unique_.constant(static_cast<std::int32_t>(Scope::Workgroup));
    auto memory_semantics = unique_.constant(fence);
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
    auto spv_ty = unique_.spv_ty(in.result(0).ty());

    if (isa<boolean_data_type>(*in.result(0).ty())) {
        if (!std::holds_alternative<bool>(in.value())) {
            throw compilation_error(in.loc(), status::internal_compiler_error);
        }
        declare(in.result(0), unique_.bool_constant(std::get<bool>(in.value())));
    } else if (auto st = dyn_cast<scalar_data_type>(in.result(0).ty()); st) {
        auto cst = make_constant(st->ty(), spv_ty, in.value());
        if (cst == nullptr) {
            throw compilation_error(in.loc(), status::internal_compiler_error);
        }
        declare(in.result(0), cst);
    } else if (auto ct = dyn_cast<coopmatrix_data_type>(in.result(0).ty()); ct) {
        auto const length = ct->length(core_cfg_.subgroup_size);
        auto cst = make_constant(ct->component_ty(), spv_ty, in.value());
        if (cst == nullptr) {
            throw compilation_error(in.loc(), status::internal_compiler_error);
        }

        multi_declare(in.result(0), std::vector<spv_inst *>(length, cst));
    } else {
        throw compilation_error(in.loc(), status::ir_expected_coopmatrix_or_scalar);
    }
}

void inst_converter::operator()(for_inst const &in) {
    const std::int64_t num_results = num_yielded_vals(in.result_begin(), in.result_end());

    auto header_label = std::make_unique<OpLabel>();
    auto body_label = std::make_unique<OpLabel>();
    auto continue_label = std::make_unique<OpLabel>();
    auto merge_label = std::make_unique<OpLabel>();

    mod_->add<OpLoopMerge>(merge_label.get(), continue_label.get(), LoopControl::None);
    mod_->add<OpBranch>(header_label.get());

    // Header block
    auto spv_bool_ty = unique_.spv_ty(boolean_data_type::get(ctx_));
    auto spv_loop_var_ty = unique_.spv_ty(in.loop_var().ty());
    auto header_block_last_label = header_label.get();
    mod_->insts(section::function).push_back(header_label.release());

    auto condition = mod_->add<OpSLessThan>(spv_bool_ty, val(in.from()), val(in.to()));
    mod_->add<OpBranchConditional>(condition, body_label.get(), merge_label.get(),
                                   std::vector<LiteralInteger>{});

    // Body block
    auto body_first_label = body_label.get();
    mod_->insts(section::function).push_back(body_label.release());
    // nullptr needs to be replaced by the loop var update once it is defined
    auto loop_var_phi = mod_->add<OpPhi>(
        spv_loop_var_ty,
        std::vector<PairIdRefIdRef>{PairIdRefIdRef{val(in.from()), header_block_last_label},
                                    PairIdRefIdRef{nullptr, continue_label.get()}});
    declare(in.loop_var(), loop_var_phi);

    auto const &make_iter_arg_phi = [&]() -> std::vector<OpPhi *> {
        auto phis = std::vector<OpPhi *>{};
        phis.reserve(num_results);
        for (std::int64_t i = 0; i < in.num_results(); ++i) {
            auto ty = unique_.spv_ty(in.iter_arg(i).ty());
            if (isa<coopmatrix_data_type>(*in.iter_arg(i).ty())) {
                auto &init_vals = multi_val(in.iter_init(i));
                auto iter_arg_vals = std::vector<spv_inst *>(init_vals.size(), nullptr);
                for (auto init_val = init_vals.begin(), iter_arg_val = iter_arg_vals.begin();
                     init_val != init_vals.end(); ++init_val, ++iter_arg_val) {
                    auto phi =
                        mod_->add<OpPhi>(ty, std::vector<PairIdRefIdRef>{
                                                 PairIdRefIdRef{*init_val, header_block_last_label},
                                                 PairIdRefIdRef{nullptr, continue_label.get()}});
                    *iter_arg_val = phi;
                    phis.emplace_back(phi);
                }
                multi_declare(in.iter_arg(i), std::move(iter_arg_vals));
            } else {
                phis.emplace_back(mod_->add<OpPhi>(
                    ty, std::vector<PairIdRefIdRef>{
                            PairIdRefIdRef{val(in.iter_init(i)), header_block_last_label},
                            PairIdRefIdRef{nullptr, continue_label.get()}}));
                declare(in.iter_arg(i), phis.back());
            }
        }
        return phis;
    };
    auto iter_arg_phis = make_iter_arg_phi();

    auto yielded_for = run_on_region_with_yield(in.body(), num_results);
    // Update phis with yielded values
    for (std::int64_t i = 0; i < num_results; ++i) {
        iter_arg_phis[i]->op0().back().first = yielded_for[i];
    }

    auto body_last_label = get_last_label();
    if (!body_last_label) {
        throw compilation_error(in.loc(), status::internal_compiler_error);
    }
    mod_->add<OpBranch>(continue_label.get());

    // Continue block
    auto continue_block_last_label = continue_label.get();
    mod_->insts(section::function).push_back(continue_label.release());
    auto step = [&]() -> spv_inst * {
        if (in.has_step()) {
            return val(in.step());
        }
        return make_constant(get_scalar_type(in.loop_var()), spv_loop_var_ty, std::int64_t{1});
    }();
    auto loop_var_update = mod_->add<OpIAdd>(spv_loop_var_ty, val(in.loop_var()), step);
    loop_var_phi->op0().back().first = loop_var_update;
    auto condition2 = mod_->add<OpSLessThan>(spv_bool_ty, loop_var_update, val(in.to()));
    mod_->add<OpBranchConditional>(condition2, body_first_label, merge_label.get(),
                                   std::vector<LiteralInteger>{});

    // Merge block
    mod_->insts(section::function).push_back(merge_label.release());

    auto const &set_results = [&] {
        std::int64_t val_no = 0;
        for (std::int64_t i = 0; i < in.num_results(); ++i) {
            auto ty = unique_.spv_ty(in.result(i).ty());
            if (isa<coopmatrix_data_type>(*in.result(i).ty())) {
                auto &init_vals = multi_val(in.iter_init(i));
                auto results = std::vector<spv_inst *>(init_vals.size(), nullptr);
                for (auto init_val = init_vals.begin(), result = results.begin();
                     init_val != init_vals.end(); ++init_val, ++result) {
                    *result = mod_->add<OpPhi>(
                        ty, std::vector<PairIdRefIdRef>{
                                PairIdRefIdRef{*init_val, header_block_last_label},
                                PairIdRefIdRef{yielded_for[val_no++], continue_block_last_label}});
                }
                multi_declare(in.result(i), std::move(results));
            } else {
                declare(
                    in.result(i),
                    mod_->add<OpPhi>(
                        ty, std::vector<PairIdRefIdRef>{
                                PairIdRefIdRef{val(in.iter_init(i)), header_block_last_label},
                                PairIdRefIdRef{yielded_for[val_no++], continue_block_last_label}}));
            }
        }
    };
    set_results();
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

void inst_converter::operator()(if_inst const &in) {
    const std::int64_t num_results = num_yielded_vals(in.result_begin(), in.result_end());

    auto then_label = std::make_unique<OpLabel>();
    auto otherwise_label = std::make_unique<OpLabel>();
    auto merge_label = std::make_unique<OpLabel>();

    auto conditionv = val(in.condition());
    mod_->add<OpSelectionMerge>(merge_label.get(), SelectionControl::None);
    mod_->add<OpBranchConditional>(conditionv, then_label.get(), otherwise_label.get(),
                                   std::vector<LiteralInteger>{});
    mod_->insts(section::function).push_back(then_label.release());
    auto yielded_then = run_on_region_with_yield(in.then(), num_results);
    mod_->add<OpBranch>(merge_label.get());
    auto then_last_label = get_last_label();
    if (!then_last_label) {
        throw compilation_error(in.loc(), status::internal_compiler_error);
    }
    mod_->insts(section::function).push_back(otherwise_label.release());
    auto yielded_otherwise = run_on_region_with_yield(in.otherwise(), num_results);
    mod_->add<OpBranch>(merge_label.get());
    auto otherwise_last_label = get_last_label();
    if (!otherwise_last_label) {
        throw compilation_error(in.loc(), status::internal_compiler_error);
    }

    mod_->insts(section::function).push_back(merge_label.release());

    std::int64_t val_no = 0;
    for (std::int64_t i = 0; i < in.num_results(); ++i) {
        auto ty = unique_.spv_ty(in.result(i).ty());
        if (auto ct = dyn_cast<coopmatrix_data_type>(in.result(i).ty()); ct) {
            const auto length = ct->length(core_cfg_.subgroup_size);
            auto phi_insts = std::vector<spv_inst *>(length, nullptr);
            for (auto &phi_inst : phi_insts) {
                phi_inst = mod_->add<OpPhi>(
                    ty, std::vector<PairIdRefIdRef>{
                            PairIdRefIdRef{yielded_then[val_no], then_last_label},
                            PairIdRefIdRef{yielded_otherwise[val_no], otherwise_last_label}});
                ++val_no;
            }
            multi_declare(in.result(i), std::move(phi_insts));
        } else {
            auto phi_inst = mod_->add<OpPhi>(
                ty, std::vector<PairIdRefIdRef>{
                        PairIdRefIdRef{yielded_then[val_no], then_last_label},
                        PairIdRefIdRef{yielded_otherwise[val_no], otherwise_last_label}});
            ++val_no;
            declare(in.result(i), phi_inst);
        }
    }
}

void inst_converter::operator()(load_inst const &in) {
    auto spv_index_ty = unique_.spv_ty(scalar_data_type::get(ctx_, scalar_type::index));
    auto spv_pointer_index_ty = unique_.spv_pointer_ty(StorageClass::CrossWorkgroup, spv_index_ty,
                                                       alignment(scalar_type::i64));
    auto spv_pointer_ty = unique_.spv_ty(in.operand().ty());
    auto spv_result_ty = unique_.spv_ty(in.result(0).ty());
    auto dv = get_dope_vector(in.operand());
    if (!dv) {
        throw compilation_error(in.loc(), status::spirv_missing_dope_vector);
    }

    if (auto group_ty = dyn_cast<group_data_type>(in.operand().ty()); group_ty) {
        auto offset = mod_->add<OpIAdd>(spv_index_ty, dv->offset(), val(in.index_list()[0]));
        auto pointer = mod_->add<OpInBoundsPtrAccessChain>(spv_pointer_ty, val(in.operand()),
                                                           offset, std::vector<spv_inst *>{});
        declare(in.result(0), mod_->add<OpLoad>(spv_result_ty, pointer));
        auto rdv = make_dope_vector(in.result(0));

        auto const make_dope_par = [&](std::int64_t static_s, spv_inst *s) -> spv_inst * {
            if (is_dynamic_value(static_s)) {
                auto pointer = mod_->add<OpInBoundsPtrAccessChain>(spv_pointer_index_ty, s, offset,
                                                                   std::vector<spv_inst *>{});
                return mod_->add<OpLoad>(spv_index_ty, pointer);
            }
            return s;
        };
        for (std::int64_t i = 0; i < rdv->dim(); ++i) {
            rdv->shape(i, make_dope_par(dv->static_shape(i), dv->shape(i)));
        }
        for (std::int64_t i = 0; i < rdv->dim(); ++i) {
            rdv->stride(i, make_dope_par(dv->static_stride(i), dv->stride(i)));
        }
    } else if (auto memref_ty = dyn_cast<memref_data_type>(in.operand().ty()); memref_ty) {
        const auto pointer = [&]() -> spv_inst * {
            if (memref_ty->dim() == 0) {
                return val(in.operand());
            }

            auto offset = unique_.null_constant(spv_index_ty);
            for (std::int64_t i = 0; i < memref_ty->dim(); ++i) {
                auto tmp = mod_->add<OpIMul>(spv_index_ty, val(in.index_list()[i]), dv->stride(i));
                offset = mod_->add<OpIAdd>(spv_index_ty, offset, tmp);
            }
            return mod_->add<OpInBoundsPtrAccessChain>(spv_pointer_ty, val(in.operand()), offset,
                                                       std::vector<spv_inst *>{});
        };
        declare(in.result(0), mod_->add<OpLoad>(spv_result_ty, pointer()));
    } else {
        throw compilation_error(in.loc(), status::ir_expected_memref_or_group);
    }
}

void inst_converter::operator()(num_subgroups_inst const &in) {
    declare(in.result(0), load_builtin(BuiltIn::NumSubgroups));
}

void inst_converter::operator()(parallel_inst const &in) { run_on_region(in.body()); }

void inst_converter::operator()(store_inst const &in) {
    auto spv_index_ty = unique_.spv_ty(scalar_data_type::get(ctx_, scalar_type::index));
    auto spv_pointer_ty = unique_.spv_ty(in.operand().ty());
    auto dv = get_dope_vector(in.operand());
    if (!dv) {
        throw compilation_error(in.loc(), status::spirv_missing_dope_vector);
    }

    if (auto memref_ty = dyn_cast<memref_data_type>(in.operand().ty()); memref_ty) {
        const auto pointer = [&]() -> spv_inst * {
            if (memref_ty->dim() == 0) {
                return val(in.operand());
            }

            auto offset = unique_.null_constant(spv_index_ty);
            for (std::int64_t i = 0; i < memref_ty->dim(); ++i) {
                auto tmp = mod_->add<OpIMul>(spv_index_ty, val(in.index_list()[i]), dv->stride(i));
                offset = mod_->add<OpIAdd>(spv_index_ty, offset, tmp);
            }
            return mod_->add<OpInBoundsPtrAccessChain>(spv_pointer_ty, val(in.operand()), offset,
                                                       std::vector<spv_inst *>{});
        };

        make_store(in.flag(), memref_ty->element_ty(), memref_ty->addrspace(), pointer(),
                   val(in.val()));
    } else {
        throw compilation_error(in.loc(), status::ir_expected_memref);
    }
}

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
        auto scope = unique_.constant(static_cast<std::int32_t>(Scope::Workgroup));
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

void inst_converter::operator()(yield_inst const &in) {
    if (yielded_vals_.empty()) {
        throw compilation_error(in.loc(), status::ir_unexpected_yield);
    }

    auto &top = yielded_vals_.top();
    const std::int64_t num = num_yielded_vals(in.op_begin(), in.op_end());
    if (static_cast<std::int64_t>(top.size()) != num) {
        throw compilation_error(in.loc(), status::ir_yield_mismatch);
    }

    std::int64_t i = 0;
    for (auto &op : in.operands()) {
        if (auto ct = dyn_cast<coopmatrix_data_type>(op.ty()); ct) {
            auto &vals = multi_val(op);
            for (auto &v : vals) {
                top[i++] = v;
            }
        } else {
            top[i++] = val(op);
        }
    }
}

void inst_converter::run_on_region(region_node const &reg) {
    for (auto const &i : reg) {
        visit(*this, i);
    }
}

auto inst_converter::run_on_region_with_yield(region_node const &reg,
                                              std::int64_t num_results) -> std::vector<spv_inst *> {
    yielded_vals_.push(std::vector<spv_inst *>(num_results, nullptr));
    run_on_region(reg);
    auto yielded_vals = std::move(yielded_vals_.top());
    if (static_cast<std::int64_t>(yielded_vals.size()) != num_results ||
        std::any_of(yielded_vals.begin(), yielded_vals.end(),
                    [](spv_inst *in) { return in == nullptr; })) {
        throw compilation_error(reg.loc(), status::ir_yield_mismatch);
    }
    yielded_vals_.pop();
    return yielded_vals;
}

void inst_converter::run_on_function(function_node const &fn, core_config const &core_cfg) {
    core_cfg_ = core_cfg;

    // Function type
    auto fun_ty = unique_.spv_function_ty([&] {
        auto params = std::vector<spv_inst *>{};
        params.reserve(fn.num_params());
        for (auto const &p : fn.params()) {
            params.emplace_back(unique_.spv_ty(p.ty()));
            auto dv = make_dope_vector(p);
            if (dv) {
                for (std::int64_t i = 0; i < dv->num_dynamic(); ++i) {
                    params.emplace_back(dv->ty());
                }
                if (is_dynamic_value(dv->static_offset())) {
                    params.emplace_back(dv->offset_ty());
                }
            }
        }
        return params;
    }());

    // Function
    auto void_ty = unique_.spv_ty(void_data_type::get(ctx_));
    auto fun = mod_->add<OpFunction>(void_ty, FunctionControl::None, fun_ty);
    for (auto const &p : fn.params()) {
        declare(p, mod_->add<OpFunctionParameter>(unique_.spv_ty(p.ty())));
        auto dv = get_dope_vector(p);
        if (dv) {
            auto const make_dope_par = [&](spv_inst *ty, std::int64_t s) {
                return is_dynamic_value(s) ? mod_->add<OpFunctionParameter>(ty)
                                           : unique_.constant(s);
            };
            for (std::int64_t i = 0; i < dv->dim(); ++i) {
                dv->shape(i, make_dope_par(dv->ty(), dv->static_shape(i)));
            }
            for (std::int64_t i = 0; i < dv->dim(); ++i) {
                dv->stride(i, make_dope_par(dv->ty(), dv->static_stride(i)));
            }
            if (dv->offset_ty()) {
                dv->offset(make_dope_par(dv->offset_ty(), dv->static_offset()));
            }
        }
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

