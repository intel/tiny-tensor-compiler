// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CHECK_IR_20240222_HPP
#define CHECK_IR_20240222_HPP

#include "node/function_node.hpp"
#include "node/inst_node.hpp"
#include "node/program_node.hpp"
#include "node/region_node.hpp"

namespace tinytc {

class check_ir_pass {
  public:
    void run_on_function(function &fn);

  private:
    bool inside_spmd_region_ = false;
};

} // namespace tinytc

#endif // CHECK_IR_20240222_HPP
