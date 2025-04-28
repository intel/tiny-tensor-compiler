// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef COOPMATRIX_IMPL_20250415_HPP
#define COOPMATRIX_IMPL_20250415_HPP

#include "spv/defs.hpp"

#include <cstdint>

namespace tinytc {
class arith_inst;
class arith_unary_inst;
class cast_inst;
class constant_inst;
class cooperative_matrix_load_inst;
class cooperative_matrix_mul_add_inst;
class cooperative_matrix_prefetch_inst;
class cooperative_matrix_scale_inst;
class cooperative_matrix_store_inst;
class coopmatrix_data_type;
enum class scalar_type;
} // namespace tinytc

namespace tinytc::spv {

class dope_vector;
class uniquifier;

struct coopmatrix_layout {
    std::int64_t rows, cols, blocks, length, shape1;
    scalar_type sty;
};

class coopmatrix_impl {
  public:
    coopmatrix_impl(uniquifier &unique);

    inline auto subgroup_size() const -> std::int32_t { return sgs_; }
    inline void subgroup_size(std::int32_t sgs) { sgs_ = sgs; }

    auto load(cooperative_matrix_load_inst const &in, dope_vector const &odv, spv_inst *operand,
              spv_inst *pos0, spv_inst *pos1) -> spv_inst *;
    auto mul_add(cooperative_matrix_mul_add_inst const &in, spv_inst *a, spv_inst *b, spv_inst *c)
        -> spv_inst *;
    void prefetch(cooperative_matrix_prefetch_inst const &in, dope_vector const &odv,
                  spv_inst *pointer, spv_inst *pos0, spv_inst *pos1);
    auto scale(cooperative_matrix_scale_inst const &in, spv_inst *a, spv_inst *b) -> spv_inst *;
    void store(cooperative_matrix_store_inst const &in, dope_vector const &odv, spv_inst *val,
               spv_inst *operand, spv_inst *pos0, spv_inst *pos1);

    auto arith(arith_inst const &in, spv_inst *a, spv_inst *b) -> spv_inst *;
    auto arith_unary(arith_unary_inst const &in, spv_inst *a) -> spv_inst *;
    auto cast(cast_inst const &in, spv_inst *a) -> spv_inst *;
    auto constant(constant_inst const &in) -> spv_inst *;

    auto spv_ty(coopmatrix_data_type const *ct) -> spv_inst *;

  private:
    auto get_layout(coopmatrix_data_type const *ct) const -> coopmatrix_layout;
    auto spv_ty(coopmatrix_layout const &layout) -> spv_inst *;
    auto extract(coopmatrix_layout const &layout, spv_inst *mat, LiteralInteger v) -> spv_inst *;
    auto insert(coopmatrix_layout const &layout, spv_inst *val, spv_inst *mat, LiteralInteger v)
        -> spv_inst *;

    uniquifier *unique_;
    std::int32_t sgs_;
};

} // namespace tinytc::spv

#endif // COOPMATRIX_IMPL_20250415_HPP
