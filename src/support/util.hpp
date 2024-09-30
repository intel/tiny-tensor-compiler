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

template <typename ItT> class iterator_range_wrapper {
  public:
    iterator_range_wrapper(ItT begin, ItT end) : begin_(std::move(begin)), end_(std::move(end)) {}
    ItT begin() const { return begin_; }
    ItT end() const { return end_; }

  private:
    ItT begin_, end_;
};

template <typename IteratorT> class indirect_iterator : public IteratorT {
  public:
    using value_type = std::decay_t<decltype(*(*std::declval<IteratorT>()))>;
    using pointer = value_type *;
    using reference = value_type &;

    auto operator*() const -> reference { return *(this->IteratorT::operator*()); }
    auto operator->() const -> pointer { return &*(this->IteratorT::operator*()); }
    auto operator[](std::size_t n) const -> reference { return *(this->IteratorT::operator[](n)); }
};

template <typename T> class pointer_iterator {
  public:
    using value_type = T;
    using pointer = value_type *;
    using reference = value_type &;
    using difference_type = std::ptrdiff_t;

    pointer_iterator() : ptr_{nullptr} {}
    pointer_iterator(pointer ptr) : ptr_{std::move(ptr)} {}

    auto operator*() const -> reference { return *ptr_; }
    auto operator->() const -> pointer { return ptr_; }
    auto operator[](std::size_t n) const -> reference { return ptr_[n]; }
    auto operator++() -> pointer_iterator & {
        ++ptr_;
        return *this;
    }
    auto operator++(int) -> pointer_iterator {
        auto tmp = ptr_++;
        return pointer_iterator{tmp};
    }
    auto operator--() -> pointer_iterator & {
        --ptr_;
        return *this;
    }
    auto operator--(int) -> pointer_iterator {
        auto tmp = ptr_--;
        return pointer_iterator{tmp};
    }
    auto operator-(pointer_iterator const &other) const -> difference_type {
        return other.ptr_ - ptr_;
    }
    auto operator+=(std::ptrdiff_t n) -> pointer_iterator & {
        ptr_ += n;
        return *this;
    }
    auto operator-=(std::ptrdiff_t n) -> pointer_iterator & {
        ptr_ -= n;
        return *this;
    }
    auto operator==(pointer_iterator const &other) const -> bool { return ptr_ == other.ptr_; }
    auto operator<=>(pointer_iterator const &other) const -> bool { return ptr_ <=> other.ptr_; }

  private:
    pointer ptr_;
};

template <typename T>
auto operator+(pointer_iterator<T> const &p, std::ptrdiff_t n) -> pointer_iterator<T> {
    auto q = pointer_iterator{p};
    return q += n;
}

template <typename T>
auto operator+(std::ptrdiff_t n, pointer_iterator<T> const &p) -> pointer_iterator<T> {
    return p + n;
}

template <typename T>
auto operator-(pointer_iterator<T> const &p, std::ptrdiff_t n) -> pointer_iterator<T> {
    auto q = pointer_iterator{p};
    return q -= n;
}

} // namespace tinytc

#endif // UTIL_20240201_HPP
