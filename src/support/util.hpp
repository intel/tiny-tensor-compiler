// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef UTIL_20240201_HPP
#define UTIL_20240201_HPP

#include <type_traits>
#include <utility>

namespace tinytc {

template <typename T, typename V> auto enum_cast(V val) {
    return T{std::underlying_type_t<T>(val)};
}

template <typename ItT> class iterator_range_wrapper {
  public:
    iterator_range_wrapper(ItT begin, ItT end) : begin_(std::move(begin)), end_(std::move(end)) {}
    ItT begin() const { return begin_; }
    ItT end() const { return end_; }

  private:
    ItT begin_, end_;
};

} // namespace tinytc

#endif // UTIL_20240201_HPP
