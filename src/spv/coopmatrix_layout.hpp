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
    scalar_type sty;
    std::int64_t rows, cols, blocks, length, shape1, blocks1;
    std::int32_t ops_per_chan;

    inline auto operator==(coopmatrix_layout const &other) const {
        return sty == other.sty && rows == other.rows && cols == other.cols &&
               blocks == other.blocks && length == other.length && shape1 == other.shape1 &&
               blocks1 == other.blocks1 && ops_per_chan == other.ops_per_chan;
    }

    inline auto component_no(std::int64_t block1, std::int64_t col, std::int64_t block2) const
        -> std::int64_t {
        return block1 + col * blocks1 + block2 * blocks1 * (length / blocks);
    }

    inline auto component_no(std::int64_t col, std::int64_t block) const -> std::int64_t {
        return component_no(block % blocks1, col, block / blocks1);
    }
};

} // namespace tinytc::spv

namespace std {
template <> struct hash<tinytc::spv::coopmatrix_layout> {
    inline auto operator()(tinytc::spv::coopmatrix_layout const &key) const -> std::size_t {
        return fnv1a_combine(key.sty, key.rows, key.cols, key.blocks, key.length, key.shape1,
                             key.blocks1, key.ops_per_chan);
    }
};
} // namespace std

#endif // COOPMATRIX_LAYOUT_20250428_HPP
