// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MATRIX_EXT_20241204_HPP
#define MATRIX_EXT_20241204_HPP

#include "tinytc/types.h"

#include <unordered_set>
#include <utility>

namespace tinytc {

class matrix_ext_analysis_result {
  public:
    matrix_ext_analysis_result() = default;
    inline matrix_ext_analysis_result(std::unordered_set<const_tinytc_value_t> mext)
        : mext_{std::move(mext)} {}

    auto get(const_tinytc_value_t a) const -> bool;
    auto get(tinytc_value const &a) const -> bool;

  private:
    std::unordered_set<const_tinytc_value_t> mext_;
};

class matrix_ext_analysis {
  public:
    auto run_on_function(tinytc_func const &fn,
                         tinytc_core_info const &info) -> matrix_ext_analysis_result;
};

} // namespace tinytc

#endif // MATRIX_EXT_20241204_HPP
