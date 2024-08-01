// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ALIAS_ANALYSIS_20230330_HPP
#define ALIAS_ANALYSIS_20230330_HPP

#include "node/function_node.hpp"
#include "node/inst_node.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "visitor/aa_results.hpp"

#include <unordered_map>

namespace tinytc {

class alias_analyser {
  public:
    /* Stmt nodes */
    void operator()(inst_node const &);
    void operator()(alloca_inst const &a);
    void operator()(loop_inst const &p);
    void operator()(expand_inst const &e);
    void operator()(fuse_inst const &f);
    void operator()(if_inst const &in);
    void operator()(parallel_inst const &p);
    void operator()(subview_inst const &s);

    /* Region nodes */
    void operator()(rgn const &b);

    /* Func nodes */
    void operator()(prototype const &);
    void operator()(function const &fn);

    aa_results get_result() const;

  private:
    std::unordered_map<value_node const *, aa_results::allocation> allocs_;
    std::unordered_map<value_node const *, value_node const *> alias_;
};

} // namespace tinytc

#endif // ALIAS_ANALYSIS_20230330_HPP
