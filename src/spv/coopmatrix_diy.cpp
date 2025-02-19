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
#include "spv/enums.hpp"
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
#include <sstream>
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
auto visa_type(scalar_type sty) -> char const * {
    switch (sty) {
    case scalar_type::i8:
        return "b";
    case scalar_type::i16:
        return "w";
    case scalar_type::i32:
        return "d";
    case scalar_type::i64:
    case scalar_type::index:
        return "q";
    case scalar_type::f16:
        return "hf";
    case scalar_type::bf16:
        return "bf";
    case scalar_type::f32:
        return "f";
    case scalar_type::f64:
        return "df";
    default:
        throw status::internal_compiler_error;
    }
}

auto lsc_vector_size_d32(std::int32_t bytes) {
    if (bytes % 4 != 0) {
        throw status::internal_compiler_error;
    }
    auto const vector_size = bytes / 4;
    switch (vector_size) {
    case 1:
    case 2:
    case 4:
    case 8:
    case 16:
    case 32:
    case 64:
        return vector_size;
    default:
        throw status::internal_compiler_error;
    }
}
auto to_string(lsc_sfid sfid) -> char const * {
    switch (sfid) {
    case lsc_sfid::ugm:
        return "ugm";
    case lsc_sfid::slm:
        return "slm";
    default:
        throw status::internal_compiler_error;
    }
}

coopmatrix_diy::coopmatrix_diy(tinytc_spv_mod &m, uniquifier &unique)
    : mod_{&m}, unique_{&unique} {}

auto coopmatrix_diy::make_tmp(char const *prefix) -> std::string {
    return (std::ostringstream{} << prefix << tmp_counter_++).str();
}

auto coopmatrix_diy::max_rows_in_block(coopmatrix_data_type const *ct) const -> std::int32_t {
    if (ct->use() == matrix_use::b) {
        std::int32_t ops_per_chan = channel_size / size(ct->component_ty());
        return ops_per_chan * sdepth;
    }
    return exec_size;
}

