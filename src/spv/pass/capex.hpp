// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CAPEX_20241113_HPP
#define CAPEX_20241113_HPP

#include "spv/defs.hpp"
#include "tinytc/types.h"

#include <array>

namespace tinytc {
enum class spirv_feature;
}

namespace tinytc::spv {

class uniquifier;

class capex {
  public:
    capex(uniquifier &unique);

    void operator()(spv_inst const &in);
    void operator()(OpAtomicStore const &in);
    void operator()(OpAtomicFAddEXT const &in);
    void operator()(OpAtomicIAdd const &in);
    void operator()(OpConvertBF16ToFINTEL const &in);
    void operator()(OpConvertFToBF16INTEL const &in);
    void operator()(OpCooperativeMatrixLoadKHR const &in);
    void operator()(OpCooperativeMatrixMulAddKHR const &in);
    void operator()(OpCooperativeMatrixStoreKHR const &in);
    void operator()(OpEntryPoint const &in);
    void operator()(OpExecutionMode const &in);
    void operator()(OpGroupBroadcast const &in);
    void operator()(OpGroupFAdd const &in);
    void operator()(OpGroupIAdd const &in);
    void operator()(OpInBoundsPtrAccessChain const &in);
    void operator()(OpMemoryModel const &in);
    void operator()(OpSubgroupBlockReadINTEL const &in);
    void operator()(OpSubgroupBlockWriteINTEL const &in);
    void operator()(OpTypeFloat const &in);
    void operator()(OpTypeInt const &in);

    void run_on_module(tinytc_spv_mod const &mod);

    inline auto requires_feature(spirv_feature f) const -> bool {
        return required_features_[static_cast<int>(f)];
    }

  private:
    uniquifier *unique_;
    std::array<bool, TINYTC_NUMBER_OF_SPIRV_FEATURES> required_features_ = {};
};

} // namespace tinytc::spv

#endif // CAPEX_20241113_HPP
