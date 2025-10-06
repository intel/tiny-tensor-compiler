// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef LOWER_FOREACH_20241118_HPP
#define LOWER_FOREACH_20241118_HPP

#include "tinytc/types.h"

namespace tinytc {

class lower_foreach_pass {
  public:
    lower_foreach_pass(::tinytc_core_info const *info);

    void run_on_function(::tinytc_func &fn);

  private:
    ::tinytc_core_info const *info_;
};

} // namespace tinytc

#endif // LOWER_FOREACH_20241118_HPP
