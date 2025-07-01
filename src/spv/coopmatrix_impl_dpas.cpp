// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "spv/coopmatrix_impl_dpas.hpp"
#include "analysis/gcd.hpp"
#include "codegen_tools.hpp"
#include "coopmatrix_layout.hpp"
#include "device_info.hpp"
#include "matrix_ext_info.hpp"
#include "node/inst_view.hpp"
#include "node/type.hpp"
#include "spv/block2d_diy.hpp"
#include "spv/coopmatrix_impl.hpp"
#include "spv/defs.hpp"
#include "spv/dope_vector.hpp"
#include "spv/enums.hpp"
#include "spv/instructions.hpp"
#include "spv/lut.hpp"
#include "spv/module.hpp"
#include "spv/uniquifier.hpp"
#include "spv/xe_constants.hpp"
#include "tinytc/builder.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"
#include "util/casting.hpp"
#include "util/math.hpp"

#include <algorithm>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace tinytc::spv {

auto precision(scalar_type sty) -> char const * {
    switch (sty) {
    case scalar_type::f16:
        return "hf";
    case scalar_type::bf16:
        return "bf";
    case scalar_type::i8:
        return "s8";
    default:
        throw status::internal_compiler_error;
    }
}

auto coopmatrix_impl_dpas::max_rows_in_block(matrix_use use, std::int32_t element_size) const
    -> std::int32_t {
    if (use == matrix_use::b) {
        std::int32_t ops_per_chan = xe::channel_size / element_size;
        return ops_per_chan * xe::sdepth;
    }
    return xe::exec_size;
}

auto coopmatrix_impl_dpas::check_2d_block_io(tinytc_value const &operand, tinytc_value const &pos0)
    -> bool {
    if (auto mi = gcd().get_memref_if(operand); mi) {
        auto const mt = get_memref_type(operand);
        const auto sty_size = size(dyn_cast<number_type>(mt->element_ty())->ty());
        auto const &block_io = cfg().matrix->block_io();

        const bool sfid_ok = mt->addrspace() == address_space::global;
        const bool base_address_alignment_ok =
            (mi->offset_gcd() * sty_size) % block_io.base_address_alignment == 0;
        const bool pos0_alignment_ok = (gcd().get(pos0) * sty_size) % block_io.pos0_alignment == 0;
        const bool stride_ok =
            mt->stride(0) == 1 && (mi->stride_gcd()[1] * sty_size) % block_io.stride_alignment == 0;
        const bool width_ok = (mi->shape_gcd()[0] * sty_size) % block_io.width_alignment == 0;

        return sfid_ok && base_address_alignment_ok && pos0_alignment_ok && stride_ok && width_ok;
    }
    return false;
}

