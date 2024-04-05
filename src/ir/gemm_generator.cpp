// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/ir/gemm_generator.hpp"
#include "codegen_tools.hpp"
#include "precision_helper.hpp"
#include "tinytc/device_info.hpp"
#include "tinytc/ir/data_type.hpp"
#include "tinytc/ir/inst.hpp"
#include "tinytc/ir/scalar_type.hpp"
#include "tinytc/ir/tiling.hpp"

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
#include <cstddef>
#include <sstream>
#include <stdexcept>

using namespace clir;

namespace tinytc::ir {

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

constexpr static int max_K_unrolling = 8;

auto max_register_block_gemm(std::uint32_t C_scalar_type_size_in_bytes, std::uint32_t sgs,
                             std::uint32_t register_space,
                             std::pair<std::uint32_t, std::uint32_t> max_fill_fraction)
    -> std::pair<std::uint32_t, std::uint32_t> {
    auto const arithmetic_intensity = [&sgs](std::uint32_t row_blocks, std::uint32_t cols) {
        return (row_blocks * sgs * cols) / static_cast<double>(row_blocks * sgs + cols);
    };

    auto const max_scalars = register_space * max_fill_fraction.first /
                             (max_fill_fraction.second * C_scalar_type_size_in_bytes);

    // The required number of scalars is given by
    // row_blocks * sgs * (cols + max_K_unrolling) + cols * max_K_unrolling
    auto const max_row_blocks = [&sgs, &max_scalars](std::uint32_t cols) {
        return (max_scalars - cols * max_K_unrolling) / (sgs * (cols + max_K_unrolling));
    };
    auto const max_cols = [&sgs, &max_scalars](std::uint32_t row_blocks) {
        return (max_scalars - row_blocks * sgs * max_K_unrolling) /
               (row_blocks * sgs + max_K_unrolling);
    };

    double max_ai = 0.0;
    std::uint32_t row_blocks = 1, cols = 1;
    for (std::uint32_t r = 1; r <= max_row_blocks(1); ++r) {
        for (std::uint32_t c = 1; c <= max_cols(r); ++c) {
            auto const ai = arithmetic_intensity(r, c);
            if (ai > max_ai) {
                max_ai = ai;
                row_blocks = r;
                cols = c;
            }
        }
    }

    return std::make_pair(row_blocks, cols);
}

class generator {
  public:
    generator(gemm_configuration const &gemm_cfg, local_tiling const &tiling,
              core_config const &core_cfg, address_space As, address_space Bs, address_space Cs)
        : gemm_cfg(gemm_cfg), tiling(tiling), core_cfg(core_cfg), Aspace(As), Bspace(Bs),
          Cspace(Cs) {}
    void add_microkernel(block_builder &bb, bool is_remainder, expr M, expr N, var A, var B, var C,
                         expr alpha, expr beta);
    void add_mloop(block_builder &bb, expr N, var A, var B, var C, expr alpha, expr beta);
    void add_function_body(block_builder &bb, var A, var B, var C, expr alpha, expr beta);
    func function(std::string_view name);

