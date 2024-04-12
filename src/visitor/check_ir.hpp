// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CHECK_IR_20240222_HPP
#define CHECK_IR_20240222_HPP

#include "node/function_node.hpp"
#include "node/inst_node.hpp"
#include "node/program_node.hpp"
#include "node/region_node.hpp"
#include "tinytc/ir/error.hpp"

namespace tinytc {

class ir_checker {
  public:
    ir_checker(error_reporter_function reporter);

    /* Stmt nodes */
    bool operator()(inst_node &in);
    bool operator()(for_inst &p);
    bool operator()(foreach_inst &p);
    bool operator()(if_inst &in);

    /* Region nodes */
    bool operator()(rgn &b);

    /* Func nodes */
    bool operator()(prototype &);
    bool operator()(function &fn);

    /* Program nodes */
    bool operator()(program &p);

  private:
    bool inside_spmd_region_ = false;
    error_reporter_function reporter_;
};

} // namespace tinytc

#endif // CHECK_IR_20240222_HPP
