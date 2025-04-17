// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "spv/coopmatrix_impl.hpp"
#include "codegen_tools.hpp"
#include "converter_aux.hpp"
#include "node/data_type_node.hpp"
#include "node/inst_node.hpp"
#include "spv/defs.hpp"
#include "spv/dope_vector.hpp"
#include "spv/enums.hpp"
#include "spv/instructions.hpp"
#include "spv/module.hpp"
#include "spv/uniquifier.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <algorithm>
#include <array>
#include <functional>
#include <utility>
#include <vector>

namespace tinytc::spv {

class matrix_walker {
  public:
    matrix_walker(uniquifier &unique, std::int32_t sgs, coopmatrix_layout const &layout,
                  matrix_use use, spv_inst *pos0, spv_inst *pos1, spv_inst *shape0,
                  spv_inst *shape1, spv_inst *stride0, spv_inst *stride1, checked_flag chk);

    inline void advance_block() {
        col_ = col0_;
        col_no_ = 0;
        row_ = unique_.mod().add<OpIAdd>(index_ty_, row_, row_inc_);
        ++block_no_;
    }
    inline void advance_column() {
        col_ = unique_.mod().add<OpIAdd>(index_ty_, col_, col_inc_);
        ++col_no_;
    }

    inline auto component_no(std::int32_t col_no) -> std::int32_t {
        if (transpose_) {
            return block_no_ + col_no * layout_.blocks;
        }
        return col_no + block_no_ * layout_.cols;
    }
    inline auto component_no() -> std::int32_t { return component_no(col_no_); }
    inline auto offset() -> spv_inst * { return unique_.mod().add<OpIAdd>(index_ty_, row_, col_); };
    inline auto rows_checked() -> bool {
        return chk_ == checked_flag::both || chk_ == checked_flag::rows;
    }
    inline auto cols_checked() -> bool {
        return chk_ == checked_flag::both || chk_ == checked_flag::cols;
    }
    inline auto needs_mask() -> bool { return (col_no_ + 1) * col_inc_factor_ > layout_.shape1; }
    inline auto may_need_mask() -> bool { return layout_.cols * col_inc_factor_ > layout_.shape1; }

    inline auto col_ok() -> spv_inst * {
        auto c0 = unique_.null_constant(index_ty_);
        auto bool_ty = unique_.bool_ty();
        auto &mod = unique_.mod();
        auto check1 = mod.add<OpSLessThanEqual>(bool_ty, c0, col_);
        auto check2 = mod.add<OpSLessThan>(bool_ty, col_, col_max_);
        return mod.add<OpLogicalAnd>(bool_ty, check1, check2);
    }
    inline auto row_ok() -> spv_inst * {
        auto c0 = unique_.null_constant(index_ty_);
        auto bool_ty = unique_.bool_ty();
        auto &mod = unique_.mod();
        auto check1 = mod.add<OpSLessThanEqual>(bool_ty, c0, row_);
        auto check2 = mod.add<OpSLessThan>(bool_ty, row_, row_max_);
        return mod.add<OpLogicalAnd>(bool_ty, check1, check2);
    }

