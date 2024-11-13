// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "spv/pass/capex.hpp"
#include "error.hpp"
#include "spv/capex_util.hpp"
#include "spv/enums.hpp"
#include "spv/instructions.hpp"
#include "spv/module.hpp"
#include "spv/visit.hpp"
#include "support/casting.hpp"
#include "support/util.hpp"

#include <cstdint>

namespace tinytc::spv {

capex::capex(uniquifier &unique) : unique_{&unique} {}

void capex::operator()(spv_inst const &) {}
void capex::operator()(OpAtomicFAddEXT const &in) {
    auto ty = dyn_cast<OpTypeFloat>(in.type());
    if (!ty) {
        throw status::internal_compiler_error;
    }
    switch (ty->op0()) {
    case 16:
        unique_->capability(Capability::AtomicFloat16AddEXT);
        unique_->extension("SPV_EXT_shader_atomic_float16_add");
        break;
    case 32:
        unique_->capability(Capability::AtomicFloat32AddEXT);
        unique_->extension("SPV_EXT_shader_atomic_float_add");
        break;
    case 64:
        unique_->capability(Capability::AtomicFloat64AddEXT);
        unique_->extension("SPV_EXT_shader_atomic_float_add");
        break;
    default:
        break;
    }
}
void capex::operator()(OpEntryPoint const &in) {
    for (auto const &cap : capabilities(in.op0())) {
        unique_->capability(cap);
    }
}
void capex::operator()(OpExecutionMode const &in) {
    for (auto const &cap : capabilities(in.op1())) {
        unique_->capability(cap);
    }
}
void capex::operator()(OpGroupFAdd const &) { unique_->capability(Capability::Groups); }
void capex::operator()(OpGroupIAdd const &) { unique_->capability(Capability::Groups); }
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
}
void capex::operator()(OpSubgroupBlockWriteINTEL const &) {
    unique_->capability(Capability::SubgroupBufferBlockIOINTEL);
    unique_->extension("SPV_INTEL_subgroups");
}
void capex::operator()(OpTypeFloat const &in) {
    switch (in.op0()) {
    case 16:
        unique_->capability(Capability::Float16);
        break;
    case 64:
        unique_->capability(Capability::Float64);
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

