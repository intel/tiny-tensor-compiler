// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef COOPMATRIX_DIY_20241213_HPP
#define COOPMATRIX_DIY_20241213_HPP

#include "support/fnv1a.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>

namespace tinytc {
class arith_inst;
class cast_inst;
class constant_inst;
class cooperative_matrix_load_inst;
class cooperative_matrix_mul_add_inst;
class cooperative_matrix_scale_inst;
class cooperative_matrix_store_inst;
class coopmatrix_data_type;
enum class arithmetic;
enum class scalar_type;
} // namespace tinytc

namespace tinytc::spv {

class dope_vector;
class spv_inst;
class uniquifier;

enum class lsc_sfid { ugm, slm };

class coopmatrix_diy {
  public:
    constexpr static std::int32_t grf_size = 64;
    constexpr static std::int32_t exec_size = 16;
    constexpr static std::int32_t channel_size = 4;
    constexpr static std::int32_t sdepth = 8;
    constexpr static std::int32_t rcount = 8;
    constexpr static std::int32_t load_batch_size = 4;
    constexpr static std::int32_t store_batch_size = 1;

    coopmatrix_diy(tinytc_spv_mod &m, uniquifier &unique);

    auto arith(arith_inst const &in, spv_inst *a, spv_inst *b) -> spv_inst *;
    auto cast(cast_inst const &in, spv_inst *a) -> spv_inst *;
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
        lsc_sfid sfid;

        inline auto byte_offset(std::int32_t row_block, std::int32_t col_block,
                                std::int32_t array_idx = 0, std::int32_t row = 0,
                                std::int32_t col = 0) const -> std::int32_t {
            const auto block_size = array_length * rows * cols;
            return (row + col * rows + array_idx * rows * cols +
                    (col_block + col_blocks * row_block) * block_size) *
                   element_size;
        }
    };
    struct gemm_hash {
        inline auto operator()(std::array<coopmatrix_data_type const *, 4u> const &key) const
            -> std::size_t {
            return fnv1a_combine(key[0], key[1], key[2], key[3]);
        }
    };
    template <typename Tuple> struct tuple_hash {
        inline auto operator()(Tuple const &key) const -> std::size_t {
            return std::apply([](auto const &...args) { return fnv1a_combine(args...); }, key);
        }
    };

    auto make_tmp(char const *prefix = "") -> std::string;
    auto max_rows_in_block(coopmatrix_data_type const *ct) const -> std::int32_t;
    auto load_config(coopmatrix_data_type const *ct, address_space addrspace) -> block_config;
    auto load_fun_asm_block2d(block_config const &cfg) -> std::string;
    auto load_fun_asm_generic(block_config const &cfg, scalar_type sty) -> std::string;
    auto load_fun(coopmatrix_data_type const *result_ty, spv_inst *spv_operand_ty,
                  address_space addrspace) -> spv_inst *;
    auto store_config(coopmatrix_data_type const *ct, address_space addrspace) -> block_config;
    auto store_fun_asm_block2d(block_config const &cfg) -> std::string;
    auto store_fun_asm_generic(block_config const &cfg, scalar_type sty) -> std::string;
    auto store_fun(coopmatrix_data_type const *val_ty, spv_inst *spv_operand_ty,
                   address_space addrspace) -> spv_inst *;
    auto mul_add_fun(coopmatrix_data_type const *at, coopmatrix_data_type const *bt,
                     coopmatrix_data_type const *ct, coopmatrix_data_type const *rt) -> spv_inst *;
    auto cast_fun(scalar_type to_ty, scalar_type from_ty, std::int32_t num_components)
        -> spv_inst *;
    auto arith_fun(arithmetic op, scalar_type cty, std::int32_t num_components) -> spv_inst *;
    auto scale_fun(scalar_type cty, std::int32_t num_components) -> spv_inst *;

    tinytc_spv_mod *mod_;
    uniquifier *unique_;

    using arith_key = std::tuple<arithmetic, scalar_type, std::int32_t>;
    using cast_key = std::tuple<scalar_type, scalar_type, std::int32_t>;
    using load_store_key = std::tuple<coopmatrix_data_type const *, spv_inst *, address_space>;
    using mul_add_key = std::array<coopmatrix_data_type const *, 4u>;
    using scale_key = std::pair<scalar_type, std::int32_t>;
    std::unordered_map<arith_key, spv_inst *, tuple_hash<arith_key>> arith_funs_;
    std::unordered_map<cast_key, spv_inst *, tuple_hash<cast_key>> cast_funs_;
    std::unordered_map<load_store_key, spv_inst *, tuple_hash<load_store_key>> load_funs_,
        store_funs_;
    std::unordered_map<mul_add_key, spv_inst *, gemm_hash> mul_add_funs_;
    std::unordered_map<scale_key, spv_inst *, tuple_hash<scale_key>> scale_funs_;
    std::int64_t tmp_counter_ = 0;
};

} // namespace tinytc::spv

#endif // COOPMATRIX_DIY_20241213_HPP
