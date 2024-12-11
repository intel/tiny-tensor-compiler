// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "spv/converter.hpp"
#include "analysis/stack.hpp"
#include "codegen_tools.hpp"
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
#include "spv/module.hpp"
#include "spv/opencl.std.hpp"
#include "spv/pass/capex.hpp"
#include "spv/uniquifier.hpp"
#include "spv/visit.hpp"
#include "support/ilist.hpp"
#include "support/ilist_base.hpp"
#include "support/util.hpp"
#include "support/visit.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <algorithm>
#include <array>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace tinytc::spv {

auto convert_prog_to_spirv(tinytc_prog const &p,
                           tinytc_core_info const &info) -> ::tinytc::spv_mod {
    auto m = ::tinytc::spv_mod{
        std::make_unique<tinytc_spv_mod>(p.share_context(), info.core_features()).release()};

    auto conv = inst_converter{*m};

    m->add_to<OpMemoryModel>(section::memory_model, AddressingModel::Physical64,
                             MemoryModel::OpenCL);

    for (auto const &fn : p) {
        conv.run_on_function(fn, info);
    }

    // Add missing capabilites and extensions
    auto cx = capex{conv.unique()};
    for (std::int32_t s = 0; s < num_module_sections; ++s) {
        for (auto const &i : m->insts(enum_cast<section>(s))) {
            visit(cx, i);
        }
    }

    for (int i = 0; i < TINYTC_NUMBER_OF_SPIRV_FEATURES; ++i) {
        const auto feature = enum_cast<spirv_feature>(i);
        if (cx.requires_feature(feature) && !info.have_spirv_feature(feature)) {
            throw compilation_error(p.loc(), status::spirv_required_feature_unavailable,
                                    to_string(feature));
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

inst_converter::inst_converter(tinytc_spv_mod &m) : mod_(&m), unique_(m) {}

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

auto inst_converter::load_builtin(BuiltIn b) -> spv_inst * {
    auto builtin = unique_.builtin_var(b);
    if (auto it = std::find(vars_used_by_function_.begin(), vars_used_by_function_.end(), builtin);
        it == vars_used_by_function_.end()) {
        vars_used_by_function_.push_back(builtin);
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

auto inst_converter::make_binary_op(scalar_type sty, arithmetic op, spv_inst *ty, spv_inst *a,
                                    spv_inst *b, location const &loc) -> spv_inst * {
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
        case arithmetic::min:
            return mod_->add<OpExtInst>(ty, unique_.opencl_ext(),
                                        static_cast<std::int32_t>(OpenCLEntrypoint::s_min),
                                        std::vector<IdRef>{a, b});
        case arithmetic::max:
            return mod_->add<OpExtInst>(ty, unique_.opencl_ext(),
                                        static_cast<std::int32_t>(OpenCLEntrypoint::s_max),
                                        std::vector<IdRef>{a, b});
        }
        throw compilation_error(loc, status::internal_compiler_error);
    };
    auto const make_float = [&](arithmetic op, spv_inst *ty, spv_inst *a,
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
        case arithmetic::min:
            return mod_->add<OpExtInst>(ty, unique_.opencl_ext(),
                                        static_cast<std::int32_t>(OpenCLEntrypoint::fmin),
                                        std::vector<IdRef>{a, b});
        case arithmetic::max:
            return mod_->add<OpExtInst>(ty, unique_.opencl_ext(),
                                        static_cast<std::int32_t>(OpenCLEntrypoint::fmax),
                                        std::vector<IdRef>{a, b});
        default:
            break;
        }
        throw compilation_error(loc, status::ir_fp_unsupported);
    };
    auto const make_complex = [&](arithmetic op, spv_inst *ty, spv_inst *float_ty, spv_inst *a,
                                  spv_inst *b) -> spv_inst * {
        switch (op) {
        case arithmetic::add:
            return mod_->add<OpFAdd>(ty, a, b);
        case arithmetic::sub:
            return mod_->add<OpFSub>(ty, a, b);
        case arithmetic::mul: {
            return make_complex_mul(ty, a, b);
        }
        case arithmetic::div: {
            auto a_times_conj_b = make_complex_mul(ty, a, b, true);

            auto b_squared = mod_->add<OpFMul>(ty, b, b);
            auto b_squared_0 =
                mod_->add<OpCompositeExtract>(float_ty, b_squared, std::vector<LiteralInteger>{0});
            auto b_squared_1 =
                mod_->add<OpCompositeExtract>(float_ty, b_squared, std::vector<LiteralInteger>{1});
            spv_inst *b_abs = mod_->add<OpFAdd>(float_ty, b_squared_0, b_squared_1);
            auto dummy = mod_->add<OpUndef>(ty);
            b_abs = mod_->add<OpCompositeInsert>(ty, b_abs, dummy, std::vector<LiteralInteger>{0});
            b_abs = mod_->add<OpVectorShuffle>(ty, b_abs, dummy, std::vector<LiteralInteger>{0, 0});
            return mod_->add<OpFDiv>(ty, a_times_conj_b, b_abs);
        }
        default:
            break;
        }
        throw compilation_error(loc, status::ir_complex_unsupported);
    };
    switch (sty) {
    case scalar_type::i8:
    case scalar_type::i16:
    case scalar_type::i32:
    case scalar_type::i64:
    case scalar_type::index:
        return make_int(op, ty, a, b);
    case scalar_type::bf16: {
        auto float_ty = unique_.spv_ty(scalar_type::f32);
        auto af = mod_->add<OpConvertBF16ToFINTEL>(float_ty, a);
        auto bf = mod_->add<OpConvertBF16ToFINTEL>(float_ty, b);
        auto af_op_bf = make_float(op, float_ty, af, bf);
        return mod_->add<OpConvertFToBF16INTEL>(ty, af_op_bf);
    }
    case scalar_type::f16:
    case scalar_type::f32:
    case scalar_type::f64:
        return make_float(op, ty, a, b);
    case scalar_type::c32:
    case scalar_type::c64:
        return make_complex(op, ty, unique_.spv_ty(component_type(sty)), a, b);
    }
    throw compilation_error(loc, status::internal_compiler_error);
}

auto inst_converter::make_cast(scalar_type to_ty, scalar_type a_ty, spv_inst *spv_to_ty,
                               spv_inst *a, location const &loc) -> spv_inst * {
    auto float_ty = unique_.spv_ty(scalar_type::f32);
    auto const cast_from_int = [&](scalar_type to_ty, spv_inst *spv_to_ty,
                                   spv_inst *a) -> spv_inst * {
        switch (to_ty) {
        case scalar_type::i8:
        case scalar_type::i16:
        case scalar_type::i32:
        case scalar_type::i64:
        case scalar_type::index:
            return mod_->add<OpSConvert>(spv_to_ty, a);
        case scalar_type::bf16: {
            auto af = mod_->add<OpConvertSToF>(float_ty, a);
            return mod_->add<OpConvertFToBF16INTEL>(spv_to_ty, af);
        }
        case scalar_type::f16:
        case scalar_type::f32:
        case scalar_type::f64:
            return mod_->add<OpConvertSToF>(spv_to_ty, a);
        case scalar_type::c32:
        case scalar_type::c64: {
            auto spv_float_ty = unique_.spv_ty(component_type(to_ty));
            auto re = mod_->add<OpConvertSToF>(spv_float_ty, a);
            return mod_->add<OpCompositeInsert>(spv_to_ty, re, unique_.null_constant(spv_to_ty),
                                                std::vector<LiteralInteger>{0});
        }
        }
        throw compilation_error(loc, status::ir_forbidden_cast);
    };
    auto const cast_from_float = [&](scalar_type to_ty, spv_inst *spv_to_ty, scalar_type a_ty,
                                     spv_inst *a) -> spv_inst * {
        switch (to_ty) {
        case scalar_type::i8:
        case scalar_type::i16:
        case scalar_type::i32:
        case scalar_type::i64:
        case scalar_type::index:
            return mod_->add<OpConvertFToS>(spv_to_ty, a);
        case scalar_type::bf16:
            return mod_->add<OpConvertFToBF16INTEL>(spv_to_ty, a);
        case scalar_type::f16:
        case scalar_type::f32:
        case scalar_type::f64:
            return mod_->add<OpFConvert>(spv_to_ty, a);
        case scalar_type::c32:
        case scalar_type::c64: {
            auto re = a;
            if (component_type(to_ty) != a_ty) {
                auto spv_float_ty = unique_.spv_ty(component_type(to_ty));
                re = mod_->add<OpFConvert>(spv_float_ty, a);
            }
            // If the line below is change, adjust make_complex_mul as well
            return mod_->add<OpCompositeInsert>(spv_to_ty, re, unique_.null_constant(spv_to_ty),
                                                std::vector<LiteralInteger>{0});
        }
        }
        throw compilation_error(loc, status::ir_forbidden_cast);
    };
    auto const cast_from_complex = [&](scalar_type to_ty, spv_inst *spv_to_ty,
                                       spv_inst *a) -> spv_inst * {
        switch (to_ty) {
        case scalar_type::c32:
        case scalar_type::c64:
            return mod_->add<OpFConvert>(spv_to_ty, a);
        default:
            throw compilation_error(loc, status::ir_forbidden_cast);
        }
    };
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
    case scalar_type::bf16: {
        auto af = mod_->add<OpConvertBF16ToFINTEL>(float_ty, a);
        return cast_from_float(to_ty, spv_to_ty, scalar_type::f32, af);
    }
    case scalar_type::f16:
    case scalar_type::f32:
    case scalar_type::f64:
        return cast_from_float(to_ty, spv_to_ty, a_ty, a);
    case scalar_type::c32:
    case scalar_type::c64: {
        return cast_from_complex(to_ty, spv_to_ty, a);
    }
    }
    throw compilation_error(loc, status::internal_compiler_error);
}

auto inst_converter::make_complex_mul(spv_inst *ty, spv_inst *a, spv_inst *b,
                                      bool conj_b) -> spv_inst * {
    const auto is_imag_zero = [&](spv_inst *a) -> bool {
        // We capture the case here if "a" stems from a non-complex -> complex cast
        if (auto ci = dyn_cast<OpCompositeInsert>(a); ci) {
            return ci->type() == ty && ci->op1() == unique_.null_constant(ty) &&
                   ci->op2().size() == 1 && ci->op2()[0] == 0;
        }
        return false;
    };

    if (is_imag_zero(a)) {
        a = mod_->add<OpVectorShuffle>(ty, a, a, std::vector<LiteralInteger>{0, 0});
        return mod_->add<OpFMul>(ty, a, b);
    } else if (is_imag_zero(b)) {
        b = mod_->add<OpVectorShuffle>(ty, b, b, std::vector<LiteralInteger>{0, 0});
        return mod_->add<OpFMul>(ty, a, b);
    }

    auto neg_a = mod_->add<OpFNegate>(ty, a);
    auto a_times_i =
        conj_b ? mod_->add<OpVectorShuffle>(ty, a, neg_a, std::vector<LiteralInteger>{1, 2})
               : mod_->add<OpVectorShuffle>(ty, neg_a, a, std::vector<LiteralInteger>{1, 2});
    auto b_1 = mod_->add<OpVectorShuffle>(ty, b, b, std::vector<LiteralInteger>{1, 1});
    auto b_1_a_times_i = mod_->add<OpFMul>(ty, b_1, a_times_i);
    auto b_0 = mod_->add<OpVectorShuffle>(ty, b, b, std::vector<LiteralInteger>{0, 0});
    return mod_->add<OpExtInst>(ty, unique_.opencl_ext(),
                                static_cast<std::int32_t>(OpenCLEntrypoint::fma),
                                std::vector<IdRef>{a, b_0, b_1_a_times_i});
}

auto inst_converter::make_conditional_execution(
    spv_inst *returned_element_ty, spv_inst *condition,
    std::function<std::vector<spv_inst *>()> conditional_code,
    location const &loc) -> std::vector<spv_inst *> {
    auto then_label = std::make_unique<OpLabel>();
    auto merge_label = std::make_unique<OpLabel>();

    auto init_last_label = get_last_label();
    if (!init_last_label) {
        throw compilation_error(loc, status::internal_compiler_error);
    }
    mod_->add<OpSelectionMerge>(merge_label.get(), SelectionControl::None);
    mod_->add<OpBranchConditional>(condition, then_label.get(), merge_label.get(),
                                   std::vector<LiteralInteger>{});
    mod_->insts(section::function).push_back(then_label.release());
    std::vector<spv_inst *> loaded_values = conditional_code();
    mod_->add<OpBranch>(merge_label.get());
    auto then_last_label = get_last_label();
    if (!then_last_label) {
        throw compilation_error(loc, status::internal_compiler_error);
    }

    mod_->insts(section::function).push_back(merge_label.release());

    auto yielded_values = std::vector<spv_inst *>{};
    yielded_values.reserve(loaded_values.size());
    auto alternative = PairIdRefIdRef{unique_.null_constant(returned_element_ty), init_last_label};
    for (auto &value : loaded_values) {
        yielded_values.emplace_back(mod_->add<OpPhi>(
            returned_element_ty,
            std::vector<PairIdRefIdRef>{PairIdRefIdRef{value, then_last_label}, alternative}));
    }
    return yielded_values;
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
            case scalar_type::bf16:
                return unique_.constant(bfloat16{static_cast<float>(d)}.bits());
            case scalar_type::f16:
                return unique_.constant(half{static_cast<float>(d)});
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

    auto spv_index_ty = unique_.spv_ty(scalar_type::index);
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

auto inst_converter::make_mixed_precision_fma(scalar_type a_ty, scalar_type b_ty, scalar_type c_ty,
                                              spv_inst *a, spv_inst *b, spv_inst *c,
                                              location const &loc) -> spv_inst * {
    const auto mul_ty = promote_or_throw(a_ty, b_ty, loc);
    const auto add_ty = promote_or_throw(mul_ty, c_ty, loc);
    auto spv_mul_ty = unique_.spv_ty(mul_ty);
    auto spv_add_ty = unique_.spv_ty(add_ty);

    // Normalize such that a is compatible with b and we cast to the type of b
    if (a_ty != mul_ty && b_ty != mul_ty) {
        throw compilation_error(loc, status::internal_compiler_error,
                                "compatible type must be either type of a or type of b");
    }
    if (b_ty != mul_ty) {
        std::swap(a, b);
        std::swap(a_ty, b_ty);
    }

    const bool a_non_complex_b_complex = !is_complex_type(a_ty) && is_complex_type(b_ty);
    const bool a_complex_b_complex = is_complex_type(a_ty) && is_complex_type(b_ty);
    const auto a_cast_ty = a_non_complex_b_complex ? component_type(mul_ty) : mul_ty;
    auto spv_a_cast_ty = unique_.spv_ty(a_cast_ty);

    if (a_ty != a_cast_ty) {
        a = make_cast(a_cast_ty, a_ty, spv_a_cast_ty, a, loc);
    }
    if (a_cast_ty != mul_ty) {
        auto dummy = mod_->add<OpUndef>(spv_mul_ty);
        a = mod_->add<OpCompositeInsert>(spv_mul_ty, a, dummy, std::vector<LiteralInteger>{0});
        a = mod_->add<OpVectorShuffle>(spv_mul_ty, a, dummy, std::vector<LiteralInteger>{0, 0});
    }
    if (!a_complex_b_complex && mul_ty == add_ty && mul_ty != scalar_type::bf16) {
        return mod_->add<OpExtInst>(spv_mul_ty, unique_.opencl_ext(),
                                    static_cast<std::int32_t>(OpenCLEntrypoint::fma),
                                    std::vector<IdRef>{a, b, c});
    }

    if (mul_ty == scalar_type::bf16) {
        auto float_ty = unique_.spv_ty(scalar_type::f32);
        a = mod_->add<OpConvertBF16ToFINTEL>(float_ty, a);
        b = mod_->add<OpConvertBF16ToFINTEL>(float_ty, b);
    }

    auto product = [&]() -> spv_inst * {
        if (a_complex_b_complex) {
            return make_complex_mul(spv_mul_ty, a, b);
        } else if (a_non_complex_b_complex || is_floating_type(mul_ty)) {
            return mod_->add<OpFMul>(spv_mul_ty, a, b);
        }
        return mod_->add<OpIMul>(spv_mul_ty, a, b);
    }();

    if (mul_ty == scalar_type::bf16) {
        if (add_ty != scalar_type::f32) {
            product = make_cast(add_ty, scalar_type::f32, spv_add_ty, product, loc);
        }
    } else if (mul_ty != add_ty) {
        product = make_cast(add_ty, mul_ty, spv_add_ty, product, loc);
    }
    return make_binary_op(add_ty, arithmetic::add, spv_add_ty, c, product, loc);
}

void inst_converter::make_store(store_flag flag, scalar_type sty, address_space as,
                                spv_inst *pointer, spv_inst *value, location const &loc) {
    auto const split_re_im = [&]() -> std::array<std::array<spv_inst *, 2u>, 2u> {
        auto component_sty = component_type(sty);
        auto float_ty = unique_.spv_ty(component_sty);
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
        case scalar_type::i32:
        case scalar_type::i64:
        case scalar_type::index:
        case scalar_type::f16:
        case scalar_type::f32:
        case scalar_type::f64:
            mod_->add<OpAtomicStore>(pointer, scope, semantics, value);
            break;
        default:
            throw compilation_error(loc, status::spirv_unsupported_atomic_data_type);
        }
        break;
    }
    case store_flag::atomic_add: {
        auto result_ty = unique_.spv_ty(sty);
        auto scope = unique_.constant(static_cast<std::int32_t>(Scope::Workgroup));
        auto semantics = unique_.constant(static_cast<std::int32_t>(MemorySemantics::Relaxed));
        switch (sty) {
        case scalar_type::i32:
        case scalar_type::i64:
        case scalar_type::index:
            mod_->add<OpAtomicIAdd>(result_ty, pointer, scope, semantics, value);
            break;
        case scalar_type::f16:
        case scalar_type::f32:
        case scalar_type::f64:
            mod_->add<OpAtomicFAddEXT>(result_ty, pointer, scope, semantics, value);
            break;
        case scalar_type::c32:
        case scalar_type::c64: {
            auto re_im = split_re_im();
            auto component_sty = component_type(sty);
            auto float_ty = unique_.spv_ty(component_sty);
            mod_->add<OpAtomicFAddEXT>(float_ty, re_im[0][0], scope, semantics, re_im[0][1]);
            mod_->add<OpAtomicFAddEXT>(float_ty, re_im[1][0], scope, semantics, re_im[1][1]);
            break;
        }
        default:
            throw compilation_error(loc, status::spirv_unsupported_atomic_data_type);
        }
        break;
    } break;
    }
}

