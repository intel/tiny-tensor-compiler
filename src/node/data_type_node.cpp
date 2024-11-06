// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "node/data_type_node.hpp"
#include "compiler_context_cache.hpp"
#include "error.hpp"
#include "support/casting.hpp"
#include "support/fnv1a.hpp"
#include "support/fnv1a_array_view.hpp"
#include "tinytc/types.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <memory>
#include <unordered_map>
#include <utility>

namespace tinytc {

auto boolean_data_type::get(tinytc_compiler_context_t ctx) -> tinytc_data_type_t {
    return ctx->cache()->bool_ty.get();
}

auto coopmatrix_data_type::get(tinytc_data_type_t ty, std::int64_t rows, std::int64_t cols,
                               matrix_use use, location const &lc) -> tinytc_data_type_t {
    auto ctx = ty->context();

    auto key = coopmatrix_data_type_key(ty, rows, cols, use);
    std::uint64_t map_key = key.hash();

    auto &tys = ctx->cache()->coopmatrix_tys;
    auto range = tys.equal_range(map_key);
    for (auto it = range.first; it != range.second; ++it) {
        if (key == *dyn_cast<coopmatrix_data_type>(it->second)) {
            return it->second;
        }
    }
    auto new_ct = std::unique_ptr<coopmatrix_data_type>(
        new coopmatrix_data_type(ctx, key.ty, key.rows, key.cols, key.use, lc));
    return tys.emplace(map_key, new_ct.release())->second;
}

coopmatrix_data_type::coopmatrix_data_type(tinytc_compiler_context_t ctx, tinytc_data_type_t ty,
                                           std::int64_t rows, std::int64_t cols, matrix_use use,
                                           location const &lc)
    : data_type_node(DTK::coopmatrix, ctx), ty_(std::move(ty)), rows_(rows), cols_(cols),
      use_(use) {
    if (!isa<scalar_data_type>(*ty_)) {
        throw compilation_error(lc, status::ir_expected_scalar);
    }
    if (rows_ < 0 || is_dynamic_value(rows_)) {
        throw compilation_error(lc, status::ir_invalid_shape);
    }
    if (cols_ < 0 || is_dynamic_value(cols_)) {
        throw compilation_error(lc, status::ir_invalid_shape);
    }
}

auto coopmatrix_data_type::component_ty() const -> scalar_type {
    return dyn_cast<scalar_data_type>(ty_)->ty();
}

auto coopmatrix_data_type_key::hash() -> std::uint64_t {
    return fnv1a_combine(ty, rows, cols, use);
}

auto coopmatrix_data_type_key::operator==(coopmatrix_data_type const &ct) -> bool {
    return ty == ct.ty() && rows == ct.rows() && cols == ct.cols() && use == ct.use();
}

auto group_data_type::get(tinytc_data_type_t ty, std::int64_t offset,
                          location const &lc) -> tinytc_data_type_t {
    auto ctx = ty->context();
    auto &value = ctx->cache()->group_tys[std::make_pair(ty, offset)];

    if (value == nullptr) {
        value =
            std::unique_ptr<group_data_type>(new group_data_type(ctx, ty, offset, lc)).release();
    }

    return value;
}

group_data_type::group_data_type(tinytc_compiler_context_t ctx, tinytc_data_type_t ty,
                                 std::int64_t offset, location const &lc)
    : data_type_node(DTK::group, ctx), ty_(std::move(ty)), offset_(offset) {
    if (!isa<memref_data_type>(*ty_)) {
        throw compilation_error(lc, status::ir_expected_memref);
    }
    if (offset < 0 && !is_dynamic_value(offset)) {
        throw compilation_error(lc, status::ir_invalid_offset);
    }
}

memref_data_type::memref_data_type(tinytc_compiler_context_t ctx, tinytc_data_type_t element_ty,
                                   std::vector<std::int64_t> shape,
                                   std::vector<std::int64_t> stride, address_space addrspace,
                                   location const &lc)
    : data_type_node(DTK::memref, ctx), element_ty_(element_ty), shape_(std::move(shape)),
      stride_(std::move(stride)), addrspace_(addrspace) {
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

auto memref_data_type::get(tinytc_data_type_t element_ty, array_view<std::int64_t> shape,
                           array_view<std::int64_t> stride, address_space addrspace,
                           location const &lc) -> tinytc_data_type_t {
    auto ctx = element_ty->context();

    auto stride_buffer = std::vector<std::int64_t>{};
    if (stride.empty()) {
        stride_buffer = canonical_stride(shape);
        stride = array_view<std::int64_t>{stride_buffer};
    }

    auto key = memref_data_type_key(element_ty, shape, stride, addrspace);
    std::uint64_t map_key = key.hash();

    auto &tys = ctx->cache()->memref_tys;
    auto range = tys.equal_range(map_key);
    for (auto it = range.first; it != range.second; ++it) {
        if (key == *dyn_cast<memref_data_type>(it->second)) {
            return it->second;
        }
    }
    auto new_mt = std::unique_ptr<memref_data_type>(
        new memref_data_type(ctx, key.element_ty, shape, stride, key.addrspace, lc));
    return tys.emplace(map_key, new_mt.release())->second;
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

auto memref_data_type_key::hash() -> std::uint64_t {
    return fnv1a_combine(element_ty, shape, stride, addrspace);
}

auto memref_data_type_key::operator==(memref_data_type const &mt) -> bool {
    return element_ty == mt.element_data_ty() && addrspace == mt.addrspace() &&
           std::equal(shape.begin(), shape.end(), mt.shape().begin(), mt.shape().end()) &&
           std::equal(stride.begin(), stride.end(), mt.stride().begin(), mt.stride().end());
}

auto scalar_data_type::get(tinytc_compiler_context_t ctx, scalar_type ty) -> tinytc_data_type_t {
    return ctx->cache()->scalar_tys[static_cast<tinytc_scalar_type_t>(ty)].get();
}

auto void_data_type::get(tinytc_compiler_context_t ctx) -> tinytc_data_type_t {
    return ctx->cache()->void_ty.get();
}

} // namespace tinytc