auto coopmatrix_diy::load_config(coopmatrix_data_type const *ct, address_space addrspace)
    -> block_config {
    auto cfg = block_config{};
    cfg.element_size = size(ct->component_ty());
    cfg.array_length = 1;
    cfg.rows = ct->rows();
    cfg.cols = ct->cols();
    cfg.row_blocks = 1;
    cfg.col_blocks = 1;
    cfg.vnni = ct->use() == matrix_use::a;
    cfg.sfid = addrspace == address_space::local ? lsc_sfid::slm : lsc_sfid::ugm;

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

auto coopmatrix_diy::load_fun_asm_block2d(block_config const &cfg) -> std::string {
    const std::uint32_t num_dst =
        std::min(31, cfg.element_size * cfg.array_length * cfg.rows * cfg.cols / grf_size);
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
    auto temp = make_tmp("temp");
    auto tempq = make_tmp("tempq");
    auto oasm = std::ostringstream{};
    oasm << "{\n"
            ".decl "
         << temp << " v_type=G type=ud num_elts=8 align=wordx32\n"
         << ".decl " << tempq << " v_type=G type=uq num_elts=4 align=wordx32 alias=<" << temp
         << ",0>\n"
         << "mov (M1,1) " << tempq << "(0,0)<1> $1(0,0)<0;1,0>\n"
         << "add (M1,1) " << temp << "(0,2)<1> $2(0,0)<0;1,0> -1:d\n"
         << "add (M1,1) " << temp << "(0,3)<1> $3(0,0)<0;1,0> -1:d\n"
         << "add (M1,1) " << temp << "(0,4)<1> $4(0,0)<0;1,0> -1:d\n"
         << "mov (M1,1) " << temp << "(0,5)<1> $5(0,0)<0;1,0>\n"
         << "mov (M1,1) " << temp << "(0,6)<1> $6(0,0)<0;1,0>\n"
         << "mov (M1,1) " << temp << "(0,7)<1> 0x" << std::hex << block_size << ":ud\n";

    for (std::int32_t m = 0; m < cfg.row_blocks; ++m) {
        for (std::int32_t n = 0; n < cfg.col_blocks; ++n) {
            oasm << std::dec << "raw_sends.15.1.0." << num_dst << " (M1, 1) 0x0:ud 0x" << std::hex
                 << desc << ":ud " << temp << ".0 %null.0 $0." << std::dec << cfg.byte_offset(m, n)
                 << "\n";
            if (n + 1 < cfg.col_blocks) {
                oasm << "add (M1,1) " << temp << "(0,6)<1> " << temp << "(0,6)<0;1,0> " << cfg.cols
                     << ":ud\n";
            }
            // oasm << "lsc_load_block2d.ugm (M1,1) $0." << cfg.byte_offset(m, n) << ":d"
            //<< cfg.element_size * 8 << "." << cfg.array_length << "x" << cfg.rows << "x"
            //<< cfg.cols << "n" << (vnni ? "t" : "n") << " flat[$1,$2,$3,$4,$5,$6]\n";
        }
        if (m + 1 < cfg.row_blocks) {
            oasm << "add (M1,1) " << temp << "(0,6)<1> " << temp << "(0,6)<0;1,0> "
                 << -(cfg.col_blocks - 1) * cfg.cols << ":ud\n";
            oasm << "add (M1,1) " << temp << "(0,5)<1> " << temp << "(0,5)<0;1,0> " << cfg.rows
                 << ":ud\n";
        }
    }
    oasm << "}\n";

    return std::move(oasm).str();
}

auto coopmatrix_diy::load_fun_asm_generic(block_config const &cfg, scalar_type sty) -> std::string {
    const auto addrsize = cfg.sfid == lsc_sfid::slm ? "a32" : "a64";
    const auto sfid = to_string(cfg.sfid);
    const auto total_rows = cfg.array_length * cfg.rows * cfg.row_blocks;
    const auto vector_size = lsc_vector_size_d32(total_rows * cfg.element_size);

    auto oasm = std::ostringstream{};
    oasm << "{\n";

    const auto make_pointer = [&] {
        auto pointer = make_tmp("pointer");
        if (cfg.sfid == lsc_sfid::slm) {
            oasm << ".decl " << pointer << " v_type=G type=ud num_elts=1 align=wordx32\n";
        } else {
            oasm << ".decl " << pointer << " v_type=G type=uq num_elts=1 align=wordx32\n";
        }
        return pointer;
    };
    const auto make_temp = [&] {
        const auto temp = make_tmp("temp");
        oasm << ".decl " << temp << " v_type=G type=" << visa_type(sty)
             << " num_elts=" << total_rows << " align=wordx32\n";
        return temp;
    };

    auto pointers = std::array<std::string, load_batch_size>{};
    auto temps = std::array<std::string, load_batch_size>{};
    for (auto &p : pointers) {
        p = make_pointer();
    }
    for (auto &t : temps) {
        t = make_temp();
    }

    const auto offset = make_tmp("offset");
    oasm << ".decl " << offset << " v_type=G type=ud num_elts=1 align=dword\n";
    oasm << "   mov (M1,1) " << pointers[0] << "(0,0)<1> $1(0,0)<0;1,0>\n";
    oasm << "   mul (M1,1) " << offset << "(0,0)<1> $5(0,0)<0;1,0> " << cfg.element_size << ":d\n";
    oasm << "   add (M1,1) " << pointers[0] << "(0,0)<1> " << pointers[0] << "(0,0)<0;1,0> "
         << offset << "(0,0)<0;1,0>\n";
    oasm << "   mul (M1,1) " << offset << "(0,0)<1> $6(0,0)<0;1,0> $4(0,0)<0;1,0>\n";
    oasm << "   add (M1,1) " << pointers[0] << "(0,0)<1> " << pointers[0] << "(0,0)<0;1,0> "
         << offset << "(0,0)<0;1,0>\n";

    std::int32_t tmp_no = 0;
    for (std::int32_t n = 0; n < cfg.col_blocks; ++n) {
        for (std::int32_t c = 0; c < cfg.cols; ++c) {
            oasm << "   lsc_load." << sfid << " (M1,1) " << temps[tmp_no] << ":d32x" << vector_size
                 << "t flat[" << pointers[tmp_no] << "]:" << addrsize << "\n";
            for (std::int32_t m = 0; m < cfg.row_blocks; ++m) {
                for (std::int32_t a = 0; a < cfg.array_length; ++a) {
                    for (std::int32_t r = 0; r < cfg.rows; r += exec_size) {
                        const auto src_offset =
                            (r + a * cfg.rows + m * cfg.rows * cfg.array_length) * cfg.element_size;
                        const auto src_brow = src_offset / grf_size;
                        const auto src_bcol = src_offset % grf_size / cfg.element_size;
                        const auto dst_offset = cfg.byte_offset(m, n, a, r, c);
                        const auto dst_brow = dst_offset / grf_size;
                        const auto dst_bcol = dst_offset % grf_size / cfg.element_size;
                        oasm << "   mov (M1," << exec_size << ") $0(" << dst_brow << "," << dst_bcol
                             << ")<1> " << temps[tmp_no] << "(" << src_brow << "," << src_bcol
                             << ")<1;1,0>\n";
                    }
                }
            }
            if (n + 1 < cfg.col_blocks || c + 1 < cfg.cols) {
                const auto next_tmp_no = (tmp_no + 1) % load_batch_size;
                oasm << "   add (M1,1) " << pointers[next_tmp_no] << "(0,0)<1> " << pointers[tmp_no]
                     << "(0,0)<0;1,0> $4(0,0)<0;1,0>\n";
                tmp_no = next_tmp_no;
            }
        }
    }
    oasm << "}\n";

    return std::move(oasm).str();
}

auto coopmatrix_diy::load_fun(coopmatrix_data_type const *result_ty, spv_inst *spv_operand_ty,
                              address_space addrspace) -> spv_inst * {
    const auto key = load_store_key{result_ty, spv_operand_ty, addrspace};
    return lookup(load_funs_, key, [&](load_store_key const &key) {
        const auto [result_ty, spv_operand_ty, addrspace] = key;

        const auto cfg = load_config(result_ty, addrspace);
        auto code = cfg.sfid == lsc_sfid::slm ? load_fun_asm_generic(cfg, result_ty->component_ty())
                                              : load_fun_asm_block2d(cfg);

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
    cfg.vnni = false;
    cfg.sfid = addrspace == address_space::local ? lsc_sfid::slm : lsc_sfid::ugm;

    if (cfg.cols > max_cols_in_block) {
        cfg.col_blocks = cfg.cols / max_cols_in_block;
        cfg.cols = max_cols_in_block;
    }

    const auto max_rows = max_rows_in_block(ct);
    if (cfg.rows > max_rows) {
        cfg.row_blocks = cfg.rows / max_rows;
        cfg.rows = max_rows;
    }

    return cfg;
}

auto coopmatrix_diy::store_fun_asm_block2d(block_config const &cfg) -> std::string {
    const std::uint32_t num_src1 = std::min(31, cfg.element_size * cfg.rows * cfg.cols / grf_size);
    const std::uint32_t desc = [&] {
        const std::uint32_t data_size = cfg.element_size == 4 ? 2 : 1;
        std::uint32_t d = 7;
        d |= data_size << 9;
        d |= 1 << 25;
        return d;
    }();
    const std::uint32_t block_size =
        ((cfg.array_length - 1) << 16) | ((cfg.cols - 1) << 8) | (cfg.rows - 1);
    auto temp = make_tmp("temp");
    auto tempq = make_tmp("tempq");
    auto oasm = std::ostringstream{};
    oasm << "{\n"
         << ".decl " << temp << " v_type=G type=ud num_elts=8 align=wordx32\n"
         << ".decl " << tempq << " v_type=G type=uq num_elts=4 align=wordx32 alias=<" << temp
         << ", 0>\n"
         << "mov (M1,1) " << tempq << "(0,0)<1> $1(0,0)<0;1,0>\n"
         << "add (M1,1) " << temp << "(0,2)<1> $2(0,0)<0;1,0> -1:d\n"
         << "add (M1,1) " << temp << "(0,3)<1> $3(0,0)<0;1,0> -1:d\n"
         << "add (M1,1) " << temp << "(0,4)<1> $4(0,0)<0;1,0> -1:d\n"
         << "mov (M1,1) " << temp << "(0,5)<1> $5(0,0)<0;1,0>\n"
         << "mov (M1,1) " << temp << "(0,6)<1> $6(0,0)<0;1,0>\n"
         << "mov (M1,1) " << temp << "(0,7)<1> 0x" << std::hex << block_size << ":ud\n";

    for (std::int32_t m = 0; m < cfg.row_blocks; ++m) {
        for (std::int32_t n = 0; n < cfg.col_blocks; ++n) {
            oasm << "raw_sends.15.1." << num_src1 << ".0 (M1, 1) 0x0:ud 0x" << std::hex << desc
                 << ":ud " << temp << ".0 $0." << std::dec << cfg.byte_offset(m, n) << " %null.0\n";
            if (n + 1 < cfg.col_blocks) {
                oasm << "add (M1,1) " << temp << "(0,6)<1> " << temp << "(0,6)<0;1,0> " << cfg.cols
                     << ":ud\n";
            }
            // oasm << "lsc_store_block2d.ugm (M1,1) flat[$1,$2,$3,$4,$5,$6] $0." <<
            // cfg.byte_offset(m)
            //<< ":d" << cfg.element_size * 8 << "." << cfg.array_length << "x" << cfg.rows
            //<< "x"
            //<< cfg.cols << "nn" << "\n";
        }
        if (m + 1 < cfg.row_blocks) {
            oasm << "add (M1,1) " << temp << "(0,6)<1> " << temp << "(0,6)<0;1,0> "
                 << -(cfg.col_blocks - 1) * cfg.cols << ":ud\n";
            oasm << "add (M1,1) " << temp << "(0,5)<1> " << temp << "(0,5)<0;1,0> " << cfg.rows
                 << ":ud\n";
        }
    }
    oasm << "}\n";

    return std::move(oasm).str();
}

auto coopmatrix_diy::store_fun_asm_generic(block_config const &cfg, scalar_type sty)
    -> std::string {
    const auto addrsize = cfg.sfid == lsc_sfid::slm ? "a32" : "a64";
    const auto sfid = to_string(cfg.sfid);
    const auto vector_size = lsc_vector_size_d32(cfg.rows * cfg.row_blocks * cfg.element_size);

    auto oasm = std::ostringstream{};
    oasm << "{\n";

    const auto make_pointer = [&] {
        auto pointer = make_tmp("pointer");
        if (cfg.sfid == lsc_sfid::slm) {
            oasm << ".decl " << pointer << " v_type=G type=ud num_elts=1 align=wordx32\n";
        } else {
            oasm << ".decl " << pointer << " v_type=G type=uq num_elts=1 align=wordx32\n";
        }
        return pointer;
    };
    const auto make_temp = [&] {
        const auto temp = make_tmp("temp");
        oasm << ".decl " << temp << " v_type=G type=" << visa_type(sty)
             << " num_elts=" << cfg.rows * cfg.row_blocks << " align=wordx32\n";
        return temp;
    };

    auto pointers = std::array<std::string, store_batch_size>{};
    auto temps = std::array<std::string, store_batch_size>{};
    for (auto &p : pointers) {
        p = make_pointer();
    }
    for (auto &t : temps) {
        t = make_temp();
    }

    const auto offset = make_tmp("offset");
    oasm << ".decl " << offset << " v_type=G type=ud num_elts=1 align=dword\n";
    oasm << "   mov (M1,1) " << pointers[0] << "(0,0)<1> $1(0,0)<0;1,0>\n";
    oasm << "   mul (M1,1) " << offset << "(0,0)<1> $5(0,0)<0;1,0> " << cfg.element_size << ":d\n";
    oasm << "   add (M1,1) " << pointers[0] << "(0,0)<1> " << pointers[0] << "(0,0)<0;1,0> "
         << offset << "(0,0)<0;1,0>\n";
    oasm << "   mul (M1,1) " << offset << "(0,0)<1> $6(0,0)<0;1,0> $4(0,0)<0;1,0>\n";
    oasm << "   add (M1,1) " << pointers[0] << "(0,0)<1> " << pointers[0] << "(0,0)<0;1,0> "
         << offset << "(0,0)<0;1,0>\n";

    std::int32_t tmp_no = 0;
    for (std::int32_t n = 0; n < cfg.col_blocks; ++n) {
        for (std::int32_t c = 0; c < cfg.cols; ++c) {
            for (std::int32_t m = 0; m < cfg.row_blocks; ++m) {
                for (std::int32_t r = 0; r < cfg.rows; r += exec_size) {
                    const auto src_offset = cfg.byte_offset(m, n, 0, r, c);
                    const auto src_brow = src_offset / grf_size;
                    const auto src_bcol = src_offset % grf_size / cfg.element_size;
                    const auto dst_offset = (r + m * cfg.rows) * cfg.element_size;
                    const auto dst_brow = dst_offset / grf_size;
                    const auto dst_bcol = dst_offset % grf_size / cfg.element_size;
                    oasm << "   mov (M1," << exec_size << ") " << temps[tmp_no] << "(" << dst_brow
                         << "," << dst_bcol << ")<1> $0(" << src_brow << "," << src_bcol
                         << ")<1;1,0>\n";
                }
            }
            oasm << "   lsc_store." << sfid << " (M1,1) flat[" << pointers[tmp_no]
                 << "]:" << addrsize << " " << temps[tmp_no] << ":d32x" << vector_size << "t\n";
            if (n + 1 < cfg.col_blocks || c + 1 < cfg.cols) {
                const auto next_tmp_no = (tmp_no + 1) % store_batch_size;
                oasm << "   add (M1,1) " << pointers[next_tmp_no] << "(0,0)<1> " << pointers[tmp_no]
                     << "(0,0)<0;1,0> $4(0,0)<0;1,0>\n";
                tmp_no = next_tmp_no;
            }
        }
    }
    oasm << "}\n";

    return std::move(oasm).str();
}

auto coopmatrix_diy::store_fun(coopmatrix_data_type const *val_ty, spv_inst *spv_operand_ty,
                               address_space addrspace) -> spv_inst * {
    const auto key = load_store_key{val_ty, spv_operand_ty, addrspace};
    return lookup(store_funs_, key, [&](load_store_key const &key) {
        const auto [val_ty, spv_operand_ty, addrspace] = key;

        const auto cfg = store_config(val_ty, addrspace);
        auto code = cfg.sfid == lsc_sfid::slm ? store_fun_asm_generic(cfg, val_ty->component_ty())
                                              : store_fun_asm_block2d(cfg);

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

        const std::int32_t ops_per_chan = channel_size / size(at->component_ty());
        const std::int32_t K = ops_per_chan * sdepth;

        oasm << "{\n";
        std::string result_placeholder = "$0";
        std::string temp = result_placeholder;
        if (rt->component_ty() != ct->component_ty() && at->cols() / K > 1) {
            temp = make_tmp("temp");
            oasm << ".decl " << temp << " v_type=G type=" << visa_type(ct->component_ty())
                 << " num_elts=" << ct->rows() * ct->cols() << " align=wordx32\n";
        }

        const auto precision_src1 = precision(at->component_ty());
        const auto precision_src2 = precision(bt->component_ty());
        for (std::int32_t k = 0; k < at->cols(); k += K) {
            char const *src0 = k > 0 ? temp.c_str() : "$3";
            char const *dst = k + K >= at->cols() ? result_placeholder.c_str() : temp.c_str();
            const auto rsize =
                k + K >= at->cols() ? size(rt->component_ty()) : size(ct->component_ty());
            for (std::int32_t m = 0; m < ct->rows(); m += exec_size) {
                for (std::int32_t n = 0; n < ct->cols(); n += rcount) {
                    const auto aoffset =
                        (k * exec_size + m * at->cols()) * size(at->component_ty());
                    const auto brow =
                        (k * bt->cols() + n * K) * size(bt->component_ty()) / grf_size;
                    const auto coffset =
                        (m * ct->cols() + n * exec_size) * size(ct->component_ty());
                    const auto roffset = (m * rt->cols() + n * exec_size) * rsize;
                    oasm << "dpas." << precision_src1 << "." << precision_src2 << "." << sdepth
                         << "." << rcount << " (M1," << exec_size << ") " << dst << "." << roffset
                         << " " << src0 << "." << coffset << " $1." << aoffset << " $2(" << brow
                         << ",0)\n";
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
        const auto num_elements = num_components * exec_size;

        auto spv_component_ty = unique_->spv_ty(to_ty);
        auto spv_operation_ty = unique_->spv_vec_ty(spv_component_ty, num_components);
        const auto to_width = grf_size / size(to_ty);
        const auto from_width = grf_size / size(from_ty);
        const auto to_visa_ty = visa_type(to_ty);
        const auto from_visa_ty = visa_type(from_ty);

        auto oasm = std::ostringstream{};
        const auto a_tmp = make_tmp("a_tmp");
        const auto b_tmp = make_tmp("b_tmp");
        oasm << "{\n";
        oasm << ".decl " << a_tmp << " v_type=G type=" << from_visa_ty
             << " num_elts=" << num_elements << " align=wordx32 alias=<$1, 0>\n";
        oasm << ".decl " << b_tmp << " v_type=G type=" << to_visa_ty << " num_elts=" << num_elements
             << " align=wordx32 alias=<$0, 0>\n";
        for (std::int32_t m = 0; m < num_elements; m += exec_size) {
            auto R_from = m / from_width;
            auto C_from = m % from_width;
            auto R_to = m / to_width;
            auto C_to = m % to_width;
            oasm << "mov (M1," << exec_size << ") " << b_tmp << "(" << R_to << "," << C_to
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
        const auto num_elements = num_components * exec_size;

        auto spv_component_ty = unique_->spv_ty(cty);
        auto spv_operation_ty = unique_->spv_vec_ty(spv_component_ty, num_components);
        const auto width = grf_size / size(cty);
        const auto visa_ty = visa_type(cty);

        auto oasm = std::ostringstream{};
        const auto a_tmp = make_tmp("a_tmp");
        const auto b_tmp = make_tmp("b_tmp");
        const auto c_tmp = make_tmp("c_tmp");
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
        for (std::int32_t m = 0; m < num_elements; m += exec_size) {
            auto R = m / width;
            auto C = m % width;
            oasm << opcode << " (M1," << exec_size << ") " << c_tmp << "(" << R << "," << C
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
        const auto num_elements = num_components * exec_size;

        auto spv_component_ty = unique_->spv_ty(cty);
        auto spv_operation_ty = unique_->spv_vec_ty(spv_component_ty, num_components);
        const auto width = grf_size / size(cty);
        const auto visa_ty = visa_type(cty);

        auto oasm = std::ostringstream{};
        const auto a_tmp = make_tmp("a_tmp");
        const auto b_tmp = make_tmp("b_tmp");
        const auto c_tmp = make_tmp("c_tmp");
        oasm << "{\n";
        oasm << ".decl " << a_tmp << " v_type=G type=" << visa_ty
             << " num_elts=1 align=word alias=<$1, 0>\n";
        oasm << ".decl " << b_tmp << " v_type=G type=" << visa_ty << " num_elts=" << num_elements
             << " align=wordx32 alias=<$2, 0>\n";
        oasm << ".decl " << c_tmp << " v_type=G type=" << visa_ty << " num_elts=" << num_elements
             << " align=wordx32 alias=<$0, 0>\n";
        for (std::int32_t m = 0; m < num_elements; m += exec_size) {
            auto R = m / width;
            auto C = m % width;
            oasm << "mul (M1," << exec_size << ") " << c_tmp << "(" << R << "," << C << ")<1> "
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
    const std::int32_t num_components = rt->rows() * rt->cols() / exec_size;
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
    const std::int32_t num_components = rt->rows() * rt->cols() / exec_size;
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
    auto fun = load_fun(ct, spv_operand_ty, ot->addrspace());

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
    const std::int32_t num_components = rt->rows() * rt->cols() / exec_size;
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

