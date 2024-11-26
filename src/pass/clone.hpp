// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CLONE_20241118_HPP
#define CLONE_20241118_HPP

#include "node/inst_node.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "tinytc/types.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace tinytc {

class inst_cloner {
  public:
    auto operator()(alloca_inst &in) -> std::unique_ptr<tinytc_inst>;
    auto operator()(axpby_inst &in) -> std::unique_ptr<tinytc_inst>;
    auto operator()(arith_inst &in) -> std::unique_ptr<tinytc_inst>;
    auto operator()(arith_unary_inst &in) -> std::unique_ptr<tinytc_inst>;
    auto operator()(barrier_inst &in) -> std::unique_ptr<tinytc_inst>;
    auto operator()(builtin_inst &in) -> std::unique_ptr<tinytc_inst>;
    auto operator()(cast_inst &in) -> std::unique_ptr<tinytc_inst>;
    auto operator()(compare_inst &in) -> std::unique_ptr<tinytc_inst>;
    auto operator()(constant_inst &in) -> std::unique_ptr<tinytc_inst>;
    auto operator()(cooperative_matrix_load_inst &in) -> std::unique_ptr<tinytc_inst>;
    auto operator()(cooperative_matrix_mul_add_inst &in) -> std::unique_ptr<tinytc_inst>;
    auto operator()(cooperative_matrix_scale_inst &in) -> std::unique_ptr<tinytc_inst>;
    auto operator()(cooperative_matrix_store_inst &in) -> std::unique_ptr<tinytc_inst>;
    auto operator()(expand_inst &in) -> std::unique_ptr<tinytc_inst>;
    auto operator()(fuse_inst &in) -> std::unique_ptr<tinytc_inst>;
    auto operator()(load_inst &in) -> std::unique_ptr<tinytc_inst>;
    auto operator()(lifetime_stop_inst &in) -> std::unique_ptr<tinytc_inst>;
    auto operator()(gemm_inst &in) -> std::unique_ptr<tinytc_inst>;
    auto operator()(gemv_inst &in) -> std::unique_ptr<tinytc_inst>;
    auto operator()(ger_inst &in) -> std::unique_ptr<tinytc_inst>;
    auto operator()(for_inst &in) -> std::unique_ptr<tinytc_inst>;
    auto operator()(foreach_inst &in) -> std::unique_ptr<tinytc_inst>;
    auto operator()(hadamard_inst &in) -> std::unique_ptr<tinytc_inst>;
    auto operator()(if_inst &in) -> std::unique_ptr<tinytc_inst>;
    auto operator()(parallel_inst &in) -> std::unique_ptr<tinytc_inst>;
    auto operator()(size_inst &in) -> std::unique_ptr<tinytc_inst>;
    auto operator()(subview_inst &in) -> std::unique_ptr<tinytc_inst>;
    auto operator()(store_inst &in) -> std::unique_ptr<tinytc_inst>;
    auto operator()(sum_inst &in) -> std::unique_ptr<tinytc_inst>;
    auto operator()(work_group_inst &in) -> std::unique_ptr<tinytc_inst>;
    auto operator()(yield_inst &in) -> std::unique_ptr<tinytc_inst>;

    void reset_subs();
    void set_subs(tinytc_value_t in_val, tinytc_value_t out_val);
    auto subs(tinytc_value_t val) -> tinytc_value_t;

    auto clone_instruction(inst_node &in) -> std::unique_ptr<tinytc_inst>;
    void clone_region(region_node &source, region_node &target);

  private:
    template <typename T> auto subs_value_range(T &&range) {
        auto vec = std::vector<tinytc_value_t>();
        vec.reserve(range.size());
        for (auto &r : range) {
            vec.emplace_back(subs(&r));
        }
        return vec;
    }

    std::unordered_map<tinytc_value_t, tinytc_value_t> subs_map_;
};

} // namespace tinytc

#endif // CLONE_20241118_HPP
