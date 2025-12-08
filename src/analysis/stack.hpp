// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef STACK_20241112_HPP
#define STACK_20241112_HPP

#include "tinytc/types.h"

#include <cstdint>

namespace tinytc {

class stack_high_water_mark {
  public:
    auto run_on_function(tinytc_func &fn) -> std::int64_t;
};

} // namespace tinytc

#endif // STACK_20241112_HPP
