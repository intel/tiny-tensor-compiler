// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef FNV1A_ARRAY_VIEW_20241010_HPP
#define FNV1A_ARRAY_VIEW_20241010_HPP

#include "tinytc/tinytc.hpp"
#include "util/fnv1a.hpp"

namespace tinytc {

template <typename T>
constexpr auto fnv1a_step(std::uint64_t hash, array_view<T> const &data) -> std::uint64_t {
    for (auto const &i : data) {
        hash = fnv1a_step(hash, i);
    }
    return hash;
}

} // namespace tinytc

#endif // FNV1A_ARRAY_VIEW_20241010_HPP
