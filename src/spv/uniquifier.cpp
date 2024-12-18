// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "spv/uniquifier.hpp"
#include "matrix_ext_info.hpp"
#include "node/data_type_node.hpp"
#include "scalar_type.hpp"
#include "spv/instructions.hpp"
#include "spv/lut.hpp"
#include "spv/module.hpp"
#include "spv/opencl.std.hpp"
#include "support/fnv1a_array_view.hpp"
#include "support/visit.hpp"
#include "tinytc/types.hpp"

#include <algorithm>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace tinytc::spv {

auto address_space_to_storage_class(address_space as) -> StorageClass {
    return as == address_space::local ? StorageClass::Workgroup : StorageClass::CrossWorkgroup;
}

uniquifier::uniquifier(tinytc_spv_mod &m, matrix_ext_info const &matrix)
    : mod_(&m), matrix_(&matrix) {}

auto uniquifier::asm_target() -> spv_inst * {
    if (!asm_target_) {
        asm_target_ =
            mod_->add_to<OpAsmTargetINTEL>(section::type_const_var, "spirv64-unknown-unknown");
    }
    return asm_target_;
}

auto uniquifier::bool2_ty() -> spv_inst * {
    return spv_vec_ty(spv_ty(boolean_data_type::get(mod_->context())), vector_size::v2);
}

