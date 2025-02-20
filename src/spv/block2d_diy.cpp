// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "spv/block2d_diy.hpp"
#include "spv/xe_constants.hpp"
#include "support/temp_counter.hpp"
#include "tinytc/types.hpp"

#include <algorithm>
#include <array>
#include <sstream>
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
auto block_config::total_rows() const -> std::int32_t { return array_length * rows * row_blocks; }

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
         << "add (M1,1) " << temp << "(0,4)<1> $4(0,0)<0;1,0> -1:d\n"
         << "mov (M1,1) " << temp << "(0,5)<1> $5(0,0)<0;1,0>\n"
         << "mov (M1,1) " << temp << "(0,6)<1> $6(0,0)<0;1,0>\n"
         << "mov (M1,1) " << temp << "(0,7)<1> 0x" << std::hex << block_size << ":ud\n";
}

template <typename F> void block2d_native_helper::walk(F &&io) {
    for (std::int32_t m = 0; m < cfg.row_blocks; ++m) {
        for (std::int32_t n = 0; n < cfg.col_blocks; ++n) {
            io(m, n);
            if (n + 1 < cfg.col_blocks) {
                oasm << "add (M1,1) " << temp << "(0,6)<1> " << temp << "(0,6)<0;1,0> " << cfg.cols
                     << ":ud\n";
            }
        }
        if (m + 1 < cfg.row_blocks) {
            oasm << "add (M1,1) " << temp << "(0,6)<1> " << temp << "(0,6)<0;1,0> "
                 << -(cfg.col_blocks - 1) * cfg.cols << ":ud\n";
            oasm << "add (M1,1) " << temp << "(0,5)<1> " << temp << "(0,5)<0;1,0> " << cfg.rows
                 << ":ud\n";
        }
    }
}

auto load_block2d_native(block_config const &cfg, temp_counter &make_tmp) -> std::string {
    const std::uint32_t num_dst = std::min(31, cfg.block_size_in_num_grf());
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

    auto oasm = std::ostringstream{};
    auto h = block2d_native_helper(oasm, cfg, make_tmp);

    oasm << "{\n";
    h.header();
    h.walk([&](std::int32_t m, std::int32_t n) {
        oasm << std::dec << "raw_sends.15.1.0." << num_dst << " (M1, 1) 0x0:ud 0x" << std::hex
             << desc << ":ud " << h.temp << ".0 %null.0 $0." << std::dec
             << cfg.byte_offset(0, 0, 0, n, m) << "\n";
    });
    oasm << "}\n";

    return std::move(oasm).str();
}
auto store_block2d_native(block_config const &cfg, temp_counter &make_tmp) -> std::string {
    const std::uint32_t num_src1 = std::min(31, cfg.block_size_in_num_grf());
    const std::uint32_t desc = [&] {
        const std::uint32_t data_size = cfg.element_size == 4 ? 2 : 1;
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
        oasm << "raw_sends.15.1." << num_src1 << ".0 (M1, 1) 0x0:ud 0x" << std::hex << desc
             << ":ud " << h.temp << ".0 $0." << std::dec << cfg.byte_offset(0, 0, 0, n, m)
             << " %null.0\n";
    });
    oasm << "}\n";

    return std::move(oasm).str();
}

struct block2d_emulated_helper {
    block2d_emulated_helper(std::ostream &oasm, block_config const &cfg, scalar_type sty,
                            std::size_t io_batch_size, temp_counter &make_tmp);
    void header();
    template <typename F> void walk(F &&io);

    std::ostream &oasm;
    block_config const &cfg;
    std::vector<std::string> temps, pointers;
    std::string offset;
};

block2d_emulated_helper::block2d_emulated_helper(std::ostream &oasm, block_config const &cfg,
                                                 scalar_type sty, std::size_t io_batch_size,
                                                 temp_counter &make_tmp)
    : oasm(oasm), cfg(cfg), temps(io_batch_size), pointers(io_batch_size),
      offset{make_tmp("offset")} {
    std::generate(temps.begin(), temps.end(), [&]() {
        const auto temp = make_tmp("temp");
        oasm << ".decl " << temp << " v_type=G type=" << visa_type(sty)
             << " num_elts=" << cfg.total_rows() << " align=wordx32\n";
        return temp;
    });
    std::generate(pointers.begin(), pointers.end(), [&]() {
        auto pointer = make_tmp("pointer");
        if (cfg.sfid == lsc_sfid::slm) {
            oasm << ".decl " << pointer << " v_type=G type=ud num_elts=1 align=wordx32\n";
        } else {
            oasm << ".decl " << pointer << " v_type=G type=uq num_elts=1 align=wordx32\n";
        }
        return pointer;
    });
}

