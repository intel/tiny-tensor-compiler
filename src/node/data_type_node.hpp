// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DATA_TYPE_NODE_20230309_HPP
#define DATA_TYPE_NODE_20230309_HPP

#include "compiler_context.hpp"
#include "support/type_list.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <algorithm>
#include <cstdint>
#include <vector>

namespace tinytc {
enum class DTK { group, memref, scalar, void_ };
using data_type_nodes = type_list<class group_data_type, class memref_data_type,
                                  class scalar_data_type, class void_data_type>;
} // namespace tinytc

struct tinytc_data_type {
  public:
    using leaves = tinytc::data_type_nodes;

    inline tinytc_data_type(tinytc::DTK tid, tinytc_compiler_context_t ctx)
        : tid_(tid), ctx_(ctx) {}
    virtual ~tinytc_data_type() = default;
    inline auto type_id() const -> tinytc::DTK { return tid_; }
    inline auto context() const -> tinytc_compiler_context_t { return ctx_; }

  private:
    tinytc::DTK tid_;
    tinytc_compiler_context_t ctx_;
};

namespace tinytc {

using data_type_node = ::tinytc_data_type;

class group_data_type : public data_type_node {
  public:
    inline static bool classof(data_type_node const &d) { return d.type_id() == DTK::group; }
    static auto get(tinytc_data_type_t ty, std::int64_t offset,
                    location const &lc = {}) -> tinytc_data_type_t;

    inline auto ty() const -> tinytc_data_type_t { return ty_; }
    inline auto offset() const -> std::int64_t { return offset_; }

  protected:
    group_data_type(tinytc_compiler_context_t ctx, tinytc_data_type_t ty, std::int64_t offset = 0,
                    location const &lc = {});

  private:
    tinytc_data_type_t ty_;
    std::int64_t offset_;
};

class memref_data_type : public data_type_node {
  public:
    inline static bool classof(data_type_node const &d) { return d.type_id() == DTK::memref; }
    static auto canonical_stride(array_view<std::int64_t> shape) -> std::vector<std::int64_t>;
    static auto get(tinytc_data_type_t element_ty, array_view<std::int64_t> shape,
                    array_view<std::int64_t> stride,
                    address_space addrspace = address_space::global,
                    location const &lc = {}) -> tinytc_data_type_t;

    scalar_type element_ty() const;
    inline tinytc_data_type_t element_data_ty() const { return element_ty_; }
    inline std::int64_t dim() const { return shape_.size(); }
    inline auto const &shape() const { return shape_; }
    inline std::int64_t shape(std::int64_t i) const { return shape_[i]; }
    inline auto const &stride() const { return stride_; }
    inline std::int64_t stride(std::int64_t i) const { return stride_[i]; }
    inline std::int64_t size_in_bytes() const {
        return is_dynamic() ? dynamic : size(element_ty()) * stride_.back() * shape_.back();
    }
    inline auto addrspace() const -> address_space { return addrspace_; }
    inline void addrspace(address_space space) { addrspace_ = space; }

    inline bool is_dynamic_shape() const {
        return std::any_of(shape_.begin(), shape_.end(), is_dynamic_value);
    }
    inline bool is_dynamic_stride() const {
        return std::any_of(stride_.begin(), stride_.end(), is_dynamic_value);
    }
    inline bool is_dynamic() const { return is_dynamic_shape() || is_dynamic_stride(); }
    inline bool is_canonical_stride() const { return stride_ == canonical_stride(shape_); }

  protected:
    memref_data_type(tinytc_compiler_context_t ctx, tinytc_data_type_t element_ty,
                     std::vector<std::int64_t> shape, std::vector<std::int64_t> stride,
                     address_space addrspace = address_space::global, location const &lc = {});

    tinytc_data_type_t element_ty_;
    std::vector<std::int64_t> shape_, stride_;
    address_space addrspace_ = address_space::global;
};

struct memref_data_type_key {
    tinytc_data_type_t element_ty;
    array_view<std::int64_t> shape, stride;
    address_space addrspace;

    auto hash() -> std::uint64_t;
    auto operator==(memref_data_type const &mt) -> bool;
};

class scalar_data_type : public data_type_node {
  public:
    inline static bool classof(data_type_node const &d) { return d.type_id() == DTK::scalar; }
    static auto get(tinytc_compiler_context_t ctx, scalar_type ty) -> tinytc_data_type_t;

    inline scalar_type ty() const { return ty_; }

  protected:
    inline scalar_data_type(tinytc_compiler_context_t ctx, scalar_type type)
        : data_type_node(DTK::scalar, ctx), ty_(type) {}
    friend class compiler_context_cache;

  private:
    scalar_type ty_;
};

class void_data_type : public data_type_node {
  public:
    inline static bool classof(data_type_node const &d) { return d.type_id() == DTK::void_; }

  protected:
    inline void_data_type(tinytc_compiler_context_t ctx) : data_type_node(DTK::void_, ctx) {}
};

} // namespace tinytc

#endif // DATA_TYPE_NODE_20230309_HPP
