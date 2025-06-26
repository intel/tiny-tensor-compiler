// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "spv/converter.hpp"
#include "analysis/gcd.hpp"
#include "analysis/stack.hpp"
#include "codegen_tools.hpp"
#include "converter_aux.hpp"
#include "error.hpp"
#include "matrix_ext_info.hpp"
#include "node/attr_node.hpp"
#include "node/data_type_node.hpp"
#include "node/function_node.hpp"
#include "node/inst_node.hpp"
#include "node/inst_view.hpp"
#include "node/program_node.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "node/visit.hpp"
#include "scalar_type.hpp"
#include "spv/coopmatrix_impl_block.hpp"
#include "spv/coopmatrix_impl_dpas.hpp"
#include "spv/enums.hpp"
#include "spv/instructions.hpp"
#include "spv/module.hpp"
#include "spv/opencl.std.hpp"
#include "spv/pass/capex.hpp"
#include "spv/uniquifier.hpp"
#include "spv/visit.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"
#include "util/casting.hpp"
#include "util/ilist.hpp"
#include "util/ilist_base.hpp"
#include "util/iterator.hpp"
#include "util/overloaded.hpp"
#include "util/visit.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace tinytc::spv {

auto convert_prog_to_spirv(tinytc_prog &p, tinytc_core_info const &info) -> ::tinytc::spv_mod {
    auto m = ::tinytc::spv_mod{
        std::make_unique<tinytc_spv_mod>(p.share_context(), info.core_features()).release()};

    auto conv = inst_converter{*m, info};

    m->add_to<OpMemoryModel>(section::memory_model, AddressingModel::Physical64,
                             MemoryModel::OpenCL);

    for (auto &fn : p) {
        conv.run_on_function(fn);
    }

    // Add missing capabilites and extensions
    auto cx = capex{conv.unique()};
    for (std::int32_t s = 0; s < num_module_sections; ++s) {
        for (auto &i : m->insts(enum_cast<section>(s))) {
            visit(cx, i);
        }
    }

    for (int i = 0; i < TINYTC_ENUM_NUM_SPIRV_FEATURE; ++i) {
        const auto feature = enum_cast<spirv_feature>(i);
        if (cx.requires_feature(feature) && !info.have_spirv_feature(feature)) {
            throw compilation_error(p.loc(), status::spirv_required_feature_unavailable,
                                    to_string(feature));
        }
    }

    return m;
}

inst_converter::inst_converter(tinytc_spv_mod &m, tinytc_core_info const &info)
    : mod_(&m), info_(&info), unique_(m) {}

auto inst_converter::get_dope_vector(tinytc_value const &v) -> dope_vector * {
    if (auto it = dope_vec_.find(&v); it != dope_vec_.end()) {
        return &it->second;
    }
    return nullptr;
}

auto inst_converter::declare(tinytc_value const &v, spv_inst *in) { vals_[&v] = in; }
auto inst_converter::val(tinytc_value const &v) -> spv_inst * {
    if (auto it = vals_.find(&v); it != vals_.end()) {
        return it->second;
    }
    throw compilation_error(v.loc(), status::spirv_undefined_value);
}

auto inst_converter::spv_ty(const_tinytc_data_type_t ty) -> spv_inst * {
    return tinytc::visit(
        overloaded{
            [&](void_data_type const &) -> spv_inst * { return unique_.void_ty(); },
            [&](boolean_data_type const &) -> spv_inst * { return unique_.bool_ty(); },
            [&](group_data_type const &g) -> spv_inst * {
                return unique_.pointer_ty(StorageClass::CrossWorkgroup, spv_ty(g.ty()),
                                          alignment(scalar_type::i64));
            },
            [&](memref_data_type const &mr) -> spv_inst * { return unique_.pointer_ty(&mr); },
            [&](scalar_data_type const &ty) -> spv_inst * { return unique_.scalar_ty(ty.ty()); },
            [&](coopmatrix_data_type const &ty) -> spv_inst * { return matrix_impl().spv_ty(&ty); },
            [](auto const &) -> spv_inst * {
                // @todo
                throw status::not_implemented;
            }},
        *ty);
}

