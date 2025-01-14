// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef COOPMATRIX_DIY_20241213_HPP
#define COOPMATRIX_DIY_20241213_HPP

#include "support/fnv1a.hpp"
#include "tinytc/types.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <utility>

namespace tinytc {
class constant_inst;
class cooperative_matrix_load_inst;
class cooperative_matrix_mul_add_inst;
class cooperative_matrix_scale_inst;
class cooperative_matrix_store_inst;
class coopmatrix_data_type;
} // namespace tinytc

namespace tinytc::spv {

class dope_vector;
class spv_inst;
class uniquifier;

class coopmatrix_diy {
  public:
    constexpr static std::int32_t grf_size = 64;
    constexpr static std::int32_t exec_size = 16;
    constexpr static std::int32_t channel_size = 4;
    constexpr static std::int32_t sdepth = 8;
    constexpr static std::int32_t rcount = 8;

    coopmatrix_diy(tinytc_spv_mod &m, uniquifier &unique);

    auto constant(constant_inst const &in) -> spv_inst *;
    auto load(cooperative_matrix_load_inst const &in, dope_vector const &odv, spv_inst *pointer,
              spv_inst *pos0, spv_inst *pos1) -> spv_inst *;
    auto mul_add(cooperative_matrix_mul_add_inst const &in, spv_inst *a, spv_inst *b, spv_inst *c)
        -> spv_inst *;
    auto scale(cooperative_matrix_scale_inst const &in, spv_inst *a, spv_inst *b) -> spv_inst *;
    void store(cooperative_matrix_store_inst const &in, dope_vector const &odv, spv_inst *val,
               spv_inst *pointer, spv_inst *pos0, spv_inst *pos1);

  private:
    struct block_config {
        std::int32_t element_size;
        std::int32_t array_length;
        std::int32_t rows;
        std::int32_t cols;
        std::int32_t row_blocks;
        std::int32_t col_blocks;
        bool vnni;

        inline auto byte_offset(std::int32_t row_block, std::int32_t col_block) const
            -> std::int32_t {
            const auto block_size = element_size * array_length * rows * cols;
            return (col_block + col_blocks * row_block) * block_size;
        }
    };
    struct load_store_hash {
        inline auto operator()(std::pair<coopmatrix_data_type const *, spv_inst *> const &key) const
            -> std::size_t {
            return fnv1a_combine(key.first, key.second);
        }
    };
    struct gemm_hash {
        inline auto operator()(std::array<coopmatrix_data_type const *, 4u> const &key) const
            -> std::size_t {
            return fnv1a_combine(key[0], key[1], key[2], key[3]);
        }
    };

    auto max_rows_in_block(coopmatrix_data_type const *ct) const -> std::int32_t;
    auto load_config(coopmatrix_data_type const *ct) -> block_config;
    auto load_fun(coopmatrix_data_type const *result_ty, spv_inst *spv_operand_ty) -> spv_inst *;
    auto store_config(coopmatrix_data_type const *ct) -> block_config;
    auto store_fun(coopmatrix_data_type const *val_ty, spv_inst *spv_operand_ty) -> spv_inst *;
    auto mul_add_fun(coopmatrix_data_type const *at, coopmatrix_data_type const *bt,
                     coopmatrix_data_type const *ct, coopmatrix_data_type const *rt) -> spv_inst *;
    auto scale_fun(coopmatrix_data_type const *rt) -> spv_inst *;

    tinytc_spv_mod *mod_;
    uniquifier *unique_;
    std::unordered_map<std::pair<coopmatrix_data_type const *, spv_inst *>, spv_inst *,
                       load_store_hash>
        load_funs_, store_funs_;
    std::unordered_map<std::array<coopmatrix_data_type const *, 4u>, spv_inst *, gemm_hash>
        mul_add_funs_;
    std::unordered_map<coopmatrix_data_type const *, spv_inst *> scale_funs_;
};

} // namespace tinytc::spv

#endif // COOPMATRIX_DIY_20241213_HPP
