// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "gemm_tools.hpp"

namespace tinytc {

auto max_register_block_gemm(std::int32_t C_scalar_type_size_in_bytes, std::int32_t sgs,
                             std::int32_t register_space,
                             std::pair<std::int32_t, std::int32_t> max_fill_fraction)
    -> std::pair<std::int32_t, std::int32_t> {
    auto const arithmetic_intensity = [&sgs](std::int32_t row_blocks, std::int32_t cols) {
        return (row_blocks * sgs * cols) / static_cast<double>(row_blocks * sgs + cols);
    };

    auto const max_scalars = register_space * max_fill_fraction.first /
                             (max_fill_fraction.second * C_scalar_type_size_in_bytes);

    // The required number of scalars is given by
    // row_blocks * sgs * (cols + max_K_unrolling) + cols * max_K_unrolling
    auto const max_row_blocks = [&sgs, &max_scalars](std::int32_t cols) {
        return (max_scalars - cols * max_K_unrolling) / (sgs * (cols + max_K_unrolling));
    };
    auto const max_cols = [&sgs, &max_scalars](std::int32_t row_blocks) {
        return (max_scalars - row_blocks * sgs * max_K_unrolling) /
               (row_blocks * sgs + max_K_unrolling);
    };

    double max_ai = 0.0;
    std::int32_t row_blocks = 1, cols = 1;
    for (std::int32_t r = 1; r <= max_row_blocks(1); ++r) {
        for (std::int32_t c = 1; c <= max_cols(r); ++c) {
            auto const ai = arithmetic_intensity(r, c);
            if (ai > max_ai) {
                max_ai = ai;
                row_blocks = r;
                cols = c;
            }
        }
    }

    return std::make_pair(row_blocks, cols);
}

} // namespace tinytc
