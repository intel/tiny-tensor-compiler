// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef COOPMATRIX_IMPL_20250415_HPP
#define COOPMATRIX_IMPL_20250415_HPP

#include "analysis/gcd.hpp"
#include "spv/coopmatrix_layout.hpp"
#include "spv/defs.hpp"

#include <cstdint>
#include <functional>
#include <utility>

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
} // namespace tinytc

namespace tinytc::spv {

class dope_vector;
class uniquifier;

class coopmatrix_impl {
  public:
    coopmatrix_impl(tinytc_core_info const &info, uniquifier &unique);
    virtual ~coopmatrix_impl() = default;

    inline auto info() const -> tinytc_core_info const & { return *info_; }

    inline auto gcd() const -> gcd_analysis_result const & { return gcd_; }
    inline void gcd(gcd_analysis_result g) { gcd_ = std::move(g); }

    inline auto subgroup_size() const -> std::int32_t { return sgs_; }
    inline void subgroup_size(std::int32_t sgs) { sgs_ = sgs; }

    virtual auto load(cooperative_matrix_load_inst const &in, dope_vector const &odv,
                      spv_inst *operand, spv_inst *pos0, spv_inst *pos1) -> spv_inst *;
    virtual auto mul_add(cooperative_matrix_mul_add_inst const &in, spv_inst *a, spv_inst *b,
                         spv_inst *c) -> spv_inst *;
    virtual void prefetch(cooperative_matrix_prefetch_inst const &in, dope_vector const &odv,
                          spv_inst *pointer, spv_inst *pos0, spv_inst *pos1);
    virtual auto scale(cooperative_matrix_scale_inst const &in, spv_inst *a, spv_inst *b)
        -> spv_inst *;
    virtual void store(cooperative_matrix_store_inst const &in, dope_vector const &odv,
                       spv_inst *val, spv_inst *operand, spv_inst *pos0, spv_inst *pos1);

    virtual auto arith(arith_inst const &in, spv_inst *a, spv_inst *b) -> spv_inst *;
    virtual auto arith_unary(arith_unary_inst const &in, spv_inst *a) -> spv_inst *;
    virtual auto cast(cast_inst const &in, spv_inst *a) -> spv_inst *;
    virtual auto constant(constant_inst const &in) -> spv_inst *;

    virtual auto spv_ty(coopmatrix_data_type const *ct) -> spv_inst *;

  protected:
    virtual auto get_layout(coopmatrix_data_type const *ct) const -> coopmatrix_layout;
    auto spv_interface_ty(coopmatrix_layout const &layout) -> spv_inst *;
    auto spv_storage_ty(coopmatrix_layout const &layout) -> spv_inst *;
    auto spv_ty(coopmatrix_layout const &layout) -> spv_inst *;
    auto extract(coopmatrix_layout const &layout, spv_inst *mat, LiteralInteger v) -> spv_inst *;
    auto insert(coopmatrix_layout const &layout, spv_inst *val, spv_inst *mat, LiteralInteger v)
        -> spv_inst *;
    auto apply_function(coopmatrix_layout const &layout,
                        std::function<spv_inst *(spv_inst *, spv_inst *)> fun, spv_inst *a,
                        spv_inst *b = nullptr) -> spv_inst *;

    inline auto unique() -> uniquifier & { return *unique_; }

  private:
    const_tinytc_core_info_t info_;
    uniquifier *unique_;
    std::int32_t sgs_;
    gcd_analysis_result gcd_;
};

} // namespace tinytc::spv

#endif // COOPMATRIX_IMPL_20250415_HPP
