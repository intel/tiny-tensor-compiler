// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "gemm_generator.hpp"
#include "codegen_tools.hpp"
#include "device_info.hpp"
#include "gemm_tools.hpp"
#include "scalar_type.hpp"
#include "tiling.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <clir/attr_defs.hpp>
#include <clir/builder.hpp>
#include <clir/builtin_function.hpp>
#include <clir/data_type.hpp>
#include <clir/expr.hpp>
#include <clir/func.hpp>
#include <clir/stmt.hpp>
#include <clir/var.hpp>
#include <clir/visitor/unique_names.hpp>
#include <clir/visitor/unsafe_simplification.hpp>

#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <memory>
#include <sstream>
#include <utility>

using namespace clir;

namespace tinytc {

gemm_scalar_type::gemm_scalar_type(scalar_type ty) : alpha(ty), A(ty), B(ty), beta(ty), C(ty) {}
gemm_scalar_type::gemm_scalar_type(scalar_type alphaAB, scalar_type betaC)
    : alpha(alphaAB), A(alphaAB), B(alphaAB), beta(betaC), C(betaC) {}
gemm_scalar_type::gemm_scalar_type(scalar_type alpha, scalar_type A, scalar_type B,
                                   scalar_type beta, scalar_type C)
    : alpha(alpha), A(A), B(B), beta(beta), C(C) {}

std::string gemm_configuration::identifier(std::string_view prefix) const {
    std::ostringstream oss;
    auto const dyn_val = [&oss](std::int64_t v) {
        if (v == dynamic) {
            oss << "d";
        } else {
            oss << v;
        }
    };
    auto const stride = [&oss, &dyn_val](char X, std::array<std::int64_t, 2> const &s) {
        oss << "_" << X << "stride";
        dyn_val(s[0]);
        oss << "_";
        dyn_val(s[1]);
    };
    oss << prefix << "_";
    if (atomic) {
        oss << "atomic_";
    }
    oss << to_string(ty.alpha) << to_string(ty.A) << to_string(ty.B) << to_string(ty.beta)
        << to_string(ty.C) << "_A" << to_string(transA) << "_B" << to_string(transB) << "_M";
    dyn_val(M);
    oss << "_N";
    dyn_val(N);
    oss << "_K";
    dyn_val(K);
    stride('A', A_stride);
    stride('B', B_stride);
    stride('C', C_stride);
    auto const format_optional = [&](std::optional<double> const &val) {
        if (val) {
            auto f = oss.flags();
            auto v = *val;
            oss << std::hex << std::bit_cast<uint64_t>(v);
            oss.flags(f);
        } else {
            oss << "d";
        }
    };
    oss << "_alpha";
    format_optional(alpha);
    oss << "_beta";
    format_optional(beta);
    return oss.str();
}

class generator {
  public:
    generator(gemm_configuration const &gemm_cfg, local_tiling const &tiling,
              core_config const &core_cfg, clir::address_space As, clir::address_space Bs,
              clir::address_space Cs)
        : gemm_cfg(gemm_cfg), tiling(tiling), core_cfg(core_cfg), Aspace(As), Bspace(Bs),
          Cspace(Cs) {}
    bool use_double_buffering() const;
    void multiply_update(block_builder &bb, expr a, expr b, int n_offset, expr c, expr c_im);
    void add_microkernel(block_builder &bb, expr M, expr N, var A, var B, var C, expr C_offset,
                         expr alpha, expr beta);
    void add_mloop(block_builder &bb, expr N, var A, var B, var C, expr C_offset, expr alpha,
                   expr beta);
    void add_function_body(block_builder &bb, var A, var B, var C, expr alpha, expr beta);
    ::clir::func function(std::string_view name);

