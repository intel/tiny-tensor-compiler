// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef INSERT_BARRIER_20230310_HPP
#define INSERT_BARRIER_20230310_HPP

#include "node/data_type_node.hpp"
#include "node/function_node.hpp"
#include "node/inst_node.hpp"
#include "node/program_node.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "visitor/aa_results.hpp"

#include <unordered_set>

namespace tinytc {

class insert_barrier {
  public:
    /* Data type nodes */
    bool operator()(void_data_type &);
    bool operator()(group_data_type &b);
    bool operator()(memref_data_type &m);
    bool operator()(scalar_data_type &s);

    /* Value nodes */
    value_node *operator()(int_imm &v);
    value_node *operator()(float_imm &v);
    value_node *operator()(val &v);

    /* Stmt nodes */
    std::unordered_set<value_node *> operator()(blas_a2_inst &inst);
    std::unordered_set<value_node *> operator()(blas_a3_inst &inst);
    std::unordered_set<value_node *> operator()(loop_inst &p);
    std::unordered_set<value_node *> operator()(scalar_inst &inst);
    std::unordered_set<value_node *> operator()(alloca_inst &a);
    std::unordered_set<value_node *> operator()(barrier_inst &b);
    std::unordered_set<value_node *> operator()(expand_inst &e);
    std::unordered_set<value_node *> operator()(fuse_inst &f);
    std::unordered_set<value_node *> operator()(load_inst &e);
    std::unordered_set<value_node *> operator()(if_inst &in);
    std::unordered_set<value_node *> operator()(lifetime_stop_inst &);
    std::unordered_set<value_node *> operator()(parallel_inst &p);
    std::unordered_set<value_node *> operator()(size_inst &s);
    std::unordered_set<value_node *> operator()(store_inst &s);
    std::unordered_set<value_node *> operator()(subview_inst &s);
    std::unordered_set<value_node *> operator()(yield_inst &y);

    /* Region nodes */
    std::unordered_set<value_node *> operator()(rgn &b);

    /* Func nodes */
    void operator()(prototype &p);
    void operator()(function &fn);

    /* Program nodes */
    void operator()(program &p);

  private:
    aa_results aa_;
    bool last_instruction_was_barrier_ = false;
};

} // namespace tinytc

#endif // INSERT_BARRIER_20230310_HPP
