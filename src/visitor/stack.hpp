// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef STACK_20230413_HPP
#define STACK_20230413_HPP

#include "node/function_node.hpp"
#include "node/inst_node.hpp"
#include "node/program_node.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"

#include <cstdint>
#include <list>

namespace tinytc {

class stack_ptr {
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
        std::int64_t start, stop;
    };
    std::list<allocation> allocs_;
};

} // namespace tinytc

#endif // STACK_20230413_HPP
