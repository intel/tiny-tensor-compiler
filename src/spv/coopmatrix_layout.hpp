// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef COOPMATRIX_LAYOUT_20250428_HPP
#define COOPMATRIX_LAYOUT_20250428_HPP

#include <cstdint>

namespace tinytc {
enum class scalar_type;
} // namespace tinytc

namespace tinytc::spv {

struct coopmatrix_layout {
    std::int64_t rows, cols, blocks, length, shape1;
    scalar_type sty;
};

} // namespace tinytc::spv

#endif // COOPMATRIX_LAYOUT_20250428_HPP
