// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef STACK_20230413_HPP
#define STACK_20230413_HPP

#include "node/func.hpp"

namespace tinytc {

class set_stack_ptr_pass {
  public:
    void run_on_function(tinytc_func &fn);
};

} // namespace tinytc

#endif // STACK_20230413_HPP
