// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ASSEMBLE_20241111_HPP
#define ASSEMBLE_20241111_HPP

#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"

namespace tinytc::spv {

class assembler {
  public:
    auto run_on_module(tinytc_spv_mod const &mod) -> binary;
};

} // namespace tinytc::spv

#endif // ASSEMBLE_20241111_HPP