  private:
    uniquifier &unique_;
    coopmatrix_layout const &layout_;
    checked_flag chk_;
    spv_inst *index_ty_;
    spv_inst *row_inc_;
    std::int64_t col_inc_factor_;
    spv_inst *col_inc_;
    spv_inst *row_;
    spv_inst *col0_;
    spv_inst *col_;
    bool transpose_;
    spv_inst *row_max_ = nullptr;
    spv_inst *col_max_ = nullptr;
    std::int32_t block_no_ = 0;
    std::int32_t col_no_ = 0;
};

matrix_walker::matrix_walker(uniquifier &unique, std::int32_t sgs, coopmatrix_layout const &layout,
                             matrix_use use, spv_inst *pos0, spv_inst *pos1, spv_inst *shape0,
                             spv_inst *shape1, spv_inst *stride0, spv_inst *stride1,
                             checked_flag chk)
    : unique_{unique}, layout_{layout}, chk_{chk} {
    index_ty_ = unique.scalar_ty(scalar_type::index);

    auto &mod = unique_.mod();
    auto crows = unique.constant(layout_.rows);
    row_inc_ = mod.add<OpIMul>(index_ty_, crows, stride0);
    col_inc_factor_ = sgs / layout.rows;
    col_inc_ = mod.add<OpIMul>(index_ty_, unique.constant(col_inc_factor_), stride1);

    spv_inst *p = unique.load_builtin(BuiltIn::SubgroupLocalInvocationId);
    p = mod.add<OpSConvert>(index_ty_, p);

    row_ = layout.rows < sgs ? mod.add<OpSRem>(index_ty_, p, crows) : p;
    row_ = mod.add<OpIAdd>(index_ty_, row_, pos0);
    row_ = mod.add<OpIMul>(index_ty_, row_, stride0);

    auto c0 = unique.null_constant(index_ty_);
    col0_ = layout.rows < sgs ? mod.add<OpSDiv>(index_ty_, p, crows) : c0;
    col0_ = mod.add<OpIAdd>(index_ty_, col0_, pos1);
    col0_ = mod.add<OpIMul>(index_ty_, col0_, stride1);
    col_ = col0_;

    transpose_ = use == matrix_use::b;

    if (rows_checked()) {
        row_max_ = mod.add<OpIMul>(index_ty_, shape0, stride0);
    }
    if (may_need_mask() || cols_checked()) {
        col_max_ = mod.add<OpIMul>(index_ty_, shape1, stride1);
    }
}

coopmatrix_impl::coopmatrix_impl(uniquifier &unique) : unique_{&unique}, sgs_{32} {}

auto coopmatrix_impl::load(cooperative_matrix_load_inst const &in, dope_vector const &odv,
                           spv_inst *operand, spv_inst *pos0, spv_inst *pos1) -> spv_inst * {
    auto ot = get_memref_type(in.operand());
    auto rt = get_coopmatrix_type(in.result(0));
    auto pointer_ty = unique_->pointer_ty(ot);

    auto layout = get_layout(rt);
    auto sty = rt->component_ty();
    auto result_ty = spv_ty(sty, layout);
    auto result_component_ty = unique_->scalar_ty(sty);

    auto shape = std::array<spv_inst *, 2u>{odv.shape(0), odv.shape(1)};
    auto stride = std::array<spv_inst *, 2u>{odv.stride(0), odv.stride(1)};
    if (in.t() == transpose::T) {
        std::swap(shape[0], shape[1]);
        std::swap(stride[0], stride[1]);
    }

    auto walker = matrix_walker(*unique_, sgs_, layout, rt->use(), pos0, pos1, shape[0], shape[1],
                                stride[0], stride[1], in.checked());

    auto &mod = unique_->mod();

    spv_inst *result = mod.add<OpUndef>(result_ty);
    for (std::int64_t w = 0; w < layout.blocks; ++w) {
        auto const ld_block = [&](tinytc_spv_mod &mod) {
            spv_inst *block_result = result;
            for (std::int64_t u = 0; u < layout.length / layout.blocks; ++u) {
                const auto ld = [&](tinytc_spv_mod &mod) {
                    auto pointer = mod.add<OpInBoundsPtrAccessChain>(
                        pointer_ty, operand, walker.offset(), std::vector<spv_inst *>{});
                    return mod.add<OpLoad>(result_component_ty, pointer);
                };
                const auto ld_chk = [&](tinytc_spv_mod &) {
                    return make_conditional_execution(
                        *unique_, result_component_ty, walker.col_ok(), ld,
                        unique_->null_constant(result_component_ty), in.loc());
                };

                spv_inst *val =
                    walker.needs_mask() || walker.cols_checked() ? ld_chk(mod) : ld(mod);
                block_result =
                    mod.add<OpCompositeInsert>(result_ty, val, block_result,
                                               std::vector<LiteralInteger>{walker.component_no()});

                if (u < layout.cols - 1) {
                    walker.advance_column();
                }
            }
            return block_result;
        };
        auto const ld_block_chk = [&](tinytc_spv_mod &) {
            auto const ld_block_zero = [&](tinytc_spv_mod &mod) {
                spv_inst *block_result = result;
                for (std::int64_t u = 0; u < layout.length / layout.blocks; ++u) {
                    block_result = mod.add<OpCompositeInsert>(
                        result_ty, unique_->null_constant(result_component_ty), block_result,
                        std::vector<LiteralInteger>{walker.component_no(u)});
                }
                return block_result;
            };
            return make_conditional_execution(*unique_, result_ty, walker.row_ok(), ld_block,
                                              ld_block_zero, in.loc());
        };
        result = walker.rows_checked() ? ld_block_chk(mod) : ld_block(mod);

        if (w < layout.blocks - 1) {
            walker.advance_block();
        }
    }
    return result;
}

void coopmatrix_impl::store(cooperative_matrix_store_inst const &in, dope_vector const &odv,
                            spv_inst *val, spv_inst *operand, spv_inst *pos0, spv_inst *pos1) {
    auto ot = get_memref_type(in.operand());
    auto vt = get_coopmatrix_type(in.val());
    auto pointer_ty = unique_->pointer_ty(ot);

    auto layout = get_layout(vt);
    auto sty = vt->component_ty();
    auto result_component_ty = unique_->scalar_ty(sty);

    auto walker = matrix_walker(*unique_, sgs_, layout, vt->use(), pos0, pos1, odv.shape(0),
                                odv.shape(1), odv.stride(0), odv.stride(1), in.checked());

    auto &mod = unique_->mod();

    for (std::int64_t w = 0; w < layout.blocks; ++w) {
        auto const st_block = [&](tinytc_spv_mod &mod) {
            for (std::int64_t u = 0; u < layout.length / layout.blocks; ++u) {
                const auto st = [&](tinytc_spv_mod &mod) {
                    auto pointer = mod.add<OpInBoundsPtrAccessChain>(
                        pointer_ty, operand, walker.offset(), std::vector<spv_inst *>{});
                    auto val_ij = mod.add<OpCompositeExtract>(
                        result_component_ty, val,
                        std::vector<LiteralInteger>{walker.component_no()});

                    make_store(*unique_, in.flag(), ot->element_ty(), ot->addrspace(), pointer,
                               val_ij, in.loc());
                };
                if (walker.needs_mask() || walker.cols_checked()) {
                    make_conditional_execution(*unique_, walker.col_ok(), st);
                } else {
                    st(mod);
                }

                if (u < layout.cols - 1) {
                    walker.advance_column();
                }
            }
        };
        if (walker.rows_checked()) {
            make_conditional_execution(*unique_, walker.row_ok(), st_block);

        } else {
            st_block(mod);
        }

        if (w < layout.blocks - 1) {
            walker.advance_block();
        }
    }
}

auto coopmatrix_impl::mul_add(cooperative_matrix_mul_add_inst const &in, spv_inst *a, spv_inst *b,
                              spv_inst *c) -> spv_inst * {
    auto at = get_coopmatrix_type(in.a());
    auto bt = get_coopmatrix_type(in.b());
    auto ct = get_coopmatrix_type(in.c());
    auto rt = get_coopmatrix_type(in.result(0));

    auto al = get_layout(at);
    auto bl = get_layout(bt);
    auto cl = get_layout(ct);
    auto rl = get_layout(rt);

    const auto a_ty = at->component_ty();
    const auto b_ty = bt->component_ty();
    // const auto b_component_ty = component_type(b_ty);
    const auto c_ty = ct->component_ty();
    const auto r_ty = rt->component_ty();
    const auto spv_a_ty = unique_->scalar_ty(a_ty);
    const auto spv_b_ty = unique_->scalar_ty(b_ty);
    // const auto spv_b_component_ty = unique_->scalar_ty(b_component_ty);
    const auto spv_c_ty = unique_->scalar_ty(c_ty);
    // const auto spv_r_ty = unique_->scalar_ty(r_ty);
    // const bool a_and_b_complex = is_complex_type(a_ty) && is_complex_type(b_ty);

    auto &mod = unique_->mod();
    auto result_ty = spv_ty(r_ty, rl);
    spv_inst *result = mod.add<OpUndef>(result_ty);

    constexpr std::int64_t nbb = 4;
    auto broadcast_scope = unique_->constant(static_cast<std::int32_t>(Scope::Subgroup));
    for (std::int64_t m_block = 0; m_block < rl.blocks; ++m_block) {
        for (std::int64_t nb = 0; nb < rl.cols; nb += nbb) {
            auto c_block = std::array<spv_inst *, nbb>{};
            for (std::int32_t n = nb; n < nb + nbb; ++n) {
                if (n < rl.cols) {
                    c_block[n - nb] = mod.add<OpCompositeExtract>(
                        spv_c_ty, c,
                        std::vector{static_cast<LiteralInteger>(n + m_block * cl.cols)});
                }
            }

            for (std::int64_t k = 0; k < bl.rows * bl.blocks; ++k) {
                auto a_mk = mod.add<OpCompositeExtract>(
                    spv_a_ty, a, std::vector{static_cast<LiteralInteger>(k + m_block * al.cols)});
                for (std::int32_t n = nb; n < nb + nbb; ++n) {
                    if (n < rl.cols) {
                        /** For matrix B we have L(i,k,j) = i + k*I + j*I*K.
                         *
                         * The k loop variable is actually indices i,k fused,
                         * and the n loop variable is equal to j. Thus,
                         * L(k,j) = k + n*I*K.
                         *
                         * We have
                         * p + vS = L
                         * Therefore,
                         * p = L%S
                         * v = L/S
                         */
                        const auto L = k + n * bl.rows * bl.blocks;
                        const auto p = unique_->constant(static_cast<std::int32_t>(L % sgs_));
                        const auto v = static_cast<LiteralInteger>(L / sgs_);

                        spv_inst *b_kn = mod.add<OpCompositeExtract>(spv_b_ty, b, std::vector{v});
                        b_kn = mod.add<OpGroupBroadcast>(spv_b_ty, broadcast_scope, b_kn, p);
                        auto &c_mn = c_block[n - nb];

                        auto ab_mn = make_binary_op_mixed_precision(
                            *unique_, c_ty, arithmetic::mul, a_ty, a_mk, b_ty, b_kn, in.loc());
                        c_mn =
                            make_binary_op(*unique_, c_ty, arithmetic::add, ab_mn, c_mn, in.loc());
                        // auto &c = result[nb + n + m_block * N];

                        // if (a_and_b_complex) {
                        // auto &c_im = result_im[nb + n + m_block * N];
                        // auto b_bc_re = mod.add<OpCompositeExtract>(
                        // spv_b_component_ty, b_bc, std::vector<LiteralInteger>{0});
                        // auto b_bc_im = mod.add<OpCompositeExtract>(
                        // spv_b_component_ty, b_bc, std::vector<LiteralInteger>{1});
                        // c = make_mixed_precision_fma(a_ty, b_component_ty, c_ty, a, b_bc_re,
                        // c, in.loc()); c_im = make_mixed_precision_fma(a_ty, b_component_ty,
                        // c_ty, a, b_bc_im, c_im, in.loc());
                        //} else {
                        // c = make_mixed_precision_fma(a_ty, b_ty, c_ty, a, b_bc, c, in.loc());
                        //}
                    }
                }
            }

            for (std::int32_t n = nb; n < nb + nbb; ++n) {
                if (n < rl.cols) {
                    auto &c_mn = c_block[n - nb];
                    if (c_ty != r_ty) {
                        c_mn = make_cast(*unique_, r_ty, c_ty, c_mn, in.loc());
                    }
                    result = mod.add<OpCompositeInsert>(
                        result_ty, c_mn, result,
                        std::vector{static_cast<LiteralInteger>(n + m_block * rl.cols)});
                }
            }
        }
    }
    return result;
}

void coopmatrix_impl::prefetch(cooperative_matrix_prefetch_inst const &, dope_vector const &,
                               spv_inst *, spv_inst *, spv_inst *) {}

auto coopmatrix_impl::scale(cooperative_matrix_scale_inst const &in, spv_inst *a, spv_inst *b)
    -> spv_inst * {
    auto rt = get_coopmatrix_type(in.result(0));
    auto layout = get_layout(rt);
    auto sty = rt->component_ty();
    auto ty = spv_ty(sty, layout);
    auto component_ty = unique_->scalar_ty(sty);

    auto &mod = unique_->mod();
    spv_inst *result = mod.add<OpUndef>(ty);
    for (LiteralInteger v = 0; v < static_cast<LiteralInteger>(layout.length); ++v) {
        auto b_v = mod.add<OpCompositeExtract>(component_ty, b, std::vector{v});
        auto r_v = make_binary_op(*unique_, sty, arithmetic::mul, a, b_v, in.loc());
        result = mod.add<OpCompositeInsert>(ty, r_v, result, std::vector{v});
    }

    return result;
}

auto coopmatrix_impl::arith(arith_inst const &in, spv_inst *a, spv_inst *b) -> spv_inst * {
    auto rt = get_coopmatrix_type(in.result(0));
    auto layout = get_layout(rt);
    auto sty = rt->component_ty();
    auto ty = spv_ty(sty, layout);
    auto component_ty = unique_->scalar_ty(sty);

    auto &mod = unique_->mod();
    spv_inst *result = mod.add<OpUndef>(ty);
    for (LiteralInteger v = 0; v < static_cast<LiteralInteger>(layout.length); ++v) {
        auto a_v = mod.add<OpCompositeExtract>(component_ty, a, std::vector{v});
        auto b_v = mod.add<OpCompositeExtract>(component_ty, b, std::vector{v});
        auto r_v = make_binary_op(*unique_, sty, in.operation(), a_v, b_v, in.loc());
        result = mod.add<OpCompositeInsert>(ty, r_v, result, std::vector{v});
    }

    return result;
}

auto coopmatrix_impl::cast(cast_inst const &in, spv_inst *a) -> spv_inst * {
    auto at = get_coopmatrix_type(in.a());
    auto rt = get_coopmatrix_type(in.result(0));
    auto layout = get_layout(rt);
    auto r_ty = rt->component_ty();
    auto ty = spv_ty(r_ty, layout);
    auto component_ty = unique_->scalar_ty(r_ty);

    auto &mod = unique_->mod();
    spv_inst *result = mod.add<OpUndef>(ty);
    for (LiteralInteger v = 0; v < static_cast<LiteralInteger>(layout.length); ++v) {
        auto a_v = mod.add<OpCompositeExtract>(component_ty, a, std::vector{v});
        auto r_v = make_cast(*unique_, r_ty, at->component_ty(), a_v, in.loc());
        result = mod.add<OpCompositeInsert>(ty, r_v, result, std::vector{v});
    }

    return result;
}

auto coopmatrix_impl::constant(constant_inst const &in) -> spv_inst * {
    auto rt = get_coopmatrix_type(in.result(0));
    auto layout = get_layout(rt);
    auto sty = rt->component_ty();
    auto spv_result_ty = spv_ty(sty, layout);

    if (in.is_zero()) {
        return unique_->null_constant(spv_result_ty);
    }

    spv_inst *cst = make_constant(*unique_, sty, in.value());
    return unique_->mod().add_to<OpConstantComposite>(section::type_const_var, spv_result_ty,
                                                      std::vector<spv_inst *>(layout.length, cst));
}

auto coopmatrix_impl::get_layout(coopmatrix_data_type const *ct) const -> coopmatrix_layout {
    auto l = coopmatrix_layout{};
    l.rows = std::min(ct->shape(0), static_cast<std::int64_t>(sgs_));
    l.cols = (1 + (l.rows * ct->shape(1) - 1) / sgs_) * sgs_ / l.rows;
    l.blocks = ct->shape(0) / l.rows;
    l.length = l.rows * l.cols * l.blocks / sgs_;
    l.shape1 = ct->shape(1);

    return l;
}

auto coopmatrix_impl::spv_ty(scalar_type sty, coopmatrix_layout const &layout) -> spv_inst * {
    return unique_->vec_ty(unique_->scalar_ty(sty), layout.length);
}

auto coopmatrix_impl::spv_ty(coopmatrix_data_type const *ct) -> spv_inst * {
    return spv_ty(ct->component_ty(), get_layout(ct));
}

} // namespace tinytc::spv
