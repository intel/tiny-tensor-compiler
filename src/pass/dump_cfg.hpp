// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DUMP_BACKWARD_CFG_20240919_HPP
#define DUMP_BACKWARD_CFG_20240919_HPP

#include "node/function_node.hpp"

#include <iosfwd>

namespace tinytc {

class dump_cfg_pass {
  public:
    dump_cfg_pass(std::ostream &os);

    void run_on_function(tinytc_func &fn);

  private:
    std::ostream *os_;
};

} // namespace tinytc

#endif // DUMP_BACKWARD_CFG_20240919_HPP
