// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SLOT_TRACKER_20240418_HPP
#define SLOT_TRACKER_20240418_HPP

#include "node/function_node.hpp"
#include "node/inst_node.hpp"
#include "node/program_node.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"

#include <cstdint>
#include <unordered_map>

namespace tinytc {

class slot_tracker {
  public:
    /* Stmt nodes */
    void operator()(inst_node const &in);
    void operator()(loop_inst const &p);
    void operator()(if_inst const &in);
    void operator()(parallel_inst const &p);

    /* Region nodes */
    void operator()(rgn const &b);

    /* Func nodes */
    void operator()(prototype const &);
    void operator()(function const &fn);

    /* Program nodes */
    void operator()(program const &p);

    auto get_slot(value_node const &v) -> std::int64_t;

  private:
    void set_slot(value_node const &v);

    std::int64_t slot_ = 0;
    std::unordered_map<value_node const *, std::int64_t> slot_map_;
};

} // namespace tinytc

#endif // SLOT_TRACKER_20240418_HPP
