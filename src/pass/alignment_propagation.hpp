// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ALIGNMENT_PROPAGATION_20241202_HPP
#define ALIGNMENT_PROPAGATION_20241202_HPP

#include "node/function_node.hpp"

namespace tinytc {

class alignment_propagation_pass {
  public:
    void run_on_function(function_node &fn);
};

} // namespace tinytc

#endif // ALIGNMENT_PROPAGATION_20241202_HPP
