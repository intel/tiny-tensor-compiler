// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "spv/coopmatrix_diy.hpp"
#include "codegen_tools.hpp"
#include "error.hpp"
#include "node/data_type_node.hpp"
#include "node/inst_node.hpp"
#include "node/value_node.hpp"
#include "spv/block2d_diy.hpp"
#include "spv/defs.hpp"
#include "spv/dope_vector.hpp"
#include "spv/enums.hpp"
#include "spv/instructions.hpp"
#include "spv/lut.hpp"
#include "spv/module.hpp"
#include "spv/uniquifier.hpp"
#include "spv/xe_constants.hpp"
#include "support/casting.hpp"
#include "support/util.hpp"
#include "support/visit.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <bit>
#include <sstream>
#include <string>
#include <utility>
#include <variant>
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

coopmatrix_diy::coopmatrix_diy(tinytc_spv_mod &m, uniquifier &unique)
    : mod_{&m}, unique_{&unique} {}

auto coopmatrix_diy::max_rows_in_block(matrix_use use, std::int32_t element_size) const
    -> std::int32_t {
    if (use == matrix_use::b) {
        std::int32_t ops_per_chan = xe::channel_size / element_size;
        return ops_per_chan * xe::sdepth;
    }
    return xe::exec_size;
}

auto coopmatrix_diy::load_config(coopmatrix_data_type const *ct, transpose trans,
                                 address_space addrspace) -> block_config {
    auto cfg = block_config{};
    cfg.element_size = size(ct->component_ty());
    cfg.array_length = 1;
    cfg.rows = ct->rows();
    cfg.cols = ct->cols();
    cfg.row_blocks = 1;
    cfg.col_blocks = 1;
    cfg.transpose = trans == transpose::T;
    cfg.vnni = ct->use() == matrix_use::a;
    cfg.sfid = addrspace == address_space::local ? lsc_sfid::slm : lsc_sfid::ugm;
    cfg.pos0_shr = 0;
    cfg.post_d32_transpose8x8 = false;

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
        adjust_cols(xe::exec_size);

        const auto ops_per_chan = 4 / cfg.element_size;
        cfg.rows /= ops_per_chan;
        cfg.element_size = 4;
        cfg.pos0_shr = ilog2(ops_per_chan);
        cfg.vnni = false;
        const auto max_rows = xe::exec_size / 2;
        adjust_rows(max_rows, 1);
    } else if (cfg.transpose) {
        std::swap(cfg.rows, cfg.cols);
        cfg.vnni = true;
        cfg.transpose = false;
        cfg.post_d32_transpose8x8 = true;

        const std::int32_t max_cols = max_rows_in_block(ct->use(), cfg.element_size);
        const std::int32_t max_rows = 8;
        adjust_cols(max_cols);
        adjust_rows(max_rows, max_array_length(max_rows));
    } else {
        const std::int32_t max_cols = 32;
        const std::int32_t max_rows = max_rows_in_block(ct->use(), cfg.element_size);

        adjust_cols(max_cols);
        adjust_rows(max_rows, max_array_length(max_rows));
    }

    return cfg;
}

auto coopmatrix_diy::load_fun(coopmatrix_data_type const *result_ty, spv_inst *spv_operand_ty,
                              transpose trans, address_space addrspace) -> spv_inst * {
    const auto key = load_key{result_ty, spv_operand_ty, trans, addrspace};
    return lookup(load_funs_, key, [&](load_key const &key) {
        const auto [result_ty, spv_operand_ty, trans, addrspace] = key;

        const auto cfg = load_config(result_ty, trans, addrspace);
        auto code = load_block2d(cfg, result_ty->component_ty(), tmp_);

        auto spv_i32_ty = unique_->spv_ty(scalar_type::i32);
        auto spv_result_ty = unique_->spv_ty(result_ty);
        auto fun_ty = unique_->spv_function_ty(
            spv_result_ty, array_view<spv_inst *>{spv_operand_ty, spv_i32_ty, spv_i32_ty,
                                                  spv_i32_ty, spv_i32_ty, spv_i32_ty});
        return mod_->add_to<OpAsmINTEL>(section::type_const_var, spv_result_ty, fun_ty,
                                        unique_->asm_target(), code,
                                        "=rw,rw.u,rw.u,rw.u,rw.u,rw.u,rw.u");
    });
}

