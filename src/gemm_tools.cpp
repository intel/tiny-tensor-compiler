// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "gemm_tools.hpp"

#include <cstddef>

namespace tinytc {

auto max_register_block_gemm(std::int32_t A_size, std::int32_t B_size, std::int32_t C_size,
                             std::int32_t subgroup_size, std::int32_t register_space,
                             std::int32_t C_blocks,
                             std::pair<std::int32_t, std::int32_t> max_fill_fraction)
    -> std::pair<std::int32_t, std::int32_t> {
    auto const arithmetic_intensity = [](std::int32_t rows, std::int32_t cols) {
        return (rows * cols) / static_cast<double>(rows + cols);
    };

    auto const max_bytes = register_space * max_fill_fraction.first / max_fill_fraction.second;

    constexpr std::int32_t max_K = standard_K_block_sizes.back();

    // The required number of bytes is given by
    // num_bytes = rows * (cols * C_blocks * C_size + max_K * A_size) + cols * max_K * B_size.
    // Thus
    // rows <= (max_bytes - cols * max_K * B_size) / (cols * C_blocks * C_size + max_K * A_size).
    // Moreover, we require rows % subgroup_size = 0, so we set rows = k * subgroup_size and get
    // k <= (max_bytes  - cols * max_K * B_size) /
    //      (subgroup_size * (cols * C_blocks * C_size + max_K * A_size)).
    auto const max_rows = [&](std::int32_t cols) {
        const auto k = (max_bytes - cols * max_K * B_size) /
                       (subgroup_size * (cols * C_blocks * C_size + max_K * A_size));
        return k * subgroup_size;
    };
    // Here, we have
    // cols <= (max_bytes - rows * max_K * A_size) / (rows * C_blocks * C_size + max_K * B_size).
    auto const max_cols = [&](std::int32_t rows) {
        return (max_bytes - rows * max_K * A_size) / (rows * C_blocks * C_size + max_K * B_size);
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
// If the above optimization does not have a solution, the minimum block size (= subgroup size) is
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

auto choose_block_size_multiple(std::int32_t min_block_size, std::int32_t max_block_size,
                                std::int32_t num_tiles, std::int64_t size) -> std::int32_t {
    auto const block_size = [&min_block_size](std::int32_t k) -> std::int32_t {
        return k * min_block_size;
    };
    auto const num_blocks = [&block_size, &size](std::int32_t k) -> std::int64_t {
        return 1 + (size - 1) / block_size(k);
    };
    std::int32_t k = 1;
    while (2 * k * min_block_size < max_block_size) {
        k *= 2;
    }
    while (k > 1 && (num_blocks(k) % num_tiles != 0 || block_size(k) - size >= min_block_size)) {
        k /= 2;
    }
    return k;
}

// Similar as compute_m_block_size for fixed sizes
// block_sizes array must be sorted in ascending order
auto choose_block_size(array_view<std::int32_t> block_sizes, std::int32_t num_tiles,
                       std::int64_t size) -> std::int32_t {
    auto const num_blocks = [&size](std::int32_t block_size) -> std::int64_t {
        return 1 + (size - 1) / block_size;
    };
    std::size_t k = block_sizes.size() - 1;
    while (k > 1 && (num_blocks(block_sizes[k]) % num_tiles != 0 ||
                     block_sizes[k] - size >= block_sizes[0])) {
        --k;
    }
    return block_sizes[k];
}

auto choose_k_block_size(array_view<std::int32_t> block_sizes, std::int64_t K) -> std::int32_t {
    std::int64_t j = static_cast<std::int64_t>(block_sizes.size()) - 1;
    for (; K < block_sizes[j] && j > 0; --j) {
    }
    return block_sizes[j];
}

} // namespace tinytc
