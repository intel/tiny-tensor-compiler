// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef COOPMATRIX_IMPL_DPAS_20250428_HPP
#define COOPMATRIX_IMPL_DPAS_20250428_HPP

#include "spv/block2d_diy.hpp"
#include "spv/coopmatrix_impl_block.hpp"
#include "spv/coopmatrix_layout.hpp"
#include "support/fnv1a.hpp"
#include "support/temp_counter.hpp"
#include "tinytc/types.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <variant>

namespace tinytc {
class coopmatrix_data_type;
} // namespace tinytc

namespace tinytc::spv {

class spv_inst;

class coopmatrix_impl_dpas : public coopmatrix_impl_block {
  public:
    using coopmatrix_impl_block::coopmatrix_impl_block;

    // auto arith(arith_inst const &in, spv_inst *a, spv_inst *b) -> spv_inst *;
    // auto cast(cast_inst const &in, spv_inst *a) -> spv_inst *;
    // auto constant(constant_inst const &in) -> spv_inst *;
    auto load(cooperative_matrix_load_inst const &in, dope_vector const &odv, spv_inst *pointer,
              spv_inst *pos0, spv_inst *pos1) -> spv_inst * override;
    auto mul_add(cooperative_matrix_mul_add_inst const &in, spv_inst *a, spv_inst *b, spv_inst *c)
        -> spv_inst * override;
    void prefetch(cooperative_matrix_prefetch_inst const &in, dope_vector const &odv,
                  spv_inst *pointer, spv_inst *pos0, spv_inst *pos1) override;
    // auto scale(cooperative_matrix_scale_inst const &in, spv_inst *a, spv_inst *b) -> spv_inst *;
    void store(cooperative_matrix_store_inst const &in, dope_vector const &odv, spv_inst *val,
               spv_inst *pointer, spv_inst *pos0, spv_inst *pos1) override;

  private:
    struct mul_add_key {
        std::array<coopmatrix_data_type const *, 4u> op_ty;
        bool is_c_zero;

        auto operator==(mul_add_key const &other) const {
            return op_ty == other.op_ty && is_c_zero == other.is_c_zero;
        }
    };
    struct mul_add_hash {
        inline auto operator()(mul_add_key const &key) const -> std::size_t {
            return fnv1a_combine(key.op_ty[0], key.op_ty[1], key.op_ty[2], key.op_ty[3],
                                 key.is_c_zero);
        }
    };
    template <typename Tuple> struct tuple_hash {
        inline auto operator()(Tuple const &key) const -> std::size_t {
            return std::apply([](auto const &...args) { return fnv1a_combine(args...); }, key);
        }
    };

    auto max_rows_in_block(matrix_use use, std::int32_t element_size) const -> std::int32_t;
    auto load_config(scalar_type sty, std::int32_t rows, std::int32_t cols, matrix_use use,
                     transpose trans, address_space addrspace, std::int32_t cache_level = -1)
        -> block_config;
    auto load_fun(coopmatrix_data_type const *result_ty, spv_inst *spv_operand_ty, transpose trans,
                  address_space addrspace) -> spv_inst *;
    auto prefetch_fun(std::int32_t cache_level, scalar_type sty, spv_inst *spv_operand_ty,
                      std::int32_t rows, std::int32_t cols, address_space addrspace) -> spv_inst *;
    auto store_config(coopmatrix_data_type const *ct, address_space addrspace) -> block_config;
    auto store_fun(coopmatrix_data_type const *val_ty, spv_inst *spv_operand_ty,
                   address_space addrspace) -> spv_inst *;
    auto mul_add_fun(coopmatrix_data_type const *at, coopmatrix_data_type const *bt,
                     coopmatrix_data_type const *ct, coopmatrix_data_type const *rt, bool is_c_zero)
        -> spv_inst *;
    auto vnni_transform_fun(coopmatrix_layout const &layout) -> spv_inst *;
    // auto cast_fun(scalar_type to_ty, matrix_use to_use, scalar_type from_ty, matrix_use from_use,
    // std::int32_t rows, std::int32_t cols) -> spv_inst *;
    // auto arith_fun(arithmetic op, scalar_type cty, std::int32_t num_components) -> spv_inst *;
    // auto scale_fun(scalar_type cty, std::int32_t num_components) -> spv_inst *;

    using arith_key = std::tuple<arithmetic, scalar_type, std::int32_t>;
    using cast_key =
        std::tuple<scalar_type, matrix_use, scalar_type, matrix_use, std::int32_t, std::int32_t>;
    using load_key = std::tuple<coopmatrix_data_type const *, spv_inst *, transpose, address_space>;
    using prefetch_key = std::tuple<std::int32_t, scalar_type, spv_inst *, std::int32_t,
                                    std::int32_t, address_space>;
    using store_key = std::tuple<coopmatrix_data_type const *, spv_inst *, address_space>;
    using scale_key = std::pair<scalar_type, std::int32_t>;
    std::unordered_map<arith_key, spv_inst *, tuple_hash<arith_key>> arith_funs_;
    std::unordered_map<cast_key, spv_inst *, tuple_hash<cast_key>> cast_funs_;
    std::unordered_map<load_key, spv_inst *, tuple_hash<load_key>> load_funs_;
    std::unordered_map<prefetch_key, spv_inst *, tuple_hash<prefetch_key>> prefetch_funs_;
    std::unordered_map<store_key, spv_inst *, tuple_hash<store_key>> store_funs_;
    std::unordered_map<mul_add_key, spv_inst *, mul_add_hash> mul_add_funs_;
    std::unordered_map<scale_key, spv_inst *, tuple_hash<scale_key>> scale_funs_;
    std::unordered_map<coopmatrix_layout, spv_inst *> vnni_transform_funs_;
    temp_counter tmp_;
};

} // namespace tinytc::spv

#endif // COOPMATRIX_IMPL_DPAS_20250428_HPP