  private:
    gemm_configuration const gemm_cfg;
    local_tiling const tiling;
    core_config const core_cfg;
    clir::address_space Aspace, Bspace, Cspace;
    int row_blocks_in_register = 1;
    int cols_in_register = 1;
    var c_acc, c_acc_im, m;
    std::array<expr, 3> MNK;
    std::array<expr, 2> A_stride, B_stride, C_stride;
};

bool generator::use_double_buffering() const {
    return is_complex_type(gemm_cfg.ty.A) && is_complex_type(gemm_cfg.ty.B);
}

void generator::multiply_update(block_builder &bb, expr a, expr b, int n_offset, expr c,
                                expr c_im) {
    if (is_complex_type(gemm_cfg.ty.A)) {
        if (is_complex_type(gemm_cfg.ty.B)) {
            assert(use_double_buffering());
            auto b_bc_re = sub_group_broadcast(b.s(0), n_offset);
            auto b_bc_im = sub_group_broadcast(b.s(1), n_offset);
            bb.add(add_into(c, a * b_bc_re));
            bb.add(add_into(c_im, a * b_bc_im));
        } else {
            auto b_bc = sub_group_broadcast(b, n_offset);
            bb.add(add_into(std::move(c), std::move(a) * std::move(b_bc)));
        }
    } else if (is_complex_type(gemm_cfg.ty.B)) {
        auto b_bc_re = sub_group_broadcast(b.s(0), n_offset);
        auto b_bc_im = sub_group_broadcast(b.s(1), n_offset);
        bb.add(add_into(c.s(0), a * b_bc_re));
        bb.add(add_into(c.s(1), a * b_bc_im));
    } else {
        auto b_bc = sub_group_broadcast(b, n_offset);
        if (gemm_cfg.ty.A == gemm_cfg.ty.B && gemm_cfg.ty.B == gemm_cfg.ty.C) {
            bb.assign(c, fma(std::move(a), std::move(b_bc), c));
        } else {
            bb.add(add_into(std::move(c), std::move(a) * std::move(b_bc)));
        }
    }
}

void generator::add_microkernel(block_builder &bb, expr M, expr N, var A, var B, var C,
                                expr C_offset, expr alpha, expr beta) {
    int n_bs = 0;
    dispatch_constant_dynamic(
        N, [&](std::int64_t n) { n_bs = n; },
        [&](expr) { n_bs = static_cast<std::int64_t>(cols_in_register); });

    auto my_row_blocks_in_register = row_blocks_in_register;
    dispatch_constant_dynamic(
        M,
        [&](std::int64_t m) { my_row_blocks_in_register = 1 + (m - 1) / core_cfg.subgroup_size; },
        [&](expr) {});
    auto const Mb = my_row_blocks_in_register * core_cfg.subgroup_size;

    auto Ab = bb.declare_assign(pointer_to(to_clir_ty(gemm_cfg.ty.A, Aspace)), "Ab", A);
    auto Bb = bb.declare_assign(pointer_to(to_clir_ty(gemm_cfg.ty.B, Bspace)), "Bb", B);

    auto c_block = block_accessor_regular(c_acc, n_bs);
    auto c_block_im = block_accessor_regular(c_acc_im, n_bs);

    for (int n = 0; n < n_bs; ++n) {
        for (int m_block = 0; m_block < my_row_blocks_in_register; ++m_block) {
            bb.assign(c_block.get(m_block, n), constant(gemm_cfg.ty.C, 0.0));
            if (use_double_buffering()) {
                bb.assign(c_block_im.get(m_block, n), constant(gemm_cfg.ty.C, 0.0));
            }
        }
    }

    auto const compute_c = [&](block_builder &bb, int Kb, ::clir::expr K0, ::clir::expr K1) {
        auto kb = var("kb");
        bb.add(
            for_loop_builder(declaration_assignment(generic_short(), kb, std::move(K0)),
                             kb < std::move(K1), add_into(kb, Kb))
                .body([&](block_builder &bb) {
                    auto const a_descr =
                        matrix_block_description{gemm_cfg.ty.A, Aspace, Mb, Kb, Ab, M, A_stride};
                    auto const am = gemm_cfg.transA == transpose::T ? 1 : 0;
                    auto const a = read_matrix_block(bb, a_descr, am, core_cfg, "a");

                    auto const b_descr =
                        matrix_block_description{gemm_cfg.ty.B, Bspace, n_bs, Kb, Bb, N, B_stride};
                    auto const bn = gemm_cfg.transB == transpose::T ? 0 : 1;
                    auto const b = read_matrix_block(bb, b_descr, bn, core_cfg, "b");

                    const int nbb = 4;
                    for (int m_block = 0; m_block < my_row_blocks_in_register; ++m_block) {
                        for (int nb = 0; nb < n_bs; nb += nbb) {
                            for (int k = 0; k < Kb; ++k) {
                                for (int n = 0; n < nbb; ++n) {
                                    if (nb + n < n_bs) {
                                        auto const n_block = (nb + n) / core_cfg.subgroup_size;
                                        auto const n_offset = (nb + n) % core_cfg.subgroup_size;
                                        /*auto my_a = a->get(m_block, k);
                                        auto my_b =
                                            sub_group_broadcast(b->get(n_block, k), n_offset);
                                        auto my_c = c_block.get(m_block, nb + n);
                                        if (gemm_cfg.ty.A == gemm_cfg.ty.B &&
                                            gemm_cfg.ty.B == gemm_cfg.ty.C) {
                                            bb.assign(my_c, fma(std::move(my_a), std::move(my_b),
                                                                my_c));
                                        } else {
                                            bb.add(add_into(std::move(my_c),
                                                            std::move(my_a) * std::move(my_b)));
                                        }*/
                                        auto my_a = a->get(m_block, k);
                                        auto my_b = b->get(n_block, k);
                                        auto c_re = c_block.get(m_block, nb + n);
                                        auto c_im = c_block_im.get(m_block, nb + n);
                                        multiply_update(bb, std::move(my_a), std::move(my_b),
                                                        n_offset, std::move(c_re), std::move(c_im));
                                    }
                                }
                            }
                        }
                    }
                })
                .attribute(opencl_unroll_hint(1))
                .get_product());
    };
    dispatch_constant_dynamic(
        MNK[2],
        [&](std::int64_t K) {
            static_assert(max_K_unrolling % 2 == 0, "max_K_unrolling must be a multiple of 2");
            auto Kb = max_K_unrolling;
            while (K < Kb && Kb > 1) {
                Kb /= 2;
            }
            auto KmultipleKb = (K / Kb) * Kb;
            compute_c(bb, Kb, 0, KmultipleKb);
            if (K - KmultipleKb > 0) {
                compute_c(bb, 1, KmultipleKb, K);
            }
        },
        [&](expr K) {
            auto KmultipleKb = bb.declare_assign(generic_uint(), "KmultipleKb",
                                                 (K / max_K_unrolling) * max_K_unrolling);
            compute_c(bb, max_K_unrolling, 0, KmultipleKb);
            bb.add(if_selection_builder(K - KmultipleKb > 0)
                       .then([&](block_builder &bb) { compute_c(bb, 1, KmultipleKb, K); })
                       .get_product());
        });

    auto Cb = bb.declare_assign(pointer_to(to_clir_ty(gemm_cfg.ty.C, Cspace)), "Cb", C + C_offset);
    auto const c_descr = matrix_block_description{gemm_cfg.ty.C, Cspace, Mb, 1, Cb, M, C_stride};
    auto n = var("n");
    c_block.offset(n);
    c_block_im.offset(n);
    bb.add(for_loop_builder(declaration_assignment(generic_short(), n, 0), n < N, ++n)
               .body([&](block_builder &bb) {
                   if (use_double_buffering()) {
                       for (int m_block = 0; m_block < my_row_blocks_in_register; ++m_block) {
                           auto c_im = c_block_im.get(m_block, 0);
                           auto c_ty = to_clir_ty(gemm_cfg.ty.C);
                           bb.add(add_into(c_block.get(m_block, 0),
                                           init_vector(c_ty, {-c_im.s(1), c_im.s(0)})));
                       }
                   }
                   for (int m_block = 0; m_block < my_row_blocks_in_register; ++m_block) {
                       auto c = c_block.get(m_block, 0);
                       bb.assign(c, multiply(gemm_cfg.ty.alpha, gemm_cfg.ty.C, alpha, c));
                   }
                   write_matrix_block(bb, c_block, c_descr, gemm_cfg.atomic, gemm_cfg.ty.beta, beta,
                                      core_cfg);
               })
               .get_product());
}

void generator::add_mloop(block_builder &bb, expr N, var A, var B, var C, expr C_offset, expr alpha,
                          expr beta) {
    auto sg_m = bb.declare_assign(generic_uint(), "sg_m", get_sub_group_id() % tiling.m_tiles());
    tile_loop_by_sgs(
        bb, MNK[0], core_cfg.subgroup_size * row_blocks_in_register, tiling.m_tiles(),
        std::move(sg_m), [&](block_builder &bb, expr block, bool, expr inner_trip_count) {
            auto Astride_m = gemm_cfg.transA == transpose::T ? A_stride[1] : A_stride[0];
            auto Ab = bb.declare_assign(pointer_to(to_clir_ty(gemm_cfg.ty.A, Aspace)), "Ab",
                                        A + std::move(Astride_m) * block);
            add_microkernel(bb, std::move(inner_trip_count), N, std::move(Ab), B, C,
                            C_stride[0] * std::move(block) + C_offset, alpha, beta);
        });
}

void generator::add_function_body(block_builder &bb, var A, var B, var C, expr alpha, expr beta) {
    m = bb.declare_assign(generic_uint(), "m", get_sub_group_local_id());
    c_acc = var("c");
    c_acc_im = var("c_im");

    auto register_space = core_cfg.register_space;
    if (use_double_buffering()) {
        // We buffer the real / imag part separately, so we only have half the register space
        // available for one of the buffers
        register_space /= 2;
    }
    auto [max_rows, max_cols] =
        max_register_block_gemm(size(gemm_cfg.ty.C), core_cfg.subgroup_size, register_space);
    const auto max_row_blocks = max_rows / core_cfg.subgroup_size;
    row_blocks_in_register = max_row_blocks;
    cols_in_register = max_cols;
    if (!is_dynamic_value(gemm_cfg.M)) {
        auto const row_blocks_needed_to_cover_M = 1 + (gemm_cfg.M - 1) / core_cfg.subgroup_size;
        if (row_blocks_needed_to_cover_M < max_row_blocks) {
            row_blocks_in_register = row_blocks_needed_to_cover_M;
        } else {
            auto blocks = gemm_cfg.M / row_blocks_in_register;
            auto sg_blocks = 1 + (blocks - 1) / tiling.m_tiles();
            while (sg_blocks < tiling.m_tiles() && row_blocks_in_register >= 2) {
                row_blocks_in_register /= 2;
                blocks = gemm_cfg.M / row_blocks_in_register;
                sg_blocks = 1 + (blocks - 1) / tiling.m_tiles();
            }
        }
    }
    if (!is_dynamic_value(gemm_cfg.N)) {
        cols_in_register =
            tile_loop_uniformly_max_block_size(gemm_cfg.N, cols_in_register, tiling.n_tiles());
    }
    bb.declare(array_of(to_clir_ty(gemm_cfg.ty.C), row_blocks_in_register * cols_in_register),
               c_acc);
    if (use_double_buffering()) {
        bb.declare(array_of(to_clir_ty(gemm_cfg.ty.C), row_blocks_in_register * cols_in_register),
                   c_acc_im);
    }

    auto sg_n = bb.declare_assign(generic_uint(), "sg_n", get_sub_group_id() / tiling.m_tiles());
    tile_loop_uniformly(bb, MNK[1], max_cols, tiling.n_tiles(), std::move(sg_n),
                        [&](block_builder &bb, expr block, expr inner_trip_count) {
                            auto Bstride_n =
                                gemm_cfg.transB == transpose::T ? B_stride[0] : B_stride[1];
                            auto Bb =
                                bb.declare_assign(pointer_to(to_clir_ty(gemm_cfg.ty.B, Bspace)),
                                                  "Bb", B + std::move(Bstride_n) * block);
                            add_mloop(bb, std::move(inner_trip_count), A, std::move(Bb), C,
                                      C_stride[1] * std::move(block), alpha, beta);
                        });
}

::clir::func generator::function(std::string_view name) {
    auto A = var("A");
    auto B = var("B");
    auto C = var("C");

    auto fb = ::clir::function_builder{std::string(name)};
    auto const scalar = [&](scalar_type ty, std::optional<double> const &val,
                            std::string const &prefix) -> expr {
        auto v = var{prefix};
        fb.argument(to_clir_ty(ty), v);
        return val ? constant(ty, *val) : v;
    };
    auto const shape = [&](std::int64_t shape, expr &target, std::string const &prefix) {
        auto v = var{prefix};
        fb.argument(to_clir_ty(scalar_type::index), v);
        target = is_dynamic_value(shape) ? expr{std::move(v)} : expr{shape};
    };
    auto const stride = [&](std::array<std::int64_t, 2> const &stride, std::array<expr, 2> &target,
                            std::string const &prefix) {
        for (std::size_t i = 0; i < stride.size(); ++i) {
            auto v = var{prefix};
            fb.argument(to_clir_ty(scalar_type::index), v);
            target[i] = is_dynamic_value(stride[i]) ? expr{std::move(v)} : expr{stride[i]};
        }
    };

    shape(gemm_cfg.M, MNK[0], "M");
    shape(gemm_cfg.N, MNK[1], "N");
    shape(gemm_cfg.K, MNK[2], "K");
    expr alpha = scalar(gemm_cfg.ty.alpha, gemm_cfg.alpha, "alpha");
    fb.argument(pointer_to(to_clir_ty(gemm_cfg.ty.A, Aspace)), A);
    stride(gemm_cfg.A_stride, A_stride, "A_stride");
    fb.argument(pointer_to(to_clir_ty(gemm_cfg.ty.B, Bspace)), B);
    stride(gemm_cfg.B_stride, B_stride, "B_stride");
    expr beta = scalar(gemm_cfg.ty.beta, gemm_cfg.beta, "beta");
    fb.argument(pointer_to(to_clir_ty(gemm_cfg.ty.C, Cspace)), C);
    stride(gemm_cfg.C_stride, C_stride, "C_stride");

    fb.body([&](block_builder &bb) { add_function_body(bb, A, B, C, alpha, beta); });

    auto f = fb.get_product();
    make_names_unique(f);
    unsafe_simplify(f);

    return f;
}

::clir::func generate_gemm(gemm_configuration const &gemm_cfg, local_tiling const &tiling,
                           core_config const &core_cfg, std::string_view name,
                           clir::address_space As, clir::address_space Bs, clir::address_space Cs) {
    return generator{gemm_cfg, tiling, core_cfg, As, Bs, Cs}.function(name);
}

} // namespace tinytc