auto coopmatrix_impl_dpas::load_config(scalar_type sty, std::int32_t rows, std::int32_t cols,
                                       matrix_use use, transpose trans, int32_t cache_level)
    -> block_config {
    auto cfg = block_config{};
    cfg.sty = sty;
    cfg.element_size = size(sty);
    cfg.array_length = 1;
    cfg.rows = rows;
    cfg.cols = cols;
    cfg.row_blocks = 1;
    cfg.col_blocks = 1;
    cfg.transpose = trans == transpose::T;
    cfg.vnni = use == matrix_use::a;
    cfg.pos0_shr = 0;
    cfg.cache_level = cache_level;

    auto const adjust_rows = [&cfg](std::int32_t max_rows, std::int32_t max_array_length) {
        if (cfg.rows > max_rows) {
            const std::int32_t num_blocks = cfg.rows / max_rows;
            if (num_blocks > max_array_length) {
                cfg.array_length = max_array_length;
                cfg.row_blocks = num_blocks / max_array_length;
            } else {
                cfg.array_length = num_blocks;
            }
            cfg.rows = max_rows;
        }
    };
    auto const adjust_cols = [&cfg](std::int32_t max_cols_in_block) {
        if (cfg.cols > max_cols_in_block) {
            cfg.col_blocks = cfg.cols / max_cols_in_block;
            cfg.cols = max_cols_in_block;
        }
    };
    auto const max_array_length = [&cfg](std::int32_t max_rows) -> std::int32_t {
        return 64 / (max_rows * cfg.element_size);
    };

    // transpose + vnni message is the same as transpose message on d32
    if (cfg.transpose && cfg.vnni) {
        std::swap(cfg.rows, cfg.cols);

        const auto ops_per_chan = 4 / cfg.element_size;
        cfg.rows /= ops_per_chan;
        cfg.sty = scalar_type::i32;
        cfg.element_size = 4;
        cfg.pos0_shr = ilog2(ops_per_chan);
        cfg.vnni = false;

        adjust_cols(xe::exec_size);

        const auto max_rows = xe::exec_size / 2;
        adjust_rows(max_rows, 1);
    } else if (cfg.transpose) {
        std::swap(cfg.rows, cfg.cols);
        // Enable VNNI as transpose loads for B matrix are missing, so we use VNNI + mov-based 8x8
        // transpose
        cfg.vnni = true;

        const std::int32_t max_cols = max_rows_in_block(use, cfg.element_size);
        const std::int32_t max_rows = 8;
        adjust_cols(max_cols);
        adjust_rows(max_rows, max_array_length(max_rows));
    } else {
        const std::int32_t max_cols = 32;
        const std::int32_t max_rows = max_rows_in_block(use, cfg.element_size);

        adjust_cols(max_cols);
        adjust_rows(max_rows, max_array_length(max_rows));
    }

    return cfg;
}

auto coopmatrix_impl_dpas::load_fun(coopmatrix_type const *result_ty, spv_inst *spv_operand_ty,
                                    transpose trans) -> spv_inst * {
    const auto key = load_key{result_ty, spv_operand_ty, trans};
    return lookup(load_funs_, key, [&](load_key const &key) {
        const auto [result_ty, spv_operand_ty, trans] = key;

        auto sty = dyn_cast<number_type>(result_ty->component_ty())->ty();
        const auto cfg =
            load_config(sty, result_ty->rows(), result_ty->cols(), result_ty->use(), trans);
        auto code = load_block2d_native(cfg, tmp_);

        auto spv_i32_ty = unique().scalar_ty(scalar_type::i32);
        auto spv_result_ty = spv_ty(result_ty);
        auto fun_ty = unique().function_ty(
            spv_result_ty, array_view<spv_inst *>{spv_operand_ty, spv_i32_ty, spv_i32_ty,
                                                  spv_i32_ty, spv_i32_ty, spv_i32_ty});
        return unique().mod().add_to<OpAsmINTEL>(section::type_const_var, spv_result_ty, fun_ty,
                                                 unique().asm_target(), code,
                                                 "=rw,rw.u,rw.u,rw.u,rw.u,rw.u,rw.u");
    });
}

auto coopmatrix_impl_dpas::prefetch_fun(std::int32_t cache_level, scalar_type sty,
                                        spv_inst *spv_operand_ty, std::int32_t rows,
                                        std::int32_t cols) -> spv_inst * {
    const auto key = prefetch_key{cache_level, sty, spv_operand_ty, rows, cols};
    return lookup(prefetch_funs_, key, [&](prefetch_key const &key) -> spv_inst * {
        const auto [cache_level, sty, spv_operand_ty, rows, cols] = key;

        const auto cfg = load_config(sty, rows, cols, matrix_use::acc, transpose::N, cache_level);
        auto code = prefetch_block2d_native(cfg, tmp_);

        auto spv_i32_ty = unique().scalar_ty(scalar_type::i32);
        auto spv_void_ty = unique().void_ty();
        auto fun_ty = unique().function_ty(
            spv_void_ty, array_view<spv_inst *>{spv_operand_ty, spv_i32_ty, spv_i32_ty, spv_i32_ty,
                                                spv_i32_ty, spv_i32_ty});
        return unique().mod().add_to<OpAsmINTEL>(section::type_const_var, spv_void_ty, fun_ty,
                                                 unique().asm_target(), code,
                                                 "rw.u,rw.u,rw.u,rw.u,rw.u,rw.u");
    });
}