  private:
    gemm_configuration const gemm_cfg;
    local_tiling const tiling;
    core_config const core_cfg;
    address_space Aspace, Bspace, Cspace;
    unsigned row_blocks_in_register = 1;
    unsigned cols_in_register = 1;
    var c, m;
    std::array<expr, 3> MNK;
    std::array<expr, 2> A_stride, B_stride, C_stride;
};

void generator::add_microkernel(block_builder &bb, bool is_remainder, expr M, expr N, var A, var B,
                                var C, expr alpha, expr beta) {
    std::int64_t n_bs = 0;
    bool is_N_constant = false;
    dispatch_constant_dynamic(
        N,
        [&](std::int64_t n) {
            n_bs = n;
            is_N_constant = true;
        },
        [&](expr) {
            n_bs = static_cast<std::int64_t>(cols_in_register);
            is_N_constant = false;
        });
    std::int64_t const n_blocks =
        1 + (n_bs - 1) / static_cast<std::int64_t>(core_cfg.subgroup_size);
    auto n = var("n");

    auto my_row_blocks_in_register = row_blocks_in_register;
    dispatch_constant_dynamic(
        M,
        [&](std::int64_t m) {
            while (my_row_blocks_in_register > 1 &&
                   m < static_cast<std::int64_t>(my_row_blocks_in_register) *
                           core_cfg.subgroup_size) {
                --my_row_blocks_in_register;
            }
        },
        [&](expr) {});

    auto const am = gemm_cfg.transA == transpose::T ? 1 : 0;
    auto const ak = gemm_cfg.transA == transpose::T ? 0 : 1;
    auto Ab = bb.declare_assign(pointer_to(precision_helper{gemm_cfg.ty.A}.type(Aspace)), "Ab", A);
    auto const Aoffset = [&](unsigned m_block) {
        return A_stride[am] * (m + m_block * core_cfg.subgroup_size);
    };

    auto const bn = gemm_cfg.transB == transpose::T ? 0 : 1;
    auto const bk = gemm_cfg.transB == transpose::T ? 1 : 0;
    auto Bb = bb.declare_assign(pointer_to(precision_helper{gemm_cfg.ty.B}.type(Bspace)), "Bb", B);
    auto const Boffset = [&](int n_block) {
        return B_stride[bn] * (m + n_block * core_cfg.subgroup_size);
    };

    auto const cmn = [&](unsigned m_block, expr n) {
        return c[m_block + row_blocks_in_register * std::move(n)];
    };

    bb.add(for_loop_builder(declaration_assignment(generic_short(), n, 0), n < n_bs, ++n)
               .body([&](block_builder &bb) {
                   for (std::size_t m_block = 0; m_block < my_row_blocks_in_register; ++m_block) {
                       bb.assign(cmn(m_block, n), precision_helper{gemm_cfg.ty.C}.zero());
                   }
               })
               .attribute(opencl_unroll_hint(n_bs))
               .get_product());

    auto const compute_c = [&](block_builder &bb, std::int64_t Kb, clir::expr K0, clir::expr K1) {
        auto kb = var("kb");
        bb.add(
            for_loop_builder(declaration_assignment(generic_short(), kb, std::move(K0)),
                             kb < std::move(K1), add_into(kb, Kb))
                .body([&](block_builder &bb) {
                    auto at = precision_helper{gemm_cfg.ty.A};
                    auto a = bb.declare(array_of(at.type(), my_row_blocks_in_register * Kb), "a");
                    auto amk = [&](unsigned m_block, unsigned k) {
                        return a[m_block + my_row_blocks_in_register * k];
                    };
                    bool const map_b_to_vec_type =
                        gemm_cfg.B_stride[bk] == 1 &&
                        (Kb == 2 || Kb == 3 || Kb == 4 || Kb == 8 || Kb == 16);
                    int k_load_block_size = map_b_to_vec_type ? Kb : 1;
                    auto bt = precision_helper{gemm_cfg.ty.B};
                    auto b = map_b_to_vec_type
                                 ? bb.declare(array_of(bt.type(Kb), n_blocks), "b")
                                 : bb.declare(array_of(bt.type(), n_blocks * Kb), "b");
                    auto const read_A = [&](block_builder &bb, unsigned m_block, unsigned k) {
                        bb.assign(amk(m_block, k), Ab[Aoffset(m_block)]);
                    };
                    auto block_read_A = [&](block_builder &bb, unsigned m_block, unsigned k) {
                        bb.assign(
                            amk(m_block, k),
                            at.sub_group_block_read(Ab + m_block * core_cfg.subgroup_size, Aspace));
                    };
                    for (unsigned k = 0; k < Kb; ++k) {
                        for (unsigned m_block = 0; m_block < my_row_blocks_in_register; ++m_block) {
                            if (is_remainder) {
                                bb.add(
                                    if_selection_builder(m + m_block * core_cfg.subgroup_size < M)
                                        .then([&](block_builder &bb) { read_A(bb, m_block, k); })
                                        .get_product());
                            } else {
                                if (gemm_cfg.A_stride[am] == 1) {
                                    block_read_A(bb, m_block, k);
                                } else {
                                    read_A(bb, m_block, k);
                                }
                            }
                        }
                        bb.add(add_into(Ab, A_stride[ak]));
                    }

                    auto const read_B = [&](block_builder &bb, int k, int n_block) {
                        if (map_b_to_vec_type) {
                            auto l = vload_helper(Kb, 0, Bb + Boffset(n_block));
                            if (l) {
                                bb.assign(b[n_block], std::move(l));
                            } else {
                                throw std::logic_error("Vload for native type missing");
                            }
                        } else {
                            bb.assign(b[k + n_block * Kb], Bb[Boffset(n_block)]);
                        }
                    };
                    int first_n_block_with_check =
                        n_bs < n_blocks * static_cast<std::int64_t>(core_cfg.subgroup_size)
                            ? n_blocks - 1
                            : n_blocks;
                    if (!is_N_constant) {
                        first_n_block_with_check = 0;
                    }
                    for (int k = 0; k < Kb; k += k_load_block_size) {
                        for (int n_block = 0; n_block < first_n_block_with_check; ++n_block) {
                            read_B(bb, k, n_block);
                        }
                        for (int n_block = first_n_block_with_check; n_block < n_blocks;
                             ++n_block) {
                            bb.add(if_selection_builder(m + n_block * core_cfg.subgroup_size < N)
                                       .then([&](block_builder &bb) { read_B(bb, k, n_block); })
                                       .get_product());
                        }
                        bb.add(add_into(Bb, k_load_block_size * B_stride[bk]));
                    }

                    const int nbb = 4;
                    for (unsigned m_block = 0; m_block < my_row_blocks_in_register; ++m_block) {
                        for (std::int64_t nb = 0; nb < n_bs; nb += nbb) {
                            for (int k = 0; k < Kb; ++k) {
                                for (std::int64_t n = 0; n < nbb; ++n) {
                                    if (nb + n < n_bs) {
                                        auto const n_block = (nb + n) / core_cfg.subgroup_size;
                                        auto const n_offset = (nb + n) % core_cfg.subgroup_size;
                                        auto my_a = amk(m_block, k);
                                        auto bkn = map_b_to_vec_type ? b[n_block].s(k)
                                                                     : b[k + n_block * Kb];
                                        auto my_b = sub_group_broadcast(std::move(bkn), n_offset);
                                        auto my_c = cmn(m_block, nb + n);
                                        if (gemm_cfg.ty.A == gemm_cfg.ty.B &&
                                            gemm_cfg.ty.B == gemm_cfg.ty.C) {
                                            bb.assign(my_c,
                                                      fma(std::move(my_a), std::move(my_b), my_c));
                                        } else {
                                            bb.add(add_into(std::move(my_c),
                                                            std::move(my_a) * std::move(my_b)));
                                        }
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
    auto write_C = [&](block_builder &bb) {
        auto n_to =
            is_N_constant ? n_bs : min(N, cast(internal::to_clir_ty(scalar_type::index), n_bs));
        auto n_unroll = is_N_constant ? n_bs : 1;
        bb.add(
            for_loop_builder(declaration_assignment(generic_short(), n, 0), n < std::move(n_to),
                             ++n)
                .body([&](block_builder &bb) {
                    for (std::size_t m_block = 0; m_block < my_row_blocks_in_register; ++m_block) {
                        auto my_c = alpha * cmn(m_block, n);
                        auto Coffset = C_stride[0] * (m + m_block * core_cfg.subgroup_size);
                        if (is_remainder || gemm_cfg.C_stride[0] != 1 || gemm_cfg.atomic) {
                            auto const write_C_mn = [&](block_builder &bb) {
                                auto Cdst = bb.declare_assign(
                                    pointer_to(precision_helper{gemm_cfg.ty.C}.type(Cspace)),
                                    "Cdst", C + Coffset);
                                store_helper(bb, gemm_cfg.atomic, std::move(Cdst), gemm_cfg.ty.C,
                                             Cspace, my_c, beta);
                            };
                            if (is_remainder) {
                                bb.add(
                                    if_selection_builder(m + m_block * core_cfg.subgroup_size < M)
                                        .then(write_C_mn)
                                        .get_product());
                            } else {
                                write_C_mn(bb);
                            }
                        } else {
                            bb.add(precision_helper{gemm_cfg.ty.C}.sub_group_block_write(
                                C + m_block * core_cfg.subgroup_size,
                                std::move(my_c) + beta * C[std::move(Coffset)], Cspace));
                        }
                    }
                    bb.add(add_into(C, cast(generic_uint(), C_stride[1])));
                })
                .attribute(opencl_unroll_hint(n_unroll))
                .get_product());
    };
    write_C(bb);
}

void generator::add_mloop(block_builder &bb, expr N, var A, var B, var C, expr alpha, expr beta) {
    auto sg_m = bb.declare_assign(generic_uint(), "sg_m", get_sub_group_id() % tiling.m_tiles());
    tile_loop_by_sgs(
        bb, MNK[0], core_cfg.subgroup_size * row_blocks_in_register, tiling.m_tiles(),
        std::move(sg_m),
        [&](block_builder &bb, expr block, bool is_remainder, expr inner_trip_count) {
            auto Astride_m = gemm_cfg.transA == transpose::T ? A_stride[1] : A_stride[0];
            auto Ab = bb.declare_assign(pointer_to(precision_helper{gemm_cfg.ty.A}.type(Aspace)),
                                        "Ab", A + std::move(Astride_m) * block);
            auto Cb = bb.declare_assign(pointer_to(precision_helper{gemm_cfg.ty.C}.type(Cspace)),
                                        "Cb", C + C_stride[0] * std::move(block));
            add_microkernel(bb, is_remainder, std::move(inner_trip_count), N, std::move(Ab), B,
                            std::move(Cb), alpha, beta);
        });
}

void generator::add_function_body(block_builder &bb, var A, var B, var C, expr alpha, expr beta) {
    m = bb.declare_assign(generic_uint(), "m", get_sub_group_local_id());
    c = var("c");

    auto [max_row_blocks, max_cols] = max_register_block_gemm(
        size(gemm_cfg.ty.C), core_cfg.subgroup_size, core_cfg.register_space);
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
    bb.declare(
        array_of(precision_helper{gemm_cfg.ty.C}.type(), row_blocks_in_register * cols_in_register),
        c);

    auto sg_n = bb.declare_assign(generic_uint(), "sg_n", get_sub_group_id() / tiling.m_tiles());
    tile_loop_uniformly(
        bb, MNK[1], max_cols, tiling.n_tiles(), std::move(sg_n),
        [&](block_builder &bb, expr block, expr inner_trip_count) {
            auto Bstride_n = gemm_cfg.transB == transpose::T ? B_stride[0] : B_stride[1];
            auto Bb = bb.declare_assign(pointer_to(precision_helper{gemm_cfg.ty.B}.type(Bspace)),
                                        "Bb", B + std::move(Bstride_n) * block);
            auto Cb = bb.declare_assign(pointer_to(precision_helper{gemm_cfg.ty.C}.type(Cspace)),
                                        "Cb", C + C_stride[1] * std::move(block));
            add_mloop(bb, std::move(inner_trip_count), A, std::move(Bb), std::move(Cb), alpha,
                      beta);
        });
}

func generator::function(std::string_view name) {
    auto A = var("A");
    auto B = var("B");
    auto C = var("C");

    auto fb = function_builder{std::string(name)};
    auto const scalar = [&](precision_helper const &fph, std::optional<double> const &val,
                            std::string const &prefix) -> expr {
        auto v = var{prefix};
        fb.argument(fph.type(), v);
        return val ? fph.constant(*val) : v;
    };
    auto const shape = [&](std::int64_t shape, expr &target, std::string const &prefix) {
        auto v = var{prefix};
        fb.argument(internal::to_clir_ty(scalar_type::index), v);
        target = is_dynamic_value(shape) ? expr{std::move(v)} : expr{shape};
    };
    auto const stride = [&](std::array<std::int64_t, 2> const &stride, std::array<expr, 2> &target,
                            std::string const &prefix) {
        for (std::size_t i = 0; i < stride.size(); ++i) {
            auto v = var{prefix};
            fb.argument(internal::to_clir_ty(scalar_type::index), v);
            target[i] = is_dynamic_value(stride[i]) ? expr{std::move(v)} : expr{stride[i]};
        }
    };

    shape(gemm_cfg.M, MNK[0], "M");
    shape(gemm_cfg.N, MNK[1], "N");
    shape(gemm_cfg.K, MNK[2], "K");
    expr alpha = scalar(precision_helper{gemm_cfg.ty.alpha}, gemm_cfg.alpha, "alpha");
    fb.argument(pointer_to(precision_helper{gemm_cfg.ty.A}.type(Aspace)), A);
    stride(gemm_cfg.A_stride, A_stride, "A_stride");
    fb.argument(pointer_to(precision_helper{gemm_cfg.ty.B}.type(Bspace)), B);
    stride(gemm_cfg.B_stride, B_stride, "B_stride");
    expr beta = scalar(precision_helper{gemm_cfg.ty.beta}, gemm_cfg.beta, "beta");
    fb.argument(pointer_to(precision_helper{gemm_cfg.ty.C}.type(Cspace)), C);
    stride(gemm_cfg.C_stride, C_stride, "C_stride");

    fb.body([&](block_builder &bb) { add_function_body(bb, A, B, C, alpha, beta); });

    auto f = fb.get_product();
    make_names_unique(f);
    unsafe_simplify(f);

    return f;
}

func generate_gemm(gemm_configuration const &gemm_cfg, local_tiling const &tiling,
                   core_config const &core_cfg, std::string_view name, address_space As,
                   address_space Bs, address_space Cs) {
    return generator{gemm_cfg, tiling, core_cfg, As, Bs, Cs}.function(name);
}

} // namespace tinytc::ir
