// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ALIAS_ANALYSIS_20230330_HPP
#define ALIAS_ANALYSIS_20230330_HPP

#include "ir/node/function_node.hpp"
#include "ir/node/inst_node.hpp"
#include "ir/node/region_node.hpp"
#include "ir/node/value_node.hpp"
#include "ir/visitor/aa_results.hpp"

#include <unordered_map>

namespace tinytc::ir {

class alias_analyser {
  public:
    /* Stmt nodes */
    void operator()(inst_node &);
    void operator()(loop_inst &p);
    void operator()(expand_inst &e);
    void operator()(fuse_inst &f);
    void operator()(if_inst &in);
    void operator()(subview_inst &s);

    /* Region nodes */
    void operator()(rgn &b);

    /* Func nodes */
    void operator()(prototype &);
    void operator()(function &fn);

    aa_results get_result() const;

  private:
    std::unordered_map<value_node *, value_node *> alias_;
};

} // namespace tinytc::ir

#endif // ALIAS_ANALYSIS_20230330_HPP
