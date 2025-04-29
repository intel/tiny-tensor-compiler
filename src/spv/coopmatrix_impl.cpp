// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "spv/coopmatrix_impl.hpp"
#include "codegen_tools.hpp"
#include "converter_aux.hpp"
#include "node/data_type_node.hpp"
#include "node/inst_node.hpp"
#include "scalar_type.hpp"
#include "spv/dope_vector.hpp"
#include "spv/enums.hpp"
#include "spv/instructions.hpp"
#include "spv/matrix_walker.hpp"
#include "spv/module.hpp"
#include "spv/uniquifier.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <algorithm>
#include <array>
#include <complex>
#include <functional>
#include <iterator>
#include <utility>
#include <variant>
#include <vector>

namespace tinytc::spv {

coopmatrix_impl::coopmatrix_impl(tinytc_core_info const &info, uniquifier &unique)
    : info_{&info}, unique_{&unique}, sgs_{32} {}

auto coopmatrix_impl::load(cooperative_matrix_load_inst const &in, dope_vector const &odv,
                           spv_inst *operand, spv_inst *pos0, spv_inst *pos1) -> spv_inst * {
    auto ot = get_memref_type(in.operand());
    auto rt = get_coopmatrix_type(in.result(0));
    auto pointer_ty = unique_->pointer_ty(ot);

    auto layout = get_layout(rt);
    auto sty = rt->component_ty();
    auto result_ty = spv_ty(layout);
    auto result_component_ty = unique_->scalar_ty(sty);

    auto shape = std::array<spv_inst *, 2u>{odv.shape(0), odv.shape(1)};
    auto stride = std::array<spv_inst *, 2u>{odv.stride(0), odv.stride(1)};
    if (in.t() == transpose::T) {
        std::swap(pos0, pos1);
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
                block_result = insert(layout, val, block_result, walker.component_no());

                if (u < layout.cols - 1) {
                    walker.advance_column();
                }
            }
            return block_result;
        };
        auto const ld_block_chk = [&](tinytc_spv_mod &) {
            auto const ld_block_zero = [&](tinytc_spv_mod &) {
                spv_inst *block_result = result;
                for (std::int64_t u = 0; u < layout.length / layout.blocks; ++u) {
                    block_result = insert(layout, unique_->null_constant(result_component_ty),
                                          block_result, walker.component_no(u));
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

    auto walker = matrix_walker(*unique_, sgs_, layout, vt->use(), pos0, pos1, odv.shape(0),
                                odv.shape(1), odv.stride(0), odv.stride(1), in.checked());

    auto &mod = unique_->mod();

    for (std::int64_t w = 0; w < layout.blocks; ++w) {
        auto const st_block = [&](tinytc_spv_mod &mod) {
            for (std::int64_t u = 0; u < layout.length / layout.blocks; ++u) {
                const auto st = [&](tinytc_spv_mod &mod) {
                    auto pointer = mod.add<OpInBoundsPtrAccessChain>(
                        pointer_ty, operand, walker.offset(), std::vector<spv_inst *>{});
                    auto val_ij = extract(layout, val, walker.component_no());

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
    const auto b_component_ty = component_type(b_ty);
    const auto c_ty = ct->component_ty();
    const auto r_ty = rt->component_ty();
    const auto spv_b_ty = unique_->scalar_ty(b_ty);
    const auto spv_b_component_ty = unique_->scalar_ty(b_component_ty);
    const auto spv_c_ty = unique_->scalar_ty(c_ty);
    const bool a_and_b_complex = is_complex_type(a_ty) && is_complex_type(b_ty);

    auto &mod = unique_->mod();
    auto result_ty = spv_ty(rl);
    spv_inst *result = mod.add<OpUndef>(result_ty);
    spv_inst *imaginary_unit =
        a_and_b_complex ? make_constant(*unique_, c_ty, std::complex<double>{0.0, 1.0}) : nullptr;

    constexpr std::int64_t nbb = 4;
    auto broadcast_scope = unique_->constant(static_cast<std::int32_t>(Scope::Subgroup));
    for (std::int64_t m_block = 0; m_block < rl.blocks; ++m_block) {
        for (std::int64_t nb = 0; nb < rl.cols; nb += nbb) {
            auto c_block = std::array<spv_inst *, nbb>{};
            auto c_im_block = std::array<spv_inst *, nbb>{};
            std::fill(std::begin(c_block), std::end(c_block), nullptr);
            std::fill(std::begin(c_im_block), std::end(c_im_block), nullptr);
            for (std::int32_t n = nb; n < nb + nbb; ++n) {
                if (n < rl.cols) {
                    c_block[n - nb] = extract(cl, c, n + m_block * cl.cols);
                    if (a_and_b_complex) {
                        c_im_block[n - nb] = unique_->null_constant(spv_c_ty);
                    }
                }
            }

            for (std::int64_t k = 0; k < bl.rows * bl.blocks; ++k) {
                auto a_mk = extract(al, a, k + m_block * al.cols);
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

                        spv_inst *b_kn = extract(bl, b, v);
                        b_kn = mod.add<OpGroupBroadcast>(spv_b_ty, broadcast_scope, b_kn, p);
                        auto &c_mn = c_block[n - nb];

                        if (a_and_b_complex) {
                            auto &c_im_mn = c_im_block[n - nb];
                            auto b_kn_re = mod.add<OpCompositeExtract>(
                                spv_b_component_ty, b_kn, std::vector<LiteralInteger>{0});
                            auto b_kn_im = mod.add<OpCompositeExtract>(
                                spv_b_component_ty, b_kn, std::vector<LiteralInteger>{1});

                            auto ab_mn = make_binary_op_mixed_precision(
                                *unique_, c_ty, arithmetic::mul, a_ty, a_mk, b_component_ty,
                                b_kn_re, in.loc());
                            c_mn = make_binary_op(*unique_, c_ty, arithmetic::add, ab_mn, c_mn,
                                                  in.loc());
                            auto ab_im_mn = make_binary_op_mixed_precision(
                                *unique_, c_ty, arithmetic::mul, a_ty, a_mk, b_component_ty,
                                b_kn_im, in.loc());
                            c_im_mn = make_binary_op(*unique_, c_ty, arithmetic::add, ab_im_mn,
                                                     c_im_mn, in.loc());
                        } else {
                            auto ab_mn = make_binary_op_mixed_precision(
                                *unique_, c_ty, arithmetic::mul, a_ty, a_mk, b_ty, b_kn, in.loc());
                            c_mn = make_binary_op(*unique_, c_ty, arithmetic::add, ab_mn, c_mn,
                                                  in.loc());
                        }
                    }
                }
            }
            if (a_and_b_complex) {
                for (std::int32_t n = nb; n < nb + nbb; ++n) {
                    if (n < rl.cols) {
                        auto &c_mn = c_block[n - nb];
                        auto &c_im_mn = c_im_block[n - nb];
                        auto c_im_mn_times_i = make_binary_op(*unique_, c_ty, arithmetic::mul,
                                                              c_im_mn, imaginary_unit, in.loc());
                        c_mn = make_binary_op(*unique_, c_ty, arithmetic::add, c_mn,
                                              c_im_mn_times_i, in.loc());
                    }
                }
            }
            for (std::int32_t n = nb; n < nb + nbb; ++n) {
                if (n < rl.cols) {
                    auto &c_mn = c_block[n - nb];
                    if (c_ty != r_ty) {
                        c_mn = make_cast(*unique_, r_ty, c_ty, c_mn, in.loc());
                    }
                    result = insert(cl, c_mn, result, n + m_block * rl.cols);
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
    auto rl = get_layout(rt);
    auto bl = get_layout(get_coopmatrix_type(in.b()));
    auto sty = rt->component_ty();
    auto ty = spv_ty(rl);

    auto &mod = unique_->mod();
    spv_inst *result = mod.add<OpUndef>(ty);
    for (LiteralInteger v = 0; v < static_cast<LiteralInteger>(rl.length); ++v) {
        auto b_v = extract(bl, b, v);
        auto r_v = make_binary_op(*unique_, sty, arithmetic::mul, a, b_v, in.loc());
        result = insert(rl, r_v, result, v);
    }

    return result;
}

auto coopmatrix_impl::arith(arith_inst const &in, spv_inst *a, spv_inst *b) -> spv_inst * {
    auto rt = get_coopmatrix_type(in.result(0));
    auto rl = get_layout(rt);
    auto al = get_layout(get_coopmatrix_type(in.a()));
    auto bl = get_layout(get_coopmatrix_type(in.b()));
    auto sty = rt->component_ty();
    auto ty = spv_ty(rl);

    auto &mod = unique_->mod();
    spv_inst *result = mod.add<OpUndef>(ty);
    for (LiteralInteger v = 0; v < static_cast<LiteralInteger>(rl.length); ++v) {
        auto a_v = extract(al, a, v);
        auto b_v = extract(bl, b, v);
        auto r_v = make_binary_op(*unique_, sty, in.operation(), a_v, b_v, in.loc());
        result = insert(rl, r_v, result, v);
    }

    return result;
}

auto coopmatrix_impl::arith_unary(arith_unary_inst const &in, spv_inst *a) -> spv_inst * {
    auto al = get_layout(get_coopmatrix_type(in.a()));
    auto rt = get_coopmatrix_type(in.result(0));
    auto rl = get_layout(rt);
    auto sty = rt->component_ty();
    auto ty = spv_ty(rl);

    auto &mod = unique_->mod();
    spv_inst *result = mod.add<OpUndef>(ty);
    for (LiteralInteger v = 0; v < static_cast<LiteralInteger>(rl.length); ++v) {
        auto a_v = extract(al, a, v);
        auto r_v = make_unary_op(*unique_, sty, in.operation(), a_v, in.loc());
        result = insert(rl, r_v, result, v);
    }

    return result;
}

auto coopmatrix_impl::cast(cast_inst const &in, spv_inst *a) -> spv_inst * {
    auto at = get_coopmatrix_type(in.a());
    auto al = get_layout(at);
    auto rt = get_coopmatrix_type(in.result(0));
    auto rl = get_layout(rt);
    auto r_ty = rt->component_ty();
    auto ty = spv_ty(rl);

    auto &mod = unique_->mod();
    spv_inst *result = mod.add<OpUndef>(ty);

    const auto P = rt->use() == matrix_use::b && at->use() == matrix_use::acc
                       ? std::function([&](LiteralInteger v) -> LiteralInteger {
                             /**
                              * Using that M >= S we have
                              * For matrix_acc we have L_{acc}(i,j,k) = i + j*S + k*S*J
                              * For matrix_b   we have     L_b(i,k,j) = i + k*S + j*S*K.
                              *
                              * We have
                              * p_b + v_bS = L_b
                              *
                              * Recovering i,j,k from L_b we have
                              * i = L_b%S = p_b
                              * k = L_b/S%K = v_b%K
                              * j = L_b/(SK) = v_b/K
                              *
                              * Recovering p_{acc}, v_{acc} from
                              * p_{acc} + v_{acc}S = L_{acc} = p_b + v_b/K*S + v_b%K*S*J
                              * we have
                              * p_{acc} = L_{acc}%S = p_b
                              * v_{acc} = L_{acc}/S = v_b/K + v_b%K*J
                              */
                             return v / rl.blocks + v % rl.blocks * al.cols;
                         })
                       : std::function([](LiteralInteger v) -> LiteralInteger { return v; });

    for (LiteralInteger v = 0; v < static_cast<LiteralInteger>(rl.length); ++v) {
        auto a_v = extract(al, a, P(v));
        auto r_v = make_cast(*unique_, r_ty, at->component_ty(), a_v, in.loc());
        result = insert(rl, r_v, result, v);
    }

    return result;
}

auto coopmatrix_impl::constant(constant_inst const &in) -> spv_inst * {
    auto rt = get_coopmatrix_type(in.result(0));
    auto rl = get_layout(rt);
    auto sty = rt->component_ty();
    auto spv_result_ty = spv_ty(rl);

    if (in.is_zero()) {
        return unique_->null_constant(spv_result_ty);
    }
    if (rl.length == 1) {
        return make_constant(*unique_, sty, in.value());
    }

    auto const init_vector = [&]() {
        if (is_complex_type(sty)) {
            const auto c = std::get<std::complex<double>>(in.value());
            auto cty = component_type(sty);
            auto re = make_constant(*unique_, cty, c.real());
            auto im = make_constant(*unique_, cty, c.imag());
            auto vec = std::vector<spv_inst *>(2 * rl.length);
            for (std::int64_t v = 0; v < rl.length; ++v) {
                vec[2 * v] = re;
                vec[2 * v + 1] = im;
            }
            return vec;
        }
        return std::vector<spv_inst *>(rl.length, make_constant(*unique_, sty, in.value()));
    };
    return unique_->mod().add_to<OpConstantComposite>(section::type_const_var, spv_result_ty,
                                                      init_vector());
}

auto coopmatrix_impl::get_layout(coopmatrix_data_type const *ct) const -> coopmatrix_layout {
    auto l = coopmatrix_layout{};
    l.rows = std::min(ct->shape(0), static_cast<std::int64_t>(sgs_));
    l.cols = (1 + (l.rows * ct->shape(1) - 1) / sgs_) * sgs_ / l.rows;
    l.blocks = ct->shape(0) / l.rows;
    l.length = l.rows * l.cols * l.blocks / sgs_;
    l.shape1 = ct->shape(1);
    l.ops_per_chan = 1;
    l.sty = ct->component_ty();

    return l;
}

auto coopmatrix_impl::spv_ty(coopmatrix_layout const &layout) -> spv_inst * {
    if (layout.length == 1) {
        return unique_->scalar_ty(layout.sty);
    }
    const auto ty = unique_->scalar_ty(component_type(layout.sty));
    const auto length = static_cast<int>(component_count(layout.sty)) * layout.length;
    return unique_->vec_ty(ty, length);
}

auto coopmatrix_impl::spv_ty(coopmatrix_data_type const *ct) -> spv_inst * {
    return spv_ty(get_layout(ct));
}

auto coopmatrix_impl::extract(coopmatrix_layout const &layout, spv_inst *mat, LiteralInteger v)
    -> spv_inst * {
    if (layout.length == 1) {
        return mat;
    }
    const auto ty = unique_->scalar_ty(component_type(layout.sty));
    if (is_complex_type(layout.sty)) {
        auto re = unique_->mod().add<OpCompositeExtract>(ty, mat, std::vector{2 * v});
        auto im = unique_->mod().add<OpCompositeExtract>(ty, mat, std::vector{2 * v + 1});
        const auto complex_ty = unique_->scalar_ty(layout.sty);
        return unique_->mod().add<OpCompositeConstruct>(complex_ty,
                                                        std::vector<spv_inst *>{re, im});
    }
    return unique_->mod().add<OpCompositeExtract>(ty, mat, std::vector{v});
}
auto coopmatrix_impl::insert(coopmatrix_layout const &layout, spv_inst *val, spv_inst *mat,
                             LiteralInteger v) -> spv_inst * {
    if (layout.length == 1) {
        return val;
    }
    auto ty = spv_ty(layout);
    if (is_complex_type(layout.sty)) {
        const auto real_ty = unique_->scalar_ty(component_type(layout.sty));
        auto re = unique_->mod().add<OpCompositeExtract>(real_ty, val, std::vector{0});
        auto im = unique_->mod().add<OpCompositeExtract>(real_ty, val, std::vector{1});
        auto tmp = unique_->mod().add<OpCompositeInsert>(ty, re, mat, std::vector{2 * v});
        return unique_->mod().add<OpCompositeInsert>(ty, im, tmp, std::vector{2 * v + 1});
    }
    return unique_->mod().add<OpCompositeInsert>(ty, val, mat, std::vector{v});
}

} // namespace tinytc::spv
