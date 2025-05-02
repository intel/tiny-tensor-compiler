// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef BLOCK2D_DIY_20250219_HPP
#define BLOCK2D_DIY_20250219_HPP

#include <array>
#include <cstdint>
#include <optional>
#include <string>

namespace tinytc {
class temp_counter;
enum class scalar_type;
} // namespace tinytc

namespace tinytc::spv {

struct block_config {
    scalar_type sty;
    std::int32_t element_size;
    std::int32_t array_length;
    std::int32_t rows;
    std::int32_t cols;
    std::int32_t row_blocks;
    std::int32_t col_blocks;
    bool transpose;
    bool vnni;
    std::int32_t pos0_shr; // Number of bits to shift pos0 to the right (= divide by 2^pos0_shr)
    std::int32_t cache_level;

    auto block_size_in_bytes() const -> std::int32_t;
    auto block_size_in_num_grf() const -> std::int32_t;
    auto byte_offset(std::int32_t row, std::int32_t col, std::int32_t array_idx,
                     std::int32_t col_block, std::int32_t row_block) const -> std::int32_t;
    auto origin(std::int32_t row, std::int32_t col, std::int32_t array_idx, std::int32_t col_block,
                std::int32_t row_block) const -> std::array<std::int32_t, 2u>;
    auto total_rows() const -> std::int32_t;
};

auto lsc_data_size(std::int32_t element_size) -> std::int32_t;
auto region_origin(std::int32_t element_size, std::int32_t byte_offset)
    -> std::array<std::int32_t, 2u>;
auto visa_type(scalar_type sty) -> char const *;

auto load_block2d_native(block_config const &cfg, temp_counter &make_tmp) -> std::string;
auto prefetch_block2d_native(block_config const &cfg, temp_counter &make_tmp) -> std::string;
auto store_block2d_native(block_config const &cfg, temp_counter &make_tmp) -> std::string;

} // namespace tinytc::spv

#endif // BLOCK2D_DIY_20250219_HPP
