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

// Alignment must be a power of two
template <std::unsigned_integral T> inline auto align_to(T size, T alignment) {
    // ceil(size/alignment) * alignment =
    // (size + alignment - 1) / alignment * alignment =
    // (size + alignment - 1) & ~(alignment - 1)
    return (size + alignment - 1) & ~(alignment - 1);
}

} // namespace tinytc

#endif // MATH_20250613_HPP
