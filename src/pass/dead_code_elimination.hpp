// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DEAD_CODE_ELIMINATION_20241007_HPP
#define DEAD_CODE_ELIMINATION_20241007_HPP

#include "tinytc/types.h"

namespace tinytc {

class dead_code_elimination_pass {
  public:
    void run_on_function(::tinytc_func &fn);
    void run_on_region(::tinytc_region &reg);
};

} // namespace tinytc

#endif // DEAD_CODE_ELIMINATION_20241007_HPP
