// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "spv/uniquifier.hpp"
#include "compiler_context.hpp"
#include "node/data_type_node.hpp"
#include "spv/instructions.hpp"
#include "spv/opencl.std.hpp"
#include "support/fnv1a.hpp"
#include "support/fnv1a_array_view.hpp"
#include "support/visit.hpp"
#include "tinytc/types.hpp"

#include <algorithm>
#include <optional>
#include <utility>
#include <vector>

namespace tinytc::spv {

uniquifier::uniquifier(tinytc_compiler_context_t ctx, mod &m) : ctx_(ctx), mod_(&m) {}

auto uniquifier::bool2_ty() -> spv_inst * {
    if (!bool2_ty_) {
        auto bool_ty = spv_ty(*boolean_data_type::get(ctx_));
        bool2_ty_ = mod_->add_to<OpTypeVector>(section::type_const_var, bool_ty, 2);
    }
    return bool2_ty_;
}

auto uniquifier::bool_constant(bool b) -> spv_inst * {
    if (b) {
        if (!bool_true_) {
            auto bool_ty = spv_ty(*boolean_data_type::get(ctx_));
            bool_true_ = mod_->add_to<OpConstantTrue>(section::type_const_var, bool_ty);
        }
        return bool_true_;
    }
    if (!bool_false_) {
        auto bool_ty = spv_ty(*boolean_data_type::get(ctx_));
        bool_false_ = mod_->add_to<OpConstantFalse>(section::type_const_var, bool_ty);
    }
    return bool_false_;
}
// inline auto builtin(BuiltIn b) -> spv_inst* {
// auto it = builtin_.find(b);
// if (it == builtin_.end()) {
// auto var = add_to<section::type_const_var,OpVariable>();
// add_to<OpDecorate>(section::decoration,);
// auto i32_ty = visit( *scalar_data_type::get(ctx_, scalar_type::i32));
// auto cst_inst = add_to<OpConstant>(section::type_const_var,
// i32_ty, LiteralContextDependentNumber{cst});
// i32_cst_[cst] = cst_inst;
// return cst_inst;
//}
// return it->second;
//}

void uniquifier::capability(Capability cap) {
    if (!capabilities_.contains(cap)) {
        mod_->add_to<OpCapability>(section::capability, cap);
        capabilities_.insert(cap);
    }
}

auto uniquifier::i32_constant(std::int32_t cst) -> spv_inst * {
    auto it = i32_cst_.find(cst);
    if (it == i32_cst_.end()) {
        auto i32_ty = spv_ty(*scalar_data_type::get(ctx_, scalar_type::i32));
        auto cst_inst = mod_->add_to<OpConstant>(section::type_const_var, i32_ty,
                                                 LiteralContextDependentNumber{cst});
        i32_cst_[cst] = cst_inst;
        return cst_inst;
    }
    return it->second;
}

auto uniquifier::null_constant(spv_inst *spv_ty) -> spv_inst * {
    auto it = null_cst_.find(spv_ty);
    if (it == null_cst_.end()) {
        auto in = mod_->add_to<OpConstantNull>(section::type_const_var, spv_ty);
        null_cst_[spv_ty] = in;
        return in;
    }
    return it->second;
}

auto uniquifier::opencl_ext() -> spv_inst * {
    if (opencl_ext_ == nullptr) {
        opencl_ext_ = mod_->add_to<OpExtInstImport>(section::ext_inst, OpenCLExt);
    }
    return opencl_ext_;
}

auto uniquifier::spv_function_ty(array_view<spv_inst *> params) -> spv_inst * {
    const auto map_key = fnv1a_step(fnv1a0(), params);
    auto range = spv_function_tys_.equal_range(map_key);
    for (auto it = range.first; it != range.second; ++it) {
        if (std::equal(params.begin(), params.end(), it->second->op1().begin(),
                       it->second->op1().end())) {
            return it->second;
        }
    }
    auto void_ty = spv_ty(*void_data_type::get(ctx_));
    return spv_function_tys_
        .emplace(map_key,
                 mod_->add_to<OpTypeFunction>(section::type_const_var, void_ty, std::move(params)))
        ->second;
}

auto uniquifier::spv_ty(data_type_node const &ty) -> spv_inst * {
    auto it = spv_tys_.find(&ty);
    if (it == spv_tys_.end()) {
        auto spv_ty_inst = visit(
            overloaded{
                [&](void_data_type const &) -> spv_inst * {
                    return mod_->add_to<OpTypeVoid>(section::type_const_var);
                },
                [&](boolean_data_type const &) -> spv_inst * {
                    return mod_->add_to<OpTypeBool>(section::type_const_var);
                },
                [&](scalar_data_type const &ty) -> spv_inst * {
                    switch (ty.ty()) {
                    case scalar_type::i8:
                        capability(Capability::Int8);
                        return mod_->add_to<OpTypeInt>(section::type_const_var, 8, 0);
                    case scalar_type::i16:
                        capability(Capability::Int16);
                        return mod_->add_to<OpTypeInt>(section::type_const_var, 16, 0);
                    case scalar_type::i32:
                        return mod_->add_to<OpTypeInt>(section::type_const_var, 32, 0);
                    case scalar_type::i64:
                        capability(Capability::Int64);
                        return mod_->add_to<OpTypeInt>(section::type_const_var, 64, 0);
                    case scalar_type::index: {
                        const auto sz = size(ty.ty());
                        if (sz == 8) {
                            capability(Capability::Int64);
                        }
                        return mod_->add_to<OpTypeInt>(section::type_const_var, sz * 8, 0);
                    }
                    case scalar_type::f32:
                    case scalar_type::f64:
                        capability(Capability::Float64);
                        return mod_->add_to<OpTypeFloat>(section::type_const_var, size(ty.ty()) * 8,
                                                         std::nullopt);
                    case scalar_type::c32: {
                        auto float_ty = spv_ty(*scalar_data_type::get(ctx_, scalar_type::f32));
                        return mod_->add_to<OpTypeVector>(section::type_const_var, float_ty, 2);
                    }
                    case scalar_type::c64: {
                        auto float_ty = spv_ty(*scalar_data_type::get(ctx_, scalar_type::f64));
                        return mod_->add_to<OpTypeVector>(section::type_const_var, float_ty, 2);
                    }
                    }
                    throw status::internal_compiler_error;
                },
                [&](coopmatrix_data_type const &ty) -> spv_inst * { return spv_ty(*ty.ty()); },
                [](auto const &) -> spv_inst * {
                    // @todo
                    throw status::not_implemented;
                }},
            ty);
        spv_tys_[&ty] = spv_ty_inst;
        return spv_ty_inst;
    }
    return it->second;
}

} // namespace tinytc::spv

