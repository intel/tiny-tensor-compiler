// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef FNV1A_20241009_HPP
#define FNV1A_20241009_HPP

#include <cstdint>
#include <cstring>
#include <string_view>
#include <type_traits>

namespace tinytc {

constexpr auto fnv1a0() -> std::uint64_t { return 0xcbf29ce484222325; }
constexpr auto fnv1a_step(std::uint64_t hash, char ch) -> std::uint64_t {
    return (hash ^ ch) * 0x00000100000001b3;
}
constexpr auto fnv1a_steps(std::uint64_t hash, char const *s, std::size_t len) -> std::uint64_t {
    for (std::size_t i = 0; i < len; ++i) {
        hash = fnv1a_step(hash, s[i]);
    }
    return hash;
}
template <typename T>
requires(std::is_trivial_v<T>)
constexpr auto fnv1a_step(std::uint64_t hash, T const &data) -> std::uint64_t {
    char buf[sizeof(T)];
    std::memcpy(buf, &data, sizeof(T));
    return fnv1a_steps(hash, buf, sizeof(T));
}

constexpr auto fnv1a_step(std::uint64_t hash, std::string_view const &str) -> std::uint64_t {
    return fnv1a_steps(hash, str.data(), str.size());
}

template <typename... T> constexpr auto fnv1a_combine(T const &...t) -> std::uint64_t {
    auto impl = [hash = fnv1a0()](auto const &ti) mutable { return hash = fnv1a_step(hash, ti); };
    return (..., impl(t));
}

constexpr auto fnv1a(char const *s, std::size_t len) -> std::uint64_t {
    return fnv1a_steps(fnv1a0(), s, len);
}
constexpr auto fnv1a(std::string_view s) -> std::uint64_t { return fnv1a_step(fnv1a0(), s); }
constexpr auto operator""_fnv1a(char const *s, std::size_t len) -> std::uint64_t {
    return fnv1a_steps(fnv1a0(), s, len);
}

} // namespace tinytc

#endif // FNV1A_20241009_HPP
