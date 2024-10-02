// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DUMP_DEF_USE_20241002_HPP
#define DUMP_DEF_USE_20241002_HPP

#include "node/function_node.hpp"

#include <iosfwd>

namespace tinytc {

class dump_def_use_pass {
  public:
    dump_def_use_pass(std::ostream &os);

    void run_on_function(function_node const &fn);

  private:
    std::ostream *os_;
};

} // namespace tinytc

#endif // DUMP_DEF_USE_20241002_HPP
