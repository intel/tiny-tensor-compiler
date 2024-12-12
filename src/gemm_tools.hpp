// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef GEMM_TOOLS_20241022_HPP
#define GEMM_TOOLS_20241022_HPP

#include "tinytc/tinytc.hpp"

#include <array>
#include <cstdint>
#include <utility>

namespace tinytc {

constexpr static std::array<std::int32_t, 4u> standard_K_block_sizes = {1, 2, 4, 8};

/**
 * @brief Calculate maximum register blocking size of GEMM
 *
 * @param A_size Size of scalar type of A matrix in bytes
 * @param B_size Size of scalar type of B matrix in bytes
 * @param C_size Size of scalar type of result matrix in bytes
 * @param subgroup_size Subgroup size
 * @param register_space Size of register file per core in bytes
 * @param C_blocks Number of register blocks needed for C, usually 1, 2 for complex
 * @param max_fill_fraction Fraction of register file that shall be blocked at most
 *
 * @return {number of rows, number of columns}
 */
auto max_register_block_gemm(std::int32_t A_size, std::int32_t B_size, std::int32_t C_size,
                             std::int32_t subgroup_size, std::int32_t register_space,
                             std::int32_t C_blocks = 1,
                             std::pair<std::int32_t, std::int32_t> max_fill_fraction = {
                                 1, 2}) -> std::pair<std::int32_t, std::int32_t>;

auto compute_m_block_size(std::int32_t subgroup_size, std::int32_t max_block_size,
                          std::int32_t num_tiles, std::int64_t size) -> std::int32_t;
auto choose_block_size_multiple(std::int32_t min_block_size, std::int32_t max_block_size,
                                std::int32_t num_tiles, std::int64_t size) -> std::int32_t;
auto choose_block_size(array_view<std::int32_t> block_sizes, std::int32_t num_tiles,
                       std::int64_t size) -> std::int32_t;
auto choose_k_block_size(array_view<std::int32_t> block_sizes, std::int64_t K) -> std::int32_t;

} // namespace tinytc

#endif // GEMM_TOOLS_20241022_HPP
