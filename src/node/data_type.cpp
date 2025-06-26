// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "node/data_type.hpp"
#include "compiler_context.hpp"
#include "compiler_context_cache.hpp"
#include "error.hpp"
#include "location.hpp"
#include "node/data_type.hpp"
#include "scalar_type.hpp"
#include "support/fnv1a_array_view.hpp" // IWYU pragma: keep
#include "tinytc/builder.h"
#include "tinytc/builder.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"
#include "util/casting.hpp"
#include "util/fnv1a.hpp"
#include "util/math.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

using namespace tinytc;

namespace tinytc {

auto boolean_data_type::get(tinytc_compiler_context_t ctx) -> tinytc_data_type_t {
    return ctx->cache()->bool_ty.get();
}

auto coopmatrix_data_type::get(tinytc_data_type_t component_ty, std::int64_t rows,
                               std::int64_t cols, matrix_use use, location const &lc)
    -> tinytc_data_type_t {
    const auto hash = fnv1a_combine(component_ty, rows, cols, use);
    const auto is_equal = [&](tinytc_data_type_t ty) {
        const auto ct = dyn_cast<coopmatrix_data_type>(ty);
        return ct && component_ty == ct->ty() && rows == ct->rows() && cols == ct->cols() &&
               use == ct->use();
    };
    const auto make = [&]() { return new coopmatrix_data_type(component_ty, rows, cols, use, lc); };

    auto &tys = component_ty->context()->cache()->coopmatrix_tys;
    return tys.get(hash, is_equal, make);
}

coopmatrix_data_type::coopmatrix_data_type(tinytc_data_type_t ty, std::int64_t rows0,
                                           std::int64_t cols0, matrix_use use, location const &lc)
    : tinytc_data_type(DTK::coopmatrix, ty->context()), ty_(std::move(ty)), shape_{rows0, cols0},
      use_(use) {
    if (!isa<scalar_data_type>(*ty_)) {
        throw compilation_error(lc, status::ir_expected_scalar);
    }
    if (rows() < 0 || is_dynamic_value(rows())) {
        throw compilation_error(lc, status::ir_invalid_shape);
    }
    if (!is_positive_power_of_two(rows())) {
        throw compilation_error(lc, status::ir_unsupported_coopmatrix_shape);
    }
    if (cols() < 0 || is_dynamic_value(cols())) {
        throw compilation_error(lc, status::ir_invalid_shape);
    }
}

auto coopmatrix_data_type::component_ty() const -> scalar_type {
    return dyn_cast<scalar_data_type>(ty_)->ty();
}

auto group_data_type::get(tinytc_data_type_t memref_ty, std::int64_t size, std::int64_t offset,
                          location const &lc) -> tinytc_data_type_t {
    const auto hash = fnv1a_combine(memref_ty, size, offset);
    const auto is_equal = [&](tinytc_data_type_t ty) {
        const auto gt = dyn_cast<group_data_type>(ty);
        return gt && memref_ty == gt->ty() && size == gt->size() && offset == gt->offset();
    };
    const auto make = [&]() { return new group_data_type(memref_ty, size, offset, lc); };

    auto &tys = memref_ty->context()->cache()->group_tys;
    return tys.get(hash, std::move(is_equal), std::move(make));
}

group_data_type::group_data_type(tinytc_data_type_t ty, std::int64_t size, std::int64_t offset,
                                 location const &lc)
    : tinytc_data_type(DTK::group, ty->context()), ty_(std::move(ty)), size_(size),
      offset_(offset) {
    if (!isa<memref_data_type>(*ty_)) {
        throw compilation_error(lc, status::ir_expected_memref);
    }
    if (size < 0 && !is_dynamic_value(size)) {
        throw compilation_error(lc, status::ir_invalid_shape);
    }
    if (offset < 0 && !is_dynamic_value(offset)) {
        throw compilation_error(lc, status::ir_invalid_offset);
    }
}

memref_data_type::memref_data_type(tinytc_data_type_t element_ty, std::vector<std::int64_t> shape,
                                   std::vector<std::int64_t> stride, address_space addrspace,
                                   location const &lc)
    : tinytc_data_type(DTK::memref, element_ty->context()), element_ty_(element_ty),
      shape_(std::move(shape)), stride_(std::move(stride)), addrspace_(addrspace) {
    if (!isa<scalar_data_type>(*element_ty_)) {
        throw compilation_error(lc, status::ir_expected_scalar);
    }
    if (stride_.size() != shape_.size()) {
        throw compilation_error(lc, status::ir_shape_stride_mismatch);
    }
    for (auto const &s : shape_) {
        if (s < 0 && !is_dynamic_value(s)) {
            throw compilation_error(lc, status::ir_invalid_shape);
        }
    }
    for (auto const &s : stride_) {
        if (s < 0 && !is_dynamic_value(s)) {
            throw compilation_error(lc, status::ir_invalid_shape);
        }
    }
}

scalar_type memref_data_type::element_ty() const {
    return dyn_cast<scalar_data_type>(element_ty_)->ty();
}

auto memref_data_type::element_alignment() const -> std::int32_t {
    return ::tinytc::alignment(element_ty());
}
auto memref_data_type::size_in_bytes() const -> std::int64_t {
    if (is_dynamic()) {
        return dynamic;
    }
    std::size_t s = size(element_ty());
    if (dim() > 0) {
        s *= stride_.back() * shape_.back();
    }
    return s;
}

auto memref_data_type::get(tinytc_data_type_t element_ty, array_view<std::int64_t> shape,
                           array_view<std::int64_t> stride, address_space addrspace,
                           location const &lc) -> tinytc_data_type_t {

    auto stride_buffer = std::vector<std::int64_t>{};
    if (stride.empty()) {
        stride_buffer = canonical_stride(shape);
        stride = array_view<std::int64_t>{stride_buffer};
    }

    const auto hash = fnv1a_combine(element_ty, shape, stride, addrspace);
    const auto is_equal = [&](tinytc_data_type_t ty) {
        const auto mt = dyn_cast<memref_data_type>(ty);
        return mt && element_ty == mt->element_data_ty() && addrspace == mt->addrspace() &&
               std::equal(shape.begin(), shape.end(), mt->shape().begin(), mt->shape().end()) &&
               std::equal(stride.begin(), stride.end(), mt->stride().begin(), mt->stride().end());
    };
    const auto make = [&]() {
        if (!stride_buffer.empty()) {
            return new memref_data_type(element_ty, shape, std::move(stride_buffer), addrspace, lc);
        }
        return new memref_data_type(element_ty, shape, stride, addrspace, lc);
    };

    auto &tys = element_ty->context()->cache()->memref_tys;
    return tys.get(hash, std::move(is_equal), std::move(make));
}

auto memref_data_type::canonical_stride(array_view<std::int64_t> shape)
    -> std::vector<std::int64_t> {
    if (shape.empty()) {
        return {};
    }
    auto stride = std::vector<std::int64_t>(shape.size(), dynamic);
    stride[0] = 1;
    for (std::size_t i = 0; i < shape.size() - 1 && !is_dynamic_value(shape[i]); ++i) {
        stride[i + 1] = stride[i] * shape[i];
    }
    return stride;
}

auto scalar_data_type::get(tinytc_compiler_context_t ctx, scalar_type ty) -> tinytc_data_type_t {
    return ctx->cache()->scalar_tys[static_cast<tinytc_scalar_type_t>(ty)].get();
}

auto void_data_type::get(tinytc_compiler_context_t ctx) -> tinytc_data_type_t {
    return ctx->cache()->void_ty.get();
}

} // namespace tinytc

