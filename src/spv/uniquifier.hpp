// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef UNIQUIFIER_20241107_HPP
#define UNIQUIFIER_20241107_HPP

#include "spv/enums.hpp"
#include "spv/module.hpp"
#include "tinytc/types.h"

#include <cstdint>
#include <unordered_map>
#include <unordered_set>

namespace tinytc::spv {

class spv_inst;

class uniquifier {
  public:
    uniquifier(tinytc_compiler_context_t ctx, spv::mod &m);

    auto bool2_ty() -> spv::spv_inst *;
    auto bool_constant(bool b) -> spv::spv_inst *;
    void capability(spv::Capability cap);
    auto i32_constant(std::int32_t cst) -> spv::spv_inst *;
    auto null_constant(spv::spv_inst *spv_ty) -> spv::spv_inst *;
    auto opencl_ext() -> spv::spv_inst *;
    auto spv_ty(tinytc_data_type const &ty) -> spv::spv_inst *;

  private:
    tinytc_compiler_context_t ctx_;
    spv::mod *mod_;
    spv::spv_inst *bool2_ty_ = nullptr;
    spv::spv_inst *bool_true_ = nullptr, *bool_false_ = nullptr;
    spv::spv_inst *opencl_ext_ = nullptr;
    std::unordered_map<spv::BuiltIn, spv::spv_inst *> builtin_;
    std::unordered_set<spv::Capability> capabilities_;
    std::unordered_map<std::int32_t, spv::spv_inst *> i32_cst_;
    std::unordered_map<spv::spv_inst *, spv::spv_inst *> null_cst_;
    std::unordered_map<const_tinytc_data_type_t, spv::spv_inst *> spv_tys_;
};

} // namespace tinytc::spv

#endif // UNIQUIFIER_20241107_HPP
