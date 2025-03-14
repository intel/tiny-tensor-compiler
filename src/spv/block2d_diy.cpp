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
    std::int32_t offset = 0;
    if (transpose) {
        offset = col_block;
        offset = row_block + offset * row_blocks;
    } else {
        offset = row_block;
        offset = col_block + offset * col_blocks;
    }
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

/**
 * This routine generates transpose code for 8x8 matrices of d32 type. Multiple 8x8 matrices may be
 * packed side-by-side. E.g. for num_8x8_blocks=2 the GRF layout, say starting at reg r93,
 * is expected to be
 *
 *  r93: a_11 ... a_18 b_11 ... b_18
 *  ...: ...      ...  ...      ...
 * r100: a_81 ... a_88 b_81 ... b_88
 *
 * The transposition is done in-place and we should get
 *
 *  r93: a_11 ... a_81 b_11 ... b_81
 *  ...: ...      ...  ...      ...
 * r100: a_18 ... a_88 b_18 ... b_88
 *
 * This routine can also be called for 16x8 half types or 32x8 i8 types.
 * Then, the routine generates transpose + VNNI transform.
 *
 */
auto make_d32_transpose8x8(std::ostream &oasm, char const *matrix, std::size_t offset,
                           temp_counter &make_tmp, std::int32_t num_8x8_blocks = 1) {
    constexpr std::int32_t element_size = 4;
    const std::int32_t stride = 8 * element_size * num_8x8_blocks;
    const std::int32_t num_elements = 8 * stride / element_size;

    auto dst_d = make_tmp("dst_d");
    auto dst_q = make_tmp("dst_q");
    oasm << ".decl " << dst_d << " v_type=G type=d num_elts=" << num_elements
         << " align=wordx32 alias=<" << matrix << "," << offset << ">\n";
    oasm << ".decl " << dst_q << " v_type=G type=q num_elts=" << num_elements / 2
         << " align=wordx32 alias=<" << matrix << "," << offset << ">\n";

    // 2x2 transpose
    const std::int32_t exec_size = 4 * num_8x8_blocks;
    for (std::int32_t r = 0; r < 4; ++r) {
        auto ttmp = make_tmp("ttmp_d");
        oasm << ".decl " << ttmp << " v_type=G type=d num_elts=" << exec_size << " align=wordx32\n";
        auto [R1, C1] = region_origin(element_size, 2 * r * stride + element_size);
        auto [R2, C2] = region_origin(element_size, 2 * r * stride + stride);
        oasm << "mov (M1," << exec_size << ") " << ttmp << "(0,0)<1> " << dst_d << "(" << R1 << ","
             << C1 << ")<2;1,0>\n";
        oasm << "mov (M1," << exec_size << ") " << dst_d << "(" << R1 << "," << C1 << ")<2> "
             << dst_d << "(" << R2 << "," << C2 << ")<2;1,0>\n";
        oasm << "mov (M1," << exec_size << ") " << dst_d << "(" << R2 << "," << C2 << ")<2> "
             << ttmp << "(0,0)<1;1,0>\n";
    }
    // 4x4 transpose
    for (std::int32_t r = 0; r < 4; r += 2) {
        auto ttmp = make_tmp("ttmp_q");
        oasm << ".decl " << ttmp << " v_type=G type=q num_elts=" << exec_size << " align=wordx32\n";
        auto [R1, C1] = region_origin(2 * element_size, 2 * r * stride + 2 * element_size);
        auto [R2, C2] = region_origin(2 * element_size, 2 * (r + 1) * stride);
        oasm << "mov (M1," << exec_size << ") " << ttmp << "(0,0)<1> " << dst_q << "(" << R1 << ","
             << C1 << ")<2;1,0>\n";
        oasm << "mov (M1," << exec_size << ") " << dst_q << "(" << R1 << "," << C1 << ")<2> "
             << dst_q << "(" << R2 << "," << C2 << ")<2;1,0>\n";
        oasm << "mov (M1," << exec_size << ") " << dst_q << "(" << R2 << "," << C2 << ")<2> "
             << ttmp << "(0,0)<1;1,0>\n";
    }
    // 8x8 transpose
    for (std::int32_t r = 0; r < 4; ++r) {
        auto ttmp = make_tmp("ttmp_d");
        auto [R1, C1] = region_origin(element_size, r * stride + 4 * element_size);
        auto [R2, C2] = region_origin(element_size, (r + 4) * stride);
        oasm << ".decl " << ttmp << " v_type=G type=d num_elts=" << exec_size << " align=wordx32\n";
        oasm << "mov (M1," << exec_size << ") " << ttmp << "(0,0)<1> " << dst_d << "(" << R1 << ","
             << C1 << ")<8;4,1>\n";
        for (std::int32_t b = 0; b < 8 * num_8x8_blocks; b += 8) {
            oasm << "mov (M1,4) " << dst_d << "(" << R1 << "," << C1 + b << ")<1> " << dst_d << "("
                 << R2 << "," << C2 + b << ")<1;1,0>\n";
            oasm << "mov (M1,4) " << dst_d << "(" << R2 << "," << C2 + b << ")<1> " << ttmp << "(0,"
                 << b / 2 << ")<1;1,0>\n";
        }
    }
}