auto uniquifier::bool_constant(bool b) -> spv_inst * {
    if (b) {
        return lookup(bool_true_, [&] {
            auto bool_ty = spv_ty(boolean_data_type::get(mod_->context()));
            return mod_->add_to<OpConstantTrue>(section::type_const_var, bool_ty);
        });
    }
    return lookup(bool_false_, [&] {
        auto bool_ty = spv_ty(boolean_data_type::get(mod_->context()));
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
        return alignment(scalar_type::index, vector_size::v3);
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
        return spv_ty(scalar_data_type::get(mod_->context(), scalar_type::i32));
    case BuiltIn::GlobalLinearId:
    case BuiltIn::LocalInvocationIndex:
        return spv_ty(scalar_data_type::get(mod_->context(), scalar_type::index));
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
        auto pointer_ty =
            spv_pointer_ty(StorageClass::Input, builtin_pointee_ty(b), builtin_alignment(b));
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

auto uniquifier::constant(LiteralContextDependentNumber cst) -> spv_inst * {
    return lookup(cst_map_, cst, [&](LiteralContextDependentNumber const &cst) {
        scalar_type sty = std::visit(
            overloaded{[](auto const &c) { return to_scalar_type_v<std::decay_t<decltype(c)>>; }},
            cst);
        auto ty = spv_ty(scalar_data_type::get(mod_->context(), sty));
        return mod_->add_to<OpConstant>(section::type_const_var, ty, cst);
    });
}

void uniquifier::extension(char const *ext_name) {
    if (!extensions_.contains(ext_name)) {
        mod_->add_to<OpExtension>(section::extension, ext_name);
        extensions_.insert(ext_name);
    }
}

auto uniquifier::index3_ty() -> spv_inst * {
    return spv_vec_ty(spv_ty(scalar_data_type::get(mod_->context(), scalar_type::index)),
                      vector_size::v3);
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

auto uniquifier::spv_array_ty(spv_inst *element_ty, std::int32_t length) -> spv_inst * {
    auto key = std::make_pair(element_ty, length);
    return lookup(spv_array_tys_, key, [&](std::pair<spv_inst *, std::int32_t> const &key) {
        return mod_->add_to<OpTypeArray>(section::type_const_var, key.first, constant(key.second));
    });
}

auto uniquifier::spv_function_ty(spv_inst *return_ty, array_view<spv_inst *> params) -> spv_inst * {
    const auto map_key = fnv1a_step(fnv1a_step(fnv1a0(), return_ty), params);
    auto range = spv_function_tys_.equal_range(map_key);
    for (auto it = range.first; it != range.second; ++it) {
        if (return_ty == it->second->op0() &&
            std::equal(params.begin(), params.end(), it->second->op1().begin(),
                       it->second->op1().end())) {
            return it->second;
        }
    }
    return spv_function_tys_
        .emplace(map_key, mod_->add_to<OpTypeFunction>(section::type_const_var, return_ty,
                                                       std::move(params)))
        ->second;
}

auto uniquifier::spv_pointer_ty(StorageClass cls, spv_inst *pointee_ty,
                                std::int32_t alignment) -> spv_inst * {
    auto key = std::make_tuple(cls, pointee_ty, alignment);
    return lookup(
        spv_pointer_tys_, key, [&](std::tuple<StorageClass, spv_inst *, std::int32_t> const &key) {
            auto pointer_ty = mod_->add_to<OpTypePointer>(section::type_const_var, std::get<0>(key),
                                                          std::get<1>(key));
            if (std::get<2>(key) > 0) {
                mod_->add_to<OpDecorate>(section::decoration, pointer_ty, Decoration::Alignment,
                                         DecorationAttr{std::get<2>(key)});
            }
            return pointer_ty;
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
                [&](group_data_type const &g) -> spv_inst * {
                    return spv_pointer_ty(StorageClass::CrossWorkgroup, spv_ty(g.ty()),
                                          alignment(scalar_type::i64));
                },
                [&](memref_data_type const &mr) -> spv_inst * {
                    const auto storage_cls = address_space_to_storage_class(mr.addrspace());
                    auto spv_element_ty = spv_ty(mr.element_data_ty());
                    const auto align = mr.alignment();
                    return spv_pointer_ty(storage_cls, spv_element_ty, align);
                },
                [&](scalar_data_type const &ty) -> spv_inst * {
                    switch (ty.ty()) {
                    case scalar_type::i8:
                        return mod_->add_to<OpTypeInt>(section::type_const_var, 8, 0);
                    case scalar_type::i16:
                        return mod_->add_to<OpTypeInt>(section::type_const_var, 16, 0);
                    case scalar_type::i32:
                        return mod_->add_to<OpTypeInt>(section::type_const_var, 32, 0);
                    case scalar_type::i64:
                        return mod_->add_to<OpTypeInt>(section::type_const_var, 64, 0);
                    case scalar_type::index: {
                        const auto sz = size(ty.ty());
                        if (sz == 8) {
                            return spv_ty(scalar_data_type::get(mod_->context(), scalar_type::i64));
                        }
                        return spv_ty(scalar_data_type::get(mod_->context(), scalar_type::i32));
                    }
                    case scalar_type::bf16:
                        return spv_ty(scalar_data_type::get(mod_->context(), scalar_type::i16));
                    case scalar_type::f16:
                    case scalar_type::f32:
                    case scalar_type::f64:
                        return mod_->add_to<OpTypeFloat>(section::type_const_var,
                                                         size(ty.ty()) * 8);
                    case scalar_type::c32: {
                        auto f32_ty = spv_ty(scalar_type::f32);
                        return spv_vec_ty(f32_ty, vector_size::v2);
                    }
                    case scalar_type::c64: {
                        auto f64_ty = spv_ty(scalar_type::f64);
                        return spv_vec_ty(f64_ty, vector_size::v2);
                    }
                    }
                    throw status::internal_compiler_error;
                },
                [&](coopmatrix_data_type const &ty) -> spv_inst * {
                    if (!matrix_->use_khr_matrix_ext()) {
                        const auto sty = [](scalar_type sty, matrix_use use) {
                            switch (sty) {
                            case scalar_type::i8:
                            case scalar_type::i16:
                            case scalar_type::i32:
                                return scalar_type::i32;
                            case scalar_type::bf16:
                            case scalar_type::f16:
                                return use == matrix_use::acc ? scalar_type::f32 : scalar_type::i32;
                            case scalar_type::f32:
                                return scalar_type::f32;
                            default:
                                throw status::internal_compiler_error;
                            }
                        }(ty.component_ty(), ty.use());
                        const auto sty_size = size(sty);
                        const std::int64_t bytes = sty_size * ty.rows() * ty.cols();
                        const auto bytes_multiple_of = sty_size * matrix_->required_subgroup_size();
                        if (bytes % bytes_multiple_of != 0) {
                            throw status::internal_compiler_error;
                        }

                        return spv_vec_ty(spv_ty(sty), bytes / bytes_multiple_of);
                    }

                    auto spv_use = [](matrix_use use) {
                        switch (use) {
                        case matrix_use::a:
                            return CooperativeMatrixUse::MatrixBKHR;
                        case matrix_use::b:
                            return CooperativeMatrixUse::MatrixAKHR;
                        case matrix_use::acc:
                            return CooperativeMatrixUse::MatrixAccumulatorKHR;
                        }
                        throw status::internal_compiler_error;
                    };
                    auto scalar_ty = spv_ty(ty.component_ty());
                    auto scope = constant(static_cast<std::int32_t>(Scope::Subgroup));
                    auto rows = constant(static_cast<std::int32_t>(ty.cols()));
                    auto cols = constant(static_cast<std::int32_t>(ty.rows()));
                    auto use = constant(static_cast<std::int32_t>(spv_use(ty.use())));
                    return mod_->add_to<OpTypeCooperativeMatrixKHR>(
                        section::type_const_var, scalar_ty, scope, rows, cols, use);
                },
                [](auto const &) -> spv_inst * {
                    // @todo
                    throw status::not_implemented;
                }},
            *ty);
    });
}

auto uniquifier::spv_ty(scalar_type sty) -> spv_inst * {
    return spv_ty(scalar_data_type::get(mod_->context(), sty));
}

auto uniquifier::spv_vec_ty(spv_inst *component_ty, std::int32_t length) -> spv_inst * {
    auto key = std::make_pair(component_ty, length);
    return lookup(spv_vec_tys_, key, [&](std::pair<spv_inst *, std::int32_t> const &key) {
        return mod_->add_to<OpTypeVector>(section::type_const_var, key.first, key.second);
    });
}
auto uniquifier::spv_vec_ty(spv_inst *component_ty, vector_size length) -> spv_inst * {
    return spv_vec_ty(component_ty, static_cast<std::int32_t>(length));
}

auto uniquifier::void_ty() -> spv_inst * { return spv_ty(void_data_type::get(mod_->context())); }

} // namespace tinytc::spv

