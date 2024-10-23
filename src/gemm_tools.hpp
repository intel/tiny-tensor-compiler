// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef GEMM_TOOLS_20241022_HPP
#define GEMM_TOOLS_20241022_HPP

#include <cstdint>
#include <utility>

namespace tinytc {

constexpr static std::int32_t max_K_unrolling = 8;

/**
 * @brief Calculate maximum register blocking size of GEMM
 *
 * @param C_scalar_type_size_in_bytes Size of scalar type of result matrix in bytes
 * @param subgroup_size Subgroup size
 * @param register_space Size of register file per core in bytes
 * @param C_blocks Number of register blocks needed for C, usually 1, 2 for complex
 * @param max_fill_fraction Fraction of register file that shall be blocked at most
 *
 * @return {number of rows, number of columns}
 */
auto max_register_block_gemm(std::int32_t C_scalar_type_size_in_bytes, std::int32_t subgroup_size,
                             std::int32_t register_space, std::int32_t C_blocks = 1,
                             std::pair<std::int32_t, std::int32_t> max_fill_fraction = {
                                 1, 2}) -> std::pair<std::int32_t, std::int32_t>;

auto compute_m_block_size(std::int32_t subgroup_size, std::int32_t max_block_size,
                          std::int32_t num_tiles, std::int64_t size) -> std::int32_t;
auto compute_k_block_size(std::int64_t K) -> std::int32_t;

} // namespace tinytc

#endif // GEMM_TOOLS_20241022_HPP
