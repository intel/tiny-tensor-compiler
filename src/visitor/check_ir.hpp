// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CHECK_IR_20240222_HPP
#define CHECK_IR_20240222_HPP

#include "node/function_node.hpp"
#include "node/inst_node.hpp"
#include "node/program_node.hpp"
#include "node/region_node.hpp"

namespace tinytc {

class ir_checker {
  public:
    /* Stmt nodes */
    void operator()(inst_node &in);
    void operator()(for_inst &p);
    void operator()(foreach_inst &p);
    void operator()(if_inst &in);

    /* Region nodes */
    void operator()(rgn &b);

    /* Func nodes */
    void operator()(prototype &);
    void operator()(function &fn);

    /* Program nodes */
    void operator()(program &p);

  private:
    bool inside_spmd_region_ = false;
};

} // namespace tinytc

#endif // CHECK_IR_20240222_HPP
