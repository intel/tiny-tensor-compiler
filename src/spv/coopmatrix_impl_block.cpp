// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "spv/coopmatrix_impl_block.hpp"
#include "analysis/gcd.hpp"
#include "codegen_tools.hpp"
#include "converter_aux.hpp"
#include "coopmatrix_layout.hpp"
#include "device_info.hpp"
#include "node/data_type.hpp"
#include "node/inst_view.hpp"
#include "spv/defs.hpp"
#include "spv/dope_vector.hpp"
#include "spv/instructions.hpp"
#include "spv/matrix_walker.hpp"
#include "spv/module.hpp"
#include "spv/uniquifier.hpp"
#include "tinytc/builder.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"
#include "util/math.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

namespace tinytc::spv {

auto max_block_io_vec_size(scalar_type sty) -> std::int64_t {
    return sty == scalar_type::i8 || sty == scalar_type::i16 ? 16 : 8;
}

auto coopmatrix_impl_block::load(cooperative_matrix_load_inst in, dope_vector const &odv,
                                 spv_inst *operand, spv_inst *pos0, spv_inst *pos1) -> spv_inst * {
    const auto ot = get_memref_type(in.operand());
    const auto rt = get_coopmatrix_type(in.result());
    const auto layout = get_layout(cfg(), rt);
    const auto sty = layout.sty;

    const std::int32_t required_alignment = ot->addrspace() == address_space::global ? 4 : 16;

    const bool layout_ok = layout.rows >= cfg().subgroup_size;
    const bool transpose_ok = in.t() == transpose::N;
    const bool alignment_ok = is_aligned(required_alignment, in.operand(), in.pos0());
    const bool checked_ok =
        in.checked() == checked_flag::none || in.checked() == checked_flag::cols;
    const bool sty_ok = sty != scalar_type::c64; // We do not have 16 byte/lane block loads
    if (!layout_ok || !transpose_ok || !alignment_ok || !checked_ok || !sty_ok) {
        return coopmatrix_impl::load(in, odv, operand, pos0, pos1);
    }

    auto walker = matrix_walker(unique(), cfg().subgroup_size, layout, pos0, pos1, odv.shape(0),
                                odv.shape(1), odv.stride(0), odv.stride(1), in.checked(), 0);

    const auto io_sty = get_io_sty(sty);
    const std::int32_t blocks_per_load =
        is_positive_power_of_two(layout.blocks)
            ? std::min(layout.blocks, max_block_io_vec_size(io_sty))
            : 1;
    const std::int32_t cols_per_load = [&]() -> std::int32_t {
        const auto ot = get_memref_type(in.operand());
        const bool is_contiguous = ot->dim() == 2 && ot->shape(0) == rt->shape(0) &&
                                   ot->stride(0) == 1 && ot->stride(1) == ot->shape(0);
        std::int32_t cols_per_load = 1;
        if (is_contiguous && !walker.cols_checked()) {
            const std::int32_t max_cols_per_load = max_block_io_vec_size(io_sty) / blocks_per_load;
            const std::int32_t num_cols = layout.length / layout.blocks;
            while (2 * cols_per_load <= max_cols_per_load && num_cols % (2 * cols_per_load) == 0) {
                cols_per_load *= 2;
            }
        }
        return cols_per_load;
    }();

    const auto matrix_ty = spv_ty(layout);
    const auto interface_ty = spv_interface_ty(layout);
    auto io_ty = unique().scalar_ty(io_sty);
    const auto io_vec_size = blocks_per_load * cols_per_load;
    spv_inst *io_vec_ty = io_vec_size > 1 ? unique().vec_ty(io_ty, io_vec_size) : io_ty;
    const auto pointer_ty = [&] {
        auto ot = get_memref_type(in.operand());
        const auto storage_cls = address_space_to_storage_class(ot->addrspace());
        const auto align = ot->element_alignment();
        return unique().pointer_ty(storage_cls, io_ty, align);
    }();

    auto &mod = unique().mod();
    operand = mod.add<OpBitcast>(pointer_ty, operand);
    spv_inst *result = mod.add<OpUndef>(matrix_ty);

    const auto ld = [&](tinytc_spv_mod &mod) -> spv_inst * {
        spv_inst *offset = walker.offset();
        auto pointer = mod.add<OpInBoundsPtrAccessChain>(pointer_ty, operand, offset,
                                                         std::vector<spv_inst *>{});
        return mod.add<OpSubgroupBlockReadINTEL>(io_vec_ty, pointer);
    };
    const auto ld_chk = [&](tinytc_spv_mod &) {
        return make_conditional_execution(unique(), interface_ty, walker.col_ok(), ld,
                                          unique().null_constant(io_vec_ty), in.loc());
    };
    auto const ld_block = [&](tinytc_spv_mod &mod) {
        spv_inst *block_result = result;
        for (std::int64_t u = 0; u < layout.length / layout.blocks; u += cols_per_load) {
            spv_inst *val = walker.needs_mask() || walker.cols_checked() ? ld_chk(mod) : ld(mod);
            if (io_vec_size > 1) {
                for (std::int32_t c = 0; c < cols_per_load; ++c) {
                    for (std::int32_t b = 0; b < blocks_per_load; ++b) {
                        spv_inst *v = mod.add<OpCompositeExtract>(
                            io_ty, val, std::vector<LiteralInteger>{b + c * blocks_per_load});
                        v = mod.add<OpBitcast>(interface_ty, v);
                        const auto comp_no =
                            layout.component_no(walker.col_no() + c, walker.block_no() + b);
                        block_result = insert(layout, v, block_result, comp_no);
                    }
                }
            } else {
                val = mod.add<OpBitcast>(interface_ty, val);
                block_result = insert(layout, val, block_result, walker.component_no());
            }

            if (u < layout.cols - 1) {
                for (std::int32_t c = 0; c < cols_per_load; ++c) {
                    walker.advance_column();
                }
            }
        }
        return block_result;
    };

    for (std::int64_t w = 0; w < layout.blocks; w += blocks_per_load) {
        result = ld_block(mod);

        if (w < layout.blocks - blocks_per_load) {
            for (std::int32_t b = 0; b < blocks_per_load; ++b) {
                walker.advance_block();
            }
        }
    }
    return result;
}

void coopmatrix_impl_block::store(cooperative_matrix_store_inst in, dope_vector const &odv,
                                  spv_inst *val, spv_inst *operand, spv_inst *pos0,
                                  spv_inst *pos1) {
    constexpr std::int32_t required_alignment = 16;

    auto vt = get_coopmatrix_type(in.val());
    auto layout = get_layout(cfg(), vt);
    auto sty = vt->component_ty();

    const bool layout_ok = layout.rows >= cfg().subgroup_size;
    const bool transpose_ok = in.t() == transpose::N;
    const bool flag_ok = in.flag() == store_flag::regular;
    const bool alignment_ok = is_aligned(required_alignment, in.operand(), in.pos0());
    const bool checked_ok =
        in.checked() == checked_flag::none || in.checked() == checked_flag::cols;
    const bool sty_ok = sty != scalar_type::c64; // We do not have 16 byte/lane block writes
    if (!layout_ok || !transpose_ok || !flag_ok || !alignment_ok || !checked_ok || !sty_ok) {
        coopmatrix_impl::store(in, odv, val, operand, pos0, pos1);
        return;
    }

    auto walker = matrix_walker(unique(), cfg().subgroup_size, layout, pos0, pos1, odv.shape(0),
                                odv.shape(1), odv.stride(0), odv.stride(1), in.checked(), 0);

    const auto io_sty = get_io_sty(sty);
    const std::int32_t blocks_per_store =
        is_positive_power_of_two(layout.blocks)
            ? std::min(layout.blocks, max_block_io_vec_size(io_sty))
            : 1;
    const std::int32_t cols_per_store = [&]() -> std::int32_t {
        const auto ot = get_memref_type(in.operand());
        const bool is_contiguous = ot->dim() == 2 && ot->shape(0) == vt->shape(0) &&
                                   ot->stride(0) == 1 && ot->stride(1) == ot->shape(0);
        std::int32_t cols_per_store = 1;
        if (is_contiguous && !walker.cols_checked()) {
            const std::int32_t max_cols_per_store =
                max_block_io_vec_size(io_sty) / blocks_per_store;
            const std::int32_t num_cols = layout.length / layout.blocks;
            while (2 * cols_per_store <= max_cols_per_store &&
                   num_cols % (2 * cols_per_store) == 0) {
                cols_per_store *= 2;
            }
        }
        return cols_per_store;
    }();

    auto io_ty = unique().scalar_ty(io_sty);
    auto const io_vec_size = blocks_per_store * cols_per_store;
    spv_inst *io_vec_ty = io_vec_size > 1 ? unique().vec_ty(io_ty, io_vec_size) : io_ty;
    const auto pointer_ty = [&] {
        auto ot = get_memref_type(in.operand());
        const auto storage_cls = address_space_to_storage_class(ot->addrspace());
        const auto align = std::max(16, ot->element_alignment());
        return unique().pointer_ty(storage_cls, io_ty, align);
    }();

    auto &mod = unique().mod();
    operand = mod.add<OpBitcast>(pointer_ty, operand);

    for (std::int64_t w = 0; w < layout.blocks; w += blocks_per_store) {
        auto const st_block = [&](tinytc_spv_mod &mod) {
            for (std::int64_t u = 0; u < layout.length / layout.blocks; u += cols_per_store) {
                const auto st = [&](tinytc_spv_mod &mod) {
                    spv_inst *offset = walker.offset();
                    auto pointer = mod.add<OpInBoundsPtrAccessChain>(pointer_ty, operand, offset,
                                                                     std::vector<spv_inst *>{});
                    spv_inst *val_ij = nullptr;
                    if (io_vec_size > 1) {
                        val_ij = mod.add<OpUndef>(io_vec_ty);
                        for (std::int32_t c = 0; c < cols_per_store; ++c) {
                            for (std::int32_t b = 0; b < blocks_per_store; ++b) {
                                const auto comp_no =
                                    layout.component_no(walker.col_no() + c, walker.block_no() + b);
                                spv_inst *v = extract(layout, val, comp_no);
                                v = mod.add<OpBitcast>(io_ty, v);
                                val_ij = mod.add<OpCompositeInsert>(
                                    io_vec_ty, v, val_ij,
                                    std::vector<LiteralInteger>{b + c * blocks_per_store});
                            }
                        }
                    } else {
                        val_ij = extract(layout, val, walker.component_no());
                        val_ij = mod.add<OpBitcast>(io_ty, val_ij);
                    }
                    mod.add<OpSubgroupBlockWriteINTEL>(pointer, val_ij);
                };
                if (walker.needs_mask() || walker.cols_checked()) {
                    make_conditional_execution(unique(), walker.col_ok(), st);
                } else {
                    st(mod);
                }

                if (u < layout.cols - 1) {
                    for (std::int32_t c = 0; c < cols_per_store; ++c) {
                        walker.advance_column();
                    }
                }
            }
        };
        st_block(mod);

        if (w < layout.blocks - blocks_per_store) {
            for (std::int32_t b = 0; b < blocks_per_store; ++b) {
                walker.advance_block();
            }
        }
    }
}

auto coopmatrix_impl_block::get_io_sty(scalar_type sty) -> scalar_type {
    switch (sty) {
    case scalar_type::bf16:
    case scalar_type::f16:
        return scalar_type::i16;
    case scalar_type::f32:
        return scalar_type::i32;
    case scalar_type::f64:
    case scalar_type::c32:
        return scalar_type::i64;
    default:
        break;
    }
    return sty;
}

auto coopmatrix_impl_block::is_aligned(std::int32_t alignment, tinytc_value const &operand,
                                       tinytc_value const &pos0) -> bool {
    auto const mt = get_memref_type(operand);
    const auto sty_size = size(mt->element_ty());
    if (sty_size >= static_cast<std::size_t>(alignment)) {
        return true;
    }
    if (auto mi = gcd().get_memref_if(operand); mi) {
        const bool base_ok = (mi->offset_gcd() * sty_size) % alignment == 0;
        const bool pos0_ok = (gcd().get(pos0) * sty_size) % alignment == 0;
        const bool stride_ok =
            mt->stride(0) == 1 && (mi->stride_gcd()[1] * sty_size) % alignment == 0;

        return base_ok && pos0_ok && stride_ok;
    }
    return false;
}

} // namespace tinytc::spv
