// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MEMREF_20250214_HPP
#define MEMREF_20250214_HPP

#include "node/function_node.hpp"
#include "tinytc/types.h"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace tinytc {

class memref_info {
  public:
    memref_info() = default;
    memref_info(std::int32_t alignment, std::int32_t sty_size, std::vector<std::int64_t> shape_gcd,
                std::vector<std::int64_t> stride_gcd);

    inline auto alignment() const { return alignment_; }
    inline auto sty_size() const { return sty_size_; }
    inline auto shape_gcd_begin() const { return shape_gcd_.begin(); }
    inline auto shape_gcd_end() const { return shape_gcd_.end(); }
    inline auto shape_gcd() const -> std::vector<std::int64_t> const & { return shape_gcd_; }
    inline auto stride_gcd_begin() const { return stride_gcd_.begin(); }
    inline auto stride_gcd_end() const { return stride_gcd_.end(); }
    inline auto stride_gcd() const -> std::vector<std::int64_t> const & { return stride_gcd_; }

    auto compute_max_alignment(std::vector<std::int64_t> const &offset_gcds) const -> std::int32_t;

  private:
    std::int32_t alignment_, sty_size_;
    std::vector<std::int64_t> shape_gcd_, stride_gcd_;
};

class memref_analysis_result {
  public:
    auto get_if(::const_tinytc_value_t a) const -> memref_info const *;
    auto get_if(::tinytc_value const &a) const -> memref_info const *;
    void set(::tinytc_value const &a, memref_info g);

  private:
    std::unordered_map<::tinytc_value const *, memref_info> memref_info_;
};

class memref_analysis {
  public:
    inline memref_analysis(std::int32_t default_alignment)
        : default_alignment_(default_alignment) {}

    auto run_on_function(function_node const &fn) -> memref_analysis_result;

  private:
    std::int32_t default_alignment_;
};

} // namespace tinytc

#endif // MEMREF_20250214_HPP