void inst_converter::emulate_cooperative_matrix_load(cooperative_matrix_load_inst const &in) {
    auto spv_boolean_ty = unique_.spv_ty(boolean_data_type::get(mod_->context()));
    auto spv_index_ty = unique_.spv_ty(scalar_type::index);
    auto spv_operand_ty = unique_.spv_ty(in.operand().ty());
    auto spv_ty = unique_.spv_ty(get_coopmatrix_type(in.result(0).ty())->ty());
    auto ot = get_memref_type(in.operand());
    auto odv = get_dope_vector(in.operand());
    if (!odv) {
        throw compilation_error(in.loc(), status::spirv_missing_dope_vector);
    }
    auto rt = get_coopmatrix_type(in.result(0));

    const int rmode = rt->distributed_mode();
    const int omode = in.t() == transpose::T ? 1 - rmode : rmode;
    const bool check_m = in.checked() == checked_flag::both ||
                         (rmode == 0 && in.checked() == checked_flag::rows) ||
                         (rmode == 1 && in.checked() == checked_flag::cols);
    const bool check_k = in.checked() == checked_flag::both ||
                         (rmode == 1 && in.checked() == checked_flag::rows) ||
                         (rmode == 0 && in.checked() == checked_flag::cols);
    const std::int32_t alignment = std::max(in.align(), ot->alignment());
    const std::int32_t sub_group_read_required_alignment =
        ot->addrspace() == address_space::local ? 16 : 4;
    const bool enable_sub_group_reads = core_cfg_.block_read_write_supported &&
                                        !is_complex_type(ot->element_ty()) &&
                                        in.t() == transpose::N && ot->stride(omode) == 1 &&
                                        alignment >= sub_group_read_required_alignment;

    spv_inst *pv[] = {val(in.pos0()), val(in.pos1())};
    auto get_rem = [&, rem = std::array<spv_inst *, 2u>{}](std::int64_t index) mutable {
        if (rem[index] == nullptr) {
            rem[index] = mod_->add<OpISub>(spv_index_ty, odv->shape(index), pv[index]);
        }
        return rem[index];
    };

    auto pv0_stride0 = mod_->add<OpIMul>(spv_index_ty, pv[0], odv->stride(0));
    auto pv1_stride1 = mod_->add<OpIMul>(spv_index_ty, pv[1], odv->stride(1));
    auto offset = mod_->add<OpIAdd>(spv_index_ty, pv0_stride0, pv1_stride1);
    auto pointer = mod_->add<OpInBoundsPtrAccessChain>(spv_operand_ty, val(in.operand()), offset,
                                                       std::vector<spv_inst *>{});

    const auto block_load = [&](spv_inst *offset) -> spv_inst * {
        auto sub_pointer = mod_->add<OpInBoundsPtrAccessChain>(spv_operand_ty, pointer, offset,
                                                               std::vector<spv_inst *>{});
        auto const cast_load_cast = [&](scalar_type int_sty) {
            auto int_ty = unique_.spv_ty(int_sty);
            const auto storage_cls = address_space_to_storage_class(ot->addrspace());
            auto int_ptr_ty = unique_.spv_pointer_ty(storage_cls, int_ty, alignment);
            auto int_ptr = mod_->add<OpBitcast>(int_ptr_ty, sub_pointer);
            auto value = mod_->add<OpSubgroupBlockReadINTEL>(int_ty, int_ptr);
            return mod_->add<OpBitcast>(spv_ty, value);
        };
        switch (ot->element_ty()) {
        case scalar_type::bf16:
        case scalar_type::f16:
            return cast_load_cast(scalar_type::i16);
        case scalar_type::f32:
            return cast_load_cast(scalar_type::i32);
        case scalar_type::f64:
            return cast_load_cast(scalar_type::i64);
        default:
            return mod_->add<OpSubgroupBlockReadINTEL>(spv_ty, sub_pointer);
        }
    };
    const auto gather_load = [&](spv_inst *offset) -> spv_inst * {
        auto sub_pointer = mod_->add<OpInBoundsPtrAccessChain>(spv_operand_ty, pointer, offset,
                                                               std::vector<spv_inst *>{});
        return mod_->add<OpLoad>(spv_ty, sub_pointer);
    };

    spv_inst *m = load_builtin(BuiltIn::SubgroupLocalInvocationId);
    m = mod_->add<OpSConvert>(spv_index_ty, m);
    const std::int64_t num_blocks = rt->num_blocks(core_cfg_.subgroup_size);
    const std::int64_t K = rt->shape(1 - rmode);
    auto loaded_values = std::vector<spv_inst *>{};
    loaded_values.reserve(K * num_blocks);
    for (std::int64_t block = 0; block < num_blocks; ++block) {
        auto const remainder = rt->shape(rmode) - core_cfg_.subgroup_size * block;
        const bool needs_mask = remainder < core_cfg_.subgroup_size;

        const auto load_block_impl = [&](auto load_impl,
                                         spv_inst *offset) -> std::vector<spv_inst *> {
            auto result = std::vector<spv_inst *>{};
            result.reserve(K);
            for (std::int64_t k = 0; k < K; ++k) {
                if (check_k) {
                    auto check1 = mod_->add<OpSLessThanEqual>(spv_boolean_ty, unique_.constant(-k),
                                                              pv[1 - omode]);
                    auto check2 = mod_->add<OpSLessThan>(spv_boolean_ty, unique_.constant(k),
                                                         get_rem(1 - omode));
                    auto cond = mod_->add<OpLogicalAnd>(spv_boolean_ty, check1, check2);
                    auto vals = make_conditional_execution(
                        spv_ty, cond, [&] { return std::vector<spv_inst *>{load_impl(offset)}; },
                        in.loc());
                    result.insert(result.end(), vals.begin(), vals.end());
                } else {
                    result.emplace_back(load_impl(offset));
                }
                if (k + 1 < K) {
                    offset = mod_->add<OpIAdd>(spv_index_ty, offset, odv->stride(1 - omode));
                }
            }
            return result;
        };

        auto const load_block = [&]() -> std::vector<spv_inst *> {
            auto block_offset = unique_.constant(block * core_cfg_.subgroup_size);
            if (enable_sub_group_reads && !needs_mask && !check_m) {
                return load_block_impl(block_load, block_offset);
            } else {
                spv_inst *offset = m;
                if (block > 0) {
                    offset = mod_->add<OpIAdd>(spv_index_ty, offset, block_offset);
                }
                offset = mod_->add<OpIMul>(spv_index_ty, offset, odv->stride(omode));
                return load_block_impl(gather_load, offset);
            }
        };
        spv_inst *cond = nullptr;
        if (check_m) {
            spv_inst *m_offset = m;
            if (block > 0) {
                m_offset = mod_->add<OpIAdd>(spv_index_ty, m_offset,
                                             unique_.constant(block * core_cfg_.subgroup_size));
            }
            auto neg = mod_->add<OpSNegate>(spv_index_ty, pv[omode]);
            auto check1 = mod_->add<OpSGreaterThanEqual>(spv_boolean_ty, m_offset, neg);
            auto check2 = mod_->add<OpSLessThan>(spv_boolean_ty, m_offset, get_rem(omode));
            cond = mod_->add<OpLogicalAnd>(spv_boolean_ty, check1, check2);
        }
        if (needs_mask) {
            spv_inst *mask = mod_->add<OpSLessThan>(spv_boolean_ty, m, unique_.constant(remainder));
            cond = cond ? mod_->add<OpLogicalAnd>(spv_boolean_ty, cond, mask) : mask;
        }
        auto block_vals =
            cond ? make_conditional_execution(spv_ty, cond, load_block, in.loc()) : load_block();
        loaded_values.insert(loaded_values.end(), block_vals.begin(), block_vals.end());
    }
    multi_declare(in.result(0), std::move(loaded_values));
}
void inst_converter::emulate_cooperative_matrix_mul_add(cooperative_matrix_mul_add_inst const &in) {
    auto at = get_coopmatrix_type(in.a());
    auto bt = get_coopmatrix_type(in.b());
    auto ct = get_coopmatrix_type(in.c());
    auto rt = get_coopmatrix_type(in.result(0));
    auto &av = multi_val(in.a());
    auto &bv = multi_val(in.b());
    auto &cv = multi_val(in.c());

    const auto a_ty = at->component_ty();
    const auto b_ty = bt->component_ty();
    const auto b_component_ty = component_type(b_ty);
    const auto c_ty = ct->component_ty();
    const auto r_ty = rt->component_ty();
    const auto spv_b_ty = unique_.spv_ty(bt->ty());
    const auto spv_b_component_ty = unique_.spv_ty(b_component_ty);
    const auto spv_c_ty = unique_.spv_ty(ct->ty());
    const auto spv_r_ty = unique_.spv_ty(rt->ty());
    const bool a_and_b_complex = is_complex_type(a_ty) && is_complex_type(b_ty);

    const std::int32_t N = rt->cols(), K = at->cols();

    const std::int32_t num_blocks = rt->num_blocks(core_cfg_.subgroup_size);
    constexpr std::int32_t nbb = 4;
    auto broadcast_scope = unique_.constant(static_cast<std::int32_t>(Scope::Subgroup));

    auto result = std::vector<spv_inst *>(cv.begin(), cv.end());
    auto result_im = a_and_b_complex
                         ? std::vector<spv_inst *>(cv.size(), unique_.null_constant(spv_c_ty))
                         : std::vector<spv_inst *>{};
    for (std::int32_t m_block = 0; m_block < num_blocks; ++m_block) {
        for (std::int32_t nb = 0; nb < N; nb += nbb) {
            for (std::int32_t k = 0; k < K; ++k) {
                for (std::int32_t n = 0; n < nbb; ++n) {
                    if (nb + n < N) {
                        auto const n_block = (nb + n) / core_cfg_.subgroup_size;
                        auto n_offset = unique_.constant((nb + n) % core_cfg_.subgroup_size);

                        auto a = av[k + m_block * K];
                        auto b = bv[k + n_block * K];
                        auto b_bc =
                            mod_->add<OpGroupBroadcast>(spv_b_ty, broadcast_scope, b, n_offset);
                        auto &c = result[nb + n + m_block * N];

                        if (a_and_b_complex) {
                            auto &c_im = result_im[nb + n + m_block * N];
                            auto b_bc_re = mod_->add<OpCompositeExtract>(
                                spv_b_component_ty, b_bc, std::vector<LiteralInteger>{0});
                            auto b_bc_im = mod_->add<OpCompositeExtract>(
                                spv_b_component_ty, b_bc, std::vector<LiteralInteger>{1});
                            c = make_mixed_precision_fma(a_ty, b_component_ty, c_ty, a, b_bc_re, c,
                                                         in.loc());
                            c_im = make_mixed_precision_fma(a_ty, b_component_ty, c_ty, a, b_bc_im,
                                                            c_im, in.loc());
                        } else {
                            c = make_mixed_precision_fma(a_ty, b_ty, c_ty, a, b_bc, c, in.loc());
                        }
                    }
                }
            }
        }
    }
    if (a_and_b_complex) {
        for (std::size_t i = 0; i < result.size(); ++i) {
            auto &c = result[i];
            auto c_im = result_im[i];
            auto neg_c_im = mod_->add<OpFNegate>(spv_c_ty, c_im);
            auto c_im_times_i = mod_->add<OpVectorShuffle>(spv_c_ty, neg_c_im, c_im,
                                                           std::vector<LiteralInteger>{1, 2});
            c = mod_->add<OpFAdd>(spv_c_ty, c, c_im_times_i);
        }
    }
    for (auto &r : result) {
        if (c_ty != r_ty) {
            r = make_cast(r_ty, c_ty, spv_r_ty, r, in.loc());
        }
    }
    multi_declare(in.result(0), std::move(result));
}
void inst_converter::emulate_cooperative_matrix_scale(cooperative_matrix_scale_inst const &in) {
    auto av = val(in.a());
    auto &bv = multi_val(in.b());

    auto insts = std::vector<spv_inst *>{};
    insts.reserve(bv.size());

    for (std::size_t i = 0; i < bv.size(); ++i) {
        insts.emplace_back(make_binary_op(get_coopmatrix_type(in.result(0))->component_ty(),
                                          arithmetic::mul, unique_.spv_ty(in.a().ty()), av, bv[i],
                                          in.loc()));
    }

    multi_declare(in.result(0), std::move(insts));
}
void inst_converter::emulate_cooperative_matrix_store(cooperative_matrix_store_inst const &in) {
    auto spv_boolean_ty = unique_.spv_ty(boolean_data_type::get(mod_->context()));
    auto spv_index_ty = unique_.spv_ty(scalar_type::index);
    auto spv_operand_ty = unique_.spv_ty(in.operand().ty());
    auto spv_ty = unique_.spv_ty(get_coopmatrix_type(in.val().ty())->ty());
    auto ot = get_memref_type(in.operand());
    auto odv = get_dope_vector(in.operand());
    if (!odv) {
        throw compilation_error(in.loc(), status::spirv_missing_dope_vector);
    }
    auto vt = get_coopmatrix_type(in.val());

    const int vmode = vt->distributed_mode();
    const int omode = vmode;
    const bool check_m = in.checked() == checked_flag::both ||
                         (vmode == 0 && in.checked() == checked_flag::rows) ||
                         (vmode == 1 && in.checked() == checked_flag::cols);
    const bool check_k = in.checked() == checked_flag::both ||
                         (vmode == 1 && in.checked() == checked_flag::rows) ||
                         (vmode == 0 && in.checked() == checked_flag::cols);
    const std::int32_t alignment = std::max(in.align(), ot->alignment());
    constexpr std::int32_t sub_group_write_required_alignment = 16;
    const bool enable_sub_group_writes =
        core_cfg_.block_read_write_supported && !is_complex_type(ot->element_ty()) &&
        in.flag() == store_flag::regular && ot->stride(omode) == 1 &&
        alignment >= sub_group_write_required_alignment;

    spv_inst *pv[] = {val(in.pos0()), val(in.pos1())};
    auto get_rem = [&, rem = std::array<spv_inst *, 2u>{}](std::int64_t index) mutable {
        if (rem[index] == nullptr) {
            rem[index] = mod_->add<OpISub>(spv_index_ty, odv->shape(index), pv[index]);
        }
        return rem[index];
    };

    auto pv0_stride0 = mod_->add<OpIMul>(spv_index_ty, pv[0], odv->stride(0));
    auto pv1_stride1 = mod_->add<OpIMul>(spv_index_ty, pv[1], odv->stride(1));
    auto offset = mod_->add<OpIAdd>(spv_index_ty, pv0_stride0, pv1_stride1);
    auto pointer = mod_->add<OpInBoundsPtrAccessChain>(spv_operand_ty, val(in.operand()), offset,
                                                       std::vector<spv_inst *>{});

    const auto block_store = [&](spv_inst *offset, spv_inst *value) {
        auto sub_pointer = mod_->add<OpInBoundsPtrAccessChain>(spv_operand_ty, pointer, offset,
                                                               std::vector<spv_inst *>{});
        auto const cast_write = [&](scalar_type int_sty) {
            auto int_ty = unique_.spv_ty(int_sty);
            auto ival = mod_->add<OpBitcast>(int_ty, value);
            const auto storage_cls = address_space_to_storage_class(ot->addrspace());
            auto int_ptr_ty = unique_.spv_pointer_ty(storage_cls, int_ty, alignment);
            auto int_ptr = mod_->add<OpBitcast>(int_ptr_ty, sub_pointer);
            mod_->add<OpSubgroupBlockWriteINTEL>(int_ptr, ival);
        };
        switch (ot->element_ty()) {
        case scalar_type::bf16:
        case scalar_type::f16:
            cast_write(scalar_type::i16);
            break;
        case scalar_type::f32:
            cast_write(scalar_type::i32);
            break;
        case scalar_type::f64:
            cast_write(scalar_type::i64);
            break;
        default:
            mod_->add<OpSubgroupBlockWriteINTEL>(sub_pointer, value);
            break;
        }
    };
    const auto scatter_store = [&](spv_inst *offset, spv_inst *value) {
        auto sub_pointer = mod_->add<OpInBoundsPtrAccessChain>(spv_operand_ty, pointer, offset,
                                                               std::vector<spv_inst *>{});
        make_store(in.flag(), ot->element_ty(), ot->addrspace(), sub_pointer, value, in.loc());
    };

    spv_inst *m = load_builtin(BuiltIn::SubgroupLocalInvocationId);
    m = mod_->add<OpSConvert>(spv_index_ty, m);
    const std::int64_t num_blocks = vt->num_blocks(core_cfg_.subgroup_size);
    const std::int64_t K = vt->shape(1 - vmode);
    auto &values = multi_val(in.val());
    for (std::int64_t block = 0; block < num_blocks; ++block) {
        auto const remainder = vt->shape(vmode) - core_cfg_.subgroup_size * block;
        const bool needs_mask = remainder < core_cfg_.subgroup_size;

        const auto store_block_impl = [&](auto store_impl,
                                          spv_inst *offset) -> std::vector<spv_inst *> {
            for (std::int64_t k = 0; k < K; ++k) {
                auto &value = values[k + block * vt->shape(1 - vmode)];
                if (check_k) {
                    auto check1 = mod_->add<OpSLessThanEqual>(spv_boolean_ty, unique_.constant(-k),
                                                              pv[1 - omode]);
                    auto check2 = mod_->add<OpSLessThan>(spv_boolean_ty, unique_.constant(k),
                                                         get_rem(1 - omode));
                    auto cond = mod_->add<OpLogicalAnd>(spv_boolean_ty, check1, check2);
                    auto vals = make_conditional_execution(
                        spv_ty, cond,
                        [&] {
                            store_impl(offset, value);
                            return std::vector<spv_inst *>{};
                        },
                        in.loc());
                } else {
                    store_impl(offset, value);
                }
                if (k + 1 < K) {
                    offset = mod_->add<OpIAdd>(spv_index_ty, offset, odv->stride(1 - omode));
                }
            }
            return {};
        };

        auto const store_block = [&]() -> std::vector<spv_inst *> {
            auto block_offset = unique_.constant(block * core_cfg_.subgroup_size);
            if (enable_sub_group_writes && !needs_mask && !check_m) {
                store_block_impl(block_store, block_offset);
            } else {
                spv_inst *offset = m;
                if (block > 0) {
                    offset = mod_->add<OpIAdd>(spv_index_ty, offset, block_offset);
                }
                offset = mod_->add<OpIMul>(spv_index_ty, offset, odv->stride(omode));
                store_block_impl(scatter_store, offset);
            }
            return {};
        };
        spv_inst *cond = nullptr;
        if (check_m) {
            spv_inst *m_offset = m;
            if (block > 0) {
                m_offset = mod_->add<OpIAdd>(spv_index_ty, m_offset,
                                             unique_.constant(block * core_cfg_.subgroup_size));
            }
            auto neg = mod_->add<OpSNegate>(spv_index_ty, pv[omode]);
            auto check1 = mod_->add<OpSGreaterThanEqual>(spv_boolean_ty, m_offset, neg);
            auto check2 = mod_->add<OpSLessThan>(spv_boolean_ty, m_offset, get_rem(omode));
            cond = mod_->add<OpLogicalAnd>(spv_boolean_ty, check1, check2);
        }
        if (needs_mask) {
            spv_inst *mask = mod_->add<OpSLessThan>(spv_boolean_ty, m, unique_.constant(remainder));
            cond = cond ? mod_->add<OpLogicalAnd>(spv_boolean_ty, cond, mask) : mask;
        }
        if (cond) {
            make_conditional_execution(spv_ty, cond, store_block, in.loc());
        } else {
            store_block();
        }
    }
}

