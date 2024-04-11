// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef AA_RESULTS_20240314_HPP
#define AA_RESULTS_20240314_HPP

#include <unordered_map>

struct tinytc_value;

namespace tinytc {

class alias_analyser;

class aa_results {
  public:
    aa_results() = default;
    ::tinytc_value *root(::tinytc_value &a);
    bool alias(::tinytc_value &a, ::tinytc_value &b);

  private:
    aa_results(std::unordered_map<::tinytc_value *, ::tinytc_value *> alias);
    std::unordered_map<::tinytc_value *, ::tinytc_value *> alias_;

    friend class alias_analyser;
};

} // namespace tinytc

#endif // AA_RESULTS_20240314_HPP
