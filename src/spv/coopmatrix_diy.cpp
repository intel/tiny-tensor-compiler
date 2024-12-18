// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "spv/coopmatrix_diy.hpp"
#include "codegen_tools.hpp"
#include "error.hpp"
#include "node/data_type_node.hpp"
#include "node/inst_node.hpp"
#include "node/value_node.hpp"
#include "spv/defs.hpp"
#include "spv/dope_vector.hpp"
#include "spv/instructions.hpp"
#include "spv/lut.hpp"
#include "spv/module.hpp"
#include "spv/uniquifier.hpp"
#include "support/casting.hpp"
#include "support/visit.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <algorithm>
#include <bit>
#include <optional>
#include <sstream>
#include <utility>
#include <variant>
#include <vector>

namespace tinytc::spv {

coopmatrix_diy::coopmatrix_diy(tinytc_spv_mod &m, uniquifier &unique)
    : mod_{&m}, unique_{&unique} {}

auto coopmatrix_diy::max_rows_in_block(coopmatrix_data_type const *ct) const -> std::int32_t {
    if (ct->use() == matrix_use::b) {
        std::int32_t ops_per_chan = channel_size / size(ct->component_ty());
        return ops_per_chan * sdepth;
    }
    return exec_size;
}

auto coopmatrix_diy::load_config(coopmatrix_data_type const *ct) -> block_config {
    auto cfg = block_config{};
    cfg.element_size = size(ct->component_ty());
    cfg.array_length = 1;
    cfg.rows = ct->rows();
    cfg.cols = ct->cols();
    cfg.row_blocks = 1;
    cfg.col_blocks = 1;
    cfg.vnni = ct->use() == matrix_use::a;

    const auto max_rows = max_rows_in_block(ct);
    if (cfg.rows > max_rows) {
        const std::int32_t num_blocks = cfg.rows / max_rows;
        const std::int32_t max_array_length = 64 / (max_rows * cfg.element_size);
        if (num_blocks > max_array_length) {
            cfg.array_length = max_array_length;
            cfg.row_blocks = num_blocks / max_array_length;
        } else {
            cfg.array_length = num_blocks;
        }
        cfg.rows = max_rows;
    }

    return cfg;
}

auto coopmatrix_diy::load_fun(coopmatrix_data_type const *result_ty,
                              spv_inst *spv_operand_ty) -> spv_inst * {
    auto key = std::make_pair(result_ty, spv_operand_ty);
    return lookup(
        load_funs_, key, [&](std::pair<coopmatrix_data_type const *, spv_inst *> const &key) {
            const auto [result_ty, spv_operand_ty] = key;

            const auto cfg = load_config(result_ty);

            const std::uint32_t num_dst =
                std::min(31, cfg.element_size * cfg.rows * cfg.cols / grf_size);
            const std::uint32_t desc = [&] {
                const std::uint32_t data_size = cfg.element_size == 4 ? 2 : 1;
                std::uint32_t d = 3;
                if (cfg.vnni) {
                    d |= 1 << 7;
                }
                d |= data_size << 9;
                d |= num_dst << 20;
                d |= 1 << 25;
                return d;
            }();
            const std::uint32_t block_size =
                ((cfg.array_length - 1) << 16) | ((cfg.cols - 1) << 8) | (cfg.rows - 1);
            auto oasm = std::ostringstream{};
            oasm << "{\n"
                    ".decl temp v_type=G type=ud num_elts=8 align=wordx32\n"
                    ".decl tempq v_type=G type=uq num_elts=4 align=wordx32 alias=<temp, 0>\n"
                    "mov (M1,1) tempq(0,0)<1> $1(0,0)<0;1,0>\n"
                    "mov (M1,1) temp(0,2)<1> $2(0,0)<0;1,0>\n"
                    "mov (M1,1) temp(0,3)<1> $3(0,0)<0;1,0>\n"
                    "mov (M1,1) temp(0,4)<1> $4(0,0)<0;1,0>\n"
                    "mov (M1,1) temp(0,5)<1> $5(0,0)<0;1,0>\n"
                    "mov (M1,1) temp(0,6)<1> $6(0,0)<0;1,0>\n"
                    "mov (M1,1) temp(0,7)<1> 0x"
                 << std::hex << block_size << ":ud\n";

            for (std::int32_t m = 0; m < cfg.row_blocks; ++m) {
                for (std::int32_t n = 0; n < cfg.col_blocks; ++n) {
                    oasm << std::dec << "raw_sends.15.1.0." << num_dst << " (M1, 1) 0x0:ud 0x"
                         << std::hex << desc << ":ud temp.0 %null.0 $0." << std::dec
                         << cfg.byte_offset(m, n) << "\n";
                    if (n + 1 < cfg.col_blocks) {
                        oasm << "add (M1,1) temp(0,6)<1> temp(0,6)<0;1,0> " << cfg.cols << ":ud\n";
                    }
                    // oasm << "lsc_load_block2d.ugm (M1,1) $0." << cfg.byte_offset(m, n) << ":d"
                    //<< cfg.element_size * 8 << "." << cfg.array_length << "x" << cfg.rows << "x"
                    //<< cfg.cols << "n" << (vnni ? "t" : "n") << " flat[$1,$2,$3,$4,$5,$6]\n";
                }
                if (m + 1 < cfg.row_blocks) {
                    oasm << "add (M1,1) temp(0,6)<1> temp(0,6)<0;1,0> "
                         << -(cfg.col_blocks - 1) * cfg.cols << ":ud\n";
                    oasm << "add (M1,1) temp(0,7)<1> temp(0,7)<0;1,0> " << cfg.rows << ":ud\n";
                }
            }
            oasm << "}\n";

            auto spv_i32_ty = unique_->spv_ty(scalar_type::i32);
            auto spv_result_ty = unique_->spv_ty(result_ty);
            auto fun_ty = unique_->spv_function_ty(
                spv_result_ty, array_view<spv_inst *>{spv_operand_ty, spv_i32_ty, spv_i32_ty,
                                                      spv_i32_ty, spv_i32_ty, spv_i32_ty});
            return mod_->add_to<OpAsmINTEL>(section::type_const_var, spv_result_ty, fun_ty,
                                            unique_->asm_target(), std::move(oasm).str(),
                                            "=rw,rw.u,rw.u,rw.u,rw.u,rw.u,rw.u");
        });
}

auto coopmatrix_diy::store_config(coopmatrix_data_type const *ct) -> block_config {
    constexpr std::int32_t max_cols_in_block = 8;

    auto cfg = block_config{};
    cfg.element_size = size(ct->component_ty());
    cfg.array_length = 1;
    cfg.rows = ct->rows();
    cfg.cols = ct->cols();
    cfg.row_blocks = 1;
    cfg.col_blocks = 1;
    cfg.vnni = false;

    if (cfg.cols > max_cols_in_block) {
        cfg.cols = max_cols_in_block;
        cfg.col_blocks = cfg.cols / max_cols_in_block;
    }

    const auto max_rows = max_rows_in_block(ct);
    if (cfg.rows > max_rows) {
        cfg.row_blocks = cfg.rows / max_rows;
        cfg.rows = max_rows;
    }

    return cfg;
}

auto coopmatrix_diy::store_fun(coopmatrix_data_type const *val_ty,
                               spv_inst *spv_operand_ty) -> spv_inst * {
    auto key = std::make_pair(val_ty, spv_operand_ty);
    return lookup(
        store_funs_, key, [&](std::pair<coopmatrix_data_type const *, spv_inst *> const &key) {
            const auto [val_ty, spv_operand_ty] = key;

            const auto cfg = store_config(val_ty);

            const std::uint32_t num_src1 =
                std::min(31, cfg.element_size * cfg.rows * cfg.cols / grf_size);
            const std::uint32_t desc = [&] {
                const std::uint32_t data_size = cfg.element_size == 4 ? 2 : 1;
                std::uint32_t d = 7;
                d |= data_size << 9;
                d |= 1 << 25;
                return d;
            }();
            const std::uint32_t block_size =
                ((cfg.array_length - 1) << 16) | ((cfg.cols - 1) << 8) | (cfg.rows - 1);
            auto oasm = std::ostringstream{};
            oasm << "{\n"
                    ".decl temp v_type=G type=ud num_elts=8 align=wordx32\n"
                    ".decl tempq v_type=G type=uq num_elts=4 align=wordx32 alias=<temp, 0>\n"
                    "mov (M1,1) tempq(0,0)<1> $1(0,0)<0;1,0>\n"
                    "mov (M1,1) temp(0,2)<1> $2(0,0)<0;1,0>\n"
                    "mov (M1,1) temp(0,3)<1> $3(0,0)<0;1,0>\n"
                    "mov (M1,1) temp(0,4)<1> $4(0,0)<0;1,0>\n"
                    "mov (M1,1) temp(0,5)<1> $5(0,0)<0;1,0>\n"
                    "mov (M1,1) temp(0,6)<1> $6(0,0)<0;1,0>\n"
                    "mov (M1,1) temp(0,7)<1> 0x"
                 << std::hex << block_size << ":ud\n";

            for (std::int32_t m = 0; m < cfg.row_blocks; ++m) {
                for (std::int32_t n = 0; n < cfg.col_blocks; ++n) {
                    oasm << "raw_sends.15.1." << num_src1 << ".0 (M1, 1) 0x0:ud 0x" << std::hex
                         << desc << ":ud temp.0 $0." << std::dec << cfg.byte_offset(m, n)
                         << " %null.0\n";
                    if (n + 1 < cfg.col_blocks) {
                        oasm << "add (M1,1) temp(0,6)<1> temp(0,6)<0;1,0> " << cfg.cols << ":ud\n";
                    }
                    // oasm << "lsc_store_block2d.ugm (M1,1) flat[$1,$2,$3,$4,$5,$6] $0." <<
                    // cfg.byte_offset(m)
                    //<< ":d" << cfg.element_size * 8 << "." << cfg.array_length << "x" << cfg.rows
                    //<< "x"
                    //<< cfg.cols << "nn" << "\n";
                }
                if (m + 1 < cfg.row_blocks) {
                    oasm << "add (M1,1) temp(0,6)<1> temp(0,6)<0;1,0> "
                         << -(cfg.col_blocks - 1) * cfg.cols << ":ud\n";
                    oasm << "add (M1,1) temp(0,7)<1> temp(0,7)<0;1,0> " << cfg.rows << ":ud\n";
                }
            }
            oasm << "}\n";

            auto spv_void_ty = unique_->void_ty();
            auto spv_val_ty = unique_->spv_ty(val_ty);
            auto spv_i32_ty = unique_->spv_ty(scalar_type::i32);
            auto fun_ty = unique_->spv_function_ty(
                spv_void_ty,
                array_view<spv_inst *>{spv_val_ty, spv_operand_ty, spv_i32_ty, spv_i32_ty,
                                       spv_i32_ty, spv_i32_ty, spv_i32_ty});
            return mod_->add_to<OpAsmINTEL>(section::type_const_var, spv_void_ty, fun_ty,
                                            unique_->asm_target(), std::move(oasm).str(),
                                            "rw,rw.u,rw.u,rw.u,rw.u,rw.u,rw.u");
        });
}

auto coopmatrix_diy::mul_add_fun(coopmatrix_data_type const *at, coopmatrix_data_type const *bt,
                                 coopmatrix_data_type const *ct,
                                 coopmatrix_data_type const *rt) -> spv_inst * {
    auto key = std::array<coopmatrix_data_type const *, 4u>{at, bt, ct, rt};
    return lookup(mul_add_funs_, key, [&](std::array<coopmatrix_data_type const *, 4u> const &key) {
        const auto [at, bt, ct, rt] = key;
        auto oasm = std::ostringstream{};

        const std::int32_t ops_per_chan = channel_size / size(at->component_ty());
        const std::int32_t K = ops_per_chan * sdepth;

        for (std::int32_t m = 0; m < ct->rows(); m += exec_size) {
            for (std::int32_t k = 0; k < at->cols(); k += K) {
                for (std::int32_t n = 0; n < ct->cols(); n += rcount) {
                    const auto aoffset =
                        (k * exec_size + m * at->cols()) * size(at->component_ty());
                    const auto brow =
                        (k * bt->cols() + n * K) * size(bt->component_ty()) / grf_size;
                    const auto coffset =
                        (m * ct->cols() + n * exec_size) * size(ct->component_ty());
                    const auto roffset =
                        (m * rt->cols() + n * exec_size) * size(rt->component_ty());
                    oasm << "dpas.hf.hf." << sdepth << "." << rcount << " (M1," << exec_size
                         << ") $0." << roffset << " $3." << coffset << " $1." << aoffset << " $2("
                         << brow << ",0)\n";
                }
            }
        }

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

auto coopmatrix_diy::constant(constant_inst const &in) -> spv_inst * {
    auto spv_result_ty = unique_->spv_ty(in.result(0).ty());
    auto spv_vec_ty = dyn_cast<OpTypeVector>(spv_result_ty);
    if (!spv_vec_ty) {
        throw compilation_error(in.loc(), status::internal_compiler_error);
    }

    const auto sty = get_coopmatrix_type(in.result(0))->component_ty();
    const auto pack_const_in_i32 =
        overloaded{[&](std::int64_t i) -> std::optional<std::int32_t> {
                       switch (sty) {
                       case scalar_type::i8: {
                           auto v8 = std::bit_cast<std::uint8_t>(static_cast<std::int8_t>(i));
                           return std::int32_t{v8 | (v8 << 8) | (v8 << 16) | (v8 << 24)};
                       }
                       case scalar_type::i32:
                           return static_cast<std::int32_t>(i);
                       default:
                           return std::nullopt;
                       }
                   },
                   [&](double d) -> std::optional<std::int32_t> {
                       switch (sty) {
                       case scalar_type::bf16: {
                           std::uint16_t v16 = bfloat16{static_cast<float>(d)}.bits();
                           return std::int32_t{v16 | (v16 << 16)};
                       }
                       case scalar_type::f16: {
                           std::uint16_t v16 = half{static_cast<float>(d)}.bits();
                           return std::int32_t{v16 | (v16 << 16)};
                       }
                       case scalar_type::f32:
                           return std::bit_cast<std::int32_t>(static_cast<float>(d));
                       default:
                           return std::nullopt;
                       }
                   },
                   [&](auto const &) -> std::optional<std::int32_t> { return std::nullopt; }};
    auto const_as_i32 = std::visit(pack_const_in_i32, in.value());
    if (!const_as_i32) {
        throw compilation_error(in.loc(), status::internal_compiler_error);
    }

    if (*const_as_i32 == 0) {
        return unique_->null_constant(spv_result_ty);
    }

    auto cst = isa<OpTypeFloat>(*spv_vec_ty->op0())
                   ? unique_->constant(std::bit_cast<float>(*const_as_i32))
                   : unique_->constant(*const_as_i32);

    return mod_->add_to<OpConstantComposite>(section::type_const_var, spv_result_ty,
                                             std::vector<spv_inst *>(spv_vec_ty->op1(), cst));
}

auto coopmatrix_diy::load(cooperative_matrix_load_inst const &in, dope_vector const &odv,
                          spv_inst *pointer, spv_inst *pos0, spv_inst *pos1) -> spv_inst * {
    auto ct = get_coopmatrix_type(in.result(0));
    auto spv_operand_ty = unique_->spv_ty(in.operand().ty());
    auto fun = load_fun(ct, spv_operand_ty);

    auto spv_i32_ty = unique_->spv_ty(scalar_type::i32);
    auto c1 = unique_->constant(std::int64_t{1});
    auto width = mod_->add<OpISub>(spv_i32_ty, odv.shape(0), c1);
    auto height = mod_->add<OpISub>(spv_i32_ty, odv.shape(1), c1);
    auto stride = mod_->add<OpISub>(spv_i32_ty, odv.stride(1), c1);

    auto spv_result_ty = unique_->spv_ty(in.result(0).ty());
    return mod_->add<OpAsmCallINTEL>(
        spv_result_ty, fun, array_view<spv_inst *>{pointer, width, height, stride, pos0, pos1});
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

void coopmatrix_diy::store(cooperative_matrix_store_inst const &in, dope_vector const &odv,
                           spv_inst *val, spv_inst *pointer, spv_inst *pos0, spv_inst *pos1) {
    auto ct = get_coopmatrix_type(in.val());
    auto spv_operand_ty = unique_->spv_ty(in.operand().ty());
    auto fun = store_fun(ct, spv_operand_ty);

    auto spv_void_ty = unique_->void_ty();
    auto spv_i32_ty = unique_->spv_ty(scalar_type::i32);
    auto c1 = unique_->constant(std::int64_t{1});
    auto width = mod_->add<OpISub>(spv_i32_ty, odv.shape(0), c1);
    auto height = mod_->add<OpISub>(spv_i32_ty, odv.shape(1), c1);
    auto stride = mod_->add<OpISub>(spv_i32_ty, odv.stride(1), c1);

    mod_->add<OpAsmCallINTEL>(
        spv_void_ty, fun, array_view<spv_inst *>{val, pointer, width, height, stride, pos0, pos1});
}

} // namespace tinytc::spv

