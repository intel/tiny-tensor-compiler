// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "spv/block2d_diy.hpp"
#include "spv/xe_constants.hpp"
#include "support/temp_counter.hpp"
#include "support/util.hpp"
#include "tinytc/types.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <sstream>
#include <string_view>
#include <utility>
#include <vector>

namespace tinytc::spv {

auto block_config::block_size_in_bytes() const -> std::int32_t {
    return element_size * array_length * rows * cols;
}
auto block_config::block_size_in_num_grf() const -> std::int32_t {
    return block_size_in_bytes() / xe::grf_size;
}
auto block_config::byte_offset(std::int32_t row, std::int32_t col, std::int32_t array_idx,
                               std::int32_t col_block, std::int32_t row_block) const
    -> std::int32_t {
    std::int32_t offset = row_block;
    offset = col_block + offset * col_blocks;
    offset = array_idx + offset * array_length;
    offset = col + offset * cols;
    offset = row + offset * rows;
    return offset * element_size;
}
auto block_config::origin(std::int32_t row, std::int32_t col, std::int32_t array_idx,
                          std::int32_t col_block, std::int32_t row_block) const
    -> std::array<std::int32_t, 2u> {
    const auto offset = byte_offset(row, col, array_idx, col_block, row_block);
    return region_origin(element_size, offset);
}
auto block_config::total_rows() const -> std::int32_t { return array_length * rows * row_blocks; }

auto lsc_data_size(std::int32_t element_size) -> std::int32_t {
    if (!is_positive_power_of_two(element_size) || element_size > 8) {
        throw status::internal_compiler_error;
    }
    return ilog2(element_size);
}
auto lsc_vector_size_d32(std::int32_t bytes) -> std::int32_t {
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
auto region_origin(std::int32_t element_size, std::int32_t byte_offset)
    -> std::array<std::int32_t, 2u> {
    return {byte_offset / xe::grf_size, byte_offset % xe::grf_size / element_size};
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

auto make_d32_transpose8x8(std::ostream &oasm, char const *matrix, std::size_t offset,
                           temp_counter &make_tmp) {
    auto dst_d = make_tmp("dst_d");
    auto dst_q = make_tmp("dst_q");
    oasm << ".decl " << dst_d << " v_type=G type=d num_elts=64 align=wordx32 alias=<" << matrix
         << "," << offset << ">\n";
    oasm << ".decl " << dst_q << " v_type=G type=q num_elts=32 align=wordx32 alias=<" << matrix
         << "," << offset << ">\n";
    // 2x2 transpose
    for (std::int32_t r = 0; r < 4; ++r) {
        auto ttmp = make_tmp("ttmp_d");
        oasm << ".decl " << ttmp << " v_type=G type=d num_elts=4 align=wordx32\n";
        oasm << "mov (M1,4) " << ttmp << "(0,0)<1> " << dst_d << "(" << r << ",1)<2;1,0>\n";
        oasm << "mov (M1,4) " << dst_d << "(" << r << ",1)<2> " << dst_d << "(" << r
             << ",8)<2;1,0>\n";
        oasm << "mov (M1,4) " << dst_d << "(" << r << ",8)<2> " << ttmp << "(0,0)<1;1,0>\n";
    }
    // 4x4 transpose
    for (std::int32_t r = 0; r < 4; r += 2) {
        auto ttmp = make_tmp("ttmp_q");
        oasm << ".decl " << ttmp << " v_type=G type=q num_elts=4 align=wordx32\n";
        oasm << "mov (M1,4) " << ttmp << "(0,0)<1> " << dst_q << "(" << r << ",1)<2;1,0>\n";
        oasm << "mov (M1,4) " << dst_q << "(" << r << ",1)<2> " << dst_q << "(" << r + 1
             << ",0)<2;1,0>\n";
        oasm << "mov (M1,4) " << dst_q << "(" << r + 1 << ",0)<2> " << ttmp << "(0,0)<1;1,0>\n";
    }
    // 8x8 transpose
    for (std::int32_t r = 0; r < 4; ++r) {
        auto ttmp = make_tmp("ttmp_d");
        const auto R = r / 2;
        const auto C = (r % 2) * 8;
        oasm << ".decl " << ttmp << " v_type=G type=d num_elts=4 align=wordx32\n";
        oasm << "mov (M1,4) " << ttmp << "(0,0)<1> " << dst_d << "(" << R << "," << C + 4
             << ")<1;1,0>\n";
        oasm << "mov (M1,4) " << dst_d << "(" << R << "," << C + 4 << ")<1> " << dst_d << "("
             << R + 2 << "," << C << ")<1;1,0>\n";
        oasm << "mov (M1,4) " << dst_d << "(" << R + 2 << "," << C << ")<1> " << ttmp
             << "(0,0)<1;1,0>\n";
    }
}

struct block2d_native_helper {
    block2d_native_helper(std::ostream &oasm, block_config const &cfg, temp_counter &make_tmp);
    void header();
    template <typename F> void walk(F &&io);

    std::ostream &oasm;
    block_config const &cfg;
    std::string temp, tempq;
};

block2d_native_helper::block2d_native_helper(std::ostream &oasm, block_config const &cfg,
                                             temp_counter &make_tmp)
    : oasm(oasm), cfg(cfg), temp{make_tmp("temp")}, tempq{make_tmp("tempq")} {}

void block2d_native_helper::header() {
    const std::uint32_t block_size =
        ((cfg.array_length - 1) << 16) | ((cfg.cols - 1) << 8) | (cfg.rows - 1);
    oasm << ".decl " << temp << " v_type=G type=ud num_elts=8 align=wordx32\n"
         << ".decl " << tempq << " v_type=G type=uq num_elts=4 align=wordx32 alias=<" << temp
         << ",0>\n"
         << "mov (M1,1) " << tempq << "(0,0)<1> $1(0,0)<0;1,0>\n"
         << "add (M1,1) " << temp << "(0,2)<1> $2(0,0)<0;1,0> -1:d\n"
         << "add (M1,1) " << temp << "(0,3)<1> $3(0,0)<0;1,0> -1:d\n"
         << "add (M1,1) " << temp << "(0,4)<1> $4(0,0)<0;1,0> -1:d\n";
    if (cfg.pos0_shr) {
        oasm << "shr (M1,1) " << temp << "(0,5)<1> $5(0,0)<0;1,0> " << cfg.pos0_shr << ":d\n";
    } else {
        oasm << "mov (M1,1) " << temp << "(0,5)<1> $5(0,0)<0;1,0>\n";
    }
    oasm << "mov (M1,1) " << temp << "(0,6)<1> $6(0,0)<0;1,0>\n"
         << "mov (M1,1) " << temp << "(0,7)<1> 0x" << std::hex << block_size << ":ud\n";
}

template <typename F> void block2d_native_helper::walk(F &&io) {
    auto pos0_C = 5;
    auto pos1_C = 6;
    auto pos0_inc = cfg.rows;
    auto pos1_inc = cfg.cols;
    if (cfg.transpose) {
        std::swap(pos0_C, pos1_C);
        std::swap(pos0_inc, pos1_inc);
    }
    for (std::int32_t m = 0; m < cfg.row_blocks; ++m) {
        for (std::int32_t n = 0; n < cfg.col_blocks; ++n) {
            io(m, n);
            if (n + 1 < cfg.col_blocks) {
                oasm << "add (M1,1) " << temp << "(0," << pos1_C << ")<1> " << temp << "(0,"
                     << pos1_C << ")<0;1,0> " << pos1_inc << ":ud\n";
            }
        }
        if (m + 1 < cfg.row_blocks) {
            if (cfg.col_blocks > 1) {
                oasm << "add (M1,1) " << temp << "(0," << pos1_C << ")<1> " << temp << "(0,"
                     << pos1_C << ")<0;1,0> " << -(cfg.col_blocks - 1) * pos1_inc << ":ud\n";
            }
            oasm << "add (M1,1) " << temp << "(0," << pos0_C << ")<1> " << temp << "(0," << pos0_C
                 << ")<0;1,0> " << pos0_inc << ":ud\n";
        }
    }
}

auto load_block2d_native(block_config const &cfg, temp_counter &make_tmp) -> std::string {
    const std::uint32_t num_dst = std::min(31, cfg.block_size_in_num_grf());
    const std::uint32_t desc = [&] {
        const std::uint32_t data_size = lsc_data_size(cfg.element_size);
        std::uint32_t d = 3;
        if (cfg.vnni) {
            d |= 1 << 7;
        }
        if (cfg.transpose) {
            d |= 1 << 15;
        }
        d |= data_size << 9;
        d |= num_dst << 20;
        d |= 1 << 25;
        return d;
    }();

    auto oasm = std::ostringstream{};
    auto h = block2d_native_helper(oasm, cfg, make_tmp);

    oasm << "{\n";
    h.header();
    h.walk([&](std::int32_t m, std::int32_t n) {
        const auto offset = cfg.byte_offset(0, 0, 0, n, m);
        oasm << std::dec << "raw_sends.15.1.0." << num_dst << " (M1, 1) 0x0:ud 0x" << std::hex
             << desc << ":ud " << h.temp << ".0 %null.0 $0." << std::dec << offset << "\n";

        if (cfg.post_d32_transpose8x8) {
            for (std::int32_t array_idx = 0; array_idx < cfg.array_length; ++array_idx) {
                make_d32_transpose8x8(oasm, "$0", cfg.byte_offset(0, 0, array_idx, n, m), make_tmp);
            }
        }
    });
    oasm << "}\n";

    return std::move(oasm).str();
}
auto store_block2d_native(block_config const &cfg, temp_counter &make_tmp) -> std::string {
    const std::uint32_t num_src1 = std::min(31, cfg.block_size_in_num_grf());
    const std::uint32_t desc = [&] {
        const std::uint32_t data_size = lsc_data_size(cfg.element_size);
        std::uint32_t d = 7;
        d |= data_size << 9;
        d |= 1 << 25;
        return d;
    }();

    auto oasm = std::ostringstream{};
    auto h = block2d_native_helper(oasm, cfg, make_tmp);

    oasm << "{\n";
    h.header();
    h.walk([&](std::int32_t m, std::int32_t n) {
        const auto offset = cfg.byte_offset(0, 0, 0, n, m);
        oasm << "raw_sends.15.1." << num_src1 << ".0 (M1, 1) 0x0:ud 0x" << std::hex << desc
             << ":ud " << h.temp << ".0 $0." << std::dec << offset << " %null.0\n";
    });
    oasm << "}\n";

    return std::move(oasm).str();
}

struct block2d_emulated_helper {
    block2d_emulated_helper(std::ostream &oasm, block_config const &cfg, std::size_t io_batch_size,
                            temp_counter &make_tmp);
    void header();
    auto dst_op(std::int32_t row, std::int32_t col, std::int32_t array_idx, std::int32_t col_block,
                std::int32_t row_block) -> std::string;
    auto temp_op(std::string_view temp, std::int32_t row, std::int32_t array_idx,
                 std::int32_t row_block) -> std::string;
    template <typename F> void walk(F &&io);

    std::ostream &oasm;
    block_config const &cfg;
    std::vector<std::string> temps, pointers;
    std::string base_pointer, tempq, dst;
};

block2d_emulated_helper::block2d_emulated_helper(std::ostream &oasm, block_config const &cfg,
                                                 std::size_t io_batch_size, temp_counter &make_tmp)
    : oasm(oasm), cfg(cfg), temps(io_batch_size), pointers(io_batch_size),
      base_pointer{make_tmp("base_pointer")}, tempq{make_tmp("tempq")}, dst{make_tmp("dst")} {
    std::generate(temps.begin(), temps.end(), [&]() { return make_tmp("temp"); });
    std::generate(pointers.begin(), pointers.end(), [&]() { return make_tmp("pointer"); });
}

void block2d_emulated_helper::header() {
    char const *visa_ty = visa_type(cfg.sty);
    std::for_each(temps.begin(), temps.end(), [&](auto const &temp) {
        oasm << ".decl " << temp << " v_type=G type=" << visa_ty << " num_elts=" << cfg.total_rows()
             << " align=wordx32\n";
    });
    std::for_each(pointers.begin(), pointers.end(), [&](auto const &pointer) {
        if (cfg.sfid == lsc_sfid::slm) {
            oasm << ".decl " << pointer << " v_type=G type=ud num_elts=1 align=wordx32\n";
        } else {
            oasm << ".decl " << pointer << " v_type=G type=uq num_elts=1 align=wordx32\n";
        }
    });
    oasm << ".decl " << dst << " v_type=G type=" << visa_ty
         << " num_elts=" << cfg.total_rows() * cfg.cols * cfg.col_blocks
         << " align=wordx32 alias=<$0,0>\n";
    oasm << ".decl " << base_pointer << " v_type=G type=uq num_elts=1 align=qword\n";
    oasm << ".decl " << tempq << " v_type=G type=q num_elts=1 align=qword\n";
    oasm << "   mov (M1,1) " << base_pointer << "(0,0)<1> $1(0,0)<1;1,0>\n";
    oasm << "   mul (M1,1) " << tempq << "(0,0)<1> $5(0,0)<0;1,0> " << cfg.element_size << ":d\n";
    oasm << "   add (M1,1) " << base_pointer << "(0,0)<1> " << base_pointer << "(0,0)<0;1,0> "
         << tempq << "(0,0)<0;1,0>\n";
    oasm << "   mul (M1,1) " << tempq << "(0,1)<1> $6(0,0)<0;1,0> $4(0,0)<0;1,0>\n";
    oasm << "   add (M1,1) " << base_pointer << "(0,0)<1> " << base_pointer << "(0,0)<0;1,0> "
         << tempq << "(0,0)<0;1,0>\n";
    oasm << "   mov (M1,1) " << pointers[0] << "(0,0)<1> " << base_pointer << "(0,0)<0;1,0>\n";
}

auto block2d_emulated_helper::dst_op(std::int32_t row, std::int32_t col, std::int32_t array_idx,
                                     std::int32_t col_block, std::int32_t row_block)
    -> std::string {
    const auto [dst_R, dst_C] = cfg.origin(row, col, array_idx, col_block, row_block);
    return (std::ostringstream{} << dst << "(" << dst_R << "," << dst_C << ")").str();
}
auto block2d_emulated_helper::temp_op(std::string_view temp, std::int32_t row,
                                      std::int32_t array_idx, std::int32_t row_block)
    -> std::string {
    const auto src_offset =
        (row + array_idx * cfg.rows + row_block * cfg.rows * cfg.array_length) * cfg.element_size;
    const auto [src_R, src_C] = region_origin(cfg.element_size, src_offset);
    return (std::ostringstream{} << temp << "(" << src_R << "," << src_C << ")").str();
}

template <typename F> void block2d_emulated_helper::walk(F &&io) {
    std::int32_t pointer_no = 0;
    for (std::int32_t n = 0; n < cfg.col_blocks; ++n) {
        for (std::int32_t c = 0; c < cfg.cols; ++c) {
            io(n, c, pointers[pointer_no], temps[pointer_no]);
            if (n + 1 < cfg.col_blocks || c + 1 < cfg.cols) {
                const auto next_pointer_no = (pointer_no + 1) % pointers.size();
                oasm << "   add (M1,1) " << pointers[next_pointer_no] << "(0,0)<1> "
                     << pointers[pointer_no] << "(0,0)<0;1,0> $4(0,0)<0;1,0>\n";
                pointer_no = next_pointer_no;
            }
        }
    }
}

auto load_block2d_emulated(block_config const &cfg, temp_counter &make_tmp) -> std::string {
    const auto addrsize = cfg.sfid == lsc_sfid::slm ? "a32" : "a64";
    const auto sfid = to_string(cfg.sfid);
    const auto total_rows = cfg.array_length * cfg.rows * cfg.row_blocks;
    const auto vector_size = lsc_vector_size_d32(total_rows * cfg.element_size);

    auto oasm = std::ostringstream{};
    auto h = block2d_emulated_helper(oasm, cfg, xe::load_batch_size, make_tmp);

    oasm << "{\n";
    h.header();

    const std::int32_t row_step_size = std::min(cfg.rows, xe::exec_size);

    const auto ops_per_chan = xe::channel_size / cfg.element_size;
    h.walk([&](std::int32_t n, std::int32_t c, std::string_view pointer, std::string_view temp) {
        oasm << "   lsc_load." << sfid << " (M1,1) " << temp << ":d32x" << vector_size << "t flat["
             << pointer << "]:" << addrsize << "\n";
        for (std::int32_t m = 0; m < cfg.row_blocks; ++m) {
            for (std::int32_t a = 0; a < cfg.array_length; ++a) {
                for (std::int32_t r = 0; r < cfg.rows; r += row_step_size) {
                    if (cfg.vnni) {
                        const auto es = row_step_size / ops_per_chan;
                        const auto cmod = c % ops_per_chan;
                        const auto cbase = c - cmod;
                        for (std::int32_t o = 0; o < ops_per_chan; ++o) {
                            oasm << "   mov (M1," << es << ") "
                                 << h.dst_op(r + cmod, cbase + o, a, n, m) << "<" << ops_per_chan
                                 << "> " << h.temp_op(temp, r + o * es, a, m) << "<1;1,0>\n";
                        }
                    } else {
                        oasm << "   mov (M1," << row_step_size << ") " << h.dst_op(r, c, a, n, m)
                             << "<1> " << h.temp_op(temp, r, a, m) << "<1;1,0>\n";
                    }
                }
            }
        }
    });
    if (cfg.transpose || cfg.post_d32_transpose8x8) {
        for (std::int32_t m = 0; m < cfg.row_blocks; ++m) {
            for (std::int32_t n = 0; n < cfg.col_blocks; ++n) {
                for (std::int32_t a = 0; a < cfg.array_length; ++a) {
                    make_d32_transpose8x8(oasm, "$0", cfg.byte_offset(0, 0, a, n, m), make_tmp);
                }
            }
        }
    }
    oasm << "}\n";

    return std::move(oasm).str();
}

auto store_block2d_emulated(block_config const &cfg, temp_counter &make_tmp) -> std::string {
    const auto addrsize = cfg.sfid == lsc_sfid::slm ? "a32" : "a64";
    const auto sfid = to_string(cfg.sfid);
    const auto vector_size = lsc_vector_size_d32(cfg.rows * cfg.row_blocks * cfg.element_size);

    auto oasm = std::ostringstream{};
    auto h = block2d_emulated_helper(oasm, cfg, xe::store_batch_size, make_tmp);

    oasm << "{\n";
    h.header();

    h.walk([&](std::int32_t n, std::int32_t c, std::string_view pointer, std::string_view temp) {
        for (std::int32_t m = 0; m < cfg.row_blocks; ++m) {
            for (std::int32_t a = 0; a < cfg.array_length; ++a) {
                for (std::int32_t r = 0; r < cfg.rows; r += xe::exec_size) {
                    oasm << "   mov (M1," << xe::exec_size << ") " << h.temp_op(temp, r, a, m)
                         << "<1> " << h.dst_op(r, c, a, n, m) << "<1;1,0>\n";
                }
            }
        }
        oasm << "   lsc_store." << sfid << " (M1,1) flat[" << pointer << "]:" << addrsize << " "
             << temp << ":d32x" << vector_size << "t\n";
    });
    oasm << "}\n";

    return std::move(oasm).str();
}

auto load_block2d(block_config const &cfg, temp_counter &make_tmp) -> std::string {
    const bool ugm_ok = cfg.sfid == lsc_sfid::ugm;
    const bool transpose_ok =
        !cfg.transpose || (cfg.element_size == 4 && cfg.rows <= xe::exec_size / 2);
    return ugm_ok && transpose_ok ? load_block2d_native(cfg, make_tmp)
                                  : load_block2d_emulated(cfg, make_tmp);
}
auto store_block2d(block_config const &cfg, temp_counter &make_tmp) -> std::string {
    const bool ugm_ok = cfg.sfid == lsc_sfid::ugm;
    return ugm_ok ? store_block2d_native(cfg, make_tmp) : store_block2d_emulated(cfg, make_tmp);
}

} // namespace tinytc::spv

