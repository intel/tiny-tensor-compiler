// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef INSERT_LIFETIME_STOP_20240912_HPP
#define INSERT_LIFETIME_STOP_20240912_HPP

#include "node/function_node.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"

#include <unordered_set>

namespace tinytc {

class aa_results;

class insert_lifetime_stop_pass {
  public:
    void run_on_function(tinytc_func &fn);

  private:
    auto run_on_region(tinytc_region &reg, aa_results const &aa)
        -> std::unordered_set<::tinytc_value const *>;
};

} // namespace tinytc

#endif // INSERT_LIFETIME_STOP_20240912_HPP
