// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef LINALG_TYPES_20241023_HPP
#define LINALG_TYPES_20241023_HPP

#include "tinytc/core.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace tinytc::test {

class tensor_layout {
  public:
    tensor_layout(array_view<std::int64_t> shape, array_view<std::int64_t> stride = {},
                  array_view<std::int64_t> static_shape = {},
                  array_view<std::int64_t> static_stride = {});

    inline auto dim() const -> std::int64_t { return shape_.size(); }
    inline auto size() const -> std::int64_t {
        return dim() > 0 ? stride_.back() * shape_.back() : 1;
    }
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

template <typename T>
concept op_blas_a2 = requires(T op, typename T::alpha_type alpha, typename T::beta_type beta,
                              typename T::A_type const *A_ref, typename T::B_type *B_ref) {
    typename T::alpha_type;
    typename T::A_type;
    typename T::beta_type;
    typename T::B_type;
    T::kernel_name;
    { op.lA() } -> std::same_as<tensor_layout const &>;
    { op.lB() } -> std::same_as<tensor_layout const &>;
    { op.make_prog() } -> std::same_as<shared_handle<tinytc_prog_t>>;
    op.reference_impl(alpha, A_ref, beta, B_ref);
};

template <typename T>
concept op_blas_a3 = requires(T op, typename T::alpha_type alpha, typename T::beta_type beta,
                              typename T::A_type const *A_ref, typename T::B_type const *B_ref,
                              typename T::C_type *C_ref) {
    typename T::alpha_type;
    typename T::A_type;
    typename T::B_type;
    typename T::beta_type;
    typename T::C_type;
    T::kernel_name;
    { op.lA() } -> std::same_as<tensor_layout const &>;
    { op.lB() } -> std::same_as<tensor_layout const &>;
    { op.lC() } -> std::same_as<tensor_layout const &>;
    { op.make_prog() } -> std::same_as<shared_handle<tinytc_prog_t>>;
    op.reference_impl(alpha, A_ref, B_ref, beta, C_ref);
};

inline auto make_index_2d(transpose t, std::int64_t m, std::int64_t n) {
    auto idx = std::array<std::int64_t, 2u>{m, n};
    if (t == transpose::T) {
        std::swap(idx[0], idx[1]);
    }
    return idx;
};

class nd_iterator {
  public:
    using value_type = array_view<std::int64_t>;
    using difference_type = std::ptrdiff_t;

    static auto begin(array_view<std::int64_t> shape) -> nd_iterator;
    static auto end(array_view<std::int64_t> shape) -> nd_iterator;

    nd_iterator(std::vector<std::int64_t> it, array_view<std::int64_t> shape);

    auto operator*() const -> value_type;
    auto operator++() -> nd_iterator &;
    auto operator++(int) -> nd_iterator;
    auto operator==(nd_iterator const &other) const -> bool;
    auto operator!=(nd_iterator const &other) const -> bool;

  private:
    std::vector<std::int64_t> it_;
    array_view<std::int64_t> shape_;
};

template <typename F> void nd_foreach(array_view<std::int64_t> shape, F &&fun) {
    auto begin = nd_iterator::begin(shape);
    auto end = nd_iterator::end(shape);
    for (; begin != end; ++begin) {
        fun(*begin);
    }
}

} // namespace tinytc::test

#endif // LINALG_TYPES_20241023_HPP
