// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "spv/dope_vector.hpp"
#include "spv/defs.hpp"
#include "tinytc/types.hpp"

#include <cstdint>
#include <utility>

namespace tinytc::spv {

dope_vector::dope_vector(spv_inst *ty, std::vector<std::int64_t> static_shape,
                         std::vector<std::int64_t> static_stride, spv_inst *size_ty,
                         std::int64_t static_size, spv_inst *offset_ty, std::int64_t static_offset)
    : ty_(ty), static_shape_(std::move(static_shape)), static_stride_(std::move(static_stride)),
      shape_(dim(), nullptr), stride_(dim(), nullptr), size_ty_(size_ty), offset_ty_(offset_ty),
      static_size_(static_size), static_offset_(static_offset) {
    if (static_shape_.size() != static_stride_.size()) {
        throw status::internal_compiler_error;
    }
}

auto dope_vector::num_dynamic() const -> std::int64_t {
    auto const sum_dynamic = [](std::vector<std::int64_t> const &vec) {
        std::int64_t num_dynamic = 0;
        for (auto &v : vec) {
            if (is_dynamic_value(v)) {
                ++num_dynamic;
            }
        }
        return num_dynamic;
    };
    return sum_dynamic(static_shape_) + sum_dynamic(static_stride_);
}

} // namespace tinytc::spv

