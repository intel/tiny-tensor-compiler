// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef LINALG_TYPES_20241023_HPP
#define LINALG_TYPES_20241023_HPP

#include "tinytc/tinytc.hpp"

#include <cstdint>
#include <vector>

namespace tinytc::test {

class tensor_layout {
  public:
    tensor_layout(array_view<std::int64_t> shape, array_view<std::int64_t> stride = {},
                  array_view<std::int64_t> static_shape = {},
                  array_view<std::int64_t> static_stride = {});

    inline auto dim() const -> std::int64_t { return shape_.size(); }
    inline auto size() const -> std::int64_t { return stride_.back() * shape_.back(); }
    inline auto shape() const -> array_view<std::int64_t> { return {shape_}; }
    inline auto shape(std::size_t i) const { return shape_[i]; }
    inline auto stride() const -> array_view<std::int64_t> { return {stride_}; }
    inline auto stride(std::size_t i) const { return stride_[i]; }
    inline auto static_shape() const -> array_view<std::int64_t> { return {static_shape_}; }
    inline auto static_shape(std::size_t i) const { return static_shape_[i]; }
    inline auto static_stride() const -> array_view<std::int64_t> { return {static_stride_}; }
    inline auto static_stride(std::size_t i) const { return static_stride_[i]; }

    auto linear_index(array_view<std::int64_t> idx) const -> std::int64_t;

  private:
    std::vector<std::int64_t> shape_, stride_, static_shape_, static_stride_;
};

} // namespace tinytc::test

#endif // LINALG_TYPES_20241023_HPP
