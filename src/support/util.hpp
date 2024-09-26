// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef UTIL_20240201_HPP
#define UTIL_20240201_HPP

#include <cstdint>
#include <cstring>
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

constexpr auto fnv1a0() -> std::uint64_t { return 0xcbf29ce484222325; }
constexpr auto fnv1a_step(std::uint64_t hash, char ch) -> std::uint64_t {
    return (hash ^ ch) * 0x00000100000001b3;
}
template <typename T> constexpr auto fnv1a_step(std::uint64_t hash, T &&t) -> std::uint64_t {
    char buf[sizeof(T)];
    std::memcpy(buf, &t, sizeof(T));
    for (std::size_t i = 0; i < sizeof(T); ++i) {
        hash = fnv1a_step(hash, buf[i]);
    }
    return hash;
}

template <typename Head, typename... Tail>
constexpr auto fnv1a_step(std::uint64_t hash, Head &&head, Tail &&...tail) -> std::uint64_t {
    return fnv1a_step(fnv1a_step(hash, std::forward<Tail>(tail)...), std::forward<Head>(head));
}

template <typename Head, typename... Tail>
constexpr auto fnv1a(Head &&head, Tail &&...tail) -> std::uint64_t {
    return fnv1a_step(fnv1a_step(fnv1a0(), std::forward<Tail>(tail)...), std::forward<Head>(head));
}

} // namespace tinytc

#endif // UTIL_20240201_HPP
