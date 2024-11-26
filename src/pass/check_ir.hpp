// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CHECK_IR_20240222_HPP
#define CHECK_IR_20240222_HPP

#include "node/function_node.hpp"
#include "node/inst_node.hpp"
#include "node/region_node.hpp"
#include "tinytc/types.hpp"

namespace tinytc {

class check_ir_pass {
  public:
    void operator()(inst_node const &in);
    void operator()(for_inst const &in);
    void operator()(if_inst const &in);

    void run_on_function(function_node &fn);

  private:
    void check_yield(region_node const &reg, inst_node const &in,
                     status yield_missing_status = status::ir_must_have_yield);

    bool inside_spmd_region_ = false;
};

} // namespace tinytc

#endif // CHECK_IR_20240222_HPP
