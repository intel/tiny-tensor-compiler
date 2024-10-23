// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "gemm_tools.hpp"

namespace tinytc {

auto max_register_block_gemm(std::int32_t C_scalar_type_size_in_bytes, std::int32_t subgroup_size,
                             std::int32_t register_space, std::int32_t C_blocks,
                             std::pair<std::int32_t, std::int32_t> max_fill_fraction)
    -> std::pair<std::int32_t, std::int32_t> {
    auto const arithmetic_intensity = [](std::int32_t rows, std::int32_t cols) {
        return (rows * cols) / static_cast<double>(rows + cols);
    };

    auto const max_scalars = register_space * max_fill_fraction.first /
                             (max_fill_fraction.second * C_scalar_type_size_in_bytes);

    // The required number of scalars is given by
    // num_scalars = rows * (cols * C_blocks + max_K_unrolling) + cols * max_K_unrolling.
    // Thus
    // rows <= (max_scalars - cols * max_K_unrolling) / (cols * C_blocks + max_K_unrolling).
    // Moreover, we require rows % subgroup_size = 0, so we set rows = k * subgroup_size and get
    // k <= (max_scalars - cols * max_K_unrolling) / (subgroup_size * (cols * C_blocks +
    // max_K_unrolling)).
    auto const max_rows = [&subgroup_size, &max_scalars, &C_blocks](std::int32_t cols) {
        const auto k = (max_scalars - cols * max_K_unrolling) /
                       (subgroup_size * (cols * C_blocks + max_K_unrolling));
        return k * subgroup_size;
    };
    // Here, we have
    // cols <= (num_scalars - rows * max_K_unrolling) / (rows * C_blocks + max_K_unrolling).
    auto const max_cols = [&max_scalars, &C_blocks](std::int32_t rows) {
        return (max_scalars - rows * max_K_unrolling) / (rows * C_blocks + max_K_unrolling);
    };

    double max_ai = 0.0;
    std::int32_t rows = subgroup_size, cols = 1;
    for (std::int32_t r = subgroup_size; r <= max_rows(1); r += subgroup_size) {
        for (std::int32_t c = 1; c <= max_cols(r); ++c) {
            auto const ai = arithmetic_intensity(r, c);
            if (ai > max_ai) {
                max_ai = ai;
                rows = r;
                cols = c;
            }
        }
    }

    return std::make_pair(rows, cols);
}

// We have block_size(k) = k * subgroup_size, where k is a positive integer,
// and num_blocks(k) = ceil(size / block_size(k))
// We want to solve
// max_k block_size(k)   s.t.
//                              block_size(k) <= max_block_size ; must not exceed max block size
//                          and num_blocks(k) % num_tiles == 0  ; no load imbalance
//                          and block_size(k) - size < sgs      ; no excessive block size
//
// If the above optimization does not have a solution, the minimum block size (= subgroup size is
// returned)
//
auto compute_m_block_size(std::int32_t subgroup_size, std::int32_t max_block_size,
                          std::int32_t num_tiles, std::int64_t size) -> std::int32_t {
    auto const block_size = [&subgroup_size](std::int32_t k) -> std::int32_t {
        return k * subgroup_size;
    };
    auto const num_blocks = [&block_size, &size](std::int32_t k) -> std::int64_t {
        return 1 + (size - 1) / block_size(k);
    };
    std::int32_t k = max_block_size / subgroup_size;
    while (k > 1 && (num_blocks(k) % num_tiles != 0 || block_size(k) - size >= subgroup_size)) {
        --k;
    }
    return k * subgroup_size;
}

auto compute_k_block_size(std::int64_t K) -> std::int32_t {
    auto k_block_size = max_K_unrolling;
    while (K < k_block_size && k_block_size > 1) {
        k_block_size /= 2;
    }
    return k_block_size;
}

} // namespace tinytc