struct block2d_native_helper {
    block2d_native_helper(std::ostream &oasm, block_config const &cfg, temp_counter &make_tmp);
    void header();
    template <typename F> void walk(F &&io);

    inline auto temp(std::int32_t m, std::int32_t n) -> std::string const & {
        return temps[n + m * cfg.col_blocks];
    }

    std::ostream &oasm;
    block_config const &cfg;
    std::vector<std::string> temps;
    std::string tempq;
};

block2d_native_helper::block2d_native_helper(std::ostream &oasm, block_config const &cfg,
                                             temp_counter &make_tmp)
    : oasm(oasm), cfg(cfg), temps(cfg.row_blocks * cfg.col_blocks), tempq{make_tmp("tempq")} {
    std::generate(temps.begin(), temps.end(), [&]() { return make_tmp("temp"); });
}

void block2d_native_helper::header() {
    const std::uint32_t block_size =
        ((cfg.array_length - 1) << 16) | ((cfg.cols - 1) << 8) | (cfg.rows - 1);
    const auto &tmp0 = temp(0, 0);
    oasm << ".decl " << tmp0 << " v_type=G type=ud num_elts=8 align=wordx32\n"
         << ".decl " << tempq << " v_type=G type=uq num_elts=4 align=wordx32 alias=<" << tmp0
         << ",0>\n"
         << "mov (M1,1) " << tempq << "(0,0)<1> $1(0,0)<0;1,0>\n"
         << "add (M1,1) " << tmp0 << "(0,2)<1> $2(0,0)<0;1,0> -1:d\n"
         << "add (M1,1) " << tmp0 << "(0,3)<1> $3(0,0)<0;1,0> -1:d\n"
         << "add (M1,1) " << tmp0 << "(0,4)<1> $4(0,0)<0;1,0> -1:d\n";
    if (cfg.pos0_shr) {
        oasm << "shr (M1,1) " << tmp0 << "(0,5)<1> $5(0,0)<0;1,0> " << cfg.pos0_shr << ":d\n";
    } else {
        oasm << "mov (M1,1) " << tmp0 << "(0,5)<1> $5(0,0)<0;1,0>\n";
    }
    oasm << "mov (M1,1) " << tmp0 << "(0,6)<1> $6(0,0)<0;1,0>\n"
         << "mov (M1,1) " << tmp0 << "(0,7)<1> 0x" << std::hex << block_size << ":ud\n";
    for (std::int32_t m = 0; m < cfg.row_blocks; ++m) {
        for (std::int32_t n = 0; n < cfg.col_blocks; ++n) {
            const auto &tmp = temp(m, n);
            if (m > 0 || n > 0) {
                oasm << ".decl " << tmp << " v_type=G type=ud num_elts=8 align=wordx32\n";
                oasm << "mov (M1,8) " << tmp << "(0,0)<1> " << tmp0 << "(0,0)<1;1,0>\n";
            }
            if (m > 0) {
                oasm << "add (M1,1) " << tmp << "(0,5)<1> " << tmp << "(0,5)<0;1,0> 0x"
                     << m * cfg.rows * cfg.array_length << ":ud\n";
            }
            if (n > 0) {
                oasm << "add (M1,1) " << tmp << "(0,6)<1> " << tmp << "(0,6)<0;1,0> 0x"
                     << n * cfg.cols << ":ud\n";
            }
        }
    }
}

