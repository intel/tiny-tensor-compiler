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
    ItT begin() { return begin_; }
    ItT end() { return end_; }

  private:
    ItT begin_, end_;
};

template <typename IteratorT> class indirection_kind_deref {
  public:
    using iterator_value_reference = decltype(*std::declval<IteratorT>());
    using value_type = std::decay_t<decltype(*std::declval<iterator_value_reference>())>;
    using reference = value_type &;
    using pointer = value_type *;

    static auto ref(iterator_value_reference v) -> reference { return *v; }
    static auto ptr(iterator_value_reference v) -> pointer { return &ref(v); }
};

template <typename IteratorT> class indirection_kind_get {
  public:
    using iterator_value_reference = decltype(*std::declval<IteratorT>());
    using value_type =
        std::remove_reference_t<decltype(*std::declval<iterator_value_reference>().get())>;
    using reference = value_type &;
    using pointer = value_type *;

    static auto ref(iterator_value_reference v) -> reference { return *v.get(); }
    static auto ptr(iterator_value_reference v) -> pointer { return v.get(); }
};

template <typename IteratorT, template <typename> class IndirectionKind = indirection_kind_deref>
class indirect_random_access_iterator {
  public:
    using value_type = typename IndirectionKind<IteratorT>::value_type;
    using pointer = value_type *;
    using reference = value_type &;
    using difference_type = std::ptrdiff_t;

    indirect_random_access_iterator() : it_{nullptr} {}
    indirect_random_access_iterator(IteratorT it) : it_{std::move(it)} {}

    auto operator*() const -> reference { return IndirectionKind<IteratorT>::ref(*it_); }
    auto operator->() const -> pointer { return IndirectionKind<IteratorT>::ptr(*it_); }
    auto operator[](std::size_t n) const -> reference {
        return IndirectionKind<IteratorT>::ref(it_[n]);
    }
    auto operator++() -> indirect_random_access_iterator & {
        ++it_;
        return *this;
    }
    auto operator++(int) -> indirect_random_access_iterator {
        auto tmp = it_++;
        return indirect_random_access_iterator{tmp};
    }
    auto operator--() -> indirect_random_access_iterator & {
        --it_;
        return *this;
    }
    auto operator--(int) -> indirect_random_access_iterator {
        auto tmp = it_--;
        return indirect_random_access_iterator{tmp};
    }
    auto operator-(indirect_random_access_iterator const &other) const -> difference_type {
        return it_ - other.it_;
    }
    auto operator+=(std::ptrdiff_t n) -> indirect_random_access_iterator & {
        it_ += n;
        return *this;
    }
    auto operator-=(std::ptrdiff_t n) -> indirect_random_access_iterator & {
        it_ -= n;
        return *this;
    }
    auto operator==(indirect_random_access_iterator const &other) const -> bool {
        return it_ == other.it_;
    }
    auto operator<=>(indirect_random_access_iterator const &other) const -> bool {
        return it_ <=> other.it_;
    }

  private:
    IteratorT it_;
};

template <typename T, template <typename> class Kind>
auto operator+(indirect_random_access_iterator<T, Kind> const &p, std::ptrdiff_t n) {
    auto q = indirect_random_access_iterator{p};
    return q += n;
}

template <typename T, template <typename> class Kind>
auto operator+(std::ptrdiff_t n, indirect_random_access_iterator<T, Kind> const &p) {
    return p + n;
}

template <typename T, template <typename> class Kind>
auto operator-(indirect_random_access_iterator<T, Kind> const &p, std::ptrdiff_t n) {
    auto q = indirect_random_access_iterator{p};
    return q -= n;
}

} // namespace tinytc

#endif // UTIL_20240201_HPP