void block2d_emulated_helper::header() {
    oasm << ".decl " << offset << " v_type=G type=ud num_elts=1 align=dword\n";
    oasm << "   mov (M1,1) " << pointers[0] << "(0,0)<1> $1(0,0)<0;1,0>\n";
    oasm << "   mul (M1,1) " << offset << "(0,0)<1> $5(0,0)<0;1,0> " << cfg.element_size << ":d\n";
    oasm << "   add (M1,1) " << pointers[0] << "(0,0)<1> " << pointers[0] << "(0,0)<0;1,0> "
         << offset << "(0,0)<0;1,0>\n";
    oasm << "   mul (M1,1) " << offset << "(0,0)<1> $6(0,0)<0;1,0> $4(0,0)<0;1,0>\n";
    oasm << "   add (M1,1) " << pointers[0] << "(0,0)<1> " << pointers[0] << "(0,0)<0;1,0> "
         << offset << "(0,0)<0;1,0>\n";
}

template <typename F> void block2d_emulated_helper::walk(F &&io) {
    std::int32_t tmp_no = 0;
    for (std::int32_t n = 0; n < cfg.col_blocks; ++n) {
        for (std::int32_t c = 0; c < cfg.cols; ++c) {
            io(n, c, pointers[tmp_no], temps[tmp_no]);
            if (n + 1 < cfg.col_blocks || c + 1 < cfg.cols) {
                const auto next_tmp_no = (tmp_no + 1) % pointers.size();
                oasm << "   add (M1,1) " << pointers[next_tmp_no] << "(0,0)<1> " << pointers[tmp_no]
                     << "(0,0)<0;1,0> $4(0,0)<0;1,0>\n";
                tmp_no = next_tmp_no;
            }
        }
    }
}

auto load_block2d_emulated(block_config const &cfg, scalar_type sty, temp_counter &make_tmp)
    -> std::string {
    const auto addrsize = cfg.sfid == lsc_sfid::slm ? "a32" : "a64";
    const auto sfid = to_string(cfg.sfid);
    const auto total_rows = cfg.array_length * cfg.rows * cfg.row_blocks;
    const auto vector_size = lsc_vector_size_d32(total_rows * cfg.element_size);

    auto oasm = std::ostringstream{};
    auto h = block2d_emulated_helper(oasm, cfg, sty, xe::load_batch_size, make_tmp);

    oasm << "{\n";
    h.header();

    h.walk([&](std::int32_t n, std::int32_t c, std::string_view pointer, std::string_view temp) {
        oasm << "   lsc_load." << sfid << " (M1,1) " << temp << ":d32x" << vector_size << "t flat["
             << pointer << "]:" << addrsize << "\n";
        for (std::int32_t m = 0; m < cfg.row_blocks; ++m) {
            for (std::int32_t a = 0; a < cfg.array_length; ++a) {
                for (std::int32_t r = 0; r < cfg.rows; r += xe::exec_size) {
                    const auto src_offset =
                        (r + a * cfg.rows + m * cfg.rows * cfg.array_length) * cfg.element_size;
                    const auto src_brow = src_offset / xe::grf_size;
                    const auto src_bcol = src_offset % xe::grf_size / cfg.element_size;
                    const auto dst_offset = cfg.byte_offset(r, c, a, n, m);
                    const auto dst_brow = dst_offset / xe::grf_size;
                    const auto dst_bcol = dst_offset % xe::grf_size / cfg.element_size;
                    oasm << "   mov (M1," << xe::exec_size << ") $0(" << dst_brow << "," << dst_bcol
                         << ")<1> " << temp << "(" << src_brow << "," << src_bcol << ")<1;1,0>\n";
                }
            }
        }
    });
    oasm << "}\n";

    return std::move(oasm).str();
}

auto store_block2d_emulated(block_config const &cfg, scalar_type sty, temp_counter &make_tmp)
    -> std::string {
    const auto addrsize = cfg.sfid == lsc_sfid::slm ? "a32" : "a64";
    const auto sfid = to_string(cfg.sfid);
    const auto vector_size = lsc_vector_size_d32(cfg.rows * cfg.row_blocks * cfg.element_size);

    auto oasm = std::ostringstream{};
    auto h = block2d_emulated_helper(oasm, cfg, sty, xe::store_batch_size, make_tmp);

    oasm << "{\n";
    h.header();

    h.walk([&](std::int32_t n, std::int32_t c, std::string_view pointer, std::string_view temp) {
        for (std::int32_t m = 0; m < cfg.row_blocks; ++m) {
            for (std::int32_t r = 0; r < cfg.rows; r += xe::exec_size) {
                const auto src_offset = cfg.byte_offset(r, c, 0, n, m);
                const auto src_brow = src_offset / xe::grf_size;
                const auto src_bcol = src_offset % xe::grf_size / cfg.element_size;
                const auto dst_offset = (r + m * cfg.rows) * cfg.element_size;
                const auto dst_brow = dst_offset / xe::grf_size;
                const auto dst_bcol = dst_offset % xe::grf_size / cfg.element_size;
                oasm << "   mov (M1," << xe::exec_size << ") " << temp << "(" << dst_brow << ","
                     << dst_bcol << ")<1> $0(" << src_brow << "," << src_bcol << ")<1;1,0>\n";
            }
        }
        oasm << "   lsc_store." << sfid << " (M1,1) flat[" << pointer << "]:" << addrsize << " "
             << temp << ":d32x" << vector_size << "t\n";
    });
    oasm << "}\n";

    return std::move(oasm).str();
}

} // namespace tinytc::spv

