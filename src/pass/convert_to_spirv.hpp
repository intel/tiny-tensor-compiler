// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CONVERT_TO_SPIRV_20241029_HPP
#define CONVERT_TO_SPIRV_20241029_HPP

#include "device_info.hpp"
#include "node/program_node.hpp"
#include "tinytc/tinytc.hpp"

namespace tinytc {

class convert_to_spirv_pass {
  public:
    convert_to_spirv_pass(::tinytc_core_info const *info);

    auto run_on_program(program_node const &p) -> spv_mod;

  private:
    ::tinytc_core_info const *info_;
};

} // namespace tinytc

#endif // CONVERT_TO_SPIRV_20241029_HPP
