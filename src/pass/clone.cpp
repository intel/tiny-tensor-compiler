// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/clone.hpp"
#include "node/visit.hpp"
#include "tinytc/types.hpp"
#include "util/ilist.hpp"
#include "util/ilist_base.hpp"

#include <cstdint>
#include <iterator>
#include <utility>

namespace tinytc {

void inst_cloner::reset_subs() { subs_map_.clear(); }
void inst_cloner::set_subs(tinytc_value_t in_val, tinytc_value_t out_val) {
    subs_map_[in_val] = out_val;
}
auto inst_cloner::subs(tinytc_value_t val) -> tinytc_value_t {
    if (auto it = subs_map_.find(val); it != subs_map_.end()) {
        return it->second;
    }
    return val;
}

auto inst_cloner::clone_instruction(inst_node &in) -> inst {
    auto cloned = visit(
        [&](auto view) {
            auto tid = view.get().type_id();
            auto layout = view.get().layout();
            auto lc = view.get().loc();
            auto clone = inst{tinytc_inst::create(tid, layout, lc)};
            for (std::int32_t ret_no = 0; ret_no < layout.num_results; ++ret_no) {
                clone->result(ret_no) = value_node{view.get().result(ret_no).ty(), clone.get(), lc};
            }
            for (std::int32_t op_no = 0; op_no < layout.num_operands; ++op_no) {
                clone->op(op_no, subs(&view.get().op(op_no)));
            }

            auto clone_view = decltype(view)(clone.get());
            clone_view.props() = view.props();
            clone_view.setup_and_check();

            return clone;
        },
        in);

    for (auto res_orig = in.result_begin(), res_cloned = cloned->result_begin();
         res_orig != in.result_end() && res_cloned != cloned->result_end();
         ++res_orig, ++res_cloned) {
        set_subs(&(*res_orig), &(*res_cloned));
    }
    for (auto reg_orig = in.child_regions_begin(), reg_cloned = cloned->child_regions_begin();
         reg_orig != in.child_regions_end() && reg_cloned != cloned->child_regions_end();
         ++reg_orig, ++reg_cloned) {
        for (auto p_orig = reg_orig->param_begin(), p_cloned = reg_cloned->param_begin();
             p_orig != reg_orig->param_end() && p_cloned != reg_cloned->param_end();
             ++p_orig, ++p_cloned) {
            set_subs(&(*p_orig), &(*p_cloned));
        }
        clone_region(*reg_orig, *reg_cloned);
    }
    return cloned;
}

void inst_cloner::clone_region(region_node &source, region_node &target) {
    for (auto &in_orig : source.insts()) {
        target.insts().push_back(clone_instruction(in_orig).release());
    }
}

} // namespace tinytc
