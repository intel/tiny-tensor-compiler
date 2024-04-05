// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef AA_RESULTS_20240314_HPP
#define AA_RESULTS_20240314_HPP

#include "tinytc/export.hpp"

#include <unordered_map>

namespace tinytc::ir::internal {

class alias_analyser;
class value_node;

class TINYTC_EXPORT aa_results {
  public:
    aa_results() = default;
    internal::value_node *root(internal::value_node &a);
    bool alias(internal::value_node &a, internal::value_node &b);

  private:
    aa_results(std::unordered_map<internal::value_node *, internal::value_node *> alias);
    std::unordered_map<internal::value_node *, internal::value_node *> alias_;

    friend class internal::alias_analyser;
};

} // namespace tinytc::ir::internal

#endif // AA_RESULTS_20240314_HPP
