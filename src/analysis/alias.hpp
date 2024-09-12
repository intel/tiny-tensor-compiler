// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ALIAS_20240912_HPP
#define ALIAS_20240912_HPP

#include "analysis/aa_results.hpp"
#include "node/function_node.hpp"

namespace tinytc {

class alias_analysis {
  public:
    auto run_on_function(function &fn) -> aa_results;
};

} // namespace tinytc

#endif // ALIAS_20240912_HPP
