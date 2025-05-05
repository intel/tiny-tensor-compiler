// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "spv/coopmatrix_impl_block.hpp"
#include "analysis/gcd.hpp"
#include "codegen_tools.hpp"
#include "converter_aux.hpp"
#include "coopmatrix_layout.hpp"
#include "device_info.hpp"
#include "node/data_type_node.hpp"
#include "node/inst_node.hpp"
#include "node/value_node.hpp"
#include "scalar_type.hpp"
#include "spv/defs.hpp"
#include "spv/dope_vector.hpp"
#include "spv/instructions.hpp"
#include "spv/matrix_walker.hpp"
#include "spv/module.hpp"
#include "spv/uniquifier.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

namespace tinytc::spv {

auto coopmatrix_impl_block::load(cooperative_matrix_load_inst const &in, dope_vector const &odv,
                                 spv_inst *operand, spv_inst *pos0, spv_inst *pos1) -> spv_inst * {
    auto ot = get_memref_type(in.operand());
    auto rt = get_coopmatrix_type(in.result(0));
    auto layout = get_layout(cfg(), rt);
    auto sty = rt->component_ty();

    const std::int32_t required_alignment = ot->addrspace() == address_space::global ? 4 : 16;

    const bool layout_ok = layout.rows >= cfg().subgroup_size;
    const bool transpose_ok = in.t() == transpose::N;
    const bool alignment_ok = is_aligned(required_alignment, in.operand(), in.pos0());
    const bool checked_ok =
        in.checked() == checked_flag::none || in.checked() == checked_flag::cols;
    const bool sty_ok = !is_complex_type(sty);
    if (!layout_ok || !transpose_ok || !alignment_ok || !checked_ok || !sty_ok) {
        return coopmatrix_impl::load(in, odv, operand, pos0, pos1);
    }

    auto matrix_ty = spv_ty(layout);
    auto interface_ty = spv_interface_ty(layout);
    auto io_sty = get_io_sty(sty);
    auto io_ty = unique().scalar_ty(io_sty);
    auto pointer_ty = [&] {
        auto ot = get_memref_type(in.operand());
        const auto storage_cls = address_space_to_storage_class(ot->addrspace());
        const auto align = ot->element_alignment();
        return unique().pointer_ty(storage_cls, io_ty, align);
    }();

    auto walker = matrix_walker(unique(), cfg().subgroup_size, layout, pos0, pos1, odv.shape(0),
                                odv.shape(1), odv.stride(0), odv.stride(1), in.checked(), 0);

    auto &mod = unique().mod();
    operand = mod.add<OpBitcast>(pointer_ty, operand);
    spv_inst *result = mod.add<OpUndef>(matrix_ty);

    const auto ld = [&](tinytc_spv_mod &mod) -> spv_inst * {
        auto pointer = mod.add<OpInBoundsPtrAccessChain>(pointer_ty, operand, walker.offset(),
                                                         std::vector<spv_inst *>{});
        auto val = mod.add<OpSubgroupBlockReadINTEL>(io_ty, pointer);
        return mod.add<OpBitcast>(interface_ty, val);
    };
    const auto ld_chk = [&](tinytc_spv_mod &) {
        return make_conditional_execution(unique(), interface_ty, walker.col_ok(), ld,
                                          unique().null_constant(interface_ty), in.loc());
    };
    auto const ld_block = [&](tinytc_spv_mod &mod) {
        spv_inst *block_result = result;
        for (std::int64_t u = 0; u < layout.length / layout.blocks; ++u) {
            spv_inst *val = walker.needs_mask() || walker.cols_checked() ? ld_chk(mod) : ld(mod);
            block_result = insert(layout, val, block_result, walker.component_no());

            if (u < layout.cols - 1) {
                walker.advance_column();
            }
        }
        return block_result;
    };

    for (std::int64_t w = 0; w < layout.blocks; ++w) {
        result = ld_block(mod);

        if (w < layout.blocks - 1) {
            walker.advance_block();
        }
    }
    return result;
}

void coopmatrix_impl_block::store(cooperative_matrix_store_inst const &in, dope_vector const &odv,
                                  spv_inst *val, spv_inst *operand, spv_inst *pos0,
                                  spv_inst *pos1) {
    constexpr std::int32_t required_alignment = 16;

    auto vt = get_coopmatrix_type(in.val());
    auto layout = get_layout(cfg(), vt);
    auto sty = vt->component_ty();

    const bool layout_ok = layout.rows >= cfg().subgroup_size;
    const bool flag_ok = in.flag() == store_flag::regular;
    const bool alignment_ok = is_aligned(required_alignment, in.operand(), in.pos0());
    const bool checked_ok =
        in.checked() == checked_flag::none || in.checked() == checked_flag::cols;
    const bool sty_ok = !is_complex_type(sty);
    if (!layout_ok || !flag_ok || !alignment_ok || !checked_ok || !sty_ok) {
        coopmatrix_impl::store(in, odv, val, operand, pos0, pos1);
        return;
    }

    auto io_sty = get_io_sty(sty);
    auto io_ty = unique().scalar_ty(io_sty);
    auto pointer_ty = [&] {
        auto ot = get_memref_type(in.operand());
        const auto storage_cls = address_space_to_storage_class(ot->addrspace());
        const auto align = std::max(16, ot->element_alignment());
        return unique().pointer_ty(storage_cls, io_ty, align);
    }();

    auto walker = matrix_walker(unique(), cfg().subgroup_size, layout, pos0, pos1, odv.shape(0),
                                odv.shape(1), odv.stride(0), odv.stride(1), in.checked(), 0);

    auto &mod = unique().mod();
    operand = mod.add<OpBitcast>(pointer_ty, operand);

    for (std::int64_t w = 0; w < layout.blocks; ++w) {
        auto const st_block = [&](tinytc_spv_mod &mod) {
            for (std::int64_t u = 0; u < layout.length / layout.blocks; ++u) {
                const auto st = [&](tinytc_spv_mod &mod) {
                    auto pointer = mod.add<OpInBoundsPtrAccessChain>(
                        pointer_ty, operand, walker.offset(), std::vector<spv_inst *>{});
                    auto val_ij = extract(layout, val, walker.component_no());
                    auto val_ij_int = mod.add<OpBitcast>(io_ty, val_ij);
                    mod.add<OpSubgroupBlockWriteINTEL>(pointer, val_ij_int);
                };
                if (walker.needs_mask() || walker.cols_checked()) {
                    make_conditional_execution(unique(), walker.col_ok(), st);
                } else {
                    st(mod);
                }

                if (u < layout.cols - 1) {
                    walker.advance_column();
                }
            }
        };
        st_block(mod);

        if (w < layout.blocks - 1) {
            walker.advance_block();
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
        return scalar_type::i64;
    default:
        break;
    }
    return sty;
}

auto coopmatrix_impl_block::is_aligned(std::int32_t alignment, value_node const &operand,
                                       value_node const &pos0) -> bool {
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
