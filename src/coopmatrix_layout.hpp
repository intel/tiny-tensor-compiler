// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef COOPMATRIX_LAYOUT_20250428_HPP
#define COOPMATRIX_LAYOUT_20250428_HPP

#include "tinytc/types.h"
#include "tinytc/types.hpp"
#include "util/fnv1a.hpp"

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace tinytc {
class core_config;

struct coopmatrix_layout {
    tinytc_type_t sty;
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

auto get_layout(core_config const &cfg, coopmatrix_type const *ct) -> coopmatrix_layout;

} // namespace tinytc

namespace std {
template <> struct hash<tinytc::coopmatrix_layout> {
    inline auto operator()(tinytc::coopmatrix_layout const &key) const -> std::size_t {
        return tinytc::fnv1a_combine(key.sty, key.rows, key.cols, key.blocks, key.length,
                                     key.shape1, key.blocks1, key.ops_per_chan);
    }
};
} // namespace std

#endif // COOPMATRIX_LAYOUT_20250428_HPP
