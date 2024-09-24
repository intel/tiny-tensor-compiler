// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "node/data_type_node.hpp"
#include "error.hpp"
#include "support/casting.hpp"
#include "tinytc/types.hpp"

#include <cstddef>
#include <utility>

namespace tinytc {

group_data_type::group_data_type(data_type ty, std::int64_t offset, location const &lc)
    : data_type_node(DTK::group), ty_(std::move(ty)), offset_(offset) {
    if (!isa<memref_data_type>(*ty_)) {
        throw compilation_error(lc, status::ir_expected_memref);
    }
    if (offset < 0 && !is_dynamic_value(offset)) {
        throw compilation_error(lc, status::ir_invalid_offset);
    }
}

memref_data_type::memref_data_type(scalar_type type, std::vector<std::int64_t> shape,
                                   std::vector<std::int64_t> stride, address_space addrspace,
                                   location const &lc)
    : data_type_node(DTK::memref), element_ty_(std::move(type)), shape_(std::move(shape)),
      stride_(std::move(stride)), addrspace_(addrspace) {
    for (auto const &s : shape_) {
        if (s < 0 && !is_dynamic_value(s)) {
            throw compilation_error(lc, status::ir_invalid_shape);
        }
    }
    if (stride_.empty()) {
        stride_ = canonical_stride();
    } else {
        for (auto const &s : stride_) {
            if (s < 0 && !is_dynamic_value(s)) {
                throw compilation_error(lc, status::ir_invalid_shape);
            }
        }
    }
    if (stride_.size() != shape_.size()) {
        throw compilation_error(lc, status::ir_shape_stride_mismatch);
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
