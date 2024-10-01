// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CONSTANT_PROPAGATION_20240807_HPP
#define CONSTANT_PROPAGATION_20240807_HPP

#include "tinytc/types.h"

namespace tinytc {

class constant_propagation_pass {
  public:
    void run_on_function(::tinytc_func &fn);
};

} // namespace tinytc

#endif // CONSTANT_PROPAGATION_20240807_HPP