auto coopmatrix_impl_dpas::store_config(coopmatrix_type const *ct) -> block_config {
    constexpr std::int32_t max_cols_in_block = 8;

    auto cfg = block_config{};
    cfg.sty = dyn_cast<number_type>(ct->component_ty())->ty();
    cfg.element_size = size(cfg.sty);
    cfg.array_length = 1;
    cfg.rows = ct->rows();
    cfg.cols = ct->cols();
    cfg.row_blocks = 1;
    cfg.col_blocks = 1;
    cfg.transpose = false;
    cfg.vnni = false;
    cfg.pos0_shr = 0;
    cfg.cache_level = -1;

    if (cfg.cols > max_cols_in_block) {
        cfg.col_blocks = cfg.cols / max_cols_in_block;
        cfg.cols = max_cols_in_block;
    }

    const auto max_rows = max_rows_in_block(ct->use(), cfg.element_size);
    if (cfg.rows > max_rows) {
        cfg.row_blocks = cfg.rows / max_rows;
        cfg.rows = max_rows;
    }

    return cfg;
}

auto coopmatrix_impl_dpas::store_fun(coopmatrix_type const *val_ty, spv_inst *spv_operand_ty)
    -> spv_inst * {
    const auto key = store_key{val_ty, spv_operand_ty};
    return lookup(store_funs_, key, [&](store_key const &key) {
        const auto [val_ty, spv_operand_ty] = key;

        const auto cfg = store_config(val_ty);
        auto code = store_block2d_native(cfg, tmp_);

        auto spv_void_ty = unique().void_ty();
        auto spv_val_ty = spv_ty(val_ty);
        auto spv_i32_ty = unique().scalar_ty(scalar_type::i32);
        auto fun_ty = unique().function_ty(
            spv_void_ty, array_view<spv_inst *>{spv_val_ty, spv_operand_ty, spv_i32_ty, spv_i32_ty,
                                                spv_i32_ty, spv_i32_ty, spv_i32_ty});
        auto &mod = unique().mod();
        auto asmop =
            mod.add_to<OpAsmINTEL>(section::type_const_var, spv_void_ty, fun_ty,
                                   unique().asm_target(), code, "rw,rw.u,rw.u,rw.u,rw.u,rw.u,rw.u");
        mod.add_to<OpDecorate>(section::decoration, asmop, Decoration::SideEffectsINTEL);
        return asmop;
    });
}

