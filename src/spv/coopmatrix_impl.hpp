// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef COOPMATRIX_IMPL_20250415_HPP
#define COOPMATRIX_IMPL_20250415_HPP

#include "analysis/gcd.hpp"
#include "coopmatrix_layout.hpp" // IWYU pragma: keep
#include "device_info.hpp"
#include "node/inst_view.hpp"
#include "spv/defs.hpp"
#include "tinytc/types.hpp"

#include <utility>

namespace tinytc {
class coopmatrix_data_type;
} // namespace tinytc

namespace tinytc::spv {

class dope_vector;
class uniquifier;

class coopmatrix_impl {
  public:
    coopmatrix_impl(uniquifier &unique, core_config const &cfg, gcd_analysis_result g);
    virtual ~coopmatrix_impl() = default;

    inline auto gcd() const -> gcd_analysis_result const & { return gcd_; }
    inline void gcd(gcd_analysis_result g) { gcd_ = std::move(g); }

    auto cfg() const -> core_config const & { return cfg_; }
    inline void cfg(core_config const &cfg) { cfg_ = cfg; }

    virtual auto extract(cooperative_matrix_extract_inst in, spv_inst *mat) -> spv_inst *;
    virtual auto insert(cooperative_matrix_insert_inst in, spv_inst *val, spv_inst *mat)
        -> spv_inst *;
    virtual auto load(cooperative_matrix_load_inst in, dope_vector const &odv, spv_inst *operand,
                      spv_inst *pos0, spv_inst *pos1) -> spv_inst *;
    virtual auto mul_add(cooperative_matrix_mul_add_inst in, spv_inst *a, spv_inst *b, spv_inst *c)
        -> spv_inst *;
    virtual void prefetch(cooperative_matrix_prefetch_inst in, dope_vector const &odv,
                          spv_inst *pointer, spv_inst *pos0, spv_inst *pos1);
    virtual auto reduce(cooperative_matrix_reduce_inst in, spv_inst *a) -> spv_inst *;
    virtual auto scale(cooperative_matrix_scale_inst in, spv_inst *a, spv_inst *b) -> spv_inst *;
    virtual void store(cooperative_matrix_store_inst in, dope_vector const &odv, spv_inst *val,
                       spv_inst *operand, spv_inst *pos0, spv_inst *pos1);

    virtual auto arith(arith_inst in, spv_inst *a, spv_inst *b) -> spv_inst *;
    virtual auto arith_unary(arith_unary_inst in, spv_inst *a) -> spv_inst *;
    virtual auto cast(cast_inst in, spv_inst *a) -> spv_inst *;
    virtual auto constant(constant_inst in) -> spv_inst *;

    virtual auto spv_ty(coopmatrix_data_type const *ct) -> spv_inst *;

  protected:
    auto spv_interface_ty(coopmatrix_layout const &layout) -> spv_inst *;
    auto spv_storage_ty(coopmatrix_layout const &layout) -> spv_inst *;
    auto spv_ty(coopmatrix_layout const &layout) -> spv_inst *;
    auto extract(coopmatrix_layout const &layout, spv_inst *mat, LiteralInteger v) -> spv_inst *;
    auto insert(coopmatrix_layout const &layout, spv_inst *val, spv_inst *mat, LiteralInteger v)
        -> spv_inst *;

    inline auto unique() -> uniquifier & { return *unique_; }

  private:
    uniquifier *unique_;
    core_config cfg_;
    gcd_analysis_result gcd_;
};

} // namespace tinytc::spv

#endif // COOPMATRIX_IMPL_20250415_HPP
