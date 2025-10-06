// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef LOWER_LINALG_20240801_HPP
#define LOWER_LINALG_20240801_HPP

#include "tinytc/types.h"

namespace tinytc {

class lower_linalg_pass {
  public:
    lower_linalg_pass(::tinytc_core_info const *info);

    void run_on_function(::tinytc_func &fn);

  private:
    ::tinytc_core_info const *info_;
};

} // namespace tinytc

#endif // LOWER_LINALG_20240801_HPP