auto coopmatrix_impl_dpas::mul_add_fun(coopmatrix_type const *at, coopmatrix_type const *bt,
                                       coopmatrix_type const *ct, coopmatrix_type const *rt,
                                       bool is_c_zero) -> spv_inst * {
    const auto key = mul_add_key{{at, bt, ct, rt}, is_c_zero};
    return lookup(mul_add_funs_, key, [&](mul_add_key const &key) {
        const auto [at, bt, ct, rt] = key.op_ty;

        auto oasm = std::ostringstream{};

        auto at_sty = dyn_cast<number_type>(at->component_ty())->ty();
        auto bt_sty = dyn_cast<number_type>(bt->component_ty())->ty();
        auto ct_sty = dyn_cast<number_type>(ct->component_ty())->ty();
        auto rt_sty = dyn_cast<number_type>(rt->component_ty())->ty();
        const std::int32_t ops_per_chan = xe::channel_size / size(at_sty);
        const std::int32_t K = ops_per_chan * xe::sdepth;

        oasm << "{\n";
        std::string result_placeholder = "$0";
        std::string temp = result_placeholder;
        if (rt->component_ty() != ct->component_ty() && at->cols() / K > 1) {
            temp = tmp_("temp");
            oasm << ".decl " << temp << " v_type=G type=" << visa_type(ct_sty)
                 << " num_elts=" << ct->rows() * ct->cols() << " align=wordx32\n";
        }
        const auto mat_A = tmp_("matrix_A");
        const auto mat_B = tmp_("matrix_B");
        oasm << ".decl " << mat_A
             << " v_type=G type=d num_elts=" << at->rows() * at->cols() / ops_per_chan
             << " align=wordx32 alias=<$1,0>\n";
        oasm << ".decl " << mat_B
             << " v_type=G type=d num_elts=" << bt->rows() * bt->cols() / ops_per_chan
             << " align=wordx32 alias=<$2,0>\n";

        /** The GRF layout must follow the layout described in the following.
         *
         * Let CM, CN, CK be the size of the coopmatrices, where
         * CM = ct->rows() = at->rows(),
         * CN = ct->cols() = bt->cols(),
         * CK = at->cols() = bt->rows(),
         * and let M, N, K be the size expected by DPAS, where
         * M = xe::exec_size,
         * N = xe::rcount
         * K = ops_per_chan * xe::sdepth
         * Let BM:=CM/M, BN:=CN/N, BK:=CK/K be the number of blocks in the respective mode.
         *
         * The blocks are laid out in the GRF as following
         *
         * A[m,k,bk,bm] = m + k * M + bk * M * K + bm * M * K * BK
         * B[k,n,bn,bk] = k + n * K + bn * K * N + bk * K * N * BN
         * C[m,n,bn,bm] = m + n * M + bn * M * N + bm * M * N * BN
         *
         * where m \in [M], n \in [N], k \in [K], bm \in [BM], bn \in [BN], bk \in [BK].
         *
         * The mapping of m,n,k,bm,bn,bk to memory address is given by
         *
         * MA[m,k,bk,bm] = m'  + bm'  * M + (k'  + bk'  * K) * A_stride1
         * MB[k,n,bn,bk] = k'' + bk'' * K + (n'' + bn'' * N) * B_stride1
         * MC[m,n,bn,bm] = m   + bm   * M + (n   + bn   * N) * C_stride1
         *
         * where
         *
         * (m',k')   = { (m%ops_per_chan + k*ops_per_chan, floor(m/ops_per_chan))     if A
         * transposed { (floor(m/ops_per_chan) + k*(M/ops_per_chan), m%ops_per_chan) else (bm',bk')
         * = { (bk,bm) if A transposed { (bm,bk) else
         *
         * and
         *
         * (k'',n'')   = { (n,k) if B transposed
         *               { (k,n) else
         * (bk'',bn'') = { (bn,bk) if B transposed
         *               { (bk,bn) else
         *
         */
        const auto precision_src1 = precision(at_sty);
        const auto precision_src2 = precision(bt_sty);
        for (std::int32_t k = 0; k < at->cols(); k += K) {
            char const *src0 = k > 0 ? temp.c_str() : (!key.is_c_zero ? "$3" : "%null");
            char const *dst = k + K >= at->cols() ? result_placeholder.c_str() : temp.c_str();
            const auto rsize = k + K >= at->cols() ? size(rt_sty) : size(ct_sty);
            for (std::int32_t m = 0; m < ct->rows(); m += xe::exec_size) {
                for (std::int32_t n = 0; n < ct->cols(); n += xe::rcount) {
                    const auto aoffset = (k * xe::exec_size + m * at->cols()) * size(at_sty);
                    const auto brow = (k * bt->cols() + n * K) * size(bt_sty) / xe::grf_size;
                    const auto coffset = !key.is_c_zero || k > 0
                                             ? (m * ct->cols() + n * xe::exec_size) * size(ct_sty)
                                             : 0;
                    const auto roffset = (m * rt->cols() + n * xe::exec_size) * rsize;
                    oasm << "dpas." << precision_src1 << "." << precision_src2 << "." << xe::sdepth
                         << "." << xe::rcount << " (M1," << xe::exec_size << ") " << dst << "."
                         << roffset << " " << src0 << "." << coffset << " " << mat_A << "."
                         << aoffset << " " << mat_B << "(" << brow << ",0)\n";
                }
            }
        }
        oasm << "}\n";

        auto spv_a_ty = spv_ty(at);
        auto spv_b_ty = spv_ty(bt);
        auto spv_c_ty = spv_ty(ct);
        auto spv_result_ty = spv_ty(rt);
        auto fun_ty = unique().function_ty(spv_result_ty,
                                           array_view<spv_inst *>{spv_a_ty, spv_b_ty, spv_c_ty});

        return unique().mod().add_to<OpAsmINTEL>(section::type_const_var, spv_result_ty, fun_ty,
                                                 unique().asm_target(), std::move(oasm).str(),
                                                 "=rw,rw,rw,rw");
    });
}

