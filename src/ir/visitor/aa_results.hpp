// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef AA_RESULTS_20240314_HPP
#define AA_RESULTS_20240314_HPP

#include <unordered_map>

namespace tinytc {

class alias_analyser;
class value_node;

class aa_results {
  public:
    aa_results() = default;
    value_node *root(value_node &a);
    bool alias(value_node &a, value_node &b);

  private:
    aa_results(std::unordered_map<value_node *, value_node *> alias);
    std::unordered_map<value_node *, value_node *> alias_;

    friend class alias_analyser;
};

} // namespace tinytc

#endif // AA_RESULTS_20240314_HPP
