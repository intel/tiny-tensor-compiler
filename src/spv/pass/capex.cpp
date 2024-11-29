// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "spv/pass/capex.hpp"
#include "spv/capex_util.hpp"
#include "spv/enums.hpp"
#include "spv/instructions.hpp"
#include "spv/module.hpp"
#include "spv/uniquifier.hpp"
#include "spv/visit.hpp"
#include "support/casting.hpp"
#include "support/ilist.hpp"
#include "support/ilist_base.hpp"
#include "support/util.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <concepts>
#include <cstdint>

namespace tinytc::spv {

template <typename T>
concept inst_with_return_type = requires(T &t) {
    { t.type() } -> std::same_as<IdResultType &>;
};

capex::capex(uniquifier &unique) : unique_{&unique} {}

void capex::operator()(spv_inst const &) {}
void capex::operator()(OpAtomicStore const &in) {
    auto ty = visit(overloaded{[](inst_with_return_type auto &a) -> spv_inst * { return a.type(); },
                               [](auto &) -> spv_inst * { return nullptr; }},
                    *in.op3());
    if (!ty) {
        throw status::internal_compiler_error;
    }
    auto ity = dyn_cast<OpTypeInt>(ty);
    if (ity && ity->op0() == 64) {
        unique_->capability(Capability::Int64Atomics);
        required_features_[tinytc_spirv_feature_int64_atomics] = true;
    }
}
void capex::operator()(OpAtomicFAddEXT const &in) {
    auto ty = dyn_cast<OpTypeFloat>(in.type());
    if (!ty) {
        throw status::internal_compiler_error;
    }

    auto pointer_ty = visit(overloaded{[](inst_with_return_type auto &a) -> OpTypePointer * {
                                           return dyn_cast<OpTypePointer>(a.type());
                                       },
                                       [](auto &) -> OpTypePointer * { return nullptr; }},
                            *in.op0());
    if (!pointer_ty) {
        throw status::internal_compiler_error;
    }
    const auto storage_cls = pointer_ty->op0();

    switch (ty->op0()) {
    case 16:
        unique_->capability(Capability::AtomicFloat16AddEXT);
        unique_->extension("SPV_EXT_shader_atomic_float16_add");
        required_features_[storage_cls == StorageClass::Workgroup
                               ? tinytc_spirv_feature_atomic_float16_add_local
                               : tinytc_spirv_feature_atomic_float16_add_global] = true;
        break;
    case 32:
        unique_->capability(Capability::AtomicFloat32AddEXT);
        unique_->extension("SPV_EXT_shader_atomic_float_add");
        required_features_[storage_cls == StorageClass::Workgroup
                               ? tinytc_spirv_feature_atomic_float32_add_local
                               : tinytc_spirv_feature_atomic_float32_add_global] = true;
        break;
    case 64:
        unique_->capability(Capability::AtomicFloat64AddEXT);
        unique_->extension("SPV_EXT_shader_atomic_float_add");
        required_features_[storage_cls == StorageClass::Workgroup
                               ? tinytc_spirv_feature_atomic_float64_add_local
                               : tinytc_spirv_feature_atomic_float64_add_global] = true;
        break;
    default:
        break;
    }
}
void capex::operator()(OpAtomicIAdd const &in) {
    auto ty = dyn_cast<OpTypeInt>(in.type());
    if (!ty) {
        throw status::internal_compiler_error;
    }
    if (ty && ty->op0() == 64) {
        unique_->capability(Capability::Int64Atomics);
        required_features_[tinytc_spirv_feature_int64_atomics] = true;
    }
}
void capex::operator()(OpConvertBF16ToFINTEL const &) {
    unique_->capability(Capability::BFloat16ConversionINTEL);
    unique_->extension("SPV_INTEL_bfloat16_conversion");
    required_features_[tinytc_spirv_feature_bfloat16_conversion] = true;
}
void capex::operator()(OpConvertFToBF16INTEL const &) {
    unique_->capability(Capability::BFloat16ConversionINTEL);
    unique_->extension("SPV_INTEL_bfloat16_conversion");
    required_features_[tinytc_spirv_feature_bfloat16_conversion] = true;
}
void capex::operator()(OpEntryPoint const &in) {
    for (auto const &cap : capabilities(in.op0())) {
        unique_->capability(cap);
    }
}
void capex::operator()(OpExecutionMode const &in) {
    for (auto const &cap : capabilities(in.op1())) {
        unique_->capability(cap);
        if (cap == Capability::SubgroupDispatch) {
            required_features_[tinytc_spirv_feature_subgroup_dispatch] = true;
        }
    }
}
void capex::operator()(OpGroupBroadcast const &) {
    unique_->capability(Capability::Groups);
    required_features_[tinytc_spirv_feature_groups] = true;
}
void capex::operator()(OpGroupFAdd const &) {
    unique_->capability(Capability::Groups);
    required_features_[tinytc_spirv_feature_groups] = true;
}
void capex::operator()(OpGroupIAdd const &) {
    unique_->capability(Capability::Groups);
    required_features_[tinytc_spirv_feature_groups] = true;
}
void capex::operator()(OpInBoundsPtrAccessChain const &) {
    unique_->capability(Capability::Addresses);
}
void capex::operator()(OpMemoryModel const &in) {
    for (auto const &cap : capabilities(in.op0())) {
        unique_->capability(cap);
    }
    for (auto const &cap : capabilities(in.op1())) {
        unique_->capability(cap);
    }
}
void capex::operator()(OpSubgroupBlockReadINTEL const &) {
    unique_->capability(Capability::SubgroupBufferBlockIOINTEL);
    unique_->extension("SPV_INTEL_subgroups");
    required_features_[tinytc_spirv_feature_subgroup_buffer_block_io] = true;
}
void capex::operator()(OpSubgroupBlockWriteINTEL const &) {
    unique_->capability(Capability::SubgroupBufferBlockIOINTEL);
    unique_->extension("SPV_INTEL_subgroups");
    required_features_[tinytc_spirv_feature_subgroup_buffer_block_io] = true;
}
void capex::operator()(OpTypeFloat const &in) {
    switch (in.op0()) {
    case 16:
        unique_->capability(Capability::Float16);
        required_features_[tinytc_spirv_feature_float16] = true;
        break;
    case 64:
        unique_->capability(Capability::Float64);
        required_features_[tinytc_spirv_feature_float64] = true;
        break;
    default:
        break;
    }
}
void capex::operator()(OpTypeInt const &in) {
    switch (in.op0()) {
    case 8:
        unique_->capability(Capability::Int8);
        break;
    case 16:
        unique_->capability(Capability::Int16);
        break;
    case 64:
        unique_->capability(Capability::Int64);
        break;
    default:
        break;
    }
}

void capex::run_on_module(tinytc_spv_mod const &mod) {
    for (std::int32_t s = 0; s < num_module_sections; ++s) {
        for (auto const &i : mod.insts(enum_cast<section>(s))) {
            visit(*this, i);
        }
    }
}

} // namespace tinytc::spv