auto coopmatrix_impl_dpas::reduce_fun(std::int32_t sgs, IK op, coopmatrix_type const *at,
                                      coopmatrix_type const *rt) -> spv_inst * {
    const auto key = std::make_tuple(sgs, op, at, rt);
    return lookup(reduce_funs_, key, [&](reduce_key const &key) {
        auto [sgs, op, at, rt] = key;
        auto rl = get_layout(cfg(), rt);
        auto al = get_layout(cfg(), at);
        auto matrix_ty = spv_ty(rl);
        const auto at_sty = dyn_cast<number_type>(at->component_ty())->ty();
        const auto sty = dyn_cast<number_type>(rt->component_ty())->ty();
        const auto sty_size = size(sty);

        auto oasm = std::ostringstream{};

        oasm << "{\n";
        auto aview = tmp_("aview");
        oasm << ".decl " << aview << " v_type=G type=" << visa_type(at_sty)
             << " num_elts=" << al.length * sgs << " align=wordx32 alias=<$1,0>\n";
        auto rview = tmp_("rview");
        oasm << ".decl " << rview << " v_type=G type=" << visa_type(sty)
             << " num_elts=" << rl.length * sgs << " align=wordx32 alias=<$0,0>\n";
        auto predicate = tmp_("predicate");
        oasm << ".decl " << predicate << " v_type=P num_elts=" << sgs << "\n";

        char const *reduce = [](IK op) {
            switch (op) {
            case IK::IK_cooperative_matrix_reduce_add:
                return "add";
            case IK::IK_cooperative_matrix_reduce_max:
                return "max";
            case IK::IK_cooperative_matrix_reduce_min:
                return "min";
            default:
                break;
            }
            throw status::internal_compiler_error;
        }(op);

        for (std::int32_t offset = 0; offset < al.shape1; offset += sgs) {
            const std::int32_t remainder =
                std::min(sgs, static_cast<std::int32_t>(al.shape1 - offset));
            std::string src = aview;
            if (al.blocks > 1) {
                auto tmp = tmp_("tmp");
                oasm << ".decl " << tmp << " v_type=G type=" << visa_type(at_sty)
                     << " num_elts=" << sgs * sgs << " align=wordx32\n";
                for (std::int32_t j0 = offset; j0 < offset + remainder; ++j0) {
                    const auto t1 = region_origin(sty_size, sgs * (j0 - offset) * sty_size);
                    const auto a1 =
                        region_origin(sty_size, sgs * al.component_no(j0, 0) * sty_size);
                    const auto a2 =
                        region_origin(sty_size, sgs * al.component_no(j0, 1) * sty_size);
                    oasm << reduce << " (M1," << sgs << ") " << tmp << "(" << t1[0] << "," << t1[1]
                         << ")<1> " << aview << "(" << a1[0] << "," << a1[1] << ")<1;1,0> " << aview
                         << "(" << a2[0] << "," << a2[1] << ")<1;1,0>\n";
                    for (std::int32_t b = 2; b < al.blocks; ++b) {
                        const auto a2 =
                            region_origin(sty_size, sgs * al.component_no(j0, b) * sty_size);
                        oasm << reduce << " (M1," << sgs << ") " << tmp << "(" << t1[0] << ","
                             << t1[1] << ")<1> " << tmp << "(" << t1[0] << "," << t1[1]
                             << ")<1;1,0> " << aview << "(" << a2[0] << "," << a2[1]
                             << ")<1;1,0>\n";
                    }
                }
                src = tmp;
            }

            for (std::int32_t v = 1; v < sgs; v *= 2) {
                std::uint32_t pval = 0;
                for (std::uint32_t j = 0; j < 32; j += 2 * v) {
                    pval |= (((1 << v) - 1) << j);
                }
                oasm << "setp (M1," << sgs << ") " << predicate << " " << pval << ":ud\n";

                std::string dst = rview;
                auto dst_offset = offset;
                if (2 * v < sgs) {
                    auto tmp = tmp_("tmp");
                    oasm << ".decl " << tmp << " v_type=G type=" << visa_type(at_sty)
                         << " num_elts=" << sgs * sgs / (2 * v) << " align=wordx32\n";
                    dst = tmp;
                    dst_offset = 0;
                }

                for (int i = 0; i < sgs / v && i < remainder; i += 2) {
                    auto tmp1 = tmp_("tmp");
                    oasm << ".decl " << tmp1 << " v_type=G type=" << visa_type(at_sty)
                         << " num_elts=" << sgs << " align=wordx32\n";
                    auto tmp2 = tmp_("tmp");
                    oasm << ".decl " << tmp2 << " v_type=G type=" << visa_type(at_sty)
                         << " num_elts=" << sgs << " align=wordx32\n";

                    const auto t0 = region_origin(sty_size, (dst_offset + sgs * i / 2) * sty_size);
                    const auto t1 = region_origin(sty_size, sgs * i * sty_size);
                    const auto t1down = region_origin(sty_size, (sgs * i + v) * sty_size);
                    const auto t2 = region_origin(sty_size, sgs * (i + 1) * sty_size);
                    const auto t2up = region_origin(sty_size, (sgs * (i + 1) - v) * sty_size);
                    oasm << "(!" << predicate << ") sel (M1," << sgs << ") " << tmp1 << "(0,0)<1> "
                         << src << "(" << t2up[0] << "," << t2up[1] << ")<1;1,0> " << src << "("
                         << t1[0] << "," << t1[1] << ")<1;1,0>\n";
                    oasm << "(" << predicate << ") sel (M1," << sgs << ") " << tmp2 << "(0,0)<1> "
                         << src << "(" << t1down[0] << "," << t1down[1] << ")<1;1,0> " << src << "("
                         << t2[0] << "," << t2[1] << ")<1;1,0>\n";
                    oasm << reduce << " (M1," << sgs << ") " << dst << "(" << t0[0] << "," << t0[1]
                         << ")<1> " << tmp1 << "(0,0)<1;1,0> " << tmp2 << "(0,0)<1;1,0>\n";
                }
                src = dst;
            }
        }
        oasm << "}\n";

        auto fun_ty = unique().function_ty(matrix_ty, array_view<spv_inst *>{spv_ty(al)});
        return unique().mod().add_to<OpAsmINTEL>(section::type_const_var, matrix_ty, fun_ty,
                                                 unique().asm_target(), std::move(oasm).str(),
                                                 "=rw,rw");
    });
}

