// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef STACK_20230413_HPP
#define STACK_20230413_HPP

#include "tinytc/export.hpp"
#include "tinytc/ir/internal/function_node.hpp"
#include "tinytc/ir/internal/inst_node.hpp"
#include "tinytc/ir/internal/program_node.hpp"
#include "tinytc/ir/internal/region_node.hpp"
#include "tinytc/ir/internal/value_node.hpp"

#include <cstddef>
#include <list>

namespace tinytc::ir::internal {

class TINYTC_EXPORT stack_ptr {
  public:
    /* Inst nodes */
    void operator()(inst_node &);
    void operator()(alloca_inst &a);
    void operator()(lifetime_stop_inst &s);
    void operator()(for_inst &p);

    /* Region nodes */
    void operator()(rgn &b);

    /* Func nodes */
    void operator()(prototype &p);
    void operator()(function &fn);

    /* Program nodes */
    void operator()(program &p);

  private:
    struct allocation {
        value_node *value;
        std::size_t start, stop;
    };
    std::list<allocation> allocs_;
};

} // namespace tinytc::ir::internal

#endif // STACK_20230413_HPP
