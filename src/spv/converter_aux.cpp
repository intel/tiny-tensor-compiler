// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "spv/converter_aux.hpp"
#include "compiler_context.hpp"
#include "node/visit.hpp"
#include "number_dispatch.hpp"
#include "spv/defs.hpp"
#include "spv/enums.hpp"
#include "spv/instructions.hpp"
#include "spv/opencl.std.hpp"
#include "tinytc/core.hpp"
#include "tinytc/types.hpp"
#include "util/ilist.hpp"
#include "util/ilist_base.hpp"
#include "util/overloaded.hpp"

#include <array>
#include <cmath>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <variant>
#include <vector>

namespace tinytc::spv {

auto get_spv_index_ty(uniquifier &unique, tinytc_compiler_context_t ctx) -> spv_inst * {
    return unique.int_ty(ctx->index_bit_width());
}
auto get_spv_ty(uniquifier &unique, memref_type const *ty) -> spv_inst * {
    const auto storage_cls = address_space_to_storage_class(ty->addrspace());
    auto pointee_ty = get_spv_ty_non_coopmatrix(unique, ty->element_ty());
    const auto align = alignment(ty->element_ty());
    return unique.pointer_ty(storage_cls, pointee_ty, align);
}
auto get_spv_pointer_index_ty(uniquifier &unique, tinytc_compiler_context_t ctx,
                              address_space addrspace) -> spv_inst * {
    auto index_ty = index_type::get(ctx);
    auto memref_ty =
        memref_type::get(index_ty, array_view{dynamic}, array_view{std::int64_t{1}}, addrspace);
    return get_spv_ty(unique, dyn_cast<memref_type>(memref_ty));
}

auto get_spv_ty_non_coopmatrix(uniquifier &unique, tinytc_type_t ty) -> spv_inst * {
    return visit(overloaded{[&](boolean_type &) -> spv_inst * { return unique.bool_ty(); },
                            [&](i8_type &) -> spv_inst * { return unique.int_ty(8); },
                            [&](i16_type &) -> spv_inst * { return unique.int_ty(16); },
                            [&](i32_type &) -> spv_inst * { return unique.int_ty(32); },
                            [&](i64_type &) -> spv_inst * { return unique.int_ty(64); },
                            [&](index_type &ty) -> spv_inst * {
                                return get_spv_index_ty(unique, ty.context());
                            },
                            [&](bf16_type &) -> spv_inst * { return unique.int_ty(16); },
                            [&](f16_type &) -> spv_inst * { return unique.float_ty(16); },
                            [&](f32_type &) -> spv_inst * { return unique.float_ty(32); },
                            [&](f64_type &) -> spv_inst * { return unique.float_ty(64); },
                            [&](c32_type &) -> spv_inst * {
                                return unique.vec_ty(unique.float_ty(32), vector_size::v2);
                            },
                            [&](c64_type &) -> spv_inst * {
                                return unique.vec_ty(unique.float_ty(64), vector_size::v2);
                            },
                            [&](group_type &ty) -> spv_inst * {
                                auto pointee_ty =
                                    get_spv_ty_non_coopmatrix(unique, ty.element_ty());
                                return unique.pointer_ty(StorageClass::CrossWorkgroup, pointee_ty,
                                                         ty.context()->index_bit_width() / 8);
                            },
                            [&](memref_type &ty) -> spv_inst * { return get_spv_ty(unique, &ty); },
                            [&](void_type &) -> spv_inst * { return unique.void_ty(); },
                            [](tinytc_type &) -> spv_inst * {
                                // Coopmatrix handled by matrix impl class
                                throw status::not_implemented;
                            }},
                 *ty);
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

auto split_re_im(uniquifier &unique, tinytc_type_t val_ty, address_space as, spv_inst *pointer,
                 spv_inst *value) -> std::array<std::array<spv_inst *, 2u>, 2u> {
    auto &mod = unique.mod();
    auto float_ty = component_type(val_ty);
    auto spv_float_ty = get_spv_ty_non_coopmatrix(unique, float_ty);
    const auto storage_cls = address_space_to_storage_class(as);
    auto pointer_ty = unique.pointer_ty(storage_cls, spv_float_ty, alignment(float_ty));
    auto c0 = unique.constant(std::int32_t{0});
    auto c1 = unique.constant(std::int32_t{1});
    auto re_ptr = mod.add<OpInBoundsAccessChain>(pointer_ty, pointer, std::vector<IdRef>{c0});
    auto im_ptr = mod.add<OpInBoundsAccessChain>(pointer_ty, pointer, std::vector<IdRef>{c1});
    auto re_val = mod.add<OpCompositeExtract>(spv_float_ty, value, std::vector<LiteralInteger>{0});
    auto im_val = mod.add<OpCompositeExtract>(spv_float_ty, value, std::vector<LiteralInteger>{1});
    return {{{re_ptr, re_val}, {im_ptr, im_val}}};
}

auto make_atomic_load(uniquifier &unique, memory_scope scope, memory_semantics semantics,
                      tinytc_type_t result_ty, address_space as, spv_inst *pointer,
                      location const &loc) -> spv_inst * {
    if ((isa<i8_type>(*result_ty) || isa<i16_type>(*result_ty) || isa<bf16_type>(*result_ty))) {
        throw compilation_error(loc, status::spirv_unsupported_atomic_data_type);
    }

    auto &mod = unique.mod();
    auto spv_result_ty = get_spv_ty_non_coopmatrix(unique, result_ty);
    auto c_scope = unique.constant(static_cast<std::int32_t>(scope));
    auto c_semantics = unique.constant(static_cast<std::int32_t>(semantics));
    if (isa<complex_type>(*result_ty)) {
        auto float_ty = component_type(result_ty);
        auto spv_float_ty = get_spv_ty_non_coopmatrix(unique, float_ty);
        const auto storage_cls = address_space_to_storage_class(as);
        auto pointer_ty = unique.pointer_ty(storage_cls, spv_float_ty, alignment(float_ty));
        auto c0 = unique.constant(std::int32_t{0});
        auto c1 = unique.constant(std::int32_t{1});
        auto re_ptr = mod.add<OpInBoundsAccessChain>(pointer_ty, pointer, std::vector<IdRef>{c0});
        auto im_ptr = mod.add<OpInBoundsAccessChain>(pointer_ty, pointer, std::vector<IdRef>{c1});
        auto re = mod.add<OpAtomicLoad>(spv_float_ty, re_ptr, c_scope, c_semantics);
        auto im = mod.add<OpAtomicLoad>(spv_float_ty, im_ptr, c_scope, c_semantics);
        auto dummy = mod.add<OpUndef>(spv_result_ty);
        auto tmp =
            mod.add<OpCompositeInsert>(spv_result_ty, re, dummy, std::vector<LiteralInteger>{0});
        return mod.add<OpCompositeInsert>(spv_result_ty, im, tmp, std::vector<LiteralInteger>{1});
    } else {
        return mod.add<OpAtomicLoad>(spv_result_ty, pointer, c_scope, c_semantics);
    }
}

void make_atomic_store(uniquifier &unique, memory_scope scope, memory_semantics semantics,
                       tinytc_type_t val_ty, address_space as, spv_inst *pointer, spv_inst *value,
                       location const &loc) {
    if ((isa<i8_type>(*val_ty) || isa<i16_type>(*val_ty) || isa<bf16_type>(*val_ty))) {
        throw compilation_error(loc, status::spirv_unsupported_atomic_data_type);
    }

    auto &mod = unique.mod();
    auto c_scope = unique.constant(static_cast<std::int32_t>(scope));
    auto c_semantics = unique.constant(static_cast<std::int32_t>(semantics));
    if (isa<complex_type>(*val_ty)) {
        auto re_im = split_re_im(unique, val_ty, as, pointer, value);
        mod.add<OpAtomicStore>(re_im[0][0], c_scope, c_semantics, re_im[0][1]);
        mod.add<OpAtomicStore>(re_im[1][0], c_scope, c_semantics, re_im[1][1]);
    } else {
        mod.add<OpAtomicStore>(pointer, c_scope, c_semantics, value);
    }
}

auto make_binary_op(uniquifier &unique, tinytc_type_t operand_ty, IK op, spv_inst *a, spv_inst *b,
                    location const &loc) -> spv_inst * {
    auto &mod = unique.mod();
    auto const make_boolean = [&](IK op, spv_inst *ty, spv_inst *a, spv_inst *b) -> spv_inst * {
        switch (op) {
        case IK::IK_and:
            return mod.add<OpLogicalAnd>(ty, a, b);
        case IK::IK_or:
            return mod.add<OpLogicalOr>(ty, a, b);
        case IK::IK_xor:
            return mod.add<OpLogicalNotEqual>(ty, a, b);
        default:
            break;
        }
        throw compilation_error(loc, status::ir_boolean_unsupported);
    };
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
    auto ty = get_spv_ty_non_coopmatrix(unique, operand_ty);
    auto binop =
        visit(overloaded{[&](boolean_type &) -> spv_inst * { return make_boolean(op, ty, a, b); },
                         [&](integer_type &) -> spv_inst * { return make_int(op, ty, a, b); },
                         [&](bf16_type &) -> spv_inst * {
                             auto float_ty = unique.float_ty(32);
                             auto af = mod.add<OpConvertBF16ToFINTEL>(float_ty, a);
                             auto bf = mod.add<OpConvertBF16ToFINTEL>(float_ty, b);
                             auto af_op_bf = make_float(op, float_ty, af, bf);
                             return mod.add<OpConvertFToBF16INTEL>(ty, af_op_bf);
                         },
                         [&](float_type &) -> spv_inst * { return make_float(op, ty, a, b); },
                         [&](complex_type &) -> spv_inst * {
                             auto float_ty =
                                 get_spv_ty_non_coopmatrix(unique, component_type(operand_ty));
                             return make_complex(op, ty, float_ty, a, b);
                         },
                         [](tinytc_type &) -> spv_inst * { return nullptr; }},
              *operand_ty);
    if (!binop) {
        throw compilation_error(loc, status::internal_compiler_error);
    }
    return binop;
}

auto make_binary_op_mixed_precision(uniquifier &unique, tinytc_type_t result_ty, IK op,
                                    tinytc_type_t a_ty, spv_inst *a, tinytc_type_t b_ty,
                                    spv_inst *b, location const &loc) -> spv_inst * {
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

auto make_cast(uniquifier &unique, tinytc_type_t to_ty, tinytc_type_t a_ty, spv_inst *a,
               location const &loc) -> spv_inst * {
    auto &mod = unique.mod();
    auto const cast_from_int = [&](tinytc_type_t to_ty, spv_inst *spv_to_ty,
                                   spv_inst *a) -> spv_inst * {
        auto op = visit(
            overloaded{
                [&](integer_type &) -> spv_inst * { return mod.add<OpSConvert>(spv_to_ty, a); },
                [&](bf16_type &) -> spv_inst * {
                    auto float_ty = unique.float_ty(32);
                    auto af = mod.add<OpConvertSToF>(float_ty, a);
                    return mod.add<OpConvertFToBF16INTEL>(spv_to_ty, af);
                },
                [&](float_type &) -> spv_inst * { return mod.add<OpConvertSToF>(spv_to_ty, a); },
                [&](complex_type &) -> spv_inst * {
                    auto cty = component_type(to_ty);
                    auto spv_float_ty = get_spv_ty_non_coopmatrix(unique, cty);
                    auto re = mod.add<OpConvertSToF>(spv_float_ty, a);
                    return mod.add<OpCompositeInsert>(spv_to_ty, re,
                                                      unique.null_constant(spv_to_ty),
                                                      std::vector<LiteralInteger>{0});
                },
                [](tinytc_type &) -> spv_inst * { return nullptr; }},
            *to_ty);
        if (!op) {
            throw compilation_error(loc, status::ir_forbidden_cast);
        }
        return op;
    };
    auto const cast_from_float = [&](tinytc_type_t to_ty, spv_inst *spv_to_ty, tinytc_type_t a_ty,
                                     spv_inst *a) -> spv_inst * {
        auto op = visit(
            overloaded{
                [&](integer_type &) -> spv_inst * { return mod.add<OpConvertFToS>(spv_to_ty, a); },
                [&](bf16_type &) -> spv_inst * {
                    return mod.add<OpConvertFToBF16INTEL>(spv_to_ty, a);
                },
                [&](float_type &) -> spv_inst * { return mod.add<OpFConvert>(spv_to_ty, a); },
                [&](complex_type &) -> spv_inst * {
                    auto re = a;
                    auto cty = component_type(to_ty);
                    if (cty != a_ty) {
                        auto spv_float_ty = get_spv_ty_non_coopmatrix(unique, cty);
                        re = mod.add<OpFConvert>(spv_float_ty, a);
                    }
                    // If the line below is change, adjust make_complex_mul as well
                    return mod.add<OpCompositeInsert>(spv_to_ty, re,
                                                      unique.null_constant(spv_to_ty),
                                                      std::vector<LiteralInteger>{0});
                },
                [](tinytc_type &) -> spv_inst * { return nullptr; }},
            *to_ty);
        if (!op) {
            throw compilation_error(loc, status::ir_forbidden_cast);
        }
        return op;
    };
    auto const cast_from_complex = [&](tinytc_type_t to_ty, spv_inst *spv_to_ty,
                                       spv_inst *a) -> spv_inst * {
        if (isa<complex_type>(*to_ty)) {
            return mod.add<OpFConvert>(spv_to_ty, a);
        }
        throw compilation_error(loc, status::ir_forbidden_cast);
    };

    auto spv_to_ty = get_spv_ty_non_coopmatrix(unique, to_ty);
    if (a_ty == to_ty) {
        return mod.add<OpCopyObject>(spv_to_ty, a);
    }

    auto castop =
        visit(overloaded{[&](integer_type &) { return cast_from_int(to_ty, spv_to_ty, a); },
                         [&](bf16_type &) {
                             auto float_ty = unique.float_ty(32);
                             auto af = mod.add<OpConvertBF16ToFINTEL>(float_ty, a);
                             return cast_from_float(to_ty, spv_to_ty,
                                                    f32_type::get(to_ty->context()), af);
                         },
                         [&](float_type &) { return cast_from_float(to_ty, spv_to_ty, a_ty, a); },
                         [&](complex_type &) { return cast_from_complex(to_ty, spv_to_ty, a); },
                         [](tinytc_type &) -> spv_inst * { return nullptr; }},
              *a_ty);
    if (!castop) {
        throw compilation_error(loc, status::internal_compiler_error);
    }
    return castop;
}

auto make_complex_mul(uniquifier &unique, spv_inst *ty, spv_inst *a, spv_inst *b, bool conj_b)
    -> spv_inst * {
    auto &mod = unique.mod();
    const auto is_constant_with_zero = [&](spv_inst *a, std::size_t part) {
        if (auto ci = dyn_cast<OpConstantComposite>(a); ci && ci->op0().size() > part) {
            auto imag_part = dyn_cast<OpConstant>(ci->op0()[part]);
            return imag_part && ci->type() == ty &&
                   std::visit(overloaded{[]<typename T>(T &number) { return number == T{0}; }},
                              imag_part->op0());
        }
        return false;
    };
    // We capture the case here if "a" stems from a non-complex -> complex cast
    const auto is_imag_zero_after_cast = [&](spv_inst *a) -> bool {
        if (auto ci = dyn_cast<OpCompositeInsert>(a); ci) {
            return ci->type() == ty && ci->op1() == unique.null_constant(ty) &&
                   ci->op2().size() == 1 && ci->op2()[0] == 0;
        }
        return false;
    };
    const auto is_real_zero = [&](spv_inst *a) -> bool { return is_constant_with_zero(a, 0); };
    const auto is_imag_zero = [&](spv_inst *a) -> bool {
        return is_constant_with_zero(a, 1) || is_imag_zero_after_cast(a);
    };

    if (is_real_zero(a)) {
        auto neg_b = mod.add<OpFNegate>(ty, b);
        auto b_times_i =
            conj_b ? mod.add<OpVectorShuffle>(ty, b, neg_b, std::vector<LiteralInteger>{1, 2})
                   : mod.add<OpVectorShuffle>(ty, neg_b, b, std::vector<LiteralInteger>{1, 2});
        auto a_1 = mod.add<OpVectorShuffle>(ty, a, a, std::vector<LiteralInteger>{1, 1});
        return mod.add<OpFMul>(ty, a_1, b_times_i);
    } else if (is_real_zero(b)) {
        auto neg_a = mod.add<OpFNegate>(ty, a);
        auto a_times_i =
            conj_b ? mod.add<OpVectorShuffle>(ty, a, neg_a, std::vector<LiteralInteger>{1, 2})
                   : mod.add<OpVectorShuffle>(ty, neg_a, a, std::vector<LiteralInteger>{1, 2});
        auto b_1 = mod.add<OpVectorShuffle>(ty, b, b, std::vector<LiteralInteger>{1, 1});
        return mod.add<OpFMul>(ty, b_1, a_times_i);
    } else if (is_imag_zero(a)) {
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

auto make_compare_op(uniquifier &unique, tinytc_type_t operand_ty, IK op, spv_inst *a, spv_inst *b,
                     location const &loc) -> spv_inst * {
    auto &mod = unique.mod();
    auto bool_ty = unique.bool_ty();
    auto const compare_int = [&](IK cond, spv_inst *a, spv_inst *b) -> spv_inst * {
        switch (cond) {
        case IK::IK_equal:
            return mod.add<OpIEqual>(bool_ty, a, b);
        case IK::IK_not_equal:
            return mod.add<OpINotEqual>(bool_ty, a, b);
        case IK::IK_greater_than:
            return mod.add<OpSGreaterThan>(bool_ty, a, b);
        case IK::IK_greater_than_equal:
            return mod.add<OpSGreaterThanEqual>(bool_ty, a, b);
        case IK::IK_less_than:
            return mod.add<OpSLessThan>(bool_ty, a, b);
        case IK::IK_less_than_equal:
            return mod.add<OpSLessThanEqual>(bool_ty, a, b);
        default:
            break;
        }
        throw compilation_error(loc, status::internal_compiler_error);
    };
    auto const compare_float = [&](IK cond, spv_inst *a, spv_inst *b) -> spv_inst * {
        switch (cond) {
        case IK::IK_equal:
            return mod.add<OpFOrdEqual>(bool_ty, a, b);
        case IK::IK_not_equal:
            return mod.add<OpFUnordNotEqual>(bool_ty, a, b);
        case IK::IK_greater_than:
            return mod.add<OpFOrdGreaterThan>(bool_ty, a, b);
        case IK::IK_greater_than_equal:
            return mod.add<OpFOrdGreaterThanEqual>(bool_ty, a, b);
        case IK::IK_less_than:
            return mod.add<OpFOrdLessThan>(bool_ty, a, b);
        case IK::IK_less_than_equal:
            return mod.add<OpFOrdLessThanEqual>(bool_ty, a, b);
        default:
            break;
        }
        throw compilation_error(loc, status::internal_compiler_error);
    };
    auto const compare_complex = [&](IK cond, spv_inst *a, spv_inst *b) -> spv_inst * {
        auto bool2_ty = unique.vec_ty(bool_ty, vector_size::v2);
        switch (cond) {
        case IK::IK_equal: {
            auto components_equal = mod.add<OpFOrdEqual>(bool2_ty, a, b);
            return mod.add<OpAll>(bool_ty, components_equal);
        }
        case IK::IK_not_equal: {
            auto components_not_equal = mod.add<OpFUnordNotEqual>(bool2_ty, a, b);
            return mod.add<OpAll>(bool_ty, components_not_equal);
        }
        default:
            throw compilation_error(loc, status::ir_complex_unsupported);
        }
    };
    auto cmpop = visit(overloaded{[&](integer_type &) { return compare_int(op, a, b); },
                                  [&](bf16_type &) {
                                      auto float_ty = unique.float_ty(32);
                                      auto af = mod.add<OpConvertBF16ToFINTEL>(float_ty, a);
                                      auto bf = mod.add<OpConvertBF16ToFINTEL>(float_ty, b);
                                      return compare_float(op, af, bf);
                                  },
                                  [&](float_type &) { return compare_float(op, a, b); },
                                  [&](complex_type &) { return compare_complex(op, a, b); },
                                  [](tinytc_type &) -> spv_inst * { return nullptr; }},
                       *operand_ty);
    if (!cmpop) {
        throw compilation_error(loc, status::internal_compiler_error);
    }
    return cmpop;
}

auto make_constant(uniquifier &unique, tinytc_type_t ty, constant_value_type const &val)
    -> spv_inst * {
    const auto visitor = overloaded{
        [&](bool b) -> spv_inst * {
            if (!isa<boolean_type>(*ty)) {
                throw status::ir_expected_boolean;
            }
            return unique.bool_constant(b);
        },
        [&](std::int64_t i) -> spv_inst * {
            return dispatch_int_to_native(
                ty, [&]<typename T>() { return unique.constant(static_cast<T>(i)); });
        },
        [&](double d) -> spv_inst * {
            return dispatch_float_to_native(ty, [&]<typename T>() {
                if constexpr (std::is_same_v<T, bfloat16>) {
                    return unique.constant(bfloat16{static_cast<float>(d)}.bits());
                } else if constexpr (std::is_same_v<T, half>) {
                    return unique.constant(half{static_cast<float>(d)});
                } else {
                    return unique.constant(static_cast<T>(d));
                }
            });
        },
        [&](std::complex<double> c) -> spv_inst * {
            return dispatch_complex_to_native(ty, [&]<typename T>() {
                auto cst = static_cast<T>(c);
                auto c_re = unique.constant(cst.real());
                auto c_im = unique.constant(cst.imag());
                auto cst_ty = get_spv_ty_non_coopmatrix(unique, ty);
                return unique.mod().add_to<OpConstantComposite>(
                    section::type_const_var, cst_ty, std::vector<spv_inst *>{c_re, c_im});
            });
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

auto make_math_unary_op(uniquifier &unique, tinytc_type_t operand_ty, IK op, spv_inst *a,
                        location const &loc) -> spv_inst * {
    auto &mod = unique.mod();
    auto const make_float = [&](IK op, spv_inst *ty, spv_inst *a) -> spv_inst * {
        auto const make_ext_inst = [&](OpenCLEntrypoint ep) {
            return mod.add<OpExtInst>(ty, unique.opencl_ext(), static_cast<std::int32_t>(ep),
                                      std::vector<IdRef>{a});
        };
        switch (op) {
        case IK::IK_cos:
            return make_ext_inst(OpenCLEntrypoint::cos);
        case IK::IK_sin:
            return make_ext_inst(OpenCLEntrypoint::sin);
        case IK::IK_exp:
            return make_ext_inst(OpenCLEntrypoint::exp);
        case IK::IK_exp2:
            return make_ext_inst(OpenCLEntrypoint::exp2);
        case IK::IK_log:
            return make_ext_inst(OpenCLEntrypoint::log);
        case IK::IK_log2:
            return make_ext_inst(OpenCLEntrypoint::log2);
        case IK::IK_native_cos:
            return make_ext_inst(OpenCLEntrypoint::native_cos);
        case IK::IK_native_sin:
            return make_ext_inst(OpenCLEntrypoint::native_sin);
        case IK::IK_native_exp:
            return make_ext_inst(OpenCLEntrypoint::native_exp);
        case IK::IK_native_exp2:
            return make_ext_inst(OpenCLEntrypoint::native_exp2);
        case IK::IK_native_log:
            return make_ext_inst(OpenCLEntrypoint::native_log);
        case IK::IK_native_log2:
            return make_ext_inst(OpenCLEntrypoint::native_log2);
        default:
            throw compilation_error(loc, status::internal_compiler_error);
        }
    };
    auto const make_complex = [&]<typename T>(IK op, tinytc_type_t operand_ty, spv_inst *ty,
                                              spv_inst *a) -> spv_inst * {
        auto cty = component_type(operand_ty);
        auto float_ty = get_spv_ty_non_coopmatrix(unique, cty);
        auto const make_complex_exp = [&](auto exp_ep, auto cos_ep, auto sin_ep,
                                          spv_inst *im_scale = nullptr) {
            auto a0 = mod.add<OpCompositeExtract>(float_ty, a, std::vector<LiteralInteger>{0});
            spv_inst *a1 = mod.add<OpCompositeExtract>(float_ty, a, std::vector<LiteralInteger>{1});
            if (im_scale) {
                a1 = mod.add<OpFMul>(float_ty, a1, im_scale);
            }
            auto e = mod.add<OpExtInst>(float_ty, unique.opencl_ext(),
                                        static_cast<std::int32_t>(exp_ep), std::vector<IdRef>{a0});
            auto c = mod.add<OpExtInst>(float_ty, unique.opencl_ext(),
                                        static_cast<std::int32_t>(cos_ep), std::vector<IdRef>{a1});
            auto s = mod.add<OpExtInst>(float_ty, unique.opencl_ext(),
                                        static_cast<std::int32_t>(sin_ep), std::vector<IdRef>{a1});
            auto r = mod.add<OpFMul>(float_ty, e, c);
            auto i = mod.add<OpFMul>(float_ty, e, s);
            auto dummy = mod.add<OpUndef>(ty);
            auto result = mod.add<OpCompositeInsert>(ty, r, dummy, std::vector<LiteralInteger>{0});
            return mod.add<OpCompositeInsert>(ty, i, result, std::vector<LiteralInteger>{1});
        };
        switch (op) {
        case IK::IK_exp:
            return make_complex_exp(OpenCLEntrypoint::exp, OpenCLEntrypoint::cos,
                                    OpenCLEntrypoint::sin);
        case IK::IK_exp2:
            return make_complex_exp(OpenCLEntrypoint::exp2, OpenCLEntrypoint::cos,
                                    OpenCLEntrypoint::sin, unique.constant(std::log(T{2})));
        case IK::IK_native_exp:
            return make_complex_exp(OpenCLEntrypoint::native_exp, OpenCLEntrypoint::native_cos,
                                    OpenCLEntrypoint::native_sin);
        case IK::IK_native_exp2:
            return make_complex_exp(OpenCLEntrypoint::native_exp2, OpenCLEntrypoint::native_cos,
                                    OpenCLEntrypoint::native_sin, unique.constant(std::log(T{2})));
        default:
            throw compilation_error(loc, status::internal_compiler_error);
        }
    };

    auto ty = get_spv_ty_non_coopmatrix(unique, operand_ty);
    auto unop =
        visit(overloaded{[&](bf16_type &) -> spv_inst * {
                             auto float_ty = unique.float_ty(32);
                             auto af = mod.add<OpConvertBF16ToFINTEL>(float_ty, a);
                             auto op_af = make_float(op, float_ty, af);
                             return mod.add<OpConvertFToBF16INTEL>(ty, op_af);
                         },
                         [&](float_type &) -> spv_inst * { return make_float(op, ty, a); },
                         [&](c32_type &) -> spv_inst * {
                             return make_complex.template operator()<float>(op, operand_ty, ty, a);
                         },
                         [&](c64_type &) -> spv_inst * {
                             return make_complex.template operator()<double>(op, operand_ty, ty, a);
                         },
                         [](tinytc_type &) -> spv_inst * { return nullptr; }},
              *operand_ty);
    if (!unop) {
        throw compilation_error(loc, status::internal_compiler_error);
    }
    return unop;
}

auto make_unary_op(uniquifier &unique, tinytc_type_t operand_ty, IK op, spv_inst *a,
                   location const &loc) -> spv_inst * {
    auto &mod = unique.mod();
    auto const make_boolean = [&](IK op, spv_inst *ty, spv_inst *a) -> spv_inst * {
        switch (op) {
        case IK::IK_not:
            return mod.add<OpLogicalNot>(ty, a);
        default:
            break;
        }
        throw compilation_error(loc, status::ir_boolean_unsupported);
    };
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
    auto const make_complex = [&](IK op, spv_inst *ty, spv_inst *float_ty,
                                  spv_inst *a) -> spv_inst * {
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

    auto ty = get_spv_ty_non_coopmatrix(unique, operand_ty);
    auto unop =
        visit(overloaded{[&](boolean_type &) -> spv_inst * { return make_boolean(op, ty, a); },
                         [&](integer_type &) -> spv_inst * { return make_int(op, ty, a); },
                         [&](bf16_type &) -> spv_inst * {
                             auto float_ty = unique.float_ty(32);
                             auto af = mod.add<OpConvertBF16ToFINTEL>(float_ty, a);
                             auto op_af = make_float(op, float_ty, af);
                             return mod.add<OpConvertFToBF16INTEL>(ty, op_af);
                         },
                         [&](float_type &) -> spv_inst * { return make_float(op, ty, a); },
                         [&](complex_type &) -> spv_inst * {
                             auto float_ty =
                                 get_spv_ty_non_coopmatrix(unique, component_type(operand_ty));
                             return make_complex(op, ty, float_ty, a);
                         },
                         [](tinytc_type &) -> spv_inst * { return nullptr; }},
              *operand_ty);
    if (!unop) {
        throw compilation_error(loc, status::internal_compiler_error);
    }
    return unop;
}

template <typename T>
concept spv_ops = requires() {
    typename T::i;
    typename T::f;
};
auto make_subgroup_op(uniquifier &unique, tinytc_type_t op_ty, IK op, spv_inst *a,
                      location const &loc) -> spv_inst * {
    auto &mod = unique.mod();
    auto const make_impl = [&]<spv_ops Ops>(tinytc_type_t op_ty, GroupOperation group_op,
                                            spv_inst *ty, spv_inst *a) -> spv_inst * {
        auto scope = unique.constant(static_cast<std::int32_t>(Scope::Subgroup));
        auto unop = visit(overloaded{[&](integer_type &) -> spv_inst * {
                                         return mod.add<typename Ops::i>(ty, scope, group_op, a);
                                     },
                                     [&](bf16_type &) -> spv_inst * {
                                         auto float_ty = unique.float_ty(32);
                                         auto af = mod.add<OpConvertBF16ToFINTEL>(float_ty, a);
                                         auto op_af =
                                             mod.add<typename Ops::f>(ty, scope, group_op, af);
                                         return mod.add<OpConvertFToBF16INTEL>(ty, op_af);
                                     },
                                     [&](float_type &) -> spv_inst * {
                                         return mod.add<typename Ops::f>(ty, scope, group_op, a);
                                     },
                                     [&](complex_type &) -> spv_inst * {
                                         return mod.add<typename Ops::f>(ty, scope, group_op, a);
                                     },
                                     [](tinytc_type &) -> spv_inst * { return nullptr; }},
                          *op_ty);
        if (!unop) {
            throw compilation_error(loc, status::internal_compiler_error);
        }
        return unop;
    };
    auto ty = get_spv_ty_non_coopmatrix(unique, op_ty);
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
    switch (op) {
    case IK::IK_subgroup_exclusive_scan_add:
        return make_impl.template operator()<add_ops>(op_ty, GroupOperation::ExclusiveScan, ty, a);
    case IK::IK_subgroup_exclusive_scan_max:
        return make_impl.template operator()<max_ops>(op_ty, GroupOperation::ExclusiveScan, ty, a);
    case IK::IK_subgroup_exclusive_scan_min:
        return make_impl.template operator()<min_ops>(op_ty, GroupOperation::ExclusiveScan, ty, a);
    case IK::IK_subgroup_inclusive_scan_add:
        return make_impl.template operator()<add_ops>(op_ty, GroupOperation::InclusiveScan, ty, a);
    case IK::IK_subgroup_inclusive_scan_max:
        return make_impl.template operator()<max_ops>(op_ty, GroupOperation::InclusiveScan, ty, a);
    case IK::IK_subgroup_inclusive_scan_min:
        return make_impl.template operator()<min_ops>(op_ty, GroupOperation::InclusiveScan, ty, a);
    case IK::IK_subgroup_reduce_add:
        return make_impl.template operator()<add_ops>(op_ty, GroupOperation::Reduce, ty, a);
    case IK::IK_subgroup_reduce_max:
        return make_impl.template operator()<max_ops>(op_ty, GroupOperation::Reduce, ty, a);
    case IK::IK_subgroup_reduce_min:
        return make_impl.template operator()<min_ops>(op_ty, GroupOperation::Reduce, ty, a);
    default:
        break;
    }
    throw compilation_error(loc, status::internal_compiler_error);
}

} // namespace tinytc::spv

