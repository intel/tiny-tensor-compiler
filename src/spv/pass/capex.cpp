// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "spv/pass/capex.hpp"
#include "spv/capex_util.hpp"
#include "spv/enums.hpp"
#include "spv/instructions.hpp"
#include "spv/module.hpp"
#include "spv/uniquifier.hpp"
#include "spv/visit.hpp"
#include "tinytc/core.hpp"
#include "tinytc/types.hpp"
#include "util/casting.hpp"
#include "util/ilist.hpp"
#include "util/ilist_base.hpp"
#include "util/overloaded.hpp"

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
                               [](spv_inst &) -> spv_inst * { return nullptr; }},
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
auto capex::float_atomic_class(spv_inst *raw_ty, spv_inst *op0)
    -> std::pair<LiteralInteger, StorageClass> {
    auto ty = dyn_cast<OpTypeFloat>(raw_ty);
    if (!ty) {
        throw status::internal_compiler_error;
    }

    auto pointer_ty = visit(overloaded{[](inst_with_return_type auto &a) -> OpTypePointer * {
                                           return dyn_cast<OpTypePointer>(a.type());
                                       },
                                       [](spv_inst &) -> OpTypePointer * { return nullptr; }},
                            *op0);
    if (!pointer_ty) {
        throw status::internal_compiler_error;
    }
    return {ty->op0(), pointer_ty->op0()};
}
void capex::operator()(OpAtomicFAddEXT const &in) {
    const auto [bits, storage_cls] = float_atomic_class(in.type(), in.op0());
    switch (bits) {
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
void capex::check_float_min_max_atomic(spv_inst *ty, spv_inst *op0) {
    const auto [bits, storage_cls] = float_atomic_class(ty, op0);
    switch (bits) {
    case 16:
        unique_->capability(Capability::AtomicFloat16MinMaxEXT);
        unique_->extension("SPV_EXT_shader_atomic_float16_min_max");
        required_features_[storage_cls == StorageClass::Workgroup
                               ? tinytc_spirv_feature_atomic_float16_min_max_local
                               : tinytc_spirv_feature_atomic_float16_min_max_global] = true;
        break;
    case 32:
        unique_->capability(Capability::AtomicFloat32MinMaxEXT);
        unique_->extension("SPV_EXT_shader_atomic_float_min_max");
        required_features_[storage_cls == StorageClass::Workgroup
                               ? tinytc_spirv_feature_atomic_float32_min_max_local
                               : tinytc_spirv_feature_atomic_float32_min_max_global] = true;
        break;
    case 64:
        unique_->capability(Capability::AtomicFloat64MinMaxEXT);
        unique_->extension("SPV_EXT_shader_atomic_float_min_max");
        required_features_[storage_cls == StorageClass::Workgroup
                               ? tinytc_spirv_feature_atomic_float64_min_max_local
                               : tinytc_spirv_feature_atomic_float64_min_max_global] = true;
        break;
    default:
        break;
    }
}
void capex::operator()(OpAtomicFMaxEXT const &in) {
    check_float_min_max_atomic(in.type(), in.op0());
}
void capex::operator()(OpAtomicFMinEXT const &in) {
    check_float_min_max_atomic(in.type(), in.op0());
}
void capex::check_int_atomic(spv_inst *raw_ty) {
    auto ty = dyn_cast<OpTypeInt>(raw_ty);
    if (!ty) {
        throw status::internal_compiler_error;
    }
    if (ty && ty->op0() == 64) {
        unique_->capability(Capability::Int64Atomics);
        required_features_[tinytc_spirv_feature_int64_atomics] = true;
    }
}
void capex::operator()(OpAtomicIAdd const &in) { check_int_atomic(in.type()); }
void capex::operator()(OpAtomicSMax const &in) { check_int_atomic(in.type()); }
void capex::operator()(OpAtomicSMin const &in) { check_int_atomic(in.type()); }
void capex::operator()(OpAsmTargetINTEL const &) {
    unique_->capability(Capability::AsmINTEL);
    unique_->extension("SPV_INTEL_inline_assembly");
}
void capex::operator()(OpAsmINTEL const &) {
    unique_->capability(Capability::AsmINTEL);
    unique_->extension("SPV_INTEL_inline_assembly");
}
void capex::operator()(OpAsmCallINTEL const &) {
    unique_->capability(Capability::AsmINTEL);
    unique_->extension("SPV_INTEL_inline_assembly");
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
void capex::operator()(OpCooperativeMatrixLoadKHR const &) {
    unique_->capability(Capability::CooperativeMatrixKHR);
    unique_->extension("SPV_KHR_cooperative_matrix");
}
void capex::operator()(OpCooperativeMatrixMulAddKHR const &) {
    unique_->capability(Capability::CooperativeMatrixKHR);
    unique_->extension("SPV_KHR_cooperative_matrix");
}
void capex::operator()(OpCooperativeMatrixStoreKHR const &) {
    unique_->capability(Capability::CooperativeMatrixKHR);
    unique_->extension("SPV_KHR_cooperative_matrix");
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
void capex::operator()(OpTypeVector const &in) {
    if (in.op1() > 4) {
        unique_->capability(Capability::Vector16);
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