extern "C" {

tinytc_status_t tinytc_boolean_type_get(tinytc_data_type_t *dt, tinytc_compiler_context_t ctx) {
    if (dt == nullptr || ctx == nullptr) {
        return tinytc_status_invalid_arguments;
    }

    return exception_to_status_code([&] { *dt = boolean_data_type::get(ctx); });
}

tinytc_status_t tinytc_scalar_type_get(tinytc_data_type_t *dt, tinytc_compiler_context_t ctx,
                                       tinytc_scalar_type_t type) {
    if (dt == nullptr || ctx == nullptr) {
        return tinytc_status_invalid_arguments;
    }

    return exception_to_status_code(
        [&] { *dt = scalar_data_type::get(ctx, enum_cast<scalar_type>(type)); });
}

tinytc_status_t tinytc_memref_type_get(tinytc_data_type_t *dt, tinytc_data_type_t scalar_ty,
                                       uint32_t shape_size, const int64_t *shape,
                                       uint32_t stride_size, const int64_t *stride,
                                       tinytc_address_space_t addrspace,
                                       const tinytc_location_t *loc) {
    if (dt == nullptr || (shape_size != 0 && shape == nullptr) ||
        (stride_size != 0 && stride == nullptr)) {
        return tinytc_status_invalid_arguments;
    }

    return exception_to_status_code([&] {
        *dt = memref_data_type::get(scalar_ty, array_view{shape, shape_size},
                                    array_view{stride, stride_size},
                                    enum_cast<address_space>(addrspace), get_optional(loc));
    });
}

tinytc_status_t tinytc_group_type_get(tinytc_data_type_t *dt, tinytc_data_type_t memref_ty,
                                      int64_t size, int64_t offset, const tinytc_location_t *loc) {
    if (dt == nullptr) {
        return tinytc_status_invalid_arguments;
    }

    return exception_to_status_code(
        [&] { *dt = group_data_type::get(memref_ty, size, offset, get_optional(loc)); });
}

tinytc_status_t tinytc_coopmatrix_type_get(tinytc_data_type_t *dt, tinytc_data_type_t scalar_ty,
                                           int64_t rows, int64_t cols, tinytc_matrix_use_t use,
                                           const tinytc_location_t *loc) {
    if (dt == nullptr || scalar_ty == nullptr) {
        return tinytc_status_invalid_arguments;
    }

    return exception_to_status_code([&] {
        *dt = coopmatrix_data_type::get(scalar_ty, rows, cols, enum_cast<matrix_use>(use),
                                        get_optional(loc));
    });
}

tinytc_status_t tinytc_void_type_get(tinytc_data_type_t *dt, tinytc_compiler_context_t ctx) {
    if (dt == nullptr || ctx == nullptr) {
        return tinytc_status_invalid_arguments;
    }

    return exception_to_status_code([&] { *dt = void_data_type::get(ctx); });
}
}
