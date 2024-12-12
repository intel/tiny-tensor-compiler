// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/clone.hpp"
#include "node/data_type_node.hpp"
#include "support/ilist.hpp"
#include "support/ilist_base.hpp"
#include "support/util.hpp"
#include "support/visit.hpp"

#include <cstdint>
#include <ranges>
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

auto inst_cloner::operator()(alloca_inst &in) -> std::unique_ptr<tinytc_inst> {
    return std::make_unique<alloca_inst>(in.result(0).ty(), in.loc());
}
auto inst_cloner::operator()(axpby_inst &in) -> std::unique_ptr<tinytc_inst> {
    return std::make_unique<axpby_inst>(in.tA(), subs(&in.alpha()), subs(&in.A()), subs(&in.beta()),
                                        subs(&in.B()), in.atomic(), in.loc());
}
auto inst_cloner::operator()(arith_inst &in) -> std::unique_ptr<tinytc_inst> {
    return std::make_unique<arith_inst>(in.operation(), subs(&in.a()), subs(&in.b()),
                                        in.result(0).ty(), in.loc());
}
auto inst_cloner::operator()(arith_unary_inst &in) -> std::unique_ptr<tinytc_inst> {
    return std::make_unique<arith_unary_inst>(in.operation(), subs(&in.a()), in.result(0).ty(),
                                              in.loc());
}
auto inst_cloner::operator()(barrier_inst &in) -> std::unique_ptr<tinytc_inst> {
    return std::make_unique<barrier_inst>(in.fence_flags(), in.loc());
}
auto inst_cloner::operator()(builtin_inst &in) -> std::unique_ptr<tinytc_inst> {
    return std::make_unique<builtin_inst>(in.builtin_type(), in.result(0).ty(), in.loc());
}
auto inst_cloner::operator()(cast_inst &in) -> std::unique_ptr<tinytc_inst> {
    return std::make_unique<cast_inst>(subs(&in.a()), in.result(0).ty(), in.loc());
}
auto inst_cloner::operator()(compare_inst &in) -> std::unique_ptr<tinytc_inst> {
    return std::make_unique<compare_inst>(in.cond(), subs(&in.a()), subs(&in.b()),
                                          in.result(0).ty(), in.loc());
}
auto inst_cloner::operator()(constant_inst &in) -> std::unique_ptr<tinytc_inst> {
    return std::make_unique<constant_inst>(in.value(), in.result(0).ty(), in.loc());
}
auto inst_cloner::operator()(cooperative_matrix_load_inst &in) -> std::unique_ptr<tinytc_inst> {
    return std::make_unique<cooperative_matrix_load_inst>(in.t(), in.checked(), subs(&in.operand()),
                                                          subs(&in.pos0()), subs(&in.pos1()),
                                                          in.align(), in.result(0).ty(), in.loc());
}
auto inst_cloner::operator()(cooperative_matrix_mul_add_inst &in) -> std::unique_ptr<tinytc_inst> {
    return std::make_unique<cooperative_matrix_mul_add_inst>(
        subs(&in.a()), subs(&in.b()), subs(&in.c()), in.result(0).ty(), in.loc());
}
auto inst_cloner::operator()(cooperative_matrix_scale_inst &in) -> std::unique_ptr<tinytc_inst> {
    return std::make_unique<cooperative_matrix_scale_inst>(subs(&in.a()), subs(&in.b()),
                                                           in.result(0).ty(), in.loc());
}
auto inst_cloner::operator()(cooperative_matrix_store_inst &in) -> std::unique_ptr<tinytc_inst> {
    return std::make_unique<cooperative_matrix_store_inst>(in.checked(), in.flag(), subs(&in.val()),
                                                           subs(&in.operand()), subs(&in.pos0()),
                                                           subs(&in.pos1()), in.align(), in.loc());
}
auto inst_cloner::operator()(expand_inst &in) -> std::unique_ptr<tinytc_inst> {
    return std::make_unique<expand_inst>(
        subs(&in.operand()), in.expanded_mode(), in.static_expand_shape(),
        subs_value_range(in.expand_shape()), in.result(0).ty(), in.loc());
}
auto inst_cloner::operator()(fuse_inst &in) -> std::unique_ptr<tinytc_inst> {
    return std::make_unique<fuse_inst>(subs(&in.operand()), in.from(), in.to(), in.result(0).ty(),
                                       in.loc());
}

auto inst_cloner::operator()(lifetime_stop_inst &in) -> std::unique_ptr<tinytc_inst> {
    return std::make_unique<lifetime_stop_inst>(subs(&in.object()), in.loc());
}
auto inst_cloner::operator()(load_inst &in) -> std::unique_ptr<tinytc_inst> {
    return std::make_unique<load_inst>(in.flag(), subs(&in.operand()),
                                       subs_value_range(in.index_list()), in.align(),
                                       in.result(0).ty(), in.loc());
}
auto inst_cloner::operator()(gemm_inst &in) -> std::unique_ptr<tinytc_inst> {
    return std::make_unique<gemm_inst>(in.tA(), in.tB(), subs(&in.alpha()), subs(&in.A()),
                                       subs(&in.B()), subs(&in.beta()), subs(&in.C()), in.atomic(),
                                       in.loc());
}

