// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CHECK_IR_20240222_HPP
#define CHECK_IR_20240222_HPP

#include "tinytc/export.hpp"
#include "tinytc/ir/error.hpp"
#include "tinytc/ir/internal/function_node.hpp"
#include "tinytc/ir/internal/inst_node.hpp"
#include "tinytc/ir/internal/program_node.hpp"
#include "tinytc/ir/internal/region_node.hpp"

namespace tinytc::ir::internal {

class TINYTC_EXPORT ir_checker {
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

} // namespace tinytc::ir::internal

#endif // CHECK_IR_20240222_HPP