void inst_converter::operator()(inst_node const &in) {
    // @todo
    throw compilation_error(in.loc(), status::not_implemented);
}

void inst_converter::operator()(alloca_inst const &in) {
    if (in.stack_ptr() < 0) {
        throw compilation_error(in.loc(), status::internal_compiler_error,
                                "Invalid stack_ptr in alloca. Did you run set_stack_ptrs?");
    }
    if (!stack_) {
        throw compilation_error(in.loc(), status::internal_compiler_error,
                                "Stack required but not allocated");
    }

    auto mt = get_memref_type(in.result(0));
    if (in.stack_ptr() % mt->alignment() != 0) {
        throw compilation_error(in.loc(), status::ir_insufficient_alignment);
    }

    auto stack_element_ty = unique_.spv_ty(scalar_type::i8);
    auto stack_ptr_ty = unique_.spv_pointer_ty(StorageClass::Workgroup, stack_element_ty,
                                               alignment(scalar_type::i8));
    auto stack_ptr = mod_->add<OpInBoundsAccessChain>(
        stack_ptr_ty, stack_, std::vector<IdRef>{unique_.constant(in.stack_ptr())});

    auto memref_ptr_ty = unique_.spv_ty(mt);
    declare(in.result(0), mod_->add<OpBitcast>(memref_ptr_ty, stack_ptr));

    // alloca only accepts fixed-size memrefs => dope vector is constant
    auto rdv = make_dope_vector(in.result(0));
    for (std::int64_t i = 0; i < mt->dim(); ++i) {
        rdv->shape(i, unique_.constant(mt->shape(i)));
    }
    for (std::int64_t i = 0; i < mt->dim(); ++i) {
        rdv->stride(i, unique_.constant(mt->stride(i)));
    }
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

    if (isa<boolean_data_type>(*in.result(0).ty())) {
        auto ty = unique_.spv_ty(in.result(0).ty());
        auto av = val(in.a());
        auto bv = val(in.b());
        declare(in.result(0), make_boolean(in.operation(), ty, av, bv));
    } else if (auto st = dyn_cast<scalar_data_type>(in.result(0).ty()); st) {
        auto ty = unique_.spv_ty(in.result(0).ty());
        auto av = val(in.a());
        auto bv = val(in.b());
        declare(in.result(0), make_binary_op(st->ty(), in.operation(), ty, av, bv, in.loc()));
    } else if (auto ct = dyn_cast<coopmatrix_data_type>(in.result(0).ty()); ct) {
        auto const length = ct->length(core_cfg_.subgroup_size);
        auto insts = std::vector<spv_inst *>{};
        insts.reserve(length);

        auto ty = unique_.spv_ty(ct->ty());
        auto &av = multi_val(in.a());
        auto &bv = multi_val(in.b());
        for (std::int64_t i = 0; i < length; ++i) {
            insts.emplace_back(
                make_binary_op(ct->component_ty(), in.operation(), ty, av[i], bv[i], in.loc()));
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
            auto spv_a_ty = unique_.spv_ty(sty);
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
            auto spv_float_ty = unique_.spv_ty(component_type(sty));
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
        case scalar_type::bf16: {
            auto float_ty = unique_.spv_ty(scalar_type::f32);
            auto af = mod_->add<OpConvertBF16ToFINTEL>(float_ty, a);
            auto op_af = make_float(op, float_ty, af);
            return mod_->add<OpConvertFToBF16INTEL>(ty, op_af);
        }
        case scalar_type::f16:
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

    if (isa<boolean_data_type>(*in.a().ty())) {
        auto ty = unique_.spv_ty(in.result(0).ty());
        auto av = val(in.a());
        declare(in.result(0), make_boolean(in.operation(), ty, av));
    } else if (auto st = dyn_cast<scalar_data_type>(in.a().ty()); st) {
        auto ty = unique_.spv_ty(in.result(0).ty());
        auto av = val(in.a());
        declare(in.result(0), make(st->ty(), in.operation(), ty, av));
    } else if (auto ct = dyn_cast<coopmatrix_data_type>(in.a().ty()); ct) {
        auto const length = ct->length(core_cfg_.subgroup_size);
        auto insts = std::vector<spv_inst *>{};
        insts.reserve(length);

        auto ty = unique_.spv_ty(ct->ty());
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

void inst_converter::operator()(builtin_inst const &in) {
    switch (in.builtin_type()) {
    case builtin::group_id: {
        auto gid = load_builtin(BuiltIn::GlobalInvocationId);
        auto index_ty = unique_.spv_ty(scalar_type::index);
        declare(in.result(0),
                mod_->add<OpCompositeExtract>(index_ty, gid, std::vector<LiteralInteger>{2}));
        break;
    }
    case builtin::group_size: {
        auto gs = load_builtin(BuiltIn::GlobalSize);
        auto index_ty = unique_.spv_ty(scalar_type::index);
        declare(in.result(0),
                mod_->add<OpCompositeExtract>(index_ty, gs, std::vector<LiteralInteger>{2}));
        break;
    }
    case builtin::num_subgroups:
        declare(in.result(0), load_builtin(BuiltIn::NumSubgroups));
        break;
    case builtin::subgroup_size:
        declare(in.result(0), load_builtin(BuiltIn::SubgroupSize));
        break;
    case builtin::subgroup_id:
        declare(in.result(0), load_builtin(BuiltIn::SubgroupId));
        break;
    case builtin::subgroup_local_id:
        declare(in.result(0), load_builtin(BuiltIn::SubgroupLocalInvocationId));
        break;
    }
}

void inst_converter::operator()(cast_inst const &in) {
    if (auto st = dyn_cast<scalar_data_type>(in.result(0).ty()); st) {
        auto spv_to_ty = unique_.spv_ty(in.result(0).ty());
        auto av = val(in.a());
        auto a_ty = get_scalar_type(in.a());
        declare(in.result(0), make_cast(st->ty(), a_ty, spv_to_ty, av, in.loc()));
    } else if (auto ct = dyn_cast<coopmatrix_data_type>(in.result(0).ty()); ct) {
        auto const length = ct->length(core_cfg_.subgroup_size);
        auto insts = std::vector<spv_inst *>{};
        insts.reserve(length);

        auto spv_to_ty = unique_.spv_ty(ct->ty());
        auto &av = multi_val(in.a());
        auto a_ty = get_coopmatrix_type(in.a())->component_ty();
        for (std::int64_t i = 0; i < length; ++i) {
            insts.emplace_back(make_cast(ct->component_ty(), a_ty, spv_to_ty, av[i], in.loc()));
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
        case scalar_type::bf16: {
            auto float_ty = unique_.spv_ty(scalar_type::f32);
            auto af = mod_->add<OpConvertBF16ToFINTEL>(float_ty, a);
            auto bf = mod_->add<OpConvertBF16ToFINTEL>(float_ty, b);
            auto af_op_bf = compare_float(cond, float_ty, af, bf);
            return mod_->add<OpConvertFToBF16INTEL>(spv_to_ty, af_op_bf);
        }
        case scalar_type::f16:
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
    if (isa<boolean_data_type>(*in.result(0).ty())) {
        if (!std::holds_alternative<bool>(in.value())) {
            throw compilation_error(in.loc(), status::internal_compiler_error);
        }
        declare(in.result(0), unique_.bool_constant(std::get<bool>(in.value())));
    } else if (auto st = dyn_cast<scalar_data_type>(in.result(0).ty()); st) {
        auto spv_ty = unique_.spv_ty(in.result(0).ty());
        auto cst = make_constant(st->ty(), spv_ty, in.value());
        if (cst == nullptr) {
            throw compilation_error(in.loc(), status::internal_compiler_error);
        }
        declare(in.result(0), cst);
    } else if (auto ct = dyn_cast<coopmatrix_data_type>(in.result(0).ty()); ct) {
        auto spv_ty = unique_.spv_ty(ct->ty());
        auto cst = make_constant(ct->component_ty(), spv_ty, in.value());
        if (cst == nullptr) {
            throw compilation_error(in.loc(), status::internal_compiler_error);
        }

        if (mext_.get(in.result(0))) {
            auto spv_ty = unique_.spv_ty(in.result(0).ty());
            declare(in.result(0), mod_->add<OpCompositeConstruct>(spv_ty, std::vector<IdRef>{cst}));
        } else {
            auto const length = ct->length(core_cfg_.subgroup_size);
            multi_declare(in.result(0), std::vector<spv_inst *>(length, cst));
        }
    } else {
        throw compilation_error(in.loc(), status::ir_expected_coopmatrix_or_scalar);
    }
}

void inst_converter::operator()(cooperative_matrix_load_inst const &in) {
    if (mext_.get(in.result(0))) {
        auto odv = get_dope_vector(in.operand());
        if (!odv) {
            throw compilation_error(in.loc(), status::spirv_missing_dope_vector);
        }
        auto spv_operand_ty = unique_.spv_ty(in.operand().ty());
        auto spv_result_ty = unique_.spv_ty(in.result(0).ty());

        auto row_major =
            unique_.constant(static_cast<std::int32_t>(CooperativeMatrixLayout::RowMajorKHR));
        if (in.checked() != checked_flag::none) {
            auto spv_i32_ty = unique_.spv_ty(scalar_type::i32);
            auto pointer = val(in.operand());
            auto x = mod_->add<OpSConvert>(spv_i32_ty, val(in.pos1()));
            auto y = mod_->add<OpSConvert>(spv_i32_ty, val(in.pos0()));
            auto height = mod_->add<OpSConvert>(spv_i32_ty, odv->shape(1));
            auto width = mod_->add<OpSConvert>(spv_i32_ty, odv->shape(0));
            auto stride = mod_->add<OpSConvert>(spv_i32_ty, odv->stride(1));
            declare(in.result(0),
                    mod_->add<OpCooperativeMatrixLoadCheckedINTEL>(
                        spv_result_ty, pointer, x, y, row_major, height, width, stride));
        } else {
            auto spv_index_ty = unique_.spv_ty(scalar_type::index);
            auto pv0_stride0 = mod_->add<OpIMul>(spv_index_ty, val(in.pos0()), odv->stride(0));
            auto pv1_stride1 = mod_->add<OpIMul>(spv_index_ty, val(in.pos1()), odv->stride(1));
            auto offset = mod_->add<OpIAdd>(spv_index_ty, pv0_stride0, pv1_stride1);
            auto pointer = mod_->add<OpInBoundsPtrAccessChain>(spv_operand_ty, val(in.operand()),
                                                               offset, std::vector<spv_inst *>{});

            declare(in.result(0), mod_->add<OpCooperativeMatrixLoadKHR>(spv_result_ty, pointer,
                                                                        row_major, odv->stride(1)));
        }
    } else {
        emulate_cooperative_matrix_load(in);
    }
}
void inst_converter::operator()(cooperative_matrix_mul_add_inst const &in) {
    if (mext_.get(in.result(0))) {
        auto spv_result_ty = unique_.spv_ty(in.result(0).ty());
        // Swap a and b here due to using RowMajorKHR instead of ColumnMajorKHR
        declare(in.result(0), mod_->add<OpCooperativeMatrixMulAddKHR>(spv_result_ty, val(in.b()),
                                                                      val(in.a()), val(in.c())));
    } else {
        emulate_cooperative_matrix_mul_add(in);
    }
}
void inst_converter::operator()(cooperative_matrix_scale_inst const &in) {
    emulate_cooperative_matrix_scale(in);
}
void inst_converter::operator()(cooperative_matrix_store_inst const &in) {
    if (mext_.get(in.val())) {
        auto odv = get_dope_vector(in.operand());
        if (!odv) {
            throw compilation_error(in.loc(), status::spirv_missing_dope_vector);
        }
        auto spv_operand_ty = unique_.spv_ty(in.operand().ty());
        auto spv_val = val(in.val());
        auto row_major =
            unique_.constant(static_cast<std::int32_t>(CooperativeMatrixLayout::RowMajorKHR));

        if (in.checked() != checked_flag::none) {
            auto spv_i32_ty = unique_.spv_ty(scalar_type::i32);
            auto pointer = val(in.operand());
            auto x = mod_->add<OpSConvert>(spv_i32_ty, val(in.pos1()));
            auto y = mod_->add<OpSConvert>(spv_i32_ty, val(in.pos0()));
            auto height = mod_->add<OpSConvert>(spv_i32_ty, odv->shape(1));
            auto width = mod_->add<OpSConvert>(spv_i32_ty, odv->shape(0));
            auto stride = mod_->add<OpSConvert>(spv_i32_ty, odv->stride(1));
            declare(in.result(0), mod_->add<OpCooperativeMatrixStoreCheckedINTEL>(
                                      pointer, x, y, spv_val, row_major, height, width, stride));
        } else {
            auto spv_index_ty = unique_.spv_ty(scalar_type::index);
            auto pv0_stride0 = mod_->add<OpIMul>(spv_index_ty, val(in.pos0()), odv->stride(0));
            auto pv1_stride1 = mod_->add<OpIMul>(spv_index_ty, val(in.pos1()), odv->stride(1));
            auto offset = mod_->add<OpIAdd>(spv_index_ty, pv0_stride0, pv1_stride1);
            auto pointer = mod_->add<OpInBoundsPtrAccessChain>(spv_operand_ty, val(in.operand()),
                                                               offset, std::vector<spv_inst *>{});

            mod_->add<OpCooperativeMatrixStoreKHR>(pointer, spv_val, row_major, odv->stride(1));
        }
    } else {
        emulate_cooperative_matrix_store(in);
    }
}

void inst_converter::operator()(expand_inst const &in) {
    auto spv_index_ty = unique_.spv_ty(scalar_type::index);

    auto shape = std::vector<spv_inst *>{};
    auto stride = std::vector<spv_inst *>{};
    auto const make_shape_stride = [&] {
        auto mt = get_memref_type(in.operand());
        auto dv = get_dope_vector(in.operand());
        if (!dv) {
            throw compilation_error(in.loc(), status::spirv_missing_dope_vector);
        }
        auto static_shape = in.static_expand_shape();
        auto dyn_shape = in.expand_shape();

        shape.reserve(mt->dim() + static_shape.size() - 1);
        stride.reserve(mt->dim() + static_shape.size() - 1);

        for (std::int64_t i = 0; i < in.expanded_mode(); ++i) {
            shape.push_back(dv->shape(i));
            stride.push_back(dv->stride(i));
        }

        auto get_shape = [&, j = std::size_t{0}](std::int64_t s) mutable {
            if (is_dynamic_value(s)) {
                return val(dyn_shape[j++]);
            }
            return unique_.constant(s);
        };
        stride.push_back(dv->stride(in.expanded_mode()));
        shape.push_back(get_shape(static_shape[0]));
        for (std::size_t j = 1; j < static_shape.size(); ++j) {
            stride.push_back(mod_->add<OpIMul>(spv_index_ty, stride.back(), shape.back()));
            shape.push_back(get_shape(static_shape[j]));
        }

        for (std::int64_t i = in.expanded_mode() + 1; i < mt->dim(); ++i) {
            shape.push_back(dv->shape(i));
            stride.push_back(dv->stride(i));
        }
    };
    make_shape_stride();
    declare(in.result(0), val(in.operand()));

    auto rdv = make_dope_vector(in.result(0));

    if (shape.size() != static_cast<std::size_t>(rdv->dim()) ||
        stride.size() != static_cast<std::size_t>(rdv->dim())) {
        throw compilation_error(in.loc(), status::internal_compiler_error);
    }
    for (std::int64_t i = 0; i < rdv->dim(); ++i) {
        rdv->shape(i, shape[i]);
    }
    for (std::int64_t i = 0; i < rdv->dim(); ++i) {
        rdv->stride(i, stride[i]);
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
    auto spv_bool_ty = unique_.spv_ty(boolean_data_type::get(mod_->context()));
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
            if (auto iter_argt = dyn_cast<coopmatrix_data_type>(in.iter_arg(i).ty());
                iter_argt && !mext_.get(in.iter_arg(i))) {
                auto ty = unique_.spv_ty(iter_argt->ty());
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
                auto ty = unique_.spv_ty(in.iter_arg(i).ty());
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
            if (auto rt = dyn_cast<coopmatrix_data_type>(in.result(i).ty());
                rt && !mext_.get(in.result(i))) {
                auto ty = unique_.spv_ty(rt->ty());
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
                auto ty = unique_.spv_ty(in.result(i).ty());
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

void inst_converter::operator()(fuse_inst const &in) {
    auto spv_index_ty = unique_.spv_ty(scalar_type::index);

    auto shape = std::vector<spv_inst *>{};
    auto stride = std::vector<spv_inst *>{};
    auto const make_shape_stride = [&] {
        auto mt = get_memref_type(in.operand());
        auto dv = get_dope_vector(in.operand());
        if (!dv) {
            throw compilation_error(in.loc(), status::spirv_missing_dope_vector);
        }
        shape.reserve(mt->dim());
        stride.reserve(mt->dim());
        std::int64_t i = 0;
        for (; i < in.from(); ++i) {
            shape.push_back(dv->shape(i));
            stride.push_back(dv->stride(i));
        }
        spv_inst *prod = dv->shape(i++);
        for (; i <= in.to(); ++i) {
            prod = mod_->add<OpIMul>(spv_index_ty, prod, dv->shape(i));
        }
        shape.push_back(prod);
        stride.push_back(dv->stride(in.from()));
        for (i = in.to() + 1; i < mt->dim(); ++i) {
            shape.push_back(dv->shape(i));
            stride.push_back(dv->stride(i));
        }
    };
    make_shape_stride();
    declare(in.result(0), val(in.operand()));

    auto rdv = make_dope_vector(in.result(0));

    if (shape.size() != static_cast<std::size_t>(rdv->dim()) ||
        stride.size() != static_cast<std::size_t>(rdv->dim())) {
        throw compilation_error(in.loc(), status::internal_compiler_error);
    }
    for (std::int64_t i = 0; i < rdv->dim(); ++i) {
        rdv->shape(i, shape[i]);
    }
    for (std::int64_t i = 0; i < rdv->dim(); ++i) {
        rdv->stride(i, stride[i]);
    }
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
        if (auto ct = dyn_cast<coopmatrix_data_type>(in.result(i).ty());
            ct && !mext_.get(in.result(i))) {
            auto ty = unique_.spv_ty(ct->ty());
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
            auto ty = unique_.spv_ty(in.result(i).ty());
            auto phi_inst = mod_->add<OpPhi>(
                ty, std::vector<PairIdRefIdRef>{
                        PairIdRefIdRef{yielded_then[val_no], then_last_label},
                        PairIdRefIdRef{yielded_otherwise[val_no], otherwise_last_label}});
            ++val_no;
            declare(in.result(i), phi_inst);
        }
    }
}

void inst_converter::operator()(lifetime_stop_inst const &) {}

void inst_converter::operator()(load_inst const &in) {
    auto spv_index_ty = unique_.spv_ty(scalar_type::index);
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

            auto idx0 = val(in.index_list()[0]);
            spv_inst *offset = memref_ty->stride(0) != 1
                                   ? mod_->add<OpIMul>(spv_index_ty, idx0, dv->stride(0))
                                   : idx0;
            for (std::int64_t i = 1; i < memref_ty->dim(); ++i) {
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

void inst_converter::operator()(parallel_inst const &in) { run_on_region(in.body()); }

void inst_converter::operator()(size_inst const &in) {
    auto dv = get_dope_vector(in.operand());
    if (!dv) {
        throw compilation_error(in.loc(), status::spirv_missing_dope_vector);
    }
    declare(in.result(0), dv->shape(in.mode()));
}

void inst_converter::operator()(subgroup_broadcast_inst const &in) {
    auto broadcast_scope = unique_.constant(static_cast<std::int32_t>(Scope::Subgroup));
    auto spv_ty = unique_.spv_ty(in.result(0).ty());
    auto av = val(in.a());
    auto idxv = val(in.idx());
    declare(in.result(0), mod_->add<OpGroupBroadcast>(spv_ty, broadcast_scope, av, idxv));
}

void inst_converter::operator()(store_inst const &in) {
    auto spv_index_ty = unique_.spv_ty(scalar_type::index);
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

            auto idx0 = val(in.index_list()[0]);
            auto offset = memref_ty->stride(0) != 1
                              ? mod_->add<OpIMul>(spv_index_ty, idx0, dv->stride(0))
                              : idx0;
            for (std::int64_t i = 1; i < memref_ty->dim(); ++i) {
                auto tmp = mod_->add<OpIMul>(spv_index_ty, val(in.index_list()[i]), dv->stride(i));
                offset = mod_->add<OpIAdd>(spv_index_ty, offset, tmp);
            }

            return mod_->add<OpInBoundsPtrAccessChain>(spv_pointer_ty, val(in.operand()), offset,
                                                       std::vector<spv_inst *>{});
        };

        make_store(in.flag(), memref_ty->element_ty(), memref_ty->addrspace(), pointer(),
                   val(in.val()), in.loc());
    } else {
        throw compilation_error(in.loc(), status::ir_expected_memref);
    }
}

void inst_converter::operator()(subview_inst const &in) {
    auto spv_index_ty = unique_.spv_ty(scalar_type::index);
    auto spv_result_ty = unique_.spv_ty(in.result(0).ty());

    auto shape_out = std::vector<spv_inst *>{};
    auto stride_out = std::vector<spv_inst *>{};
    auto const make_offset_and_shape_stride = [&] {
        auto mt = get_memref_type(in.operand());
        auto dv = get_dope_vector(in.operand());
        if (!dv) {
            throw compilation_error(in.loc(), status::spirv_missing_dope_vector);
        }

        int j = 0;
        shape_out.reserve(mt->dim());
        stride_out.reserve(mt->dim());
        auto dyn_offsets = in.offsets();
        auto dyn_sizes = in.sizes();
        auto offset_acc = unique_.null_constant(spv_index_ty);
        for (std::int64_t i = 0, joffset = 0, jsize = 0; i < mt->dim(); ++i) {
            const std::int64_t offset = in.static_offsets()[i];

            auto const offset_inst = [&]() -> spv_inst * {
                if (is_dynamic_value(offset)) {
                    return val(dyn_offsets[joffset++]);
                }
                return unique_.constant(offset);
            };
            auto tmp = mod_->add<OpIMul>(spv_index_ty, offset_inst(), dv->stride(j));
            offset_acc = mod_->add<OpIAdd>(spv_index_ty, offset_acc, tmp);

            const std::int64_t size = in.static_sizes()[i];
            if (size > 0 || is_dynamic_value(size)) {
                auto const size_inst = [&]() -> spv_inst * {
                    if (is_dynamic_value(size)) {
                        return val(dyn_sizes[jsize++]);
                    }
                    return unique_.constant(size);
                };
                shape_out.emplace_back(size_inst());
                stride_out.emplace_back(dv->stride(j));
            }
            ++j;
        }
        return offset_acc;
    };

    auto offset = make_offset_and_shape_stride();
    declare(in.result(0), mod_->add<OpInBoundsPtrAccessChain>(spv_result_ty, val(in.operand()),
                                                              offset, std::vector<spv_inst *>{}));

    auto rdv = make_dope_vector(in.result(0));

    if (shape_out.size() != static_cast<std::size_t>(rdv->dim()) ||
        stride_out.size() != static_cast<std::size_t>(rdv->dim())) {
        throw compilation_error(in.loc(), status::internal_compiler_error);
    }
    for (std::int64_t i = 0; i < rdv->dim(); ++i) {
        rdv->shape(i, shape_out[i]);
    }
    for (std::int64_t i = 0; i < rdv->dim(); ++i) {
        rdv->stride(i, stride_out[i]);
    }
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
            case scalar_type::bf16: {
                auto float_ty = unique_.spv_ty(scalar_type::f32);
                auto operandf = mod_->add<OpConvertBF16ToFINTEL>(float_ty, operand);
                auto reduced =
                    mod_->add<OpGroupFAdd>(float_ty, scope, GroupOperation::Reduce, operandf);
                return mod_->add<OpConvertFToBF16INTEL>(spv_ty, reduced);
            }
            case scalar_type::f16:
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
        if (auto ct = dyn_cast<coopmatrix_data_type>(op.ty()); ct && !mext_.get(op)) {
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

void inst_converter::run_on_function(function_node const &fn, tinytc_core_info const &info) {
    try {
        core_cfg_ = info.get_core_config(fn.subgroup_size());
    } catch (std::out_of_range const &e) {
        throw compilation_error(fn.loc(), status::unsupported_subgroup_size);
    }
    mext_ = matrix_ext_analysis{}.run_on_function(fn, info);

    // Stack
    auto const make_stack = [&] {
        const auto high_water_mark = stack_high_water_mark{}.run_on_function(fn);
        if (high_water_mark > 0) {
            auto stack_element_ty = unique_.spv_ty(scalar_type::i8);
            auto stack_array_ty = unique_.spv_array_ty(stack_element_ty, high_water_mark);
            auto stack_ptr_ty =
                unique_.spv_pointer_ty(StorageClass::Workgroup, stack_array_ty,
                                       alignment(scalar_type::f64, vector_size::v2));
            stack_ = mod_->add_to<OpVariable>(section::type_const_var, stack_ptr_ty,
                                              StorageClass::Workgroup);
            vars_used_by_function_.emplace_back(stack_);
        } else {
            stack_ = nullptr;
        }
    };
    make_stack();

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
    auto void_ty = unique_.spv_ty(void_data_type::get(mod_->context()));
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
                               std::string{fn.name()}, std::move(vars_used_by_function_));

    // Execution mode
    auto const work_group_size = fn.work_group_size();
    mod_->add_to<OpExecutionMode>(
        section::execution_mode, fun, ExecutionMode::LocalSize,
        ExecutionModeAttr{std::array<std::int32_t, 3u>{work_group_size[0], work_group_size[1], 1}});
    mod_->add_to<OpExecutionMode>(section::execution_mode, fun, ExecutionMode::SubgroupSize,
                                  ExecutionModeAttr{fn.subgroup_size()});
}

} // namespace tinytc::spv

