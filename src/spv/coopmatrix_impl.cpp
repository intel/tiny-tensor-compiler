// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "spv/coopmatrix_impl.hpp"
#include "codegen_tools.hpp"
#include "converter_aux.hpp"
#include "error.hpp"
#include "node/data_type_node.hpp"
#include "node/inst_node.hpp"
#include "scalar_type.hpp"
#include "spv/dope_vector.hpp"
#include "spv/enums.hpp"
#include "spv/instructions.hpp"
#include "spv/matrix_walker.hpp"
#include "spv/module.hpp"
#include "spv/uniquifier.hpp"
#include "support/visit.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <complex>
#include <cstdint>
#include <functional>
#include <iterator>
#include <utility>
#include <variant>
#include <vector>

namespace tinytc::spv {

coopmatrix_impl::coopmatrix_impl(uniquifier &unique, core_config const &cfg, gcd_analysis_result g)
    : unique_{&unique}, cfg_{cfg}, gcd_{std::move(g)} {}

auto coopmatrix_impl::extract(cooperative_matrix_extract_inst const &in, spv_inst *mat)
    -> spv_inst * {
    auto matt = get_coopmatrix_type(in.mat());
    auto matl = get_layout(cfg(), matt);
    const auto idx = in.index();
    if (idx < 0 || idx >= matl.length) {
        throw compilation_error(in.loc(), status::ir_out_of_bounds);
    }
    return extract(matl, mat, idx);
}
auto coopmatrix_impl::insert(cooperative_matrix_insert_inst const &in, spv_inst *val, spv_inst *mat)
    -> spv_inst * {
    auto matt = get_coopmatrix_type(in.mat());
    auto matl = get_layout(cfg(), matt);
    const auto idx = in.index();
    if (idx < 0 || idx >= matl.length) {
        throw compilation_error(in.loc(), status::ir_out_of_bounds);
    }
    return insert(matl, val, mat, idx);
}

auto coopmatrix_impl::load(cooperative_matrix_load_inst const &in, dope_vector const &odv,
                           spv_inst *operand, spv_inst *pos0, spv_inst *pos1) -> spv_inst * {
    auto ot = get_memref_type(in.operand());
    auto rt = get_coopmatrix_type(in.result(0));
    auto pointer_ty = unique_->pointer_ty(ot);

    auto layout = get_layout(cfg(), rt);
    auto matrix_ty = spv_ty(layout);
    auto interface_ty = spv_interface_ty(layout);

    auto shape = std::array<spv_inst *, 2u>{odv.shape(0), odv.shape(1)};
    auto stride = std::array<spv_inst *, 2u>{odv.stride(0), odv.stride(1)};
    if (in.t() == transpose::T) {
        std::swap(pos0, pos1);
        std::swap(shape[0], shape[1]);
        std::swap(stride[0], stride[1]);
    }

    auto walker = matrix_walker(*unique_, cfg().subgroup_size, layout, pos0, pos1, shape[0],
                                shape[1], stride[0], stride[1], in.checked());

    auto &mod = unique_->mod();
    spv_inst *result = mod.add<OpUndef>(matrix_ty);

    const auto ld = [&](tinytc_spv_mod &mod) -> spv_inst * {
        auto pointer = mod.add<OpInBoundsPtrAccessChain>(pointer_ty, operand, walker.offset(),
                                                         std::vector<spv_inst *>{});
        return mod.add<OpLoad>(interface_ty, pointer);
    };
    const auto ld_chk = [&](tinytc_spv_mod &) {
        return make_conditional_execution(*unique_, interface_ty, walker.col_ok(), ld,
                                          unique_->null_constant(interface_ty), in.loc());
    };
    auto const ld_block = [&](tinytc_spv_mod &mod) {
        spv_inst *block_result = result;
        for (std::int64_t u = 0; u < layout.length / layout.blocks; ++u) {
            spv_inst *val = walker.needs_mask() || walker.cols_checked() ? ld_chk(mod) : ld(mod);
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
                block_result = insert(layout, unique_->null_constant(interface_ty), block_result,
                                      walker.component_no(u));
            }
            return block_result;
        };
        return make_conditional_execution(*unique_, matrix_ty, walker.row_ok(), ld_block,
                                          ld_block_zero, in.loc());
    };