auto inst_cloner::operator()(gemv_inst &in) -> std::unique_ptr<tinytc_inst> {
    return std::make_unique<gemv_inst>(in.tA(), subs(&in.alpha()), subs(&in.A()), subs(&in.B()),
                                       subs(&in.beta()), subs(&in.C()), in.atomic(), in.loc());
}

auto inst_cloner::operator()(ger_inst &in) -> std::unique_ptr<tinytc_inst> {
    return std::make_unique<ger_inst>(subs(&in.alpha()), subs(&in.A()), subs(&in.B()),
                                      subs(&in.beta()), subs(&in.C()), in.atomic(), in.loc());
}
auto inst_cloner::operator()(for_inst &in) -> std::unique_ptr<tinytc_inst> {
    auto return_types = std::vector<tinytc_data_type_t>(in.num_results());
    for (std::int64_t i = 0; i < in.num_results(); ++i) {
        return_types[i] = in.result(0).ty();
    }
    return std::make_unique<for_inst>(in.body().param(0).ty(), subs(&in.from()), subs(&in.to()),
                                      in.has_step() ? subs(&in.step()) : nullptr,
                                      subs_value_range(in.iter_init()), return_types, in.loc());
}

auto inst_cloner::operator()(foreach_inst &in) -> std::unique_ptr<tinytc_inst> {
    return std::make_unique<foreach_inst>(in.body().param(0).ty(), subs_value_range(in.from()),
                                          subs_value_range(in.to()), in.loc());
}

auto inst_cloner::operator()(hadamard_inst &in) -> std::unique_ptr<tinytc_inst> {
    return std::make_unique<hadamard_inst>(subs(&in.alpha()), subs(&in.A()), subs(&in.B()),
                                           subs(&in.beta()), subs(&in.C()), in.atomic(), in.loc());
}

auto inst_cloner::operator()(if_inst &in) -> std::unique_ptr<tinytc_inst> {
    auto return_types = std::vector<tinytc_data_type_t>(in.num_results());
    for (std::int64_t i = 0; i < in.num_results(); ++i) {
        return_types[i] = in.result(i).ty();
    }
    return std::make_unique<if_inst>(subs(&in.condition()), return_types, in.loc());
}

auto inst_cloner::operator()(parallel_inst &in) -> std::unique_ptr<tinytc_inst> {
    return std::make_unique<parallel_inst>(in.loc());
}

auto inst_cloner::operator()(size_inst &in) -> std::unique_ptr<tinytc_inst> {
    return std::make_unique<size_inst>(subs(&in.operand()), in.mode(), in.result(0).ty(), in.loc());
}

auto inst_cloner::operator()(subgroup_broadcast_inst &in) -> std::unique_ptr<tinytc_inst> {
    return std::make_unique<subgroup_broadcast_inst>(subs(&in.a()), subs(&in.idx()),
                                                     in.result(0).ty(), in.loc());
}

auto inst_cloner::operator()(subview_inst &in) -> std::unique_ptr<tinytc_inst> {
    return std::make_unique<subview_inst>(
        subs(&in.operand()), in.static_offsets(), in.static_sizes(), subs_value_range(in.offsets()),
        subs_value_range(in.sizes()), in.result(0).ty(), in.loc());
}

auto inst_cloner::operator()(store_inst &in) -> std::unique_ptr<tinytc_inst> {
    return std::make_unique<store_inst>(in.flag(), subs(&in.val()), subs(&in.operand()),
                                        subs_value_range(in.index_list()), in.align(), in.loc());
}

auto inst_cloner::operator()(sum_inst &in) -> std::unique_ptr<tinytc_inst> {
    return std::make_unique<sum_inst>(in.tA(), subs(&in.alpha()), subs(&in.A()), subs(&in.beta()),
                                      subs(&in.B()), in.atomic(), in.loc());
}

auto inst_cloner::operator()(work_group_inst &in) -> std::unique_ptr<tinytc_inst> {
    return std::make_unique<work_group_inst>(in.operation(), subs(&in.operand()), in.result(0).ty(),
                                             in.loc());
}

auto inst_cloner::operator()(yield_inst &in) -> std::unique_ptr<tinytc_inst> {
    return std::make_unique<yield_inst>(subs_value_range(std::views::all(in.operands())), in.loc());
}

auto inst_cloner::clone_instruction(inst_node &in) -> std::unique_ptr<tinytc_inst> {
    auto cloned = visit(*this, in);
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
