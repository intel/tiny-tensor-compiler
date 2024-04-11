// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef UTIL_20240201_HPP
#define UTIL_20240201_HPP

#include <type_traits>

namespace tinytc {

template <class... Ts> struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

template <typename T, typename V> auto enum_cast(V val) {
    return T{std::underlying_type_t<T>(val)};
}

} // namespace tinytc

#endif // UTIL_20240201_HPP