    for (std::int64_t w = 0; w < layout.blocks; ++w) {
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

    auto layout = get_layout(cfg(), vt);

    auto walker = matrix_walker(*unique_, cfg().subgroup_size, layout, pos0, pos1, odv.shape(0),
                                odv.shape(1), odv.stride(0), odv.stride(1), in.checked());

    auto &mod = unique_->mod();
    const auto st = [&](tinytc_spv_mod &mod) {
        auto pointer = mod.add<OpInBoundsPtrAccessChain>(pointer_ty, operand, walker.offset(),
                                                         std::vector<spv_inst *>{});
        auto val_ij = extract(layout, val, walker.component_no());

        make_store(*unique_, in.flag(), ot->element_ty(), ot->addrspace(), pointer, val_ij,
                   in.loc());
    };
    auto const st_block = [&](tinytc_spv_mod &mod) {
        for (std::int64_t u = 0; u < layout.length / layout.blocks; ++u) {
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

    for (std::int64_t w = 0; w < layout.blocks; ++w) {
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

    auto al = get_layout(cfg(), at);
    auto bl = get_layout(cfg(), bt);
    auto cl = get_layout(cfg(), ct);
    auto rl = get_layout(cfg(), rt);

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
                    c_block[n - nb] = extract(cl, c, cl.component_no(n, m_block));
                    if (a_and_b_complex) {
                        c_im_block[n - nb] = unique_->null_constant(spv_c_ty);
                    }
                }
            }

            for (std::int64_t k = 0; k < bl.rows * bl.blocks; ++k) {
                auto a_mk = extract(al, a, al.component_no(k, m_block));
                for (std::int32_t n = nb; n < nb + nbb; ++n) {
                    if (n < rl.cols) {
                        /** For matrix B we have L(i,k_1,j,k_2) = i + k_1*I + j*I*K_1 +
                         * k_2*I*K_1*J.
                         *
                         * The n loop variable is equal to j and the k loop variable fuses
                         * iteration over indices i,k_1,k_2, such that k = i + k_1*I +
                         * k_2*I*K_1: The k loop variable fuses iteration over indices
                         * i,k_1,k_2, We recover i+k_1*I = k%(IK_1) k_2 = k/(IK_1)
                         *
                         * We have
                         * p + vS = L
                         * Therefore,
                         * p = L%S
                         * v = L/S
                         */
                        const auto IK_1 = bl.rows * bl.blocks1;
                        const auto L = k % IK_1 + n * IK_1 + (k / IK_1) * IK_1 * bl.cols;
                        const auto p =
                            unique_->constant(static_cast<std::int32_t>(L % cfg().subgroup_size));
                        const auto v = static_cast<LiteralInteger>(L / cfg().subgroup_size);

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
    auto rl = get_layout(cfg(), rt);
    auto bl = get_layout(cfg(), get_coopmatrix_type(in.b()));
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
    auto rl = get_layout(cfg(), rt);
    auto al = get_layout(cfg(), get_coopmatrix_type(in.a()));
    auto bl = get_layout(cfg(), get_coopmatrix_type(in.b()));
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
    auto al = get_layout(cfg(), get_coopmatrix_type(in.a()));
    auto rt = get_coopmatrix_type(in.result(0));
    auto rl = get_layout(cfg(), rt);
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
    auto al = get_layout(cfg(), at);
    auto a_ty = at->component_ty();
    auto rt = get_coopmatrix_type(in.result(0));
    auto rl = get_layout(cfg(), rt);
    auto r_ty = rt->component_ty();
    auto ty = spv_ty(rl);

    auto &mod = unique_->mod();
    spv_inst *result = mod.add<OpUndef>(ty);

    const auto P =
        rt->use() == matrix_use::b && at->use() == matrix_use::acc
            ? std::function([&](LiteralInteger v) -> LiteralInteger {
                  /**
                   * Using that M >= S we have for matrix_b
                   * L_b(i,k_1,j,k_2) = i + k_1*S + j*S*K_1 + k_2*S*K_1*J.
                   *
                   * We have
                   * p_b + v_bS = L_b
                   *
                   * Recovering i,k_1,j,k_2 from L_b we have
                   *   i = L_b%S = p_b
                   * k_1 = L_b/S%K_1 = v_b%K_1
                   *   j = L_b/(SK_1)%J = v_b/K_1%J
                   * k_2 = L_b/(SK_1J) = v_b/(K_1J)
                   *
                   * Let k=k_1 + k_2K_1, and L_1, L_2 be the block sizes of matrix
                   * acc. We have L_{acc} = i + (k%L_1)*S + j*S*L_1 +
                   * (k/L_1)*S*L_1*J.
                   *
                   * Recovering p_{acc}, v_{acc} from
                   * p_{acc} + v_{acc}S = L_{acc}
                   * we have
                   * p_{acc} = L_{acc}%S = p_b
                   * v_{acc} = L_{acc}/S = k%L_1 + j*L_1 + (k/L_1)*L_1*J
                   */
                  auto const k_1 = v % rl.blocks1;
                  auto const j = v / rl.blocks1 % rl.cols;
                  auto const k_2 = v / (rl.blocks1 * rl.cols);
                  auto const k = k_1 + k_2 * rl.blocks1;
                  return k % al.blocks1 + j * al.blocks1 + (k / al.blocks1) * al.blocks1 * al.cols;
              })
            : std::function([](LiteralInteger v) -> LiteralInteger { return v; });
    for (LiteralInteger v = 0; v < static_cast<LiteralInteger>(rl.length); ++v) {
        auto a_v = extract(al, a, P(v));
        auto r_v = make_cast(*unique_, r_ty, a_ty, a_v, in.loc());
        result = insert(rl, r_v, result, v);
    }

    return result;
}

auto coopmatrix_impl::constant(constant_inst const &in) -> spv_inst * {
    auto rt = get_coopmatrix_type(in.result(0));
    auto rl = get_layout(cfg(), rt);
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
        } else if (rl.ops_per_chan > 1) {
            auto cst = std::visit(
                overloaded{[&](std::int64_t i) -> spv_inst * {
                               switch (rl.sty) {
                               case scalar_type::i8: {
                                   auto v8 =
                                       std::bit_cast<std::uint8_t>(static_cast<std::int8_t>(i));
                                   return unique_->constant(
                                       std::int32_t{v8 | (v8 << 8) | (v8 << 16) | (v8 << 24)});
                               }
                               case scalar_type::i16: {
                                   auto v16 =
                                       std::bit_cast<std::uint16_t>(static_cast<std::int16_t>(i));
                                   return unique_->constant(std::int32_t{v16 | (v16 << 16)});
                               }
                               default:
                                   return nullptr;
                               }
                           },
                           [&](double d) -> spv_inst * {
                               const float f = static_cast<float>(d);
                               switch (rl.sty) {
                               case scalar_type::bf16: {
                                   std::uint16_t v16 = bfloat16{f}.bits();
                                   return unique_->constant(std::int32_t{v16 | (v16 << 16)});
                               }
                               case scalar_type::f16: {
                                   std::uint16_t v16 = half{f}.bits();
                                   return unique_->constant(std::int32_t{v16 | (v16 << 16)});
                               }
                               default:
                                   return nullptr;
                               }
                           },
                           [&](auto const &) -> spv_inst * { return nullptr; }},
                in.value());
            if (!cst) {
                throw status::internal_compiler_error;
            }
            return std::vector<spv_inst *>(rl.length, cst);
        }
        return std::vector<spv_inst *>(rl.length, make_constant(*unique_, sty, in.value()));
    };
    return unique_->mod().add_to<OpConstantComposite>(section::type_const_var, spv_result_ty,
                                                      init_vector());
}

auto coopmatrix_impl::spv_interface_ty(coopmatrix_layout const &layout) -> spv_inst * {
    return unique_->scalar_ty(layout.sty);
}

auto coopmatrix_impl::spv_storage_ty(coopmatrix_layout const &layout) -> spv_inst * {
    if (layout.ops_per_chan > 1) {
        if (layout.ops_per_chan * size(layout.sty) != 4) {
            throw status::internal_compiler_error;
        }
        return unique_->scalar_ty(scalar_type::i32);
    }
    return unique_->scalar_ty(component_type(layout.sty));
}

auto coopmatrix_impl::spv_ty(coopmatrix_layout const &layout) -> spv_inst * {
    if (layout.length == 1) {
        return spv_interface_ty(layout);
    }
    const auto length =
        static_cast<int>(component_count(layout.sty)) * layout.length / layout.ops_per_chan;
    auto storage_ty = spv_storage_ty(layout);
    return length > 1 ? unique_->vec_ty(storage_ty, length) : storage_ty;
}

auto coopmatrix_impl::spv_ty(coopmatrix_data_type const *ct) -> spv_inst * {
    return spv_ty(get_layout(cfg(), ct));
}

auto coopmatrix_impl::extract(coopmatrix_layout const &layout, spv_inst *mat, LiteralInteger v)
    -> spv_inst * {
    if (layout.length == 1) {
        return mat;
    }
    const auto ty = spv_interface_ty(layout);
    auto &mod = unique_->mod();
    if (is_complex_type(layout.sty)) {
        const auto storage_ty = spv_storage_ty(layout);
        auto re = mod.add<OpCompositeExtract>(storage_ty, mat, std::vector{2 * v});
        auto im = mod.add<OpCompositeExtract>(storage_ty, mat, std::vector{2 * v + 1});
        return mod.add<OpCompositeConstruct>(ty, std::vector<spv_inst *>{re, im});
    } else if (layout.ops_per_chan > 1) {
        if (layout.blocks1 != 1) {
            throw status::internal_compiler_error;
        }
        const auto storage_ty = spv_storage_ty(layout);
        const auto chan_ty = unique_->vec_ty(ty, layout.ops_per_chan);
        spv_inst *val =
            layout.length > layout.ops_per_chan
                ? mod.add<OpCompositeExtract>(storage_ty, mat, std::vector{v / layout.ops_per_chan})
                : mat;
        val = mod.add<OpBitcast>(chan_ty, val);
        return mod.add<OpCompositeExtract>(ty, val, std::vector{v % layout.ops_per_chan});
    }
    return mod.add<OpCompositeExtract>(ty, mat, std::vector{v});
}
auto coopmatrix_impl::insert(coopmatrix_layout const &layout, spv_inst *val, spv_inst *mat,
                             LiteralInteger v) -> spv_inst * {
    if (layout.length == 1) {
        return val;
    }
    auto matrix_ty = spv_ty(layout);
    auto &mod = unique_->mod();
    if (is_complex_type(layout.sty)) {
        const auto storage_ty = spv_storage_ty(layout);
        auto re = mod.add<OpCompositeExtract>(storage_ty, val, std::vector{0});
        auto im = mod.add<OpCompositeExtract>(storage_ty, val, std::vector{1});
        auto tmp = mod.add<OpCompositeInsert>(matrix_ty, re, mat, std::vector{2 * v});
        return mod.add<OpCompositeInsert>(matrix_ty, im, tmp, std::vector{2 * v + 1});
    } else if (layout.ops_per_chan > 1) {
        if (layout.blocks1 != 1) {
            throw status::internal_compiler_error;
        }
        const auto storage_ty = spv_storage_ty(layout);
        const auto channels_ty = unique_->vec_ty(spv_interface_ty(layout), layout.ops_per_chan);
        const auto entry_no = std::vector{v / layout.ops_per_chan};
        spv_inst *channels = layout.length > layout.ops_per_chan
                                 ? mod.add<OpCompositeExtract>(storage_ty, mat, entry_no)
                                 : mat;
        channels = mod.add<OpBitcast>(channels_ty, channels);
        channels = mod.add<OpCompositeInsert>(channels_ty, val, channels,
                                              std::vector{v % layout.ops_per_chan});
        channels = mod.add<OpBitcast>(storage_ty, channels);
        return layout.length > layout.ops_per_chan
                   ? mod.add<OpCompositeInsert>(matrix_ty, channels, mat, entry_no)
                   : channels;
    }
    return mod.add<OpCompositeInsert>(matrix_ty, val, mat, std::vector{v});
}

} // namespace tinytc::spv