template <typename F> void block2d_native_helper::walk(F &&io) {
    for (std::int32_t m = 0; m < cfg.row_blocks; ++m) {
        for (std::int32_t n = 0; n < cfg.col_blocks; ++n) {
            io(m, n);
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
        if (cfg.transpose && !cfg.vnni) {
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
             << desc << ":ud " << h.temp(m, n) << ".0 %null.0 $0." << std::dec << offset << "\n";

        if (cfg.vnni && cfg.transpose) {
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
             << ":ud " << h.temp(m, n) << ".0 $0." << std::dec << offset << " %null.0\n";
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
    std::string offset_x, offset_y, total_offset, dst;
};

block2d_emulated_helper::block2d_emulated_helper(std::ostream &oasm, block_config const &cfg,
                                                 std::size_t io_batch_size, temp_counter &make_tmp)
    : oasm(oasm), cfg(cfg), temps(io_batch_size),
      pointers(io_batch_size), offset_x{make_tmp("offset_x")}, offset_y{make_tmp("offset_y")},
      total_offset{make_tmp("total_offset")}, dst{make_tmp("dst")} {
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
            oasm << ".decl " << pointer << " v_type=G type=uq num_elts=1 align=wordx64\n";
        }
    });
    oasm << ".decl " << dst << " v_type=G type=" << visa_ty
         << " num_elts=" << cfg.total_rows() * cfg.cols * cfg.col_blocks
         << " align=wordx32 alias=<$0,0>\n";
    oasm << ".decl " << offset_x << " v_type=G type=d num_elts=1 align=qword\n";
    oasm << ".decl " << offset_y << " v_type=G type=d num_elts=1 align=qword\n";
    oasm << ".decl " << total_offset << " v_type=G type=d num_elts=1 align=qword\n";
    oasm << "   mul (M1,1) " << offset_x << "(0,0)<1> $5(0,0)<0;1,0> " << cfg.element_size
         << ":w\n";
    oasm << "   mul (M1,1) " << offset_y << "(0,0)<1> $6(0,0)<0;1,0> $4(0,0)<0;1,0>\n";
    oasm << "   add (M1,1) " << total_offset << "(0,0)<1> " << offset_x << "(0,0)<0;1,0> "
         << offset_y << "(0,0)<0;1,0>\n";
    oasm << "   add (M1,1) " << pointers[0] << "(0,0)<1> $1(0,0)<0;1,0> " << total_offset
         << "(0,0)<0;1,0>\n";
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
    const auto vector_size = lsc_vector_size_d32(cfg.total_rows() * cfg.element_size);

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
                        if (cfg.transpose) {
                            const auto spread_factor = cfg.cols / 8;
                            const auto C = c % 8 * spread_factor + c / 8;
                            oasm << "   mov (M1," << row_step_size << ") "
                                 << h.dst_op(r, C, a, n, m) << "<1> " << h.temp_op(temp, r, a, m)
                                 << "<1;1,0>\n";
                        } else {
                            oasm << "   mov (M1," << row_step_size << ") "
                                 << h.dst_op(r, c, a, n, m) << "<1> " << h.temp_op(temp, r, a, m)
                                 << "<1;1,0>\n";
                        }
                    }
                }
            }
        }
    });
    if (cfg.transpose) {
        for (std::int32_t m = 0; m < cfg.row_blocks; ++m) {
            for (std::int32_t n = 0; n < cfg.col_blocks; ++n) {
                for (std::int32_t a = 0; a < cfg.array_length; ++a) {
                    make_d32_transpose8x8(oasm, "$0", cfg.byte_offset(0, 0, a, n, m), make_tmp,
                                          cfg.cols / (8 * ops_per_chan));
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
