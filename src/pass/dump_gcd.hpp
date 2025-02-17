// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DUMP_GCD_20241203_HPP
#define DUMP_GCD_20241203_HPP

#include "node/function_node.hpp"
#include "tinytc/types.h"

#include <iosfwd>

namespace tinytc {

class dump_gcd_pass {
  public:
    dump_gcd_pass(std::ostream &os, ::tinytc_core_info const *info);

    void run_on_function(function_node const &fn);

  private:
    std::ostream *os_;
    ::tinytc_core_info const *info_;
};

} // namespace tinytc

#endif // DUMP_GCD_20241203_HPP
