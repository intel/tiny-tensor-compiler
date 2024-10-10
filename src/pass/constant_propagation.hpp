// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CONSTANT_PROPAGATION_20240807_HPP
#define CONSTANT_PROPAGATION_20240807_HPP

#include "tinytc/types.h"
#include "tinytc/types.hpp"

namespace tinytc {

class constant_propagation_pass {
  public:
    void run_on_function(::tinytc_func &fn);
    void run_on_region(::tinytc_region &reg);

    void set_opt_flag(tinytc::optflag flag, bool enabled);

  private:
    bool enable_unsafe_fp_math_;
};

} // namespace tinytc

#endif // CONSTANT_PROPAGATION_20240807_HPP
