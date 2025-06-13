// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MATH_20250613_HPP
#define MATH_20250613_HPP

#include <concepts>

namespace tinytc {

template <std::integral T> auto is_positive_power_of_two(T x) -> bool {
    return x >= 1 && ((x & (x - 1)) == 0);
}

template <std::integral T> auto ilog2(T x) -> T {
    T il2 = 0;
    while (x >>= 1) {
        ++il2;
    }
    return il2;
}

} // namespace tinytc

#endif // MATH_20250613_HPP