auto coopmatrix_diy::store_config(coopmatrix_data_type const *ct, address_space addrspace)
    -> block_config {
    constexpr std::int32_t max_cols_in_block = 8;

    auto cfg = block_config{};
    cfg.element_size = size(ct->component_ty());
    cfg.array_length = 1;
    cfg.rows = ct->rows();
    cfg.cols = ct->cols();
    cfg.row_blocks = 1;
    cfg.col_blocks = 1;
    cfg.transpose = false;
    cfg.vnni = false;
    cfg.sfid = addrspace == address_space::local ? lsc_sfid::slm : lsc_sfid::ugm;
    cfg.pos0_shr = 0;
    cfg.post_d32_transpose8x8 = false;

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

auto coopmatrix_diy::store_fun(coopmatrix_data_type const *val_ty, spv_inst *spv_operand_ty,
                               address_space addrspace) -> spv_inst * {
    const auto key = store_key{val_ty, spv_operand_ty, addrspace};
    return lookup(store_funs_, key, [&](store_key const &key) {
        const auto [val_ty, spv_operand_ty, addrspace] = key;

        const auto cfg = store_config(val_ty, addrspace);
        auto code = store_block2d(cfg, val_ty->component_ty(), tmp_);

        auto spv_void_ty = unique_->void_ty();
        auto spv_val_ty = unique_->spv_ty(val_ty);
        auto spv_i32_ty = unique_->spv_ty(scalar_type::i32);
        auto fun_ty = unique_->spv_function_ty(
            spv_void_ty, array_view<spv_inst *>{spv_val_ty, spv_operand_ty, spv_i32_ty, spv_i32_ty,
                                                spv_i32_ty, spv_i32_ty, spv_i32_ty});
        auto asmop = mod_->add_to<OpAsmINTEL>(section::type_const_var, spv_void_ty, fun_ty,
                                              unique_->asm_target(), code,
                                              "rw,rw.u,rw.u,rw.u,rw.u,rw.u,rw.u");
        mod_->add_to<OpDecorate>(section::decoration, asmop, Decoration::SideEffectsINTEL);
        return asmop;
    });
}

auto coopmatrix_diy::mul_add_fun(coopmatrix_data_type const *at, coopmatrix_data_type const *bt,
                                 coopmatrix_data_type const *ct, coopmatrix_data_type const *rt)
    -> spv_inst * {
    const auto key = mul_add_key{at, bt, ct, rt};
    return lookup(mul_add_funs_, key, [&](mul_add_key const &key) {
        const auto [at, bt, ct, rt] = key;

        auto oasm = std::ostringstream{};

        const std::int32_t ops_per_chan = xe::channel_size / size(at->component_ty());
        const std::int32_t K = ops_per_chan * xe::sdepth;

        oasm << "{\n";
        std::string result_placeholder = "$0";
        std::string temp = result_placeholder;
        if (rt->component_ty() != ct->component_ty() && at->cols() / K > 1) {
            temp = tmp_("temp");
            oasm << ".decl " << temp << " v_type=G type=" << visa_type(ct->component_ty())
                 << " num_elts=" << ct->rows() * ct->cols() << " align=wordx32\n";
        }

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
        const auto precision_src1 = precision(at->component_ty());
        const auto precision_src2 = precision(bt->component_ty());
        for (std::int32_t k = 0; k < at->cols(); k += K) {
            char const *src0 = k > 0 ? temp.c_str() : "$3";
            char const *dst = k + K >= at->cols() ? result_placeholder.c_str() : temp.c_str();
            const auto rsize =
                k + K >= at->cols() ? size(rt->component_ty()) : size(ct->component_ty());
            for (std::int32_t m = 0; m < ct->rows(); m += xe::exec_size) {
                for (std::int32_t n = 0; n < ct->cols(); n += xe::rcount) {
                    const auto aoffset =
                        (k * xe::exec_size + m * at->cols()) * size(at->component_ty());
                    const auto brow =
                        (k * bt->cols() + n * K) * size(bt->component_ty()) / xe::grf_size;
                    const auto coffset =
                        (m * ct->cols() + n * xe::exec_size) * size(ct->component_ty());
                    const auto roffset = (m * rt->cols() + n * xe::exec_size) * rsize;
                    oasm << "dpas." << precision_src1 << "." << precision_src2 << "." << xe::sdepth
                         << "." << xe::rcount << " (M1," << xe::exec_size << ") " << dst << "."
                         << roffset << " " << src0 << "." << coffset << " $1." << aoffset << " $2("
                         << brow << ",0)\n";
                }
            }
        }
        oasm << "}\n";

        auto spv_a_ty = unique_->spv_ty(at);
        auto spv_b_ty = unique_->spv_ty(bt);
        auto spv_c_ty = unique_->spv_ty(ct);
        auto spv_result_ty = unique_->spv_ty(rt);
        auto fun_ty = unique_->spv_function_ty(
            spv_result_ty, array_view<spv_inst *>{spv_a_ty, spv_b_ty, spv_c_ty});

        return mod_->add_to<OpAsmINTEL>(section::type_const_var, spv_result_ty, fun_ty,
                                        unique_->asm_target(), std::move(oasm).str(),
                                        "=rw,rw,rw,rw");
    });
}

auto coopmatrix_diy::cast_fun(scalar_type to_ty, scalar_type from_ty, std::int32_t num_components)
    -> spv_inst * {
    const auto key = cast_key{to_ty, from_ty, num_components};
    return lookup(cast_funs_, key, [&](cast_key const &key) {
        auto &[to_ty, from_ty, num_components] = key;
        const auto num_elements = num_components * xe::exec_size;

        auto spv_component_ty = unique_->spv_ty(to_ty);
        auto spv_operation_ty = unique_->spv_vec_ty(spv_component_ty, num_components);
        const auto to_width = xe::grf_size / size(to_ty);
        const auto from_width = xe::grf_size / size(from_ty);
        const auto to_visa_ty = visa_type(to_ty);
        const auto from_visa_ty = visa_type(from_ty);

        auto oasm = std::ostringstream{};
        const auto a_tmp = tmp_("a_tmp");
        const auto b_tmp = tmp_("b_tmp");
        oasm << "{\n";
        oasm << ".decl " << a_tmp << " v_type=G type=" << from_visa_ty
             << " num_elts=" << num_elements << " align=wordx32 alias=<$1, 0>\n";
        oasm << ".decl " << b_tmp << " v_type=G type=" << to_visa_ty << " num_elts=" << num_elements
             << " align=wordx32 alias=<$0, 0>\n";
        for (std::int32_t m = 0; m < num_elements; m += xe::exec_size) {
            auto R_from = m / from_width;
            auto C_from = m % from_width;
            auto R_to = m / to_width;
            auto C_to = m % to_width;
            oasm << "mov (M1," << xe::exec_size << ") " << b_tmp << "(" << R_to << "," << C_to
                 << ")<1> " << a_tmp << "(" << R_from << "," << C_from << ")<1;1,0>\n";
        }
        oasm << "}\n";

        auto fun_ty =
            unique_->spv_function_ty(spv_operation_ty, array_view<spv_inst *>{spv_operation_ty});
        return mod_->add_to<OpAsmINTEL>(section::type_const_var, spv_operation_ty, fun_ty,
                                        unique_->asm_target(), std::move(oasm).str(), "=rw,rw");
    });
}

auto coopmatrix_diy::arith_fun(arithmetic op, scalar_type cty, std::int32_t num_components)
    -> spv_inst * {
    const auto key = arith_key{op, cty, num_components};
    return lookup(arith_funs_, key, [&](arith_key const &key) {
        const auto &[op, cty, num_components] = key;
        const auto num_elements = num_components * xe::exec_size;

        auto spv_component_ty = unique_->spv_ty(cty);
        auto spv_operation_ty = unique_->spv_vec_ty(spv_component_ty, num_components);
        const auto width = xe::grf_size / size(cty);
        const auto visa_ty = visa_type(cty);

        auto oasm = std::ostringstream{};
        const auto a_tmp = tmp_("a_tmp");
        const auto b_tmp = tmp_("b_tmp");
        const auto c_tmp = tmp_("c_tmp");
        oasm << "{\n";
        oasm << ".decl " << a_tmp << " v_type=G type=" << visa_ty << " num_elts=" << num_elements
             << " align=wordx32 alias=<$1, 0>\n";
        oasm << ".decl " << b_tmp << " v_type=G type=" << visa_ty << " num_elts=" << num_elements
             << " align=wordx32 alias=<$2, 0>\n";
        oasm << ".decl " << c_tmp << " v_type=G type=" << visa_ty << " num_elts=" << num_elements
             << " align=wordx32 alias=<$0, 0>\n";
        auto [opcode, neg_prefix] = [](arithmetic op) -> std::pair<char const *, char const *> {
            switch (op) {
            case arithmetic::add:
                return {"add", ""};
            case arithmetic::sub:
                return {"add", "(-)"};
            case arithmetic::mul:
                return {"mul", ""};
            case arithmetic::div:
                return {"div", ""};
            default:
                throw status::ir_coopmatrix_unsupported;
            }
        }(op);
        for (std::int32_t m = 0; m < num_elements; m += xe::exec_size) {
            auto R = m / width;
            auto C = m % width;
            oasm << opcode << " (M1," << xe::exec_size << ") " << c_tmp << "(" << R << "," << C
                 << ")<1> " << a_tmp << "(" << R << "," << C << ")<1;1,0> " << neg_prefix << b_tmp
                 << "(" << R << "," << C << ")<1;1,0>\n";
        }
        oasm << "}\n";

        auto fun_ty = unique_->spv_function_ty(
            spv_operation_ty, array_view<spv_inst *>{spv_component_ty, spv_operation_ty});
        return mod_->add_to<OpAsmINTEL>(section::type_const_var, spv_operation_ty, fun_ty,
                                        unique_->asm_target(), std::move(oasm).str(), "=rw,rw,rw");
    });
}

auto coopmatrix_diy::scale_fun(scalar_type cty, std::int32_t num_components) -> spv_inst * {
    const auto key = scale_key{cty, num_components};
    return lookup(scale_funs_, key, [&](scale_key const &key) {
        auto &[cty, num_components] = key;
        const auto num_elements = num_components * xe::exec_size;

        auto spv_component_ty = unique_->spv_ty(cty);
        auto spv_operation_ty = unique_->spv_vec_ty(spv_component_ty, num_components);
        const auto width = xe::grf_size / size(cty);
        const auto visa_ty = visa_type(cty);

        auto oasm = std::ostringstream{};
        const auto a_tmp = tmp_("a_tmp");
        const auto b_tmp = tmp_("b_tmp");
        const auto c_tmp = tmp_("c_tmp");
        oasm << "{\n";
        oasm << ".decl " << a_tmp << " v_type=G type=" << visa_ty
             << " num_elts=1 align=word alias=<$1, 0>\n";
        oasm << ".decl " << b_tmp << " v_type=G type=" << visa_ty << " num_elts=" << num_elements
             << " align=wordx32 alias=<$2, 0>\n";
        oasm << ".decl " << c_tmp << " v_type=G type=" << visa_ty << " num_elts=" << num_elements
             << " align=wordx32 alias=<$0, 0>\n";
        for (std::int32_t m = 0; m < num_elements; m += xe::exec_size) {
            auto R = m / width;
            auto C = m % width;
            oasm << "mul (M1," << xe::exec_size << ") " << c_tmp << "(" << R << "," << C << ")<1> "
                 << a_tmp << "(0,0)<0;1,0> " << b_tmp << "(" << R << "," << C << ")<1;1,0>\n";
        }
        oasm << "}\n";

        auto fun_ty = unique_->spv_function_ty(
            spv_operation_ty, array_view<spv_inst *>{spv_component_ty, spv_operation_ty});
        return mod_->add_to<OpAsmINTEL>(section::type_const_var, spv_operation_ty, fun_ty,
                                        unique_->asm_target(), std::move(oasm).str(),
                                        "=rw,rw.u,rw");
    });
}

auto coopmatrix_diy::arith(arith_inst const &in, spv_inst *a, spv_inst *b) -> spv_inst * {
    auto rt = get_coopmatrix_type(in.result(0));

    const scalar_type cty = rt->component_ty();
    const std::int32_t num_components = rt->rows() * rt->cols() / xe::exec_size;
    auto spv_component_ty = unique_->spv_ty(cty);
    auto spv_operation_ty = unique_->spv_vec_ty(spv_component_ty, num_components);

    auto fun = arith_fun(in.operation(), cty, num_components);
    auto c = mod_->add<OpAsmCallINTEL>(spv_operation_ty, fun, array_view<spv_inst *>{a, b});
    return mod_->add<OpBitcast>(unique_->spv_ty(rt), c);
}

auto coopmatrix_diy::cast(cast_inst const &in, spv_inst *a) -> spv_inst * {
    auto at = get_coopmatrix_type(in.a());
    auto rt = get_coopmatrix_type(in.result(0));

    const scalar_type to_ty = rt->component_ty();
    const scalar_type from_ty = at->component_ty();
    const std::int32_t num_components = rt->rows() * rt->cols() / xe::exec_size;
    auto spv_component_ty = unique_->spv_ty(to_ty);
    auto spv_operation_ty = unique_->spv_vec_ty(spv_component_ty, num_components);

    auto fun = cast_fun(to_ty, from_ty, num_components);
    auto b = mod_->add<OpAsmCallINTEL>(spv_operation_ty, fun, array_view<spv_inst *>{a});
    return mod_->add<OpBitcast>(unique_->spv_ty(rt), b);
}

auto coopmatrix_diy::constant(constant_inst const &in) -> spv_inst * {
    auto spv_result_ty = unique_->spv_ty(in.result(0).ty());
    if (in.is_zero()) {
        return unique_->null_constant(spv_result_ty);
    }

    auto spv_vec_ty = dyn_cast<OpTypeVector>(spv_result_ty);
    if (!spv_vec_ty) {
        throw compilation_error(in.loc(), status::internal_compiler_error);
    }
    const auto sty = get_coopmatrix_type(in.result(0))->component_ty();

    spv_inst *cst = nullptr;
    if (sty == scalar_type::i32) {
        cst = std::visit(
            overloaded{[&](std::int64_t i) -> spv_inst * {
                           switch (sty) {
                           case scalar_type::i8: {
                               auto v8 = std::bit_cast<std::uint8_t>(static_cast<std::int8_t>(i));
                               return unique_->constant(
                                   std::int32_t{v8 | (v8 << 8) | (v8 << 16) | (v8 << 24)});
                               return unique_->constant(static_cast<std::int8_t>(i));
                           }
                           case scalar_type::i32:
                               return unique_->constant(static_cast<std::int32_t>(i));
                           default:
                               return nullptr;
                           }
                       },
                       [&](double d) -> spv_inst * {
                           const float f = static_cast<float>(d);
                           switch (sty) {
                           case scalar_type::bf16: {
                               std::uint16_t v16 = bfloat16{f}.bits();
                               return unique_->constant(std::int32_t{v16 | (v16 << 16)});
                           }
                           case scalar_type::f16: {
                               std::uint16_t v16 = half{f}.bits();
                               return unique_->constant(std::int32_t{v16 | (v16 << 16)});
                           }
                           case scalar_type::f32:
                               return unique_->constant(std::bit_cast<std::int32_t>(f));
                           default:
                               return nullptr;
                           }
                       },
                       [&](auto const &) -> spv_inst * { return nullptr; }},
            in.value());
    } else {
        cst = std::visit(overloaded{[&](std::int64_t i) -> spv_inst * {
                                        switch (sty) {
                                        case scalar_type::i8: {
                                            return unique_->constant(static_cast<std::int8_t>(i));
                                        }
                                        case scalar_type::i32:
                                            return unique_->constant(static_cast<std::int32_t>(i));
                                        default:
                                            return nullptr;
                                        }
                                    },
                                    [&](double d) -> spv_inst * {
                                        const float f = static_cast<float>(d);
                                        switch (sty) {
                                        case scalar_type::bf16: {
                                            return unique_->constant(
                                                std::bit_cast<std::int16_t>(bfloat16{f}.bits()));
                                        }
                                        case scalar_type::f16: {
                                            return unique_->constant(half{f});
                                        }
                                        case scalar_type::f32:
                                            return unique_->constant(f);
                                        default:
                                            return nullptr;
                                        }
                                    },
                                    [&](auto const &) -> spv_inst * { return nullptr; }},
                         in.value());
    }

    if (!cst) {
        throw compilation_error(in.loc(), status::internal_compiler_error);
    }
    return mod_->add_to<OpConstantComposite>(section::type_const_var, spv_result_ty,
                                             std::vector<spv_inst *>(spv_vec_ty->op1(), cst));
}

auto coopmatrix_diy::load(cooperative_matrix_load_inst const &in, dope_vector const &odv,
                          spv_inst *pointer, spv_inst *pos0, spv_inst *pos1) -> spv_inst * {
    auto ot = get_memref_type(in.operand());
    auto ct = get_coopmatrix_type(in.result(0));
    auto spv_operand_ty = unique_->spv_ty(in.operand().ty());
    auto fun = load_fun(ct, spv_operand_ty, in.t(), ot->addrspace());

    auto spv_i32_ty = unique_->spv_ty(scalar_type::i32);
    auto csize = unique_->constant(static_cast<std::int32_t>(size(ot->element_ty())));
    auto shape0_i32 = mod_->add<OpSConvert>(spv_i32_ty, odv.shape(0));
    auto width_in_bytes = mod_->add<OpIMul>(spv_i32_ty, shape0_i32, csize);
    auto height = mod_->add<OpSConvert>(spv_i32_ty, odv.shape(1));
    auto stride1_i32 = mod_->add<OpSConvert>(spv_i32_ty, odv.stride(1));
    auto stride_in_bytes = mod_->add<OpIMul>(spv_i32_ty, stride1_i32, csize);
    auto pos0_i32 = mod_->add<OpSConvert>(spv_i32_ty, pos0);
    auto pos1_i32 = mod_->add<OpSConvert>(spv_i32_ty, pos1);

    auto spv_result_ty = unique_->spv_ty(in.result(0).ty());
    return mod_->add<OpAsmCallINTEL>(spv_result_ty, fun,
                                     array_view<spv_inst *>{pointer, width_in_bytes, height,
                                                            stride_in_bytes, pos0_i32, pos1_i32});
}

auto coopmatrix_diy::mul_add(cooperative_matrix_mul_add_inst const &in, spv_inst *a, spv_inst *b,
                             spv_inst *c) -> spv_inst * {
    auto at = get_coopmatrix_type(in.a());
    auto bt = get_coopmatrix_type(in.b());
    auto ct = get_coopmatrix_type(in.c());
    auto rt = get_coopmatrix_type(in.result(0));
    auto spv_result_ty = unique_->spv_ty(rt);

    auto fun = mul_add_fun(at, bt, ct, rt);
    return mod_->add<OpAsmCallINTEL>(spv_result_ty, fun, array_view<spv_inst *>{a, b, c});
}

auto coopmatrix_diy::scale(cooperative_matrix_scale_inst const &in, spv_inst *a, spv_inst *b)
    -> spv_inst * {
    auto rt = get_coopmatrix_type(in.result(0));

    const scalar_type cty = rt->component_ty();
    const std::int32_t num_components = rt->rows() * rt->cols() / xe::exec_size;
    auto spv_component_ty = unique_->spv_ty(cty);
    auto spv_operation_ty = unique_->spv_vec_ty(spv_component_ty, num_components);

    auto fun = scale_fun(cty, num_components);
    auto c = mod_->add<OpAsmCallINTEL>(spv_operation_ty, fun, array_view<spv_inst *>{a, b});
    return mod_->add<OpBitcast>(unique_->spv_ty(rt), c);
}

void coopmatrix_diy::store(cooperative_matrix_store_inst const &in, dope_vector const &odv,
                           spv_inst *val, spv_inst *pointer, spv_inst *pos0, spv_inst *pos1) {
    auto ot = get_memref_type(in.operand());
    auto ct = get_coopmatrix_type(in.val());
    auto spv_operand_ty = unique_->spv_ty(in.operand().ty());
    auto fun = store_fun(ct, spv_operand_ty, ot->addrspace());

    auto spv_void_ty = unique_->void_ty();
    auto spv_i32_ty = unique_->spv_ty(scalar_type::i32);
    auto csize = unique_->constant(static_cast<std::int32_t>(size(ot->element_ty())));
    auto shape0_i32 = mod_->add<OpSConvert>(spv_i32_ty, odv.shape(0));
    auto width_in_bytes = mod_->add<OpIMul>(spv_i32_ty, shape0_i32, csize);
    auto height = mod_->add<OpSConvert>(spv_i32_ty, odv.shape(1));
    auto stride1_i32 = mod_->add<OpSConvert>(spv_i32_ty, odv.stride(1));
    auto stride_in_bytes = mod_->add<OpIMul>(spv_i32_ty, stride1_i32, csize);
    auto pos0_i32 = mod_->add<OpSConvert>(spv_i32_ty, pos0);
    auto pos1_i32 = mod_->add<OpSConvert>(spv_i32_ty, pos1);

    mod_->add<OpAsmCallINTEL>(spv_void_ty, fun,
                              array_view<spv_inst *>{val, pointer, width_in_bytes, height,
                                                     stride_in_bytes, pos0_i32, pos1_i32});
}

} // namespace tinytc::spv