auto coopmatrix_impl_dpas::load(cooperative_matrix_load_inst in, dope_vector const &odv,
                                spv_inst *pointer, spv_inst *pos0, spv_inst *pos1) -> spv_inst * {
    auto rt = get_coopmatrix_type(in.result());
    const bool sgs_ok = cfg().subgroup_size == cfg().matrix->required_subgroup_size();
    const auto type_ok = cfg().matrix->have_type(rt);
    const auto block_io_ok = check_2d_block_io(in.operand(), in.pos0());
    const bool transpose_ok = in.t() == transpose::N || rt->use() == matrix_use::a;

    if (!sgs_ok || !type_ok || !block_io_ok || !transpose_ok) {
        return coopmatrix_impl_block::load(in, odv, pointer, pos0, pos1);
    }

    auto ot = get_memref_type(in.operand());
    auto ot_sty = dyn_cast<number_type>(ot->element_ty())->ty();
    auto ct = get_coopmatrix_type(in.result());
    auto fun = load_fun(ct, unique().pointer_ty(ot), in.t());

    auto &mod = unique().mod();
    auto spv_i32_ty = unique().scalar_ty(scalar_type::i32);
    auto csize = unique().constant(static_cast<std::int32_t>(size(ot_sty)));
    auto shape0_i32 = mod.add<OpSConvert>(spv_i32_ty, odv.shape(0));
    auto width_in_bytes = mod.add<OpIMul>(spv_i32_ty, shape0_i32, csize);
    auto height = mod.add<OpSConvert>(spv_i32_ty, odv.shape(1));
    auto stride1_i32 = mod.add<OpSConvert>(spv_i32_ty, odv.stride(1));
    auto stride_in_bytes = mod.add<OpIMul>(spv_i32_ty, stride1_i32, csize);
    auto pos0_i32 = mod.add<OpSConvert>(spv_i32_ty, pos0);
    auto pos1_i32 = mod.add<OpSConvert>(spv_i32_ty, pos1);

    return mod.add<OpAsmCallINTEL>(spv_ty(ct), fun,
                                   array_view<spv_inst *>{pointer, width_in_bytes, height,
                                                          stride_in_bytes, pos0_i32, pos1_i32});
}

