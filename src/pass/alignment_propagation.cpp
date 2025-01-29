// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/alignment_propagation.hpp"
#include "analysis/gcd.hpp"
#include "codegen_tools.hpp"
#include "error.hpp"
#include "node/data_type_node.hpp"
#include "node/inst_node.hpp"
#include "node/value_node.hpp"
#include "support/casting.hpp"
#include "support/util.hpp"
#include "support/visit.hpp"
#include "support/walk.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <ranges>
#include <unordered_map>
#include <utility>
#include <vector>

namespace tinytc {

auto is_aligned(std::vector<std::int64_t> const &offset_gcds,
                std::vector<std::int64_t> const &stride, std::int32_t alignment) -> bool {
    if (offset_gcds.size() != stride.size()) {
        throw internal_compiler_error{};
    }

    auto const check = [&alignment](std::int64_t a, std::int64_t b) {
        if (b == dynamic) {
            return (a % alignment) == 0;
        } else {
            return ((a * b) % alignment) == 0;
        }
    };

    auto oit = offset_gcds.begin();
    auto sit = stride.begin();

    for (; oit != offset_gcds.end() && sit != stride.end(); ++oit, ++sit) {
        if (!check(*oit, *sit)) {
            return false;
        }
    }
    return true;
}

class alignment_propagation_helper {
  public:
    inline alignment_propagation_helper(gcd_analysis_result &&gcd) : gcd_{std::move(gcd)} {}

    constexpr static std::int64_t alloca_max_alignment = 64;

    void operator()(inst_node &in);
    void operator()(alloca_inst &in);
    void operator()(cooperative_matrix_load_inst &in);
    void operator()(cooperative_matrix_store_inst &in);
    void operator()(expand_inst &in);
    void operator()(fuse_inst &in);
    void operator()(load_inst &in);
    void operator()(store_inst &in);
    void operator()(subview_inst &in);

    auto known_alignment(value_node const &val) const -> std::int32_t;
    void known_alignment(value_node const &val, std::int32_t);

  private:
    auto compute_max_alignment(tinytc_value const &operand,
                               std::vector<std::int64_t> const &offset_gcds) const -> std::int32_t;
    template <typename T> auto get_gcds(T &&val_range) -> std::vector<std::int64_t> {
        auto result = std::vector<std::int64_t>{};
        result.reserve(val_range.size());
        for (auto &val : val_range) {
            result.emplace_back(gcd_.get(val));
        }
        return result;
    }

    gcd_analysis_result gcd_;
    std::unordered_map<value_node const *, std::int32_t> known_alignment_;
};

auto alignment_propagation_helper::compute_max_alignment(
    tinytc_value const &operand,
    std::vector<std::int64_t> const &offset_gcds) const -> std::int32_t {
    const auto op_align = known_alignment(operand);
    const auto ot = get_memref_type(operand);

    const auto &stride = ot->stride();
    const auto sty_size = size(ot->element_ty());

    for (std::int32_t align = op_align; align > ot->alignment(); align /= 2) {
        if (is_aligned(offset_gcds, stride, align / sty_size)) {
            return align;
        }
    }
    return 0;
}

void alignment_propagation_helper::operator()(inst_node &) {}
void alignment_propagation_helper::operator()(alloca_inst &in) {
    if (in.stack_ptr() >= 0) {
        const auto rt = get_memref_type(in.result(0).ty());
        std::int32_t i = rt->alignment();
        while (i < alloca_max_alignment) {
            const auto i2 = 2 * i;
            if (in.stack_ptr() % i2 != 0) {
                break;
            }
            i = i2;
        }
        if (i > rt->alignment()) {
            known_alignment(in.result(0), i);
        }
    }
}
void alignment_propagation_helper::operator()(expand_inst &in) {
    known_alignment(in.result(0), known_alignment(in.operand()));
}
void alignment_propagation_helper::operator()(fuse_inst &in) {
    known_alignment(in.result(0), known_alignment(in.operand()));
}
void alignment_propagation_helper::operator()(cooperative_matrix_load_inst &in) {
    auto index_list = std::array<const_tinytc_value_t, 2u>{&in.pos0(), &in.pos1()};
    const auto align = compute_max_alignment(in.operand(), get_gcds(index_list));
    if (align != 0) {
        in.align(align);
    }
}
void alignment_propagation_helper::operator()(cooperative_matrix_store_inst &in) {
    auto index_list = std::array<const_tinytc_value_t, 2u>{&in.pos0(), &in.pos1()};
    const auto align = compute_max_alignment(in.operand(), get_gcds(index_list));
    if (align != 0) {
        in.align(align);
    }
}
void alignment_propagation_helper::operator()(load_inst &in) {
    if (isa<group_data_type>(*in.operand().ty())) {
        known_alignment(in.result(0), known_alignment(in.operand()));
        return;
    }
    const auto align = compute_max_alignment(in.operand(), get_gcds(in.index_list()));
    if (align != 0) {
        in.align(align);
    }
}
void alignment_propagation_helper::operator()(subview_inst &in) {
    auto offset_gcds = std::vector<std::int64_t>{};
    std::int64_t dim = in.static_offsets().size();
    offset_gcds.reserve(dim);
    std::int64_t ri = 0;
    for (std::int64_t i = 0; i < dim; ++i) {
        auto stat_off = in.static_offsets()[i];
        if (is_dynamic_value(stat_off)) {
            offset_gcds.emplace_back(gcd_.get(in.offsets()[ri++]));
            ++ri;
        } else {
            offset_gcds.emplace_back(stat_off);
        }
    }

    const auto align = compute_max_alignment(in.operand(), offset_gcds);
    if (align != 0) {
        known_alignment(in.result(0), align);
    }
}
void alignment_propagation_helper::operator()(store_inst &in) {
    const auto align = compute_max_alignment(in.operand(), get_int_constants(in.index_list()));
    if (align != 0) {
        in.align(align);
    }
}

auto alignment_propagation_helper::known_alignment(value_node const &val) const -> std::int32_t {
    if (auto it = known_alignment_.find(&val); it != known_alignment_.end()) {
        return it->second;
    }
    return 0;
}
void alignment_propagation_helper::known_alignment(value_node const &val, std::int32_t align) {
    if (align != 0) {
        known_alignment_[&val] = align;
    }
}

void alignment_propagation_pass::run_on_function(function_node &fn) {
    auto visitor = alignment_propagation_helper{gcd_analysis{}.run_on_function(fn)};
    for (std::int32_t arg_no = 0; arg_no < fn.num_params(); ++arg_no) {
        auto align = fn.align(arg_no);
        visitor.known_alignment(fn.params()[arg_no], align);
    }

    walk<walk_order::pre_order>(fn, [&visitor](inst_node &in) { visit(visitor, in); });
}

} // namespace tinytc
