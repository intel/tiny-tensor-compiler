// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CONSTANT_PROPAGATION_20240807_HPP
#define CONSTANT_PROPAGATION_20240807_HPP

#include "node/function_node.hpp"
#include "node/inst_node.hpp"
#include "node/program_node.hpp"
#include "node/region_node.hpp"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace tinytc {

class constant_propagation {
  public:
    /* Inst nodes */
    void operator()(inst_node &);
    void operator()(arith_inst &arith);
    void operator()(parallel_inst &p);

    /* Region nodes */
    void operator()(region_node &b);

    /* Func nodes */
    void operator()(function_node &fn);

    /* Program nodes */
    void operator()(program &p);

  private:
    std::unordered_map<uintptr_t, value> known_constants_;
};

} // namespace tinytc

#endif // CONSTANT_PROPAGATION_20240807_HPP
