// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "spv/uniquifier.hpp"
#include "node/type.hpp"
#include "number.hpp"
#include "spv/defs.hpp"
#include "spv/instructions.hpp"
#include "spv/lut.hpp"
#include "spv/module.hpp"
#include "spv/opencl.std.hpp"
#include "support/fnv1a_array_view.hpp"
#include "tinytc/builder.hpp"
#include "tinytc/types.hpp"
#include "util/casting.hpp"
#include "util/overloaded.hpp"

#include <algorithm>
#include <iterator>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace tinytc::spv {

auto address_space_to_storage_class(address_space as) -> StorageClass {
    return as == address_space::local ? StorageClass::Workgroup : StorageClass::CrossWorkgroup;
}

uniquifier::uniquifier(tinytc_spv_mod &m) : mod_(&m) {}

auto uniquifier::asm_target() -> spv_inst * {
    if (!asm_target_) {
        asm_target_ =
            mod_->add_to<OpAsmTargetINTEL>(section::type_const_var, "spirv64-unknown-unknown");
    }
    return asm_target_;
}

auto uniquifier::bool_constant(bool b) -> spv_inst * {
    if (b) {
        return lookup(bool_true_, [&] {
            return mod_->add_to<OpConstantTrue>(section::type_const_var, bool_ty());
        });
    }
    return lookup(bool_false_, [&] {
        return mod_->add_to<OpConstantFalse>(section::type_const_var, bool_ty());
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
        return 4; // i32
    case BuiltIn::GlobalLinearId:
    case BuiltIn::LocalInvocationIndex:
        return mod().context()->index_bit_width() / 8; // index
    case BuiltIn::GlobalSize:
    case BuiltIn::GlobalInvocationId:
    case BuiltIn::WorkgroupSize:
    case BuiltIn::EnqueuedWorkgroupSize:
    case BuiltIn::LocalInvocationId:
    case BuiltIn::NumWorkgroups:
    case BuiltIn::WorkgroupId:
    case BuiltIn::GlobalOffset:
        return 4 * (mod().context()->index_bit_width() / 8); // index x 3
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
        return int_ty(32); // i32
    case BuiltIn::GlobalLinearId:
    case BuiltIn::LocalInvocationIndex:
        return int_ty(mod().context()->index_bit_width()); // index
    case BuiltIn::GlobalSize:
    case BuiltIn::GlobalInvocationId:
    case BuiltIn::WorkgroupSize:
    case BuiltIn::EnqueuedWorkgroupSize:
    case BuiltIn::LocalInvocationId:
    case BuiltIn::NumWorkgroups:
    case BuiltIn::WorkgroupId:
    case BuiltIn::GlobalOffset: {
        auto index_ty = int_ty(mod().context()->index_bit_width());
        return vec_ty(index_ty, vector_size::v3); // index x 3
    }
    default:
        throw status::internal_compiler_error;
    }
}

auto uniquifier::builtin_var(BuiltIn b) -> spv_inst * {
    return lookup(builtin_, b, [&](BuiltIn b) {
        auto ty = pointer_ty(StorageClass::Input, builtin_pointee_ty(b), builtin_alignment(b));
        auto var = mod_->add_to<OpVariable>(section::type_const_var, ty, StorageClass::Input,
                                            std::nullopt);
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
    return lookup(cst_map_, cst, [&](LiteralContextDependentNumber cst) {
        const auto visitor = overloaded{
            [&](std::int8_t &) -> spv_inst * {
                return mod_->add_to<OpConstant>(section::type_const_var, int_ty(8), cst);
            },
            [&](std::int16_t &) -> spv_inst * {
                return mod_->add_to<OpConstant>(section::type_const_var, int_ty(16), cst);
            },
            [&](std::int32_t &) -> spv_inst * {
                return mod_->add_to<OpConstant>(section::type_const_var, int_ty(32), cst);
            },
            [&](std::int64_t &) -> spv_inst * {
                return mod_->add_to<OpConstant>(section::type_const_var, int_ty(64), cst);
            },
            [&](half &) -> spv_inst * {
                return mod_->add_to<OpConstant>(section::type_const_var, float_ty(16), cst);
            },
            [&](float &) -> spv_inst * {
                return mod_->add_to<OpConstant>(section::type_const_var, float_ty(32), cst);
            },
            [&](double &) -> spv_inst * {
                return mod_->add_to<OpConstant>(section::type_const_var, float_ty(64), cst);
            }};
        return std::visit(visitor, cst);
    });
}

void uniquifier::extension(char const *ext_name) {
    if (!extensions_.contains(ext_name)) {
        mod_->add_to<OpExtension>(section::extension, ext_name);
        extensions_.insert(ext_name);
    }
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

auto uniquifier::array_ty(spv_inst *element_ty, std::int32_t length) -> spv_inst * {
    auto key = std::make_pair(element_ty, length);
    return lookup(array_tys_, key, [&](std::pair<spv_inst *, std::int32_t> const &key) {
        return mod_->add_to<OpTypeArray>(section::type_const_var, key.first, constant(key.second));
    });
}

auto uniquifier::bool_ty() -> spv_inst * {
    if (!bool_ty_) {
        bool_ty_ = mod_->add_to<OpTypeBool>(section::type_const_var);
    }
    return bool_ty_;
}

auto uniquifier::float_ty(std::int32_t width) -> spv_inst * {
    return lookup(float_tys_, width, [&](std::int32_t width) {
        return mod_->add_to<OpTypeFloat>(section::type_const_var, width);
    });
}

auto uniquifier::function_ty(spv_inst *return_ty, array_view<spv_inst *> params) -> spv_inst * {
    const auto map_key = fnv1a_step(fnv1a_step(fnv1a0(), return_ty), params);
    auto range = function_tys_.equal_range(map_key);
    for (auto it = range.first; it != range.second; ++it) {
        if (return_ty == it->second->op0() &&
            std::equal(params.begin(), params.end(), it->second->op1().begin(),
                       it->second->op1().end())) {
            return it->second;
        }
    }
    return function_tys_
        .emplace(map_key, mod_->add_to<OpTypeFunction>(section::type_const_var, return_ty,
                                                       std::move(params)))
        ->second;
}

auto uniquifier::int_ty(std::int32_t width) -> spv_inst * {
    return lookup(int_tys_, width, [&](std::int32_t width) {
        return mod_->add_to<OpTypeInt>(section::type_const_var, width, 0);
    });
}

auto uniquifier::pointer_ty(StorageClass cls, spv_inst *pointee_ty, std::int32_t alignment)
    -> spv_inst * {
    auto key = std::make_tuple(cls, pointee_ty, alignment);
    return lookup(
        pointer_tys_, key, [&](std::tuple<StorageClass, spv_inst *, std::int32_t> const &key) {
            auto pointer_ty = mod_->add_to<OpTypePointer>(section::type_const_var, std::get<0>(key),
                                                          std::get<1>(key));
            if (std::get<2>(key) > 0) {
                mod_->add_to<OpDecorate>(section::decoration, pointer_ty, Decoration::Alignment,
                                         DecorationAttr{std::get<2>(key)});
            }
            return pointer_ty;
        });
}

auto uniquifier::vec_ty(spv_inst *component_ty, std::int32_t length) -> spv_inst * {
    auto key = std::make_pair(component_ty, length);
    return lookup(vec_tys_, key, [&](std::pair<spv_inst *, std::int32_t> const &key) {
        return mod_->add_to<OpTypeVector>(section::type_const_var, key.first, key.second);
    });
}
auto uniquifier::vec_ty(spv_inst *component_ty, vector_size length) -> spv_inst * {
    return vec_ty(component_ty, static_cast<std::int32_t>(length));
}

auto uniquifier::void_ty() -> spv_inst * {
    if (!void_ty_) {
        void_ty_ = mod_->add_to<OpTypeVoid>(section::type_const_var);
    }
    return void_ty_;
}

auto uniquifier::load_builtin(BuiltIn b) -> spv_inst * {
    auto builtin = builtin_var(b);
    return mod_->add<OpLoad>(builtin_pointee_ty(b), builtin, MemoryAccess::Aligned,
                             builtin_alignment(b));
}

} // namespace tinytc::spv

