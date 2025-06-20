// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef COOPMATRIX_IMPL_DPAS_20250428_HPP
#define COOPMATRIX_IMPL_DPAS_20250428_HPP

#include "spv/block2d_diy.hpp"
#include "spv/coopmatrix_impl_block.hpp"
#include "support/temp_counter.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"
#include "util/fnv1a.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <unordered_map>
#include <variant>

namespace tinytc {
class coopmatrix_data_type;
} // namespace tinytc

namespace tinytc::spv {

class spv_inst;

class coopmatrix_impl_dpas : public coopmatrix_impl_block {
  public:
    using coopmatrix_impl_block::coopmatrix_impl_block;

    auto load(cooperative_matrix_load_inst in, dope_vector const &odv, spv_inst *pointer,
              spv_inst *pos0, spv_inst *pos1) -> spv_inst * override;
    auto mul_add(cooperative_matrix_mul_add_inst in, spv_inst *a, spv_inst *b, spv_inst *c)
        -> spv_inst * override;
    void prefetch(cooperative_matrix_prefetch_inst in, dope_vector const &odv, spv_inst *pointer,
                  spv_inst *pos0, spv_inst *pos1) override;
    void store(cooperative_matrix_store_inst in, dope_vector const &odv, spv_inst *val,
               spv_inst *pointer, spv_inst *pos0, spv_inst *pos1) override;
    auto reduce(cooperative_matrix_reduce_inst in, spv_inst *a) -> spv_inst * override;

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
    auto check_2d_block_io(tinytc_value const &operand, tinytc_value const &pos0) -> bool;
    auto load_config(scalar_type sty, std::int32_t rows, std::int32_t cols, matrix_use use,
                     transpose trans, std::int32_t cache_level = -1) -> block_config;
    auto load_fun(coopmatrix_data_type const *result_ty, spv_inst *spv_operand_ty, transpose trans)
        -> spv_inst *;
    auto prefetch_fun(std::int32_t cache_level, scalar_type sty, spv_inst *spv_operand_ty,
                      std::int32_t rows, std::int32_t cols) -> spv_inst *;
    auto store_config(coopmatrix_data_type const *ct) -> block_config;
    auto store_fun(coopmatrix_data_type const *val_ty, spv_inst *spv_operand_ty) -> spv_inst *;
    auto mul_add_fun(coopmatrix_data_type const *at, coopmatrix_data_type const *bt,
                     coopmatrix_data_type const *ct, coopmatrix_data_type const *rt, bool is_c_zero)
        -> spv_inst *;
    auto reduce_fun(std::int32_t sgs, group_arithmetic arith, coopmatrix_data_type const *at,
                    coopmatrix_data_type const *rt) -> spv_inst *;

    using load_key = std::tuple<coopmatrix_data_type const *, spv_inst *, transpose>;
    using prefetch_key =
        std::tuple<std::int32_t, scalar_type, spv_inst *, std::int32_t, std::int32_t>;
    using store_key = std::tuple<coopmatrix_data_type const *, spv_inst *>;
    using reduce_key = std::tuple<std::int32_t, group_arithmetic, coopmatrix_data_type const *,
                                  coopmatrix_data_type const *>;
    std::unordered_map<load_key, spv_inst *, tuple_hash<load_key>> load_funs_;
    std::unordered_map<prefetch_key, spv_inst *, tuple_hash<prefetch_key>> prefetch_funs_;
    std::unordered_map<store_key, spv_inst *, tuple_hash<store_key>> store_funs_;
    std::unordered_map<mul_add_key, spv_inst *, mul_add_hash> mul_add_funs_;
    std::unordered_map<reduce_key, spv_inst *, tuple_hash<reduce_key>> reduce_funs_;
    temp_counter tmp_;
};

} // namespace tinytc::spv

#endif // COOPMATRIX_IMPL_DPAS_20250428_HPP