auto inst_converter::make_dope_vector(tinytc_value const &v) -> dope_vector * {
    if (dope_vec_.contains(&v)) {
        throw compilation_error(v.loc(), status::internal_compiler_error);
    }

    auto spv_index_ty = unique_.scalar_ty(scalar_type::index);
    return ::tinytc::visit(
        overloaded{[&](memref_data_type const &mr) -> dope_vector * {
                       return &(dope_vec_[&v] = dope_vector{spv_index_ty, mr.shape(), mr.stride()});
                   },
                   [&](group_data_type const &g) -> dope_vector * {
                       if (auto mt = dyn_cast<memref_data_type>(g.ty()); mt) {
                           auto pointer_ty =
                               unique_.pointer_ty(StorageClass::CrossWorkgroup, spv_index_ty,
                                                  alignment(scalar_type::i64));
                           return &(dope_vec_[&v] = dope_vector{
                                        pointer_ty, mt->shape(), mt->stride(), spv_index_ty,
                                        g.size(), spv_index_ty, g.offset()});
                       } else {
                           throw compilation_error(v.loc(), status::ir_expected_memref);
                       }
                   },
                   [](auto const &) -> dope_vector * { return nullptr; }},
        *v.ty());
}

auto inst_converter::matrix_impl() -> coopmatrix_impl & {
    if (matrix_impl_) {
        return *matrix_impl_;
    }
    throw status::internal_compiler_error;
}

void inst_converter::operator()(inst_view in) {
    // @todo
    throw compilation_error(in.loc(), status::not_implemented);
}

void inst_converter::operator()(alloca_inst in) {
    if (in.stack_ptr() < 0) {
        throw compilation_error(in.loc(), status::internal_compiler_error,
                                "Invalid stack_ptr in alloca. Did you run set_stack_ptrs?");
    }
    if (!stack_) {
        throw compilation_error(in.loc(), status::internal_compiler_error,
                                "Stack required but not allocated");
    }

    auto mt = get_memref_type(in.result());
    if (in.stack_ptr() % mt->element_alignment() != 0) {
        throw compilation_error(in.loc(), status::ir_insufficient_alignment);
    }

    auto stack_element_ty = unique_.scalar_ty(scalar_type::i8);
    auto stack_ptr_ty =
        unique_.pointer_ty(StorageClass::Workgroup, stack_element_ty, alignment(scalar_type::i8));
    auto stack_ptr = mod_->add<OpInBoundsAccessChain>(
        stack_ptr_ty, stack_, std::vector<IdRef>{unique_.constant(in.stack_ptr())});

    auto memref_ptr_ty = unique_.pointer_ty(mt);
    declare(in.result(), mod_->add<OpBitcast>(memref_ptr_ty, stack_ptr));

    // alloca only accepts fixed-size memrefs => dope vector is constant
    auto rdv = make_dope_vector(in.result());
    for (std::int64_t i = 0; i < mt->dim(); ++i) {
        rdv->shape(i, unique_.constant(mt->shape(i)));
    }
    for (std::int64_t i = 0; i < mt->dim(); ++i) {
        rdv->stride(i, unique_.constant(mt->stride(i)));
    }
}

void inst_converter::operator()(arith_inst in) {
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

    if (isa<boolean_data_type>(*in.result().ty())) {
        auto ty = unique_.bool_ty();
        auto av = val(in.a());
        auto bv = val(in.b());
        declare(in.result(), make_boolean(in.operation(), ty, av, bv));
    } else if (auto st = dyn_cast<scalar_data_type>(in.result().ty()); st) {
        auto av = val(in.a());
        auto bv = val(in.b());
        declare(in.result(), make_binary_op(unique_, st->ty(), in.operation(), av, bv, in.loc()));
    } else if (auto ct = dyn_cast<coopmatrix_data_type>(in.result().ty()); ct) {
        auto av = val(in.a());
        auto bv = val(in.b());
        declare(in.result(), matrix_impl().arith(in, av, bv));
    } else {
        throw compilation_error(in.loc(), status::ir_expected_coopmatrix_or_scalar);
    }
}

void inst_converter::operator()(arith_unary_inst in) {
    auto const make_boolean = [&](arithmetic_unary op, spv_inst *ty, spv_inst *a) -> spv_inst * {
        switch (op) {
        case arithmetic_unary::not_:
            return mod_->add<OpLogicalNot>(ty, a);
        default:
            break;
        }
        throw compilation_error(in.loc(), status::ir_boolean_unsupported);
    };
    if (isa<boolean_data_type>(*in.a().ty())) {
        auto ty = unique_.bool_ty();
        auto av = val(in.a());
        declare(in.result(), make_boolean(in.operation(), ty, av));
    } else if (auto st = dyn_cast<scalar_data_type>(in.a().ty()); st) {
        auto av = val(in.a());
        declare(in.result(), make_unary_op(unique_, st->ty(), in.operation(), av, in.loc()));
    } else if (auto ct = dyn_cast<coopmatrix_data_type>(in.a().ty()); ct) {
        auto av = val(in.a());
        declare(in.result(), matrix_impl().arith_unary(in, av));
    } else {
        throw compilation_error(in.loc(), status::ir_expected_coopmatrix_or_scalar);
    }
}

