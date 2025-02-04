// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ALIGNMENT_PROPAGATION_20241202_HPP
#define ALIGNMENT_PROPAGATION_20241202_HPP

#include "node/function_node.hpp"
#include "tinytc/types.h"

namespace tinytc {

class alignment_propagation_pass {
  public:
    alignment_propagation_pass(const_tinytc_core_info_t info);
    void run_on_function(function_node &fn);

  private:
    std::uint32_t default_alignment_;
};

} // namespace tinytc

#endif // ALIGNMENT_PROPAGATION_20241202_HPP
