// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DATA_TYPE_NODE_20230309_HPP
#define DATA_TYPE_NODE_20230309_HPP

#include "support/type_list.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

namespace tinytc {
enum class DTK { bool_, coopmatrix, group, memref, scalar, void_ };
using data_type_nodes =
    type_list<class boolean_data_type, class coopmatrix_data_type, class group_data_type,
              class memref_data_type, class scalar_data_type, class void_data_type>;
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

class boolean_data_type : public data_type_node {
  public:
    inline static bool classof(data_type_node const &d) { return d.type_id() == DTK::bool_; }
    static auto get(tinytc_compiler_context_t ctx) -> tinytc_data_type_t;

  protected:
    inline boolean_data_type(tinytc_compiler_context_t ctx) : data_type_node(DTK::bool_, ctx) {}
    friend class compiler_context_cache;
};

class coopmatrix_data_type : public data_type_node {
  public:
    inline static bool classof(data_type_node const &d) { return d.type_id() == DTK::coopmatrix; }
    static auto get(tinytc_data_type_t ty, std::int64_t rows, std::int64_t cols, matrix_use use,
                    location const &lc = {}) -> tinytc_data_type_t;

    inline auto ty() const -> tinytc_data_type_t { return ty_; }
    auto component_ty() const -> scalar_type;
    inline auto shape() const -> std::array<std::int64_t, 2u> { return shape_; }
    inline auto shape(int mode) const -> std::int64_t { return shape_[mode]; }
    inline auto rows() const -> std::int64_t { return shape_[0]; }
    inline auto cols() const -> std::int64_t { return shape_[1]; }
    inline auto use() const -> matrix_use { return use_; }
    inline auto distributed_mode() const -> int { return use_ == matrix_use::b ? 1 : 0; }
    inline auto num_blocks(std::int32_t subgroup_size) const -> std::int64_t {
        return 1 + (shape(distributed_mode()) - 1) / subgroup_size;
    }
    // Number of components per work-item
    inline auto length(std::int32_t subgroup_size) const -> std::int64_t {
        return num_blocks(subgroup_size) * shape(1 - distributed_mode());
    }

  protected:
    coopmatrix_data_type(tinytc_compiler_context_t ctx, tinytc_data_type_t ty, std::int64_t rows,
                         std::int64_t cols, matrix_use use, location const &lc = {});

  private:
    tinytc_data_type_t ty_;
    std::array<std::int64_t, 2u> shape_;
    matrix_use use_;
};

struct coopmatrix_data_type_key {
    tinytc_data_type_t ty;
    std::int64_t rows, cols;
    matrix_use use;

    auto hash() -> std::uint64_t;
    auto operator==(coopmatrix_data_type const &ct) -> bool;
};

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

    auto element_alignment() const -> std::int32_t;
    auto size_in_bytes() const -> std::int64_t;

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
    static auto get(tinytc_compiler_context_t ctx) -> tinytc_data_type_t;

  protected:
    inline void_data_type(tinytc_compiler_context_t ctx) : data_type_node(DTK::void_, ctx) {}
    friend class compiler_context_cache;
};

} // namespace tinytc

#endif // DATA_TYPE_NODE_20230309_HPP