void inst_converter::operator()(barrier_inst in) {
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

void inst_converter::operator()(builtin_inst in) {
    switch (in.builtin_type()) {
    case builtin::group_id_x:
    case builtin::group_id_y:
    case builtin::group_id_z: {
        auto gid = unique_.load_builtin(BuiltIn::WorkgroupId);
        auto index_ty = unique_.scalar_ty(scalar_type::index);
        const std::int32_t mode = static_cast<std::int32_t>(in.builtin_type()) -
                                  static_cast<std::int32_t>(builtin::group_id_x);
        declare(in.result(),
                mod_->add<OpCompositeExtract>(index_ty, gid, std::vector<LiteralInteger>{mode}));
        break;
    }
    case builtin::num_groups_x:
    case builtin::num_groups_y:
    case builtin::num_groups_z: {
        auto ng = unique_.load_builtin(BuiltIn::NumWorkgroups);
        auto index_ty = unique_.scalar_ty(scalar_type::index);
        const std::int32_t mode = static_cast<std::int32_t>(in.builtin_type()) -
                                  static_cast<std::int32_t>(builtin::num_groups_x);
        declare(in.result(),
                mod_->add<OpCompositeExtract>(index_ty, ng, std::vector<LiteralInteger>{mode}));
        break;
    }
    case builtin::num_subgroups_x:
        declare(in.result(), unique_.constant(tiling_.m_tiles()));
        break;
    case builtin::num_subgroups_y:
        declare(in.result(), unique_.constant(tiling_.n_tiles()));
        break;
    case builtin::subgroup_size:
        declare(in.result(), unique_.load_builtin(BuiltIn::SubgroupSize));
        break;
    case builtin::subgroup_id_x: {
        auto i32_ty = unique_.scalar_ty(scalar_type::i32);
        auto m_tiles = unique_.constant(tiling_.m_tiles());
        auto sgid = unique_.load_builtin(BuiltIn::SubgroupId);
        declare(in.result(), mod_->add<OpSRem>(i32_ty, sgid, m_tiles));
        break;
    }
    case builtin::subgroup_id_y: {
        auto i32_ty = unique_.scalar_ty(scalar_type::i32);
        auto m_tiles = unique_.constant(tiling_.m_tiles());
        auto sgid = unique_.load_builtin(BuiltIn::SubgroupId);
        declare(in.result(), mod_->add<OpSDiv>(i32_ty, sgid, m_tiles));
        break;
    }
    case builtin::subgroup_linear_id:
        declare(in.result(), unique_.load_builtin(BuiltIn::SubgroupId));
        break;
    case builtin::subgroup_local_id:
        declare(in.result(), unique_.load_builtin(BuiltIn::SubgroupLocalInvocationId));
        break;
    }
}

void inst_converter::operator()(cast_inst in) {
    if (auto st = dyn_cast<scalar_data_type>(in.result().ty()); st) {
        auto av = val(in.a());
        auto a_ty = get_scalar_type(in.a());
        declare(in.result(), make_cast(unique_, st->ty(), a_ty, av, in.loc()));
    } else if (auto ct = dyn_cast<coopmatrix_data_type>(in.result().ty()); ct) {
        declare(in.result(), matrix_impl().cast(in, val(in.a())));
    } else {
        throw compilation_error(in.loc(), status::ir_expected_coopmatrix_or_scalar);
    }
}

void inst_converter::operator()(compare_inst in) {
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
            auto float_ty = unique_.scalar_ty(scalar_type::f32);
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

    auto spv_to_ty = spv_ty(in.result().ty());
    auto av = val(in.a());
    auto bv = val(in.b());
    auto a_ty = get_scalar_type(in.a());
    declare(in.result(), make(a_ty, in.cond(), spv_to_ty, av, bv));
}

void inst_converter::operator()(constant_inst in) {
    if (isa<boolean_data_type>(*in.result().ty())) {
        if (!std::holds_alternative<bool>(in.value())) {
            throw compilation_error(in.loc(), status::internal_compiler_error);
        }
        declare(in.result(), unique_.bool_constant(std::get<bool>(in.value())));
    } else if (auto st = dyn_cast<scalar_data_type>(in.result().ty()); st) {
        auto cst = make_constant(unique_, st->ty(), in.value());
        if (cst == nullptr) {
            throw compilation_error(in.loc(), status::internal_compiler_error);
        }
        declare(in.result(), cst);
    } else if (auto ct = dyn_cast<coopmatrix_data_type>(in.result().ty()); ct) {
        declare(in.result(), matrix_impl().constant(in));
    } else {
        throw compilation_error(in.loc(), status::ir_expected_coopmatrix_or_scalar);
    }
}

void inst_converter::operator()(cooperative_matrix_extract_inst in) {
    declare(in.result(), matrix_impl().extract(in, val(in.mat())));
}
void inst_converter::operator()(cooperative_matrix_insert_inst in) {
    declare(in.result(), matrix_impl().insert(in, val(in.val()), val(in.mat())));
}

void inst_converter::operator()(cooperative_matrix_load_inst in) {
    auto odv = get_dope_vector(in.operand());
    if (!odv) {
        throw compilation_error(in.loc(), status::spirv_missing_dope_vector);
    }
    declare(in.result(),
            matrix_impl().load(in, *odv, val(in.operand()), val(in.pos0()), val(in.pos1())));
}

void inst_converter::operator()(cooperative_matrix_mul_add_inst in) {
    declare(in.result(), matrix_impl().mul_add(in, val(in.a()), val(in.b()), val(in.c())));
}
void inst_converter::operator()(cooperative_matrix_prefetch_inst in) {
    auto odv = get_dope_vector(in.operand());
    if (!odv) {
        throw compilation_error(in.loc(), status::spirv_missing_dope_vector);
    }
    matrix_impl().prefetch(in, *odv, val(in.operand()), val(in.pos0()), val(in.pos1()));
}
void inst_converter::operator()(cooperative_matrix_reduce_inst in) {
    declare(in.result(), matrix_impl().reduce(in, val(in.a())));
}
void inst_converter::operator()(cooperative_matrix_scale_inst in) {
    declare(in.result(), matrix_impl().scale(in, val(in.a()), val(in.b())));
}
void inst_converter::operator()(cooperative_matrix_store_inst in) {
    auto odv = get_dope_vector(in.operand());
    if (!odv) {
        throw compilation_error(in.loc(), status::spirv_missing_dope_vector);
    }
    matrix_impl().store(in, *odv, val(in.val()), val(in.operand()), val(in.pos0()), val(in.pos1()));
}

void inst_converter::operator()(expand_inst in) {
    auto spv_index_ty = unique_.scalar_ty(scalar_type::index);

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
    declare(in.result(), val(in.operand()));

    auto rdv = make_dope_vector(in.result());

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

void inst_converter::operator()(for_inst in) {
    auto header_label_op = std::make_unique<OpLabel>();
    auto body_label_op = std::make_unique<OpLabel>();
    auto continue_label_op = std::make_unique<OpLabel>();
    auto merge_label_op = std::make_unique<OpLabel>();
    auto header_label = header_label_op.get();
    auto body_label = body_label_op.get();
    auto continue_label = continue_label_op.get();
    auto merge_label = merge_label_op.get();

    auto entry_label = get_last_label(*mod_);
    if (!entry_label) {
        throw compilation_error(in.loc(), status::internal_compiler_error);
    }

    mod_->add<OpBranch>(header_label);

    // Header block
    auto spv_bool_ty = unique_.bool_ty();
    auto spv_loop_var_ty = spv_ty(in.loop_var().ty());
    mod_->insts(section::function).push_back(header_label_op.release());
    // nullptr needs to be replaced by the loop var update once it is defined
    auto loop_var_phi = mod_->add<OpPhi>(
        spv_loop_var_ty, std::vector<PairIdRefIdRef>{PairIdRefIdRef{val(in.from()), entry_label},
                                                     PairIdRefIdRef{nullptr, continue_label}});
    declare(in.loop_var(), loop_var_phi);
    auto const make_iter_arg_phi = [&]() -> std::vector<OpPhi *> {
        auto phis = std::vector<OpPhi *>{};
        auto iter_init = in.iter_init();
        phis.reserve(iter_init.size());
        for (std::int64_t i = 0; i < iter_init.size(); ++i) {
            auto ty = spv_ty(in.iter_arg(i).ty());
            phis.emplace_back(mod_->add<OpPhi>(
                ty, std::vector<PairIdRefIdRef>{PairIdRefIdRef{val(iter_init[i]), entry_label},
                                                PairIdRefIdRef{nullptr, continue_label}}));
            declare(in.iter_arg(i), phis.back());
        }
        return phis;
    };
    auto iter_arg_phis = make_iter_arg_phi();

    auto condition = mod_->add<OpSLessThan>(spv_bool_ty, loop_var_phi, val(in.to()));
    auto loop_control = [&]() -> std::pair<LoopControl, std::optional<LoopControlAttr>> {
        auto unroll = get_attr(in.get().attr(), "unroll");
        if (unroll) {
            auto ba = dyn_cast<boolean_attr>(unroll);
            if (ba) {
                return {ba->value() ? LoopControl::Unroll : LoopControl::DontUnroll, std::nullopt};
            }
            auto ia = dyn_cast<integer_attr>(unroll);
            if (ia) {
                return {LoopControl::PartialCount, ia->value()};
            }
            throw status::ir_expected_boolean_attribute;
        }
        return {LoopControl::None, std::nullopt};
    }();
    mod_->add<OpLoopMerge>(merge_label, continue_label, loop_control.first, loop_control.second);
    mod_->add<OpBranchConditional>(condition, body_label, merge_label,
                                   std::vector<LiteralInteger>{});

    // Body block
    mod_->insts(section::function).push_back(body_label_op.release());

    auto results = in.results();
    auto yielded_for = run_on_region_with_yield(in.body(), results.size());
    // Update phis with yielded values
    for (std::int64_t i = 0; i < results.size(); ++i) {
        iter_arg_phis[i]->op0().back().first = yielded_for[i];
    }

    mod_->add<OpBranch>(continue_label);

    // Continue block
    mod_->insts(section::function).push_back(continue_label_op.release());
    auto step = [&]() -> spv_inst * {
        if (in.has_step()) {
            return val(in.step());
        }
        return make_constant(unique_, get_scalar_type(in.loop_var()), std::int64_t{1});
    }();
    auto loop_var_update = mod_->add<OpIAdd>(spv_loop_var_ty, loop_var_phi, step);
    loop_var_phi->op0().back().first = loop_var_update;
    mod_->add<OpBranch>(header_label);

    // Merge block
    mod_->insts(section::function).push_back(merge_label_op.release());

    auto const set_results = [&] {
        for (std::int64_t i = 0; i < results.size(); ++i) {
            declare(results[i], val(in.iter_arg(i)));
        }
    };
    set_results();
}

void inst_converter::operator()(fuse_inst in) {
    auto spv_index_ty = unique_.scalar_ty(scalar_type::index);

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
    declare(in.result(), val(in.operand()));

    auto rdv = make_dope_vector(in.result());

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

void inst_converter::operator()(if_inst in) {
    auto then_label = std::make_unique<OpLabel>();
    auto otherwise_label = std::make_unique<OpLabel>();
    auto merge_label = std::make_unique<OpLabel>();

    auto conditionv = val(in.condition());
    mod_->add<OpSelectionMerge>(merge_label.get(), SelectionControl::None);
    mod_->add<OpBranchConditional>(conditionv, then_label.get(), otherwise_label.get(),
                                   std::vector<LiteralInteger>{});
    mod_->insts(section::function).push_back(then_label.release());
    auto results = in.results();
    auto yielded_then = run_on_region_with_yield(in.then(), results.size());
    mod_->add<OpBranch>(merge_label.get());
    auto then_last_label = get_last_label(*mod_);
    if (!then_last_label) {
        throw compilation_error(in.loc(), status::internal_compiler_error);
    }
    mod_->insts(section::function).push_back(otherwise_label.release());
    auto yielded_otherwise = run_on_region_with_yield(in.otherwise(), results.size());
    mod_->add<OpBranch>(merge_label.get());
    auto otherwise_last_label = get_last_label(*mod_);
    if (!otherwise_last_label) {
        throw compilation_error(in.loc(), status::internal_compiler_error);
    }

    mod_->insts(section::function).push_back(merge_label.release());

    std::int64_t val_no = 0;
    for (std::int64_t i = 0; i < results.size(); ++i) {
        auto ty = spv_ty(results[i].ty());
        auto phi_inst = mod_->add<OpPhi>(
            ty, std::vector<PairIdRefIdRef>{
                    PairIdRefIdRef{yielded_then[val_no], then_last_label},
                    PairIdRefIdRef{yielded_otherwise[val_no], otherwise_last_label}});
        ++val_no;
        declare(results[i], phi_inst);
    }
}

void inst_converter::operator()(lifetime_stop_inst) {}

void inst_converter::operator()(load_inst in) {
    auto spv_index_ty = unique_.scalar_ty(scalar_type::index);
    auto spv_pointer_index_ty =
        unique_.pointer_ty(StorageClass::CrossWorkgroup, spv_index_ty, alignment(scalar_type::i64));
    auto spv_pointer_ty = spv_ty(in.operand().ty());
    auto spv_result_ty = spv_ty(in.result().ty());
    auto dv = get_dope_vector(in.operand());
    if (!dv) {
        throw compilation_error(in.loc(), status::spirv_missing_dope_vector);
    }

    if (auto group_ty = dyn_cast<group_data_type>(in.operand().ty()); group_ty) {
        auto offset = mod_->add<OpIAdd>(spv_index_ty, dv->offset(), val(in.index_list()[0]));
        auto pointer = mod_->add<OpInBoundsPtrAccessChain>(spv_pointer_ty, val(in.operand()),
                                                           offset, std::vector<spv_inst *>{});
        declare(in.result(), mod_->add<OpLoad>(spv_result_ty, pointer));
        auto rdv = make_dope_vector(in.result());

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
        const auto pointer = [&](spv_inst *additional_offset0 = nullptr) -> spv_inst * {
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
            if (additional_offset0) {
                offset = mod_->add<OpIAdd>(spv_index_ty, offset, additional_offset0);
            }
            return mod_->add<OpInBoundsPtrAccessChain>(spv_pointer_ty, val(in.operand()), offset,
                                                       std::vector<spv_inst *>{});
        };
        declare(in.result(), mod_->add<OpLoad>(spv_result_ty, pointer()));
    } else {
        throw compilation_error(in.loc(), status::ir_expected_memref_or_group);
    }
}

void inst_converter::operator()(math_unary_inst in) {
    auto const make_float = [&](math_unary op, spv_inst *ty, spv_inst *a) -> spv_inst * {
        auto const make_ext_inst = [&](OpenCLEntrypoint ep) {
            return mod_->add<OpExtInst>(ty, unique_.opencl_ext(), static_cast<std::int32_t>(ep),
                                        std::vector<IdRef>{a});
        };
        switch (op) {
        case math_unary::cos:
            return make_ext_inst(OpenCLEntrypoint::cos);
        case math_unary::sin:
            return make_ext_inst(OpenCLEntrypoint::sin);
        case math_unary::exp:
            return make_ext_inst(OpenCLEntrypoint::exp);
        case math_unary::exp2:
            return make_ext_inst(OpenCLEntrypoint::exp2);
        case math_unary::native_cos:
            return make_ext_inst(OpenCLEntrypoint::native_cos);
        case math_unary::native_sin:
            return make_ext_inst(OpenCLEntrypoint::native_sin);
        case math_unary::native_exp:
            return make_ext_inst(OpenCLEntrypoint::native_exp);
        case math_unary::native_exp2:
            return make_ext_inst(OpenCLEntrypoint::native_exp2);
        default:
            throw compilation_error(in.loc(), status::internal_compiler_error);
        }
    };
    auto const make_complex = [&](math_unary op, scalar_type sty, spv_inst *ty, spv_inst *a,
                                  LiteralContextDependentNumber log2) -> spv_inst * {
        auto spv_float_ty = unique_.scalar_ty(component_type(sty));
        auto const make_complex_exp = [&](auto exp_ep, auto cos_ep, auto sin_ep,
                                          spv_inst *im_scale = nullptr) {
            auto a0 =
                mod_->add<OpCompositeExtract>(spv_float_ty, a, std::vector<LiteralInteger>{0});
            spv_inst *a1 =
                mod_->add<OpCompositeExtract>(spv_float_ty, a, std::vector<LiteralInteger>{1});
            if (im_scale) {
                a1 = mod_->add<OpFMul>(spv_float_ty, a1, im_scale);
            }
            auto e =
                mod_->add<OpExtInst>(spv_float_ty, unique_.opencl_ext(),
                                     static_cast<std::int32_t>(exp_ep), std::vector<IdRef>{a0});
            auto c =
                mod_->add<OpExtInst>(spv_float_ty, unique_.opencl_ext(),
                                     static_cast<std::int32_t>(cos_ep), std::vector<IdRef>{a1});
            auto s =
                mod_->add<OpExtInst>(spv_float_ty, unique_.opencl_ext(),
                                     static_cast<std::int32_t>(sin_ep), std::vector<IdRef>{a1});
            auto r = mod_->add<OpFMul>(spv_float_ty, e, c);
            auto i = mod_->add<OpFMul>(spv_float_ty, e, s);
            auto dummy = mod_->add<OpUndef>(ty);
            auto result =
                mod_->add<OpCompositeInsert>(ty, r, dummy, std::vector<LiteralInteger>{0});
            return mod_->add<OpCompositeInsert>(ty, i, result, std::vector<LiteralInteger>{1});
        };
        switch (op) {
        case math_unary::exp:
            return make_complex_exp(OpenCLEntrypoint::exp, OpenCLEntrypoint::cos,
                                    OpenCLEntrypoint::sin);
        case math_unary::exp2:
            return make_complex_exp(OpenCLEntrypoint::exp2, OpenCLEntrypoint::cos,
                                    OpenCLEntrypoint::sin, unique_.constant(log2));
        case math_unary::native_exp:
            return make_complex_exp(OpenCLEntrypoint::native_exp, OpenCLEntrypoint::native_cos,
                                    OpenCLEntrypoint::native_sin);
        case math_unary::native_exp2:
            return make_complex_exp(OpenCLEntrypoint::native_exp2, OpenCLEntrypoint::native_cos,
                                    OpenCLEntrypoint::native_sin, unique_.constant(log2));
        default:
            throw compilation_error(in.loc(), status::internal_compiler_error);
        }
    };
    auto const make = [&](scalar_type sty, math_unary op, spv_inst *ty, spv_inst *a) -> spv_inst * {
        switch (sty) {
        case scalar_type::bf16: {
            auto float_ty = unique_.scalar_ty(scalar_type::f32);
            auto af = mod_->add<OpConvertBF16ToFINTEL>(float_ty, a);
            auto op_af = make_float(op, float_ty, af);
            return mod_->add<OpConvertFToBF16INTEL>(ty, op_af);
        }
        case scalar_type::f16:
        case scalar_type::f32:
        case scalar_type::f64:
            return make_float(op, ty, a);
        case scalar_type::c32:
            return make_complex(op, sty, ty, a, std::log(2.0f));
        case scalar_type::c64:
            return make_complex(op, sty, ty, a, std::log(2.0));
        default:
            throw compilation_error(in.loc(), status::internal_compiler_error);
        }
    };

    auto sty = get_scalar_type(in.a());
    auto ty = spv_ty(in.result().ty());
    auto av = val(in.a());
    declare(in.result(), make(sty, in.operation(), ty, av));
}

void inst_converter::operator()(parallel_inst in) { run_on_region(in.body()); }

void inst_converter::operator()(size_inst in) {
    auto dv = get_dope_vector(in.operand());
    if (!dv) {
        throw compilation_error(in.loc(), status::spirv_missing_dope_vector);
    }

    const auto shape = ::tinytc::visit(
        overloaded{[&](group_data_type const &) -> spv_inst * { return dv->size(); },
                   [&](memref_data_type const &) -> spv_inst * { return dv->shape(in.mode()); },
                   [&](auto const &) -> spv_inst * {
                       throw compilation_error(in.loc(), status::ir_expected_memref_or_group);
                   }},
        *in.operand().ty());
    declare(in.result(), shape);
}

void inst_converter::operator()(subgroup_broadcast_inst in) {
    auto broadcast_scope = unique_.constant(static_cast<std::int32_t>(Scope::Subgroup));
    auto ty = spv_ty(in.result().ty());
    auto av = val(in.a());
    auto idxv = val(in.idx());
    declare(in.result(), mod_->add<OpGroupBroadcast>(ty, broadcast_scope, av, idxv));
}

void inst_converter::operator()(subgroup_operation_inst in) {
    auto sty = get_scalar_type(in.a());
    declare(in.result(),
            make_subgroup_op(unique_, sty, in.arith(), in.operation(), val(in.a()), in.loc()));
}

void inst_converter::operator()(store_inst in) {
    auto spv_index_ty = unique_.scalar_ty(scalar_type::index);
    auto spv_pointer_ty = spv_ty(in.operand().ty());
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

        make_store(unique_, in.flag(), memref_ty->element_ty(), memref_ty->addrspace(), pointer(),
                   val(in.val()), in.loc());
    } else {
        throw compilation_error(in.loc(), status::ir_expected_memref);
    }
}

void inst_converter::operator()(subview_inst in) {
    auto spv_index_ty = unique_.scalar_ty(scalar_type::index);
    auto spv_result_ty = spv_ty(in.result().ty());

    auto shape_out = std::vector<spv_inst *>{};
    auto stride_out = std::vector<spv_inst *>{};
    auto const make_offset_and_shape_stride = [&] {
        auto mt = get_memref_type(in.operand());
        auto dv = get_dope_vector(in.operand());
        if (!dv) {
            throw compilation_error(in.loc(), status::spirv_missing_dope_vector);
        }

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
            auto tmp = mod_->add<OpIMul>(spv_index_ty, offset_inst(), dv->stride(i));
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
                stride_out.emplace_back(dv->stride(i));
            }
        }
        return offset_acc;
    };

    auto offset = make_offset_and_shape_stride();
    declare(in.result(), mod_->add<OpInBoundsPtrAccessChain>(spv_result_ty, val(in.operand()),
                                                             offset, std::vector<spv_inst *>{}));

    auto rdv = make_dope_vector(in.result());

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

void inst_converter::operator()(yield_inst in) {
    if (yielded_vals_.empty()) {
        throw compilation_error(in.loc(), status::ir_unexpected_yield);
    }

    auto &top = yielded_vals_.top();
    if (static_cast<std::int64_t>(top.size()) != in.yielded_vals().size()) {
        throw compilation_error(in.loc(), status::ir_yield_mismatch);
    }

    std::int64_t i = 0;
    for (auto &op : in.yielded_vals()) {
        top[i++] = val(op);
    }
}

void inst_converter::run_on_region(tinytc_region &reg) {
    for (auto &i : reg) {
        visit(*this, i);
    }
}

auto inst_converter::run_on_region_with_yield(tinytc_region &reg, std::int64_t num_results)
    -> std::vector<spv_inst *> {
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

void inst_converter::run_on_function(tinytc_func &fn) {
    try {
        core_cfg_ = info_->get_core_config(fn.subgroup_size());
    } catch (std::out_of_range const &e) {
        throw compilation_error(fn.loc(), status::unsupported_subgroup_size);
    }

    auto vars_used_by_function = std::vector<spv_inst *>{};

    // Stack
    auto const make_stack = [&] {
        const auto high_water_mark = stack_high_water_mark{}.run_on_function(fn);
        if (high_water_mark > 0) {
            auto stack_element_ty = unique_.scalar_ty(scalar_type::i8);
            auto stack_array_ty = unique_.array_ty(stack_element_ty, high_water_mark);
            auto stack_ptr_ty = unique_.pointer_ty(StorageClass::Workgroup, stack_array_ty, 0);
            stack_ = mod_->add_to<OpVariable>(section::type_const_var, stack_ptr_ty,
                                              StorageClass::Workgroup);
            const std::int32_t alignment = info_->alignment();
            mod_->add_to<OpDecorate>(section::decoration, stack_, Decoration::Alignment,
                                     DecorationAttr{alignment});
            vars_used_by_function.emplace_back(stack_);
        } else {
            stack_ = nullptr;
        }
    };
    make_stack();

    // Function type
    auto fun_ty = unique_.function_ty(unique_.void_ty(), [&] {
        auto params = std::vector<spv_inst *>{};
        params.reserve(fn.num_params());
        for (auto const &p : fn.params()) {
            params.emplace_back(spv_ty(p.ty()));
            auto dv = make_dope_vector(p);
            if (dv) {
                for (std::int64_t i = 0; i < dv->num_dynamic(); ++i) {
                    params.emplace_back(dv->ty());
                }
                if (is_dynamic_value(dv->static_size())) {
                    params.emplace_back(dv->size_ty());
                }
                if (is_dynamic_value(dv->static_offset())) {
                    params.emplace_back(dv->offset_ty());
                }
            }
        }
        return params;
    }());

    // Function
    auto const subgroup_size = fn.subgroup_size();
    auto const work_group_size = fn.work_group_size();
    tiling_[0] = work_group_size[0] / subgroup_size;
    tiling_[1] = work_group_size[1];

    matrix_impl_ = [&]() -> std::unique_ptr<coopmatrix_impl> {
        const auto gcd = gcd_analysis{info_->alignment()}.run_on_function(fn);
        if (info_->matrix().have_dpas()) {
            return std::make_unique<coopmatrix_impl_dpas>(unique_, core_cfg_, std::move(gcd));
        } else if (info_->have_spirv_feature(spirv_feature::subgroup_buffer_block_io)) {
            return std::make_unique<coopmatrix_impl_block>(unique_, core_cfg_, std::move(gcd));
        }
        return std::make_unique<coopmatrix_impl>(unique_, core_cfg_, std::move(gcd));
    }();

    auto void_ty = unique_.void_ty();
    auto fun = mod_->add<OpFunction>(void_ty, FunctionControl::None, fun_ty);
    for (auto const &p : fn.params()) {
        declare(p, mod_->add<OpFunctionParameter>(spv_ty(p.ty())));
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
            if (dv->size_ty()) {
                dv->size(make_dope_par(dv->size_ty(), dv->static_size()));
            }
            if (dv->offset_ty()) {
                dv->offset(make_dope_par(dv->offset_ty(), dv->static_offset()));
            }
        }
    }

    mod_->add<OpLabel>();
    auto func_begin = --mod_->insts(section::function).end();
    run_on_region(fn.body());

    auto func_end = mod_->insts(section::function).end();
    for (auto it = func_begin; it != func_end; ++it) {
        if (auto ld = dyn_cast<OpLoad>(it.get()); ld && isa<OpVariable>(*ld->op0())) {
            if (std::find(vars_used_by_function.begin(), vars_used_by_function.end(), ld->op0()) ==
                vars_used_by_function.end()) {
                vars_used_by_function.push_back(ld->op0());
            }
        }
    }

    mod_->add<OpReturn>();
    mod_->add<OpFunctionEnd>();

    tiling_ = {};

    // Entry point
    mod_->add_to<OpEntryPoint>(section::entry_point, ExecutionModel::Kernel, fun,
                               std::string{fn.name()}, std::move(vars_used_by_function));

    // Execution mode
    mod_->add_to<OpExecutionMode>(
        section::execution_mode, fun, ExecutionMode::LocalSize,
        ExecutionModeAttr{std::array<std::int32_t, 3u>{work_group_size[0], work_group_size[1], 1}});
    mod_->add_to<OpExecutionMode>(section::execution_mode, fun, ExecutionMode::SubgroupSize,
                                  ExecutionModeAttr{subgroup_size});
}

} // namespace tinytc::spv

