// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "spv/converter_aux.hpp"
#include "error.hpp"
#include "scalar_type.hpp"
#include "spv/defs.hpp"
#include "spv/enums.hpp"
#include "spv/instructions.hpp"
#include "spv/module.hpp"
#include "spv/opencl.std.hpp"
#include "spv/uniquifier.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"
#include "util/casting.hpp"
#include "util/ilist.hpp"
#include "util/ilist_base.hpp"
#include "util/overloaded.hpp"

#include <array>
#include <complex>
#include <cstdint>
#include <memory>
#include <variant>
#include <vector>

namespace tinytc::spv {

auto convert_group_operation(group_operation op) -> GroupOperation {
    switch (op) {
    case group_operation::exclusive_scan:
        return GroupOperation::ExclusiveScan;
    case group_operation::inclusive_scan:
        return GroupOperation::InclusiveScan;
    case group_operation::reduce:
        return GroupOperation::Reduce;
    }
    throw status::internal_compiler_error;
}

auto get_last_label(tinytc_spv_mod &mod) -> spv_inst * {
    auto &insts = mod.insts(section::function);
    auto it = insts.end();
    while (it != insts.begin()) {
        auto in = (--it).get();
        if (isa<OpLabel>(*in)) {
            return in;
        }
    }
    return nullptr;
}

auto make_binary_op(uniquifier &unique, scalar_type sty, IK op, spv_inst *a, spv_inst *b,
                    location const &loc) -> spv_inst * {
    auto &mod = unique.mod();
    auto const make_int = [&](IK op, spv_inst *ty, spv_inst *a, spv_inst *b) -> spv_inst * {
        switch (op) {
        case IK::IK_add:
            return mod.add<OpIAdd>(ty, a, b);
        case IK::IK_sub:
            return mod.add<OpISub>(ty, a, b);
        case IK::IK_mul:
            return mod.add<OpIMul>(ty, a, b);
        case IK::IK_div:
            return mod.add<OpSDiv>(ty, a, b);
        case IK::IK_rem:
            return mod.add<OpSRem>(ty, a, b);
        case IK::IK_shl:
            return mod.add<OpShiftLeftLogical>(ty, a, b);
        case IK::IK_shr:
            return mod.add<OpShiftRightArithmetic>(ty, a, b);
        case IK::IK_and:
            return mod.add<OpBitwiseAnd>(ty, a, b);
        case IK::IK_or:
            return mod.add<OpBitwiseOr>(ty, a, b);
        case IK::IK_xor:
            return mod.add<OpBitwiseXor>(ty, a, b);
        case IK::IK_min:
            return mod.add<OpExtInst>(ty, unique.opencl_ext(),
                                      static_cast<std::int32_t>(OpenCLEntrypoint::s_min),
                                      std::vector<IdRef>{a, b});
        case IK::IK_max:
            return mod.add<OpExtInst>(ty, unique.opencl_ext(),
                                      static_cast<std::int32_t>(OpenCLEntrypoint::s_max),
                                      std::vector<IdRef>{a, b});
        default:
            break;
        }
        throw compilation_error(loc, status::internal_compiler_error);
    };
    auto const make_float = [&](IK op, spv_inst *ty, spv_inst *a, spv_inst *b) -> spv_inst * {
        switch (op) {
        case IK::IK_add:
            return mod.add<OpFAdd>(ty, a, b);
        case IK::IK_sub:
            return mod.add<OpFSub>(ty, a, b);
        case IK::IK_mul:
            return mod.add<OpFMul>(ty, a, b);
        case IK::IK_div:
            return mod.add<OpFDiv>(ty, a, b);
        case IK::IK_rem:
            return mod.add<OpFRem>(ty, a, b);
        case IK::IK_min:
            return mod.add<OpExtInst>(ty, unique.opencl_ext(),
                                      static_cast<std::int32_t>(OpenCLEntrypoint::fmin),
                                      std::vector<IdRef>{a, b});
        case IK::IK_max:
            return mod.add<OpExtInst>(ty, unique.opencl_ext(),
                                      static_cast<std::int32_t>(OpenCLEntrypoint::fmax),
                                      std::vector<IdRef>{a, b});
        default:
            break;
        }
        throw compilation_error(loc, status::ir_fp_unsupported);
    };
    auto const make_complex = [&](IK op, spv_inst *ty, spv_inst *float_ty, spv_inst *a,
                                  spv_inst *b) -> spv_inst * {
        switch (op) {
        case IK::IK_add:
            return mod.add<OpFAdd>(ty, a, b);
        case IK::IK_sub:
            return mod.add<OpFSub>(ty, a, b);
        case IK::IK_mul: {
            return make_complex_mul(unique, ty, a, b);
        }
        case IK::IK_div: {
            auto a_times_conj_b = make_complex_mul(unique, ty, a, b, true);

            auto b_squared = mod.add<OpFMul>(ty, b, b);
            auto b_squared_0 =
                mod.add<OpCompositeExtract>(float_ty, b_squared, std::vector<LiteralInteger>{0});
            auto b_squared_1 =
                mod.add<OpCompositeExtract>(float_ty, b_squared, std::vector<LiteralInteger>{1});
            spv_inst *b_abs = mod.add<OpFAdd>(float_ty, b_squared_0, b_squared_1);
            auto dummy = mod.add<OpUndef>(ty);
            b_abs = mod.add<OpCompositeInsert>(ty, b_abs, dummy, std::vector<LiteralInteger>{0});
            b_abs = mod.add<OpVectorShuffle>(ty, b_abs, dummy, std::vector<LiteralInteger>{0, 0});
            return mod.add<OpFDiv>(ty, a_times_conj_b, b_abs);
        }
        default:
            break;
        }
        throw compilation_error(loc, status::ir_complex_unsupported);
    };
    auto ty = unique.scalar_ty(sty);
    switch (sty) {
    case scalar_type::i8:
    case scalar_type::i16:
    case scalar_type::i32:
    case scalar_type::i64:
    case scalar_type::index:
        return make_int(op, ty, a, b);
    case scalar_type::bf16: {
        auto float_ty = unique.scalar_ty(scalar_type::f32);
        auto af = mod.add<OpConvertBF16ToFINTEL>(float_ty, a);
        auto bf = mod.add<OpConvertBF16ToFINTEL>(float_ty, b);
        auto af_op_bf = make_float(op, float_ty, af, bf);
        return mod.add<OpConvertFToBF16INTEL>(ty, af_op_bf);
    }
    case scalar_type::f16:
    case scalar_type::f32:
    case scalar_type::f64:
        return make_float(op, ty, a, b);
    case scalar_type::c32:
    case scalar_type::c64:
        return make_complex(op, ty, unique.scalar_ty(component_type(sty)), a, b);
    }
    throw compilation_error(loc, status::internal_compiler_error);
}

auto make_binary_op_mixed_precision(uniquifier &unique, scalar_type result_ty, IK op,
                                    scalar_type a_ty, spv_inst *a, scalar_type b_ty, spv_inst *b,
                                    location const &loc) -> spv_inst * {
    if (!promotable(a_ty, result_ty) || !promotable(b_ty, result_ty)) {
        throw compilation_error(loc, status::ir_forbidden_promotion);
    }
    if (a_ty != result_ty) {
        a = make_cast(unique, result_ty, a_ty, a, loc);
    }
    if (b_ty != result_ty) {
        b = make_cast(unique, result_ty, b_ty, b, loc);
    }
    return make_binary_op(unique, result_ty, op, a, b, loc);
}

auto make_cast(uniquifier &unique, scalar_type to_ty, scalar_type a_ty, spv_inst *a,
               location const &loc) -> spv_inst * {
    auto &mod = unique.mod();
    auto float_ty = unique.scalar_ty(scalar_type::f32);
    auto const cast_from_int = [&](scalar_type to_ty, spv_inst *spv_to_ty,
                                   spv_inst *a) -> spv_inst * {
        switch (to_ty) {
        case scalar_type::i8:
        case scalar_type::i16:
        case scalar_type::i32:
        case scalar_type::i64:
        case scalar_type::index:
            return mod.add<OpSConvert>(spv_to_ty, a);
        case scalar_type::bf16: {
            auto af = mod.add<OpConvertSToF>(float_ty, a);
            return mod.add<OpConvertFToBF16INTEL>(spv_to_ty, af);
        }
        case scalar_type::f16:
        case scalar_type::f32:
        case scalar_type::f64:
            return mod.add<OpConvertSToF>(spv_to_ty, a);
        case scalar_type::c32:
        case scalar_type::c64: {
            auto spv_float_ty = unique.scalar_ty(component_type(to_ty));
            auto re = mod.add<OpConvertSToF>(spv_float_ty, a);
            return mod.add<OpCompositeInsert>(spv_to_ty, re, unique.null_constant(spv_to_ty),
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
            return mod.add<OpConvertFToS>(spv_to_ty, a);
        case scalar_type::bf16:
            return mod.add<OpConvertFToBF16INTEL>(spv_to_ty, a);
        case scalar_type::f16:
        case scalar_type::f32:
        case scalar_type::f64:
            return mod.add<OpFConvert>(spv_to_ty, a);
        case scalar_type::c32:
        case scalar_type::c64: {
            auto re = a;
            if (component_type(to_ty) != a_ty) {
                auto spv_float_ty = unique.scalar_ty(component_type(to_ty));
                re = mod.add<OpFConvert>(spv_float_ty, a);
            }
            // If the line below is change, adjust make_complex_mul as well
            return mod.add<OpCompositeInsert>(spv_to_ty, re, unique.null_constant(spv_to_ty),
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
            return mod.add<OpFConvert>(spv_to_ty, a);
        default:
            throw compilation_error(loc, status::ir_forbidden_cast);
        }
    };
    auto spv_to_ty = unique.scalar_ty(to_ty);
    if (a_ty == to_ty) {
        return mod.add<OpCopyObject>(spv_to_ty, a);
    }
    switch (a_ty) {
    case scalar_type::i8:
    case scalar_type::i16:
    case scalar_type::i32:
    case scalar_type::i64:
    case scalar_type::index:
        return cast_from_int(to_ty, spv_to_ty, a);
    case scalar_type::bf16: {
        auto af = mod.add<OpConvertBF16ToFINTEL>(float_ty, a);
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

auto make_complex_mul(uniquifier &unique, spv_inst *ty, spv_inst *a, spv_inst *b, bool conj_b)
    -> spv_inst * {
    auto &mod = unique.mod();
    const auto is_imag_zero = [&](spv_inst *a) -> bool {
        // We capture the case here if "a" stems from a non-complex -> complex cast
        if (auto ci = dyn_cast<OpCompositeInsert>(a); ci) {
            return ci->type() == ty && ci->op1() == unique.null_constant(ty) &&
                   ci->op2().size() == 1 && ci->op2()[0] == 0;
        }
        return false;
    };

    if (is_imag_zero(a)) {
        a = mod.add<OpVectorShuffle>(ty, a, a, std::vector<LiteralInteger>{0, 0});
        return mod.add<OpFMul>(ty, a, b);
    } else if (is_imag_zero(b)) {
        b = mod.add<OpVectorShuffle>(ty, b, b, std::vector<LiteralInteger>{0, 0});
        return mod.add<OpFMul>(ty, a, b);
    }

    auto neg_a = mod.add<OpFNegate>(ty, a);
    auto a_times_i =
        conj_b ? mod.add<OpVectorShuffle>(ty, a, neg_a, std::vector<LiteralInteger>{1, 2})
               : mod.add<OpVectorShuffle>(ty, neg_a, a, std::vector<LiteralInteger>{1, 2});
    auto b_1 = mod.add<OpVectorShuffle>(ty, b, b, std::vector<LiteralInteger>{1, 1});
    auto b_1_a_times_i = mod.add<OpFMul>(ty, b_1, a_times_i);
    auto b_0 = mod.add<OpVectorShuffle>(ty, b, b, std::vector<LiteralInteger>{0, 0});
    return mod.add<OpExtInst>(ty, unique.opencl_ext(),
                              static_cast<std::int32_t>(OpenCLEntrypoint::fma),
                              std::vector<IdRef>{a, b_0, b_1_a_times_i});
}

auto make_constant(uniquifier &unique, scalar_type sty, constant_value_type const &val)
    -> spv_inst * {
    auto const add_constant_complex = [&](auto cst) -> spv_inst * {
        auto c_re = unique.constant(cst.real());
        auto c_im = unique.constant(cst.imag());
        auto ty = unique.scalar_ty(sty);
        return unique.mod().add_to<OpConstantComposite>(section::type_const_var, ty,
                                                        std::vector<spv_inst *>{c_re, c_im});
    };
    const auto visitor = overloaded{
        [&](bool) -> spv_inst * { return nullptr; },
        [&](std::int64_t i) -> spv_inst * {
            switch (sty) {
            case scalar_type::i8:
                return unique.constant(static_cast<std::int8_t>(i));
            case scalar_type::i16:
                return unique.constant(static_cast<std::int16_t>(i));
            case scalar_type::i32:
                return unique.constant(static_cast<std::int32_t>(i));
            case scalar_type::i64:
            case scalar_type::index:
                return unique.constant(i);
            default:
                return nullptr;
            }
        },
        [&](double d) -> spv_inst * {
            switch (sty) {
            case scalar_type::bf16:
                return unique.constant(bfloat16{static_cast<float>(d)}.bits());
            case scalar_type::f16:
                return unique.constant(half{static_cast<float>(d)});
            case scalar_type::f32:
                return unique.constant(static_cast<float>(d));
            case scalar_type::f64:
                return unique.constant(d);
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
    auto cst = std::visit(visitor, val);
    if (!cst) {
        throw status::internal_compiler_error;
    }
    return cst;
}

void make_conditional_execution(uniquifier &unique, spv_inst *condition,
                                std::function<void(tinytc_spv_mod &)> then) {
    auto then_label = std::make_unique<OpLabel>();
    auto merge_label = std::make_unique<OpLabel>();

    auto &mod = unique.mod();
    mod.add<OpSelectionMerge>(merge_label.get(), SelectionControl::None);
    mod.add<OpBranchConditional>(condition, then_label.get(), merge_label.get(),
                                 std::vector<LiteralInteger>{});
    mod.insts(section::function).push_back(then_label.release());
    then(mod);
    mod.add<OpBranch>(merge_label.get());
    mod.insts(section::function).push_back(merge_label.release());
}

auto make_conditional_execution(uniquifier &unique, spv_inst *return_ty, spv_inst *condition,
                                std::function<spv_inst *(tinytc_spv_mod &)> then,
                                spv_inst *otherwise, location const &loc) -> spv_inst * {
    auto then_label = std::make_unique<OpLabel>();
    auto merge_label = std::make_unique<OpLabel>();

    auto &mod = unique.mod();
    auto init_last_label = get_last_label(mod);
    if (!init_last_label) {
        throw compilation_error(loc, status::internal_compiler_error);
    }

    mod.add<OpSelectionMerge>(merge_label.get(), SelectionControl::None);
    mod.add<OpBranchConditional>(condition, then_label.get(), merge_label.get(),
                                 std::vector<LiteralInteger>{});
    mod.insts(section::function).push_back(then_label.release());
    spv_inst *yielded_then = then(mod);
    mod.add<OpBranch>(merge_label.get());
    auto then_last_label = get_last_label(mod);
    if (!then_last_label) {
        throw compilation_error(loc, status::internal_compiler_error);
    }

    mod.insts(section::function).push_back(merge_label.release());
    return mod.add<OpPhi>(return_ty,
                          std::vector<PairIdRefIdRef>{PairIdRefIdRef{yielded_then, then_last_label},
                                                      PairIdRefIdRef{otherwise, init_last_label}});
}

auto make_conditional_execution(uniquifier &unique, spv_inst *return_ty, spv_inst *condition,
                                std::function<spv_inst *(tinytc_spv_mod &)> then,
                                std::function<spv_inst *(tinytc_spv_mod &)> otherwise,
                                location const &loc) -> spv_inst * {
    auto then_label = std::make_unique<OpLabel>();
    auto otherwise_label = std::make_unique<OpLabel>();
    auto merge_label = std::make_unique<OpLabel>();

    auto &mod = unique.mod();
    mod.add<OpSelectionMerge>(merge_label.get(), SelectionControl::None);
    mod.add<OpBranchConditional>(condition, then_label.get(), otherwise_label.get(),
                                 std::vector<LiteralInteger>{});
    mod.insts(section::function).push_back(then_label.release());
    spv_inst *yielded_then = then(mod);
    mod.add<OpBranch>(merge_label.get());
    auto then_last_label = get_last_label(mod);
    if (!then_last_label) {
        throw compilation_error(loc, status::internal_compiler_error);
    }
    mod.insts(section::function).push_back(otherwise_label.release());
    spv_inst *yielded_otherwise = otherwise(mod);
    mod.add<OpBranch>(merge_label.get());
    auto otherwise_last_label = get_last_label(mod);
    if (!otherwise_last_label) {
        throw compilation_error(loc, status::internal_compiler_error);
    }

    mod.insts(section::function).push_back(merge_label.release());
    return mod.add<OpPhi>(return_ty, std::vector<PairIdRefIdRef>{
                                         PairIdRefIdRef{yielded_then, then_last_label},
                                         PairIdRefIdRef{yielded_otherwise, otherwise_last_label}});
}

void make_store(uniquifier &unique, store_flag flag, scalar_type sty, address_space as,
                spv_inst *pointer, spv_inst *value, location const &loc) {
    auto &mod = unique.mod();
    auto const split_re_im = [&]() -> std::array<std::array<spv_inst *, 2u>, 2u> {
        auto component_sty = component_type(sty);
        auto float_ty = unique.scalar_ty(component_sty);
        const auto storage_cls = address_space_to_storage_class(as);
        auto pointer_ty = unique.pointer_ty(storage_cls, float_ty, alignment(component_sty));
        auto c0 = unique.constant(std::int32_t{0});
        auto c1 = unique.constant(std::int32_t{1});
        auto re_ptr = mod.add<OpInBoundsAccessChain>(pointer_ty, pointer, std::vector<IdRef>{c0});
        auto im_ptr = mod.add<OpInBoundsAccessChain>(pointer_ty, pointer, std::vector<IdRef>{c1});
        auto re_val = mod.add<OpCompositeExtract>(float_ty, value, std::vector<LiteralInteger>{0});
        auto im_val = mod.add<OpCompositeExtract>(float_ty, value, std::vector<LiteralInteger>{1});
        return {{{re_ptr, re_val}, {im_ptr, im_val}}};
    };
    auto const make_atomic_something = [&]<typename SpvIOp, typename SpvFOp>() {
        auto result_ty = unique.scalar_ty(sty);
        auto scope = unique.constant(static_cast<std::int32_t>(Scope::Workgroup));
        auto semantics = unique.constant(static_cast<std::int32_t>(MemorySemantics::Relaxed));
        switch (sty) {
        case scalar_type::i32:
        case scalar_type::i64:
        case scalar_type::index:
            mod.add<SpvIOp>(result_ty, pointer, scope, semantics, value);
            break;
        case scalar_type::f16:
        case scalar_type::f32:
        case scalar_type::f64:
            mod.add<SpvFOp>(result_ty, pointer, scope, semantics, value);
            break;
        case scalar_type::c32:
        case scalar_type::c64: {
            auto re_im = split_re_im();
            auto component_sty = component_type(sty);
            auto float_ty = unique.scalar_ty(component_sty);
            mod.add<SpvFOp>(float_ty, re_im[0][0], scope, semantics, re_im[0][1]);
            mod.add<SpvFOp>(float_ty, re_im[1][0], scope, semantics, re_im[1][1]);
            break;
        }
        default:
            throw compilation_error(loc, status::spirv_unsupported_atomic_data_type);
        }
    };
    switch (flag) {
    case store_flag::regular:
        mod.add<OpStore>(pointer, value);
        break;
    case store_flag::atomic: {
        auto scope = unique.constant(static_cast<std::int32_t>(Scope::Workgroup));
        auto semantics = unique.constant(static_cast<std::int32_t>(MemorySemantics::Relaxed));
        switch (sty) {
        case scalar_type::c32:
        case scalar_type::c64: {
            auto re_im = split_re_im();
            mod.add<OpAtomicStore>(re_im[0][0], scope, semantics, re_im[0][1]);
            mod.add<OpAtomicStore>(re_im[1][0], scope, semantics, re_im[1][1]);
            break;
        }
        case scalar_type::i32:
        case scalar_type::i64:
        case scalar_type::index:
        case scalar_type::f16:
        case scalar_type::f32:
        case scalar_type::f64:
            mod.add<OpAtomicStore>(pointer, scope, semantics, value);
            break;
        default:
            throw compilation_error(loc, status::spirv_unsupported_atomic_data_type);
        }
        break;
    }
    case store_flag::atomic_add:
        make_atomic_something.template operator()<OpAtomicIAdd, OpAtomicFAddEXT>();
        break;
    case store_flag::atomic_max:
        make_atomic_something.template operator()<OpAtomicSMax, OpAtomicFMaxEXT>();
        break;
    case store_flag::atomic_min:
        make_atomic_something.template operator()<OpAtomicSMin, OpAtomicFMinEXT>();
        break;
    }
}

auto make_unary_op(uniquifier &unique, scalar_type sty, IK op, spv_inst *a, location const &loc)
    -> spv_inst * {
    auto &mod = unique.mod();
    auto const make_int = [&](IK op, spv_inst *ty, spv_inst *a) -> spv_inst * {
        switch (op) {
        case IK::IK_abs:
            return mod.add<OpExtInst>(ty, unique.opencl_ext(),
                                      static_cast<std::int32_t>(OpenCLEntrypoint::s_abs),
                                      std::vector<IdRef>{a});
        case IK::IK_neg:
            return mod.add<OpSNegate>(ty, a);
        case IK::IK_not:
            return mod.add<OpNot>(ty, a);
        default:
            break;
        }
        throw compilation_error(loc, status::internal_compiler_error);
    };
    auto const make_float = [&](IK op, spv_inst *ty, spv_inst *a) -> spv_inst * {
        switch (op) {
        case IK::IK_abs:
            return mod.add<OpExtInst>(ty, unique.opencl_ext(),
                                      static_cast<std::int32_t>(OpenCLEntrypoint::fabs),
                                      std::vector<IdRef>{a});
        case IK::IK_neg:
            return mod.add<OpFNegate>(ty, a);
        default:
            break;
        }
        throw compilation_error(loc, status::internal_compiler_error);
    };
    auto const make_complex = [&](IK op, scalar_type sty, spv_inst *ty, spv_inst *a) -> spv_inst * {
        auto float_ty = unique.scalar_ty(component_type(sty));
        switch (op) {
        case IK::IK_abs: {
            auto a2 = mod.add<OpFMul>(ty, a, a);
            auto a2_0 = mod.add<OpCompositeExtract>(float_ty, a2, std::vector<LiteralInteger>{0});
            auto a2_1 = mod.add<OpCompositeExtract>(float_ty, a2, std::vector<LiteralInteger>{1});
            auto a2_0p1 = mod.add<OpFAdd>(float_ty, a2_0, a2_1);
            return mod.add<OpExtInst>(float_ty, unique.opencl_ext(),
                                      static_cast<std::int32_t>(OpenCLEntrypoint::sqrt),
                                      std::vector<IdRef>{a2_0p1});
        }
        case IK::IK_neg:
            return mod.add<OpFNegate>(ty, a);
        case IK::IK_conj: {
            auto a_im = mod.add<OpCompositeExtract>(float_ty, a, std::vector<LiteralInteger>{1});
            auto neg_a_im = mod.add<OpFNegate>(float_ty, a_im);
            return mod.add<OpCompositeInsert>(ty, neg_a_im, a, std::vector<LiteralInteger>{1});
        }
        case IK::IK_im:
            return mod.add<OpCompositeExtract>(float_ty, a, std::vector<LiteralInteger>{1});
        case IK::IK_re:
            return mod.add<OpCompositeExtract>(float_ty, a, std::vector<LiteralInteger>{0});
        default:
            break;
        }
        throw compilation_error(loc, status::internal_compiler_error);
    };

    spv_inst *ty = unique.scalar_ty(sty);
    switch (sty) {
    case scalar_type::i8:
    case scalar_type::i16:
    case scalar_type::i32:
    case scalar_type::i64:
    case scalar_type::index:
        return make_int(op, ty, a);
    case scalar_type::bf16: {
        auto float_ty = unique.scalar_ty(scalar_type::f32);
        auto af = mod.add<OpConvertBF16ToFINTEL>(float_ty, a);
        auto op_af = make_float(op, float_ty, af);
        return mod.add<OpConvertFToBF16INTEL>(ty, op_af);
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
    throw compilation_error(loc, status::internal_compiler_error);
}

auto make_subgroup_op(uniquifier &unique, scalar_type sty, group_arithmetic arith,
                      group_operation op, spv_inst *a, location const &loc) -> spv_inst * {
    auto &mod = unique.mod();
    auto const make_impl = [&]<typename Ops>(scalar_type sty, GroupOperation group_op, spv_inst *ty,
                                             spv_inst *operand) -> spv_inst * {
        auto scope = unique.constant(static_cast<std::int32_t>(Scope::Subgroup));
        switch (sty) {
        case scalar_type::i8:
        case scalar_type::i16:
        case scalar_type::i32:
        case scalar_type::i64:
        case scalar_type::index:
            return mod.add<typename Ops::i>(ty, scope, group_op, operand);
        case scalar_type::bf16: {
            auto float_ty = unique.scalar_ty(scalar_type::f32);
            auto operandf = mod.add<OpConvertBF16ToFINTEL>(float_ty, operand);
            auto resultf = mod.add<OpGroupFMax>(float_ty, scope, group_op, operandf);
            return mod.add<OpConvertFToBF16INTEL>(ty, resultf);
        }
        case scalar_type::f16:
        case scalar_type::f32:
        case scalar_type::f64:
            return mod.add<typename Ops::f>(ty, scope, group_op, operand);
        case scalar_type::c32:
        case scalar_type::c64:
            return mod.add<typename Ops::f>(ty, scope, group_op, operand);
        }
        throw compilation_error(loc, status::internal_compiler_error);
    };
    auto ty = unique.scalar_ty(sty);
    struct add_ops {
        using i = OpGroupIAdd;
        using f = OpGroupFAdd;
    };
    struct max_ops {
        using i = OpGroupSMax;
        using f = OpGroupFMax;
    };
    struct min_ops {
        using i = OpGroupSMin;
        using f = OpGroupFMin;
    };
    auto spv_op = convert_group_operation(op);
    switch (arith) {
    case group_arithmetic::add:
        return make_impl.template operator()<add_ops>(sty, spv_op, ty, a);
    case group_arithmetic::max:
        return make_impl.template operator()<max_ops>(sty, spv_op, ty, a);
    case group_arithmetic::min:
        return make_impl.template operator()<min_ops>(sty, spv_op, ty, a);
    }
    throw compilation_error(loc, status::internal_compiler_error);
}

} // namespace tinytc::spv

