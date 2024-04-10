// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef LIFETIME_ANALYSIS_20230329_HPP
#define LIFETIME_ANALYSIS_20230329_HPP

#include "ir/node/function_node.hpp"
#include "ir/node/inst_node.hpp"
#include "ir/node/program_node.hpp"
#include "ir/node/region_node.hpp"
#include "ir/visitor/aa_results.hpp"
#include "tinytc/ir/value.hpp"

#include <unordered_set>
#include <vector>

namespace tinytc {

class value_node;

class find_alloca {
  public:
    find_alloca(bool recursive = false);

    /* Inst nodes */
    value operator()(inst_node &);
    value operator()(alloca_inst &a);
    value operator()(for_inst &p);

    /* Region nodes */
    value operator()(rgn &);

    std::vector<value> allocas() const;

  private:
    bool recursive_;
    std::vector<value> alloca_;
};

class lifetime_inserter {
  public:
    /* Inst nodes */
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
    std::unordered_set<value_node *> operator()(size_inst &s);
    std::unordered_set<value_node *> operator()(store_inst &s);
    std::unordered_set<value_node *> operator()(subview_inst &s);
    std::unordered_set<value_node *> operator()(yield_inst &in);

    /* Region nodes */
    std::unordered_set<value_node *> operator()(rgn &b);

    /* Func nodes */
    void operator()(prototype &p);
    void operator()(function &fn);

    /* Program nodes */
    void operator()(program &p);

  private:
    aa_results aa_;
};

} // namespace tinytc

#endif // LIFETIME_ANALYSIS_20230329_HPP
