// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef GEMM_TOOLS_20241022_HPP
#define GEMM_TOOLS_20241022_HPP

#include <cstdint>
#include <utility>

namespace tinytc {

constexpr static int max_K_unrolling = 8;

/**
 * @brief Calculate maximum register blocking size of GEMM
 *
 * @param C_scalar_type_size_in_bytes Size of scalar type of result matrix in bytes
 * @param sgs Subgroup size
 * @param register_space Size of register file per core in bytes
 * @param max_fill_fraction Fraction of register file that shall be blocked at most
 *
 * @return {number of row-blocks (block size = subgroup size), number of columns}
 */
auto max_register_block_gemm(std::int32_t C_scalar_type_size_in_bytes, std::int32_t sgs,
                             std::int32_t register_space,
                             std::pair<std::int32_t, std::int32_t> max_fill_fraction = {
                                 1, 2}) -> std::pair<std::int32_t, std::int32_t>;

} // namespace tinytc

#endif // GEMM_TOOLS_20241022_HPP
