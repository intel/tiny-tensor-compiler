// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef COOPMATRIX_LAYOUT_20250428_HPP
#define COOPMATRIX_LAYOUT_20250428_HPP

#include "support/fnv1a.hpp"

#include <cstdint>

namespace tinytc {
enum class scalar_type;
} // namespace tinytc

namespace tinytc::spv {

struct coopmatrix_layout {
    std::int64_t rows, cols, blocks, length, shape1;
    std::int32_t ops_per_chan;
    scalar_type sty;

    inline auto operator==(coopmatrix_layout const &other) const {
        return rows == other.rows && cols == other.cols && blocks == other.blocks &&
               length == other.length && shape1 == other.shape1 &&
               ops_per_chan == other.ops_per_chan && sty == other.sty;
    }
};

} // namespace tinytc::spv

namespace std {
template <> struct hash<tinytc::spv::coopmatrix_layout> {
    inline auto operator()(tinytc::spv::coopmatrix_layout const &key) const -> std::size_t {
        return fnv1a_combine(key.rows, key.cols, key.blocks, key.length, key.shape1,
                             key.ops_per_chan, key.sty);
    }
};
} // namespace std

#endif // COOPMATRIX_LAYOUT_20250428_HPP
