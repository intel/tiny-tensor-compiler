// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "spv/uniquifier.hpp"
#include "compiler_context.hpp"
#include "node/data_type_node.hpp"
#include "scalar_type.hpp"
#include "spv/instructions.hpp"
#include "spv/opencl.std.hpp"
#include "support/fnv1a_array_view.hpp"
#include "support/visit.hpp"
#include "tinytc/types.hpp"

#include <algorithm>
#include <optional>
#include <vector>

namespace tinytc::spv {

uniquifier::uniquifier(tinytc_compiler_context_t ctx, mod &m) : ctx_(ctx), mod_(&m) {}

auto uniquifier::bool2_ty() -> spv_inst * {
    return lookup(bool2_ty_, [&] {
        auto bool_ty = spv_ty(boolean_data_type::get(ctx_));
        return mod_->add_to<OpTypeVector>(section::type_const_var, bool_ty, 2);
    });
}

auto uniquifier::bool_constant(bool b) -> spv_inst * {
    if (b) {
        return lookup(bool_true_, [&] {
            auto bool_ty = spv_ty(boolean_data_type::get(ctx_));
            return mod_->add_to<OpConstantTrue>(section::type_const_var, bool_ty);
        });
    }
    return lookup(bool_false_, [&] {
        auto bool_ty = spv_ty(boolean_data_type::get(ctx_));
        return mod_->add_to<OpConstantFalse>(section::type_const_var, bool_ty);
    });
}

auto uniquifier::builtin_alignment(BuiltIn b) -> std::int32_t {
    switch (b) {
    case BuiltIn::WorkDim:
    case BuiltIn::SubgroupSize:
    case BuiltIn::SubgroupMaxSize:
    case BuiltIn::NumSubgroups:
    case BuiltIn::NumEnqueuedSubgroups:
    case BuiltIn::SubgroupId:
    case BuiltIn::SubgroupLocalInvocationId:
        return alignment(scalar_type::i32);
    case BuiltIn::GlobalLinearId:
    case BuiltIn::LocalInvocationIndex:
        return alignment(scalar_type::index);
    case BuiltIn::GlobalSize:
    case BuiltIn::GlobalInvocationId:
    case BuiltIn::WorkgroupSize:
    case BuiltIn::EnqueuedWorkgroupSize:
    case BuiltIn::LocalInvocationId:
    case BuiltIn::NumWorkgroups:
    case BuiltIn::WorkgroupId:
    case BuiltIn::GlobalOffset:
        return alignment(scalar_type::index, component_count::v3);
        break;
    default:
        throw status::internal_compiler_error;
    }
}

auto uniquifier::builtin_pointee_ty(BuiltIn b) -> spv_inst * {
    switch (b) {
    case BuiltIn::WorkDim:
    case BuiltIn::SubgroupSize:
    case BuiltIn::SubgroupMaxSize:
    case BuiltIn::NumSubgroups:
    case BuiltIn::NumEnqueuedSubgroups:
    case BuiltIn::SubgroupId:
    case BuiltIn::SubgroupLocalInvocationId:
        return spv_ty(scalar_data_type::get(ctx_, scalar_type::i32));
    case BuiltIn::GlobalLinearId:
    case BuiltIn::LocalInvocationIndex:
        return spv_ty(scalar_data_type::get(ctx_, scalar_type::index));
    case BuiltIn::GlobalSize:
    case BuiltIn::GlobalInvocationId:
    case BuiltIn::WorkgroupSize:
    case BuiltIn::EnqueuedWorkgroupSize:
    case BuiltIn::LocalInvocationId:
    case BuiltIn::NumWorkgroups:
    case BuiltIn::WorkgroupId:
    case BuiltIn::GlobalOffset:
        return index3_ty();
        break;
    default:
        throw status::internal_compiler_error;
    }
}

auto uniquifier::builtin_var(BuiltIn b) -> spv_inst * {
    return lookup(builtin_, b, [&](BuiltIn b) {
        auto pointer_ty = spv_pointer_ty(StorageClass::Input, builtin_pointee_ty(b));
        auto var = mod_->add_to<OpVariable>(section::type_const_var, pointer_ty,
                                            StorageClass::Input, std::nullopt);
        mod_->add_to<OpDecorate>(section::decoration, var, Decoration::Constant);
        mod_->add_to<OpDecorate>(section::decoration, var, Decoration::BuiltIn, b);
        return var;
    });
}

void uniquifier::capability(Capability cap) {
    if (!capabilities_.contains(cap)) {
        mod_->add_to<OpCapability>(section::capability, cap);
        capabilities_.insert(cap);
    }
}

auto uniquifier::i32_constant(std::int32_t cst) -> spv_inst * {
    return lookup(i32_cst_, cst, [&](std::int32_t cst) {
        auto i32_ty = spv_ty(scalar_data_type::get(ctx_, scalar_type::i32));
        return mod_->add_to<OpConstant>(section::type_const_var, i32_ty,
                                        LiteralContextDependentNumber{cst});
    });
}

auto uniquifier::index3_ty() -> spv_inst * {
    return lookup(index3_ty_, [&] {
        auto index_ty = spv_ty(scalar_data_type::get(ctx_, scalar_type::index));
        return mod_->add_to<OpTypeVector>(section::type_const_var, index_ty, 3);
    });
}

auto uniquifier::null_constant(spv_inst *spv_ty) -> spv_inst * {
    return lookup(null_cst_, spv_ty, [&](spv_inst *spv_ty) {
        return mod_->add_to<OpConstantNull>(section::type_const_var, spv_ty);
    });
}

auto uniquifier::opencl_ext() -> spv_inst * {
    return lookup(opencl_ext_,
                  [&] { return mod_->add_to<OpExtInstImport>(section::ext_inst, OpenCLExt); });
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
    auto void_ty = spv_ty(void_data_type::get(ctx_));
    return spv_function_tys_
        .emplace(map_key,
                 mod_->add_to<OpTypeFunction>(section::type_const_var, void_ty, std::move(params)))
        ->second;
}

auto uniquifier::spv_pointer_ty(StorageClass cls, spv_inst *pointee_ty) -> spv_inst * {
    auto key = std::make_pair(cls, pointee_ty);
    return lookup(spv_pointer_tys_, key, [&](std::pair<StorageClass, spv_inst *> const &key) {
        return mod_->add_to<OpTypePointer>(section::type_const_var, key.first, key.second);
    });
}

auto uniquifier::spv_ty(const_tinytc_data_type_t ty) -> spv_inst * {
    return lookup(spv_tys_, ty, [&](const_tinytc_data_type_t ty) {
        return visit(
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
                            return spv_ty(scalar_data_type::get(ctx_, scalar_type::i64));
                        }
                        return spv_ty(scalar_data_type::get(ctx_, scalar_type::i32));
                    }
                    case scalar_type::f32:
                    case scalar_type::f64:
                        capability(Capability::Float64);
                        return mod_->add_to<OpTypeFloat>(section::type_const_var, size(ty.ty()) * 8,
                                                         std::nullopt);
                    case scalar_type::c32: {
                        auto float_ty = spv_ty(scalar_data_type::get(ctx_, scalar_type::f32));
                        return mod_->add_to<OpTypeVector>(section::type_const_var, float_ty, 2);
                    }
                    case scalar_type::c64: {
                        auto float_ty = spv_ty(scalar_data_type::get(ctx_, scalar_type::f64));
                        return mod_->add_to<OpTypeVector>(section::type_const_var, float_ty, 2);
                    }
                    }
                    throw status::internal_compiler_error;
                },
                [&](coopmatrix_data_type const &ty) -> spv_inst * { return spv_ty(ty.ty()); },
                [](auto const &) -> spv_inst * {
                    // @todo
                    throw status::not_implemented;
                }},
            *ty);
    });
}

} // namespace tinytc::spv