auto coopmatrix_impl_dpas::mul_add(cooperative_matrix_mul_add_inst in, spv_inst *a, spv_inst *b,
                                   spv_inst *c) -> spv_inst * {
    auto at = get_coopmatrix_type(in.a());
    auto bt = get_coopmatrix_type(in.b());
    auto ct = get_coopmatrix_type(in.c());
    auto rt = get_coopmatrix_type(in.result());
    auto at_sty = dyn_cast<number_type>(at->component_ty())->ty();
    auto bt_sty = dyn_cast<number_type>(bt->component_ty())->ty();
    auto ct_sty = dyn_cast<number_type>(ct->component_ty())->ty();
    auto rt_sty = dyn_cast<number_type>(rt->component_ty())->ty();
    const bool sgs_ok = cfg().subgroup_size == cfg().matrix->required_subgroup_size();
    const bool have_gemm =
        cfg().matrix->have_gemm(at_sty, bt_sty, ct_sty, rt_sty, rt->rows(), rt->cols(), at->cols());
    if (!sgs_ok || !have_gemm) {
        return coopmatrix_impl_block::mul_add(in, a, b, c);
    }

    auto fun = mul_add_fun(at, bt, ct, rt, in.is_c_zero());
    return unique().mod().add<OpAsmCallINTEL>(spv_ty(rt), fun, array_view<spv_inst *>{a, b, c});
}

