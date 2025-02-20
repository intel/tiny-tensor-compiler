// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef COOPMATRIX_DIY_20241213_HPP
#define COOPMATRIX_DIY_20241213_HPP

#include "spv/block2d_diy.hpp"
#include "support/fnv1a.hpp"
#include "support/temp_counter.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
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
} // namespace tinytc

namespace tinytc::spv {

class dope_vector;
class spv_inst;
class uniquifier;

class coopmatrix_diy {
  public:
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

    auto max_rows_in_block(coopmatrix_data_type const *ct) const -> std::int32_t;
    auto load_config(coopmatrix_data_type const *ct, address_space addrspace) -> block_config;
    auto load_fun(coopmatrix_data_type const *result_ty, spv_inst *spv_operand_ty,
                  address_space addrspace) -> spv_inst *;
    auto store_config(coopmatrix_data_type const *ct, address_space addrspace) -> block_config;
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
    temp_counter tmp_;
};

} // namespace tinytc::spv

#endif // COOPMATRIX_DIY_20241213_HPP
