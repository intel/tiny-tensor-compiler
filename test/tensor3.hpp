// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TENSOR3_20240314_HPP
#define TENSOR3_20240314_HPP

#include "doctest/doctest.h"

#include <algorithm>
#include <array>
#include <complex>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

template <typename T> class tensor3 {
  public:
    using real_t = T;

    tensor3(std::array<std::uint32_t, 3> const &shape, std::array<std::uint32_t, 3> const &stride)
        : shape_(shape), stride_(stride), data_(size()) {}

    std::size_t size() const { return stride_.back() * shape_.back(); }
    auto shape(std::size_t i) const { return shape_[i]; }
    auto stride(std::size_t i) const { return stride_[i]; }

    T const &operator()(std::uint32_t i, std::uint32_t j, std::uint32_t k) const {
        return data_[i * stride_[0] + j * stride_[1] + k * stride_[2]];
    }
    T &operator()(std::uint32_t i, std::uint32_t j, std::uint32_t k) {
        return data_[i * stride_[0] + j * stride_[1] + k * stride_[2]];
    }

    T *data() { return data_.data(); }
    T const *data() const { return data_.data(); }

    void set_zero() { std::fill(data_.begin(), data_.end(), T(0)); }

  private:
    std::array<std::uint32_t, 3> shape_, stride_;
    std::vector<T> data_;
};

template <typename T> bool compare(tensor3<T> const &A, tensor3<T> const &B) {
    bool compatible =
        A.shape(0) == B.shape(0) && A.shape(1) == B.shape(1) && A.shape(2) == B.shape(2);
    if (!compatible) {
        throw std::runtime_error("incompatible compare");
    }
    for (std::uint32_t k = 0; k < A.shape(2); ++k) {
        for (std::uint32_t j = 0; j < A.shape(1); ++j) {
            for (std::uint32_t i = 0; i < A.shape(0); ++i) {
                constexpr auto eps = 10.0 * std::numeric_limits<decltype(std::abs(T{}))>::epsilon();
                REQUIRE(std::abs(A(i, j, k) - B(i, j, k)) == doctest::Approx(0.0).epsilon(eps));
            }
        }
    }
    return true;
}

#endif // TENSOR3_20240314_HPP