void coopmatrix_impl_dpas::prefetch(cooperative_matrix_prefetch_inst in, dope_vector const &odv,
                                    spv_inst *pointer, spv_inst *pos0, spv_inst *pos1) {
    auto ot = get_memref_type(in.operand());
    auto ot_sty = dyn_cast<number_type>(ot->element_ty())->ty();
    const bool sgs_ok = cfg().subgroup_size == cfg().matrix->required_subgroup_size();
    const auto type_ok = size(ot_sty) <= 4;
    const auto block_io_ok = check_2d_block_io(in.operand(), in.pos0());

    if (!sgs_ok || !type_ok || !block_io_ok) {
        coopmatrix_impl_block::prefetch(in, odv, pointer, pos0, pos1);
    } else {
        auto fun =
            prefetch_fun(in.cache_level(), ot_sty, unique().pointer_ty(ot), in.rows(), in.cols());

        if (fun) {
            auto &mod = unique().mod();
            auto spv_void_ty = unique().void_ty();
            auto spv_i32_ty = unique().scalar_ty(scalar_type::i32);
            auto csize = unique().constant(static_cast<std::int32_t>(size(ot_sty)));
            auto shape0_i32 = mod.add<OpSConvert>(spv_i32_ty, odv.shape(0));
            auto width_in_bytes = mod.add<OpIMul>(spv_i32_ty, shape0_i32, csize);
            auto height = mod.add<OpSConvert>(spv_i32_ty, odv.shape(1));
            auto stride1_i32 = mod.add<OpSConvert>(spv_i32_ty, odv.stride(1));
            auto stride_in_bytes = mod.add<OpIMul>(spv_i32_ty, stride1_i32, csize);
            auto pos0_i32 = mod.add<OpSConvert>(spv_i32_ty, pos0);
            auto pos1_i32 = mod.add<OpSConvert>(spv_i32_ty, pos1);

            mod.add<OpAsmCallINTEL>(spv_void_ty, fun,
                                    array_view<spv_inst *>{pointer, width_in_bytes, height,
                                                           stride_in_bytes, pos0_i32, pos1_i32});
        }
    }
}

void coopmatrix_impl_dpas::store(cooperative_matrix_store_inst in, dope_vector const &odv,
                                 spv_inst *val, spv_inst *pointer, spv_inst *pos0, spv_inst *pos1) {
    auto ct = get_coopmatrix_type(in.val());
    const bool transpose_ok = in.t() == transpose::N;
    const bool sgs_ok = cfg().subgroup_size == cfg().matrix->required_subgroup_size();
    const auto type_ok = cfg().matrix->have_type(ct);
    const auto block_io_ok = check_2d_block_io(in.operand(), in.pos0());

    if (!transpose_ok || !sgs_ok || !type_ok || !block_io_ok) {
        coopmatrix_impl_block::store(in, odv, val, pointer, pos0, pos1);
    } else {
        auto ot = get_memref_type(in.operand());
        auto ot_sty = dyn_cast<number_type>(ot->element_ty())->ty();
        auto fun = store_fun(ct, unique().pointer_ty(ot));

        auto &mod = unique().mod();
        auto spv_void_ty = unique().void_ty();
        auto spv_i32_ty = unique().scalar_ty(scalar_type::i32);
        auto csize = unique().constant(static_cast<std::int32_t>(size(ot_sty)));
        auto shape0_i32 = mod.add<OpSConvert>(spv_i32_ty, odv.shape(0));
        auto width_in_bytes = mod.add<OpIMul>(spv_i32_ty, shape0_i32, csize);
        auto height = mod.add<OpSConvert>(spv_i32_ty, odv.shape(1));
        auto stride1_i32 = mod.add<OpSConvert>(spv_i32_ty, odv.stride(1));
        auto stride_in_bytes = mod.add<OpIMul>(spv_i32_ty, stride1_i32, csize);
        auto pos0_i32 = mod.add<OpSConvert>(spv_i32_ty, pos0);
        auto pos1_i32 = mod.add<OpSConvert>(spv_i32_ty, pos1);

        mod.add<OpAsmCallINTEL>(spv_void_ty, fun,
                                array_view<spv_inst *>{val, pointer, width_in_bytes, height,
                                                       stride_in_bytes, pos0_i32, pos1_i32});
    }
}

auto coopmatrix_impl_dpas::reduce(cooperative_matrix_reduce_inst in, spv_inst *a) -> spv_inst * {
    auto at = get_coopmatrix_type(in.a());
    const auto sgs = cfg().subgroup_size;

    if (in.mode() != reduce_mode::column || at->rows() % sgs != 0 || at->use() == matrix_use::a) {
        return coopmatrix_impl::reduce(in, a);
    }

    auto rt = get_coopmatrix_type(in.result());
    auto fun = reduce_fun(sgs, in.get().type_id(), at, rt);
    return unique().mod().add<OpAsmCallINTEL>(spv_ty(rt), fun, array_view<spv_inst *>{a});
}

} // namespace tinytc::spv

