// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CHECK_IR_20240222_HPP
#define CHECK_IR_20240222_HPP

#include "node/inst_view.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

namespace tinytc {

class check_ir_pass {
  public:
    void operator()(inst_view in);
    void operator()(for_inst in);
    void operator()(if_inst in);

    void run_on_function(tinytc_func &fn);

  private:
    void check_yield(tinytc_region &reg, tinytc_inst &in,
                     status yield_missing_status = status::ir_must_have_yield);
};

} // namespace tinytc

#endif // CHECK_IR_20240222_HPP
