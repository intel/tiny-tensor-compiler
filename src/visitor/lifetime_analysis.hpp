// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef LIFETIME_ANALYSIS_20230329_HPP
#define LIFETIME_ANALYSIS_20230329_HPP

#include "node/function_node.hpp"
#include "node/inst_node.hpp"
#include "node/program_node.hpp"
#include "node/region_node.hpp"
#include "tinytc/tinytc.hpp"
#include "visitor/aa_results.hpp"

#include <unordered_set>
#include <vector>

namespace tinytc {

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
    std::unordered_set<::tinytc_value *> operator()(blas_a2_inst &inst);
    std::unordered_set<::tinytc_value *> operator()(blas_a3_inst &inst);
    std::unordered_set<::tinytc_value *> operator()(loop_inst &p);
    std::unordered_set<::tinytc_value *> operator()(scalar_inst &inst);
    std::unordered_set<::tinytc_value *> operator()(alloca_inst &a);
    std::unordered_set<::tinytc_value *> operator()(barrier_inst &b);
    std::unordered_set<::tinytc_value *> operator()(expand_inst &e);
    std::unordered_set<::tinytc_value *> operator()(fuse_inst &f);
    std::unordered_set<::tinytc_value *> operator()(load_inst &e);
    std::unordered_set<::tinytc_value *> operator()(if_inst &in);
    std::unordered_set<::tinytc_value *> operator()(lifetime_stop_inst &);
    std::unordered_set<::tinytc_value *> operator()(size_inst &s);
    std::unordered_set<::tinytc_value *> operator()(store_inst &s);
    std::unordered_set<::tinytc_value *> operator()(subview_inst &s);
    std::unordered_set<::tinytc_value *> operator()(yield_inst &in);

    /* Region nodes */
    std::unordered_set<::tinytc_value *> operator()(rgn &b);

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
