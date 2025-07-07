// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "spv/converter.hpp"
#include "analysis/gcd.hpp"
#include "analysis/stack.hpp"
#include "codegen_tools.hpp"
#include "compiler_context.hpp"
#include "converter_aux.hpp"
#include "error.hpp"
#include "matrix_ext_info.hpp"
#include "node/attr.hpp"
#include "node/func.hpp"
#include "node/inst.hpp"
#include "node/inst_view.hpp"
#include "node/prog.hpp"
#include "node/region.hpp"
#include "node/type.hpp"
#include "node/value.hpp"
#include "node/visit.hpp"
#include "spv/coopmatrix_impl_block.hpp"
#include "spv/coopmatrix_impl_dpas.hpp"
#include "spv/enums.hpp"
#include "spv/instructions.hpp"
#include "spv/module.hpp"
#include "spv/pass/capex.hpp"
#include "spv/uniquifier.hpp"
#include "spv/visit.hpp"
#include "tinytc/core.hpp"
#include "tinytc/types.hpp"
#include "util/casting.hpp"
#include "util/ilist.hpp"
#include "util/ilist_base.hpp"
#include "util/iterator.hpp"
#include "util/overloaded.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace tinytc::spv {

auto convert_prog_to_spirv(tinytc_prog &p, tinytc_core_info const &info)
    -> shared_handle<tinytc_spv_mod_t> {
    auto m = shared_handle{
        std::make_unique<tinytc_spv_mod>(p.share_context(), info.core_features()).release()};

    auto conv = inst_converter{*m, info};

    if (m->context()->index_bit_width() == 64) {
        m->add_to<OpMemoryModel>(section::memory_model, AddressingModel::Physical64,
                                 MemoryModel::OpenCL);
    } else {
        m->add_to<OpMemoryModel>(section::memory_model, AddressingModel::Physical32,
                                 MemoryModel::OpenCL);
    }

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

auto inst_converter::spv_ty(tinytc_type_t ty) -> spv_inst * {
    if (auto ct = dyn_cast<coopmatrix_type>(ty); ct) {
        return matrix_impl().spv_ty(ct);
    }
    return get_spv_ty_non_coopmatrix(unique_, ty);
}

auto inst_converter::make_dope_vector(tinytc_value const &v) -> dope_vector * {
    if (dope_vec_.contains(&v)) {
        throw compilation_error(v.loc(), status::internal_compiler_error);
    }

    auto spv_index_ty = get_spv_index_ty(unique_, v.context());
    return ::tinytc::visit(
        overloaded{[&](memref_type const &mr) -> dope_vector * {
                       return &(dope_vec_[&v] = dope_vector{spv_index_ty, mr.shape(), mr.stride()});
                   },
                   [&](group_type const &g) -> dope_vector * {
                       if (auto mt = dyn_cast<memref_type>(g.element_ty()); mt) {
                           auto pointer_ty = get_spv_pointer_index_ty(unique_, g.context());
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

auto inst_converter::get_pointer(memory_write_inst in) -> spv_inst * {
    auto dv = get_dope_vector(in.operand());
    if (!dv) {
        throw compilation_error(in.loc(), status::spirv_missing_dope_vector);
    }

    if (auto memref_ty = dyn_cast<memref_type>(in.operand().ty()); memref_ty) {
        auto spv_index_ty = get_spv_index_ty(unique_, memref_ty->context());
        auto spv_pointer_ty = spv_ty(memref_ty);

        if (memref_ty->dim() == 0) {
            return val(in.operand());
        }

        auto idx0 = val(in.index_list()[0]);
        auto offset =
            memref_ty->stride(0) != 1 ? mod_->add<OpIMul>(spv_index_ty, idx0, dv->stride(0)) : idx0;
        for (std::int64_t i = 1; i < memref_ty->dim(); ++i) {
            auto tmp = mod_->add<OpIMul>(spv_index_ty, val(in.index_list()[i]), dv->stride(i));
            offset = mod_->add<OpIAdd>(spv_index_ty, offset, tmp);
        }

        return mod_->add<OpInBoundsPtrAccessChain>(spv_pointer_ty, val(in.operand()), offset,
                                                   std::vector<spv_inst *>{});
    }
    throw compilation_error(in.loc(), status::ir_expected_memref);
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

    auto stack_element_ty = unique_.int_ty(8);
    auto stack_ptr_ty = unique_.pointer_ty(StorageClass::Workgroup, stack_element_ty, 1);
    auto stack_ptr = mod_->add<OpInBoundsAccessChain>(
        stack_ptr_ty, stack_, std::vector<IdRef>{unique_.constant(in.stack_ptr())});

    auto memref_ptr_ty = get_spv_ty(unique_, mt);
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
    if (isa<coopmatrix_type>(*in.result().ty())) {
        auto av = val(in.a());
        auto bv = val(in.b());
        declare(in.result(), matrix_impl().arith(in, av, bv));
    } else {
        auto av = val(in.a());
        auto bv = val(in.b());
        auto ty = in.result().ty();
        auto ik = in.get().type_id();
        declare(in.result(), make_binary_op(unique_, ty, ik, av, bv, in.loc()));
    }
}

void inst_converter::operator()(arith_unary_inst in) {
    if (isa<coopmatrix_type>(*in.a().ty())) {
        auto av = val(in.a());
        declare(in.result(), matrix_impl().arith_unary(in, av));
    } else {
        auto av = val(in.a());
        auto ty = in.a().ty();
        auto ik = in.get().type_id();
        declare(in.result(), make_unary_op(unique_, ty, ik, av, in.loc()));
    }
}

void inst_converter::operator()(atomic_store_inst in) {
    auto ot = get_memref_type(in.operand());
    auto pointer = get_pointer(in);
    make_atomic_store(unique_, in.scope(), in.semantics(), ot->element_ty(), ot->addrspace(),
                      pointer, val(in.val()), in.loc());
}

void inst_converter::operator()(atomic_update_inst in) {
    auto ot = get_memref_type(in.operand());
    auto pointer = get_pointer(in);
    auto const make = [&]<typename SpvIOp, typename SpvFOp>() {
        declare(in.result(), make_atomic_update<SpvIOp, SpvFOp>(unique_, in.scope(), in.semantics(),
                                                                ot->element_ty(), ot->addrspace(),
                                                                pointer, val(in.val()), in.loc()));
    };
    switch (in.get().type_id()) {
    case IK::IK_atomic_add:
        make.template operator()<OpAtomicIAdd, OpAtomicFAddEXT>();
        break;
    case IK::IK_atomic_max:
        make.template operator()<OpAtomicSMax, OpAtomicFMaxEXT>();
        break;
    case IK::IK_atomic_min:
        make.template operator()<OpAtomicSMin, OpAtomicFMinEXT>();
        break;
    default:
        throw compilation_error(in.loc(), status::internal_compiler_error);
        break;
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

void inst_converter::operator()(cast_inst in) {
    if (auto ct = dyn_cast<coopmatrix_type>(in.result().ty()); ct) {
        declare(in.result(), matrix_impl().cast(in, val(in.a())));
    } else {
        auto av = val(in.a());
        auto to_ty = in.result().ty();
        auto a_ty = in.a().ty();
        declare(in.result(), make_cast(unique_, to_ty, a_ty, av, in.loc()));
    }
}

void inst_converter::operator()(compare_inst in) {
    auto av = val(in.a());
    auto bv = val(in.b());
    auto tid = in.get().type_id();
    auto a_ty = in.a().ty();
    declare(in.result(), make_compare_op(unique_, a_ty, tid, av, bv, in.loc()));
}

void inst_converter::operator()(constant_inst in) {
    if (auto ct = dyn_cast<coopmatrix_type>(in.result().ty()); ct) {
        declare(in.result(), matrix_impl().constant(in));
    } else {
        auto ty = in.result().ty();
        declare(in.result(), make_constant(unique_, ty, in.value()));
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
    auto spv_index_ty = get_spv_index_ty(unique_, in.operand().context());

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
        return make_constant(unique_, in.loop_var().ty(), std::int64_t{1});
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
    auto spv_index_ty = get_spv_index_ty(unique_, in.operand().context());

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
    auto spv_index_ty = get_spv_index_ty(unique_, in.operand().context());
    auto spv_pointer_index_ty = get_spv_pointer_index_ty(unique_, in.operand().context());
    auto spv_pointer_ty = spv_ty(in.operand().ty());
    auto spv_result_ty = spv_ty(in.result().ty());
    auto dv = get_dope_vector(in.operand());
    if (!dv) {
        throw compilation_error(in.loc(), status::spirv_missing_dope_vector);
    }

    if (auto group_ty = dyn_cast<group_type>(in.operand().ty()); group_ty) {
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
    } else if (auto memref_ty = dyn_cast<memref_type>(in.operand().ty()); memref_ty) {
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
    auto av = val(in.a());
    auto ty = in.result().ty();
    auto ik = in.get().type_id();
    declare(in.result(), make_math_unary_op(unique_, ty, ik, av, in.loc()));
}

void inst_converter::operator()(parallel_inst in) { run_on_region(in.body()); }

void inst_converter::operator()(size_inst in) {
    auto dv = get_dope_vector(in.operand());
    if (!dv) {
        throw compilation_error(in.loc(), status::spirv_missing_dope_vector);
    }

    const auto shape = ::tinytc::visit(
        overloaded{[&](group_type const &) -> spv_inst * { return dv->size(); },
                   [&](memref_type const &) -> spv_inst * { return dv->shape(in.mode()); },
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
    auto a_ty = in.a().ty();
    auto ik = in.get().type_id();
    declare(in.result(), make_subgroup_op(unique_, a_ty, ik, val(in.a()), in.loc()));
}

void inst_converter::operator()(store_inst in) {
    auto pointer = get_pointer(in);
    mod_->add<OpStore>(pointer, val(in.val()));
}

void inst_converter::operator()(subview_inst in) {
    auto spv_index_ty = get_spv_index_ty(unique_, in.operand().context());
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

void inst_converter::operator()(group_id_inst in) {
    auto gid = unique_.load_builtin(BuiltIn::WorkgroupId);
    const std::int32_t mode =
        static_cast<std::int32_t>(in.mode()) - static_cast<std::int32_t>(comp3::x);
    auto rty = spv_ty(in.result().ty());
    declare(in.result(),
            mod_->add<OpCompositeExtract>(rty, gid, std::vector<LiteralInteger>{mode}));
}
void inst_converter::operator()(num_groups_inst in) {
    auto ng = unique_.load_builtin(BuiltIn::NumWorkgroups);
    const std::int32_t mode =
        static_cast<std::int32_t>(in.mode()) - static_cast<std::int32_t>(comp3::x);
    auto rty = spv_ty(in.result().ty());
    declare(in.result(), mod_->add<OpCompositeExtract>(rty, ng, std::vector<LiteralInteger>{mode}));
}
void inst_converter::operator()(num_subgroups_inst in) {
    auto make_constant = [&](comp3 c) -> std::int32_t {
        switch (c) {
        case comp3::x:
            return tiling_.m_tiles();
        case comp3::y:
            return tiling_.n_tiles();
        default:
            break;
        }
        return 1;
    };
    auto cst = make_constant(in.mode());
    declare(in.result(), unique_.constant(cst));
}
void inst_converter::operator()(subgroup_size_inst in) {
    declare(in.result(), unique_.load_builtin(BuiltIn::SubgroupSize));
}
void inst_converter::operator()(subgroup_id_inst in) {
    auto make_value = [&](comp3 c) -> spv_inst * {
        auto rty = spv_ty(in.result().ty());
        const auto m_tiles = unique_.constant(tiling_.m_tiles());
        const auto sgid = unique_.load_builtin(BuiltIn::SubgroupId);
        switch (c) {
        case comp3::x:
            return mod_->add<OpSRem>(rty, sgid, m_tiles);
        case comp3::y:
            return mod_->add<OpSDiv>(rty, sgid, m_tiles);
        default:
            break;
        }
        return unique_.constant(std::int32_t{0});
    };
    declare(in.result(), make_value(in.mode()));
}
void inst_converter::operator()(subgroup_linear_id_inst in) {
    declare(in.result(), unique_.load_builtin(BuiltIn::SubgroupId));
}
void inst_converter::operator()(subgroup_local_id_inst in) {
    declare(in.result(), unique_.load_builtin(BuiltIn::SubgroupLocalInvocationId));
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
            auto stack_element_ty = unique_.int_ty(8);
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

