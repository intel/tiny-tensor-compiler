// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "node/data_type_node.hpp"
#include "error.hpp"
#include "tinytc/types.hpp"

#include <cstddef>
#include <utility>

namespace tinytc {

memref_data_type::memref_data_type(scalar_type type, std::vector<std::int64_t> shape,
                                   std::vector<std::int64_t> stride, location const &lc)
    : element_ty_(std::move(type)), shape_(std::move(shape)), stride_(std::move(stride)) {
    loc(lc);
    if (stride_.empty()) {
        stride_ = canonical_stride();
    }
    if (stride_.size() != shape_.size()) {
        throw compilation_error(loc(), status::ir_shape_stride_mismatch);
    }
}

auto memref_data_type::canonical_stride() const -> std::vector<std::int64_t> {
    if (shape_.empty()) {
        return {};
    }
    auto stride = std::vector<std::int64_t>(shape_.size(), dynamic);
    stride[0] = 1;
    for (std::size_t i = 0; i < shape_.size() - 1 && !is_dynamic_value(shape_[i]); ++i) {
        stride[i + 1] = stride[i] * shape_[i];
    }
    return stride;
}

} // namespace tinytc
