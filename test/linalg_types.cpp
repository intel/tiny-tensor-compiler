// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "linalg_types.hpp"

namespace tinytc::test {

tensor_layout::tensor_layout(array_view<std::int64_t> shape, array_view<std::int64_t> stride,
                             array_view<std::int64_t> static_shape,
                             array_view<std::int64_t> static_stride)
    : shape_(shape), stride_(stride), static_shape_(static_shape), static_stride_(static_stride) {
    if (!shape_.empty()) {
        if (stride_.empty() && !shape_.empty()) {
            stride_.reserve(shape_.size());
            stride_.push_back(1);
            for (auto &s : shape_) {
                stride_.push_back(stride_.back() * s);
            }
            stride_.pop_back();
        }
        if (static_shape_.empty()) {
            static_shape_ = shape_;
        }
    }
    if (static_stride_.empty()) {
        static_stride_ = stride_;
    }

    if (stride_.size() != shape_.size()) {
        throw std::runtime_error("Invalid stride");
    }
    if (static_shape_.size() != shape_.size()) {
        throw std::runtime_error("Invalid static shape");
    }
    if (static_stride_.size() != stride_.size()) {
        throw std::runtime_error("Invalid static stride");
    }
}

auto tensor_layout::linear_index(array_view<std::int64_t> idx) const -> std::int64_t {
    if (static_cast<std::int64_t>(idx.size()) != dim()) {
        throw std::runtime_error("index order mismatch");
    }
    std::int64_t l = 0;
    for (std::size_t i = 0; i < idx.size(); ++i) {
        l += idx[i] * stride_[i];
    }
    return l;
}

} // namespace tinytc::test

