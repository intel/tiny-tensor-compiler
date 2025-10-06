// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef LOWER_COOPMATRIX_20241206_HPP
#define LOWER_COOPMATRIX_20241206_HPP

#include "tinytc/types.h"

namespace tinytc {

class lower_coopmatrix_pass {
  public:
    lower_coopmatrix_pass(::tinytc_core_info const *info);

    void run_on_function(::tinytc_func &fn);

  private:
    ::tinytc_core_info const *info_;
};

} // namespace tinytc

#endif // LOWER_COOPMATRIX_20241206_HPP
