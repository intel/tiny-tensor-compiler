// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DATA_TYPE_NODE_20230309_HPP
#define DATA_TYPE_NODE_20230309_HPP

#include "tinytc/ir/data_type.hpp"
#include "tinytc/ir/location.hpp"
#include "tinytc/ir/scalar_type.hpp"

#include <clir/builtin_type.hpp>
#include <clir/data_type.hpp>
#include <clir/virtual_type_list.hpp>

#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

namespace tinytc::ir {

class data_type_node
    : public clir::virtual_type_list<class void_data_type, class group_data_type,
                                     class memref_data_type, class scalar_data_type> {
  public:
    inline location const &loc() const { return loc_; }
    inline void loc(location const &loc) { loc_ = loc; }

  private:
    location loc_;
};

class group_data_type : public clir::visitable<group_data_type, data_type_node> {
  public:
    inline group_data_type(data_type ty) : ty_(std::move(ty)) {}

    inline data_type &ty() { return ty_; }

  private:
    data_type ty_;
};

class void_data_type : public clir::visitable<void_data_type, data_type_node> {};

class memref_data_type : public clir::visitable<memref_data_type, data_type_node> {
  public:
    memref_data_type(scalar_type type, std::vector<std::int64_t> shape,
                     std::vector<std::int64_t> stride = {}, location const &lc = {});

    inline scalar_type element_ty() { return element_ty_; }
    inline clir::data_type clir_element_ty() {
        return internal::to_clir_ty(element_ty_, addrspace_);
    }
    inline clir::data_type clir_atomic_element_ty() {
        return internal::to_clir_atomic_ty(element_ty_, addrspace_);
    }
    inline std::int64_t dim() const { return shape_.size(); }
    inline auto const &shape() const { return shape_; }
    inline std::int64_t shape(std::int64_t i) const { return shape_[i]; }
    inline auto const &stride() const { return stride_; }
    inline std::int64_t stride(std::int64_t i) const { return stride_[i]; }
    inline std::int64_t size_in_bytes() const {
        return is_dynamic() ? dynamic : size(element_ty_) * stride_.back() * shape_.back();
    }
    inline clir::address_space addrspace() const { return addrspace_; }
    inline void addrspace(clir::address_space space) { addrspace_ = space; }

    inline bool is_dynamic_shape() const {
        return std::any_of(shape_.begin(), shape_.end(), is_dynamic_value);
    }
    inline bool is_dynamic_stride() const {
        return std::any_of(stride_.begin(), stride_.end(), is_dynamic_value);
    }
    inline bool is_dynamic() const { return is_dynamic_shape() || is_dynamic_stride(); }
    inline bool is_canonical_stride() const { return stride_ == canonical_stride(); }

  private:
    auto canonical_stride() const -> std::vector<std::int64_t>;

    scalar_type element_ty_;
    std::vector<std::int64_t> shape_, stride_;
    clir::address_space addrspace_ = clir::address_space::global_t;
};

class scalar_data_type : public clir::visitable<scalar_data_type, data_type_node> {
  public:
    inline scalar_data_type(scalar_type type) : ty_(type) {}

    inline scalar_type ty() { return ty_; }
    inline clir::data_type clir_ty() { return internal::to_clir_ty(ty_); }

  private:
    scalar_type ty_;
};

} // namespace tinytc::ir

#endif // DATA_TYPE_NODE_20230309_HPP
