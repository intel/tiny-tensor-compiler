// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CONVERTER_20241111_HPP
#define CONVERTER_20241111_HPP

#include "device_info.hpp"
#include "node/inst_node.hpp"
#include "node/region_node.hpp"
#include "spv/coopmatrix_diy.hpp"
#include "spv/defs.hpp"
#include "spv/dope_vector.hpp"
#include "spv/uniquifier.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <cstdint>
#include <functional>
#include <stack>
#include <unordered_map>
#include <vector>

namespace tinytc::spv {

enum class BuiltIn;

auto convert_prog_to_spirv(tinytc_prog const &p, tinytc_core_info const &info) -> ::tinytc::spv_mod;

class inst_converter {
  public:
    inst_converter(tinytc_spv_mod &m, tinytc_core_info const &info);

    // Instruction nodes
    void operator()(inst_node const &in);
    void operator()(alloca_inst const &in);
    void operator()(arith_inst const &in);
    void operator()(arith_unary_inst const &in);
    void operator()(barrier_inst const &in);
    void operator()(builtin_inst const &in);
    void operator()(cast_inst const &in);
    void operator()(compare_inst const &in);
    void operator()(constant_inst const &in);
    void operator()(cooperative_matrix_load_inst const &in);
    void operator()(cooperative_matrix_mul_add_inst const &in);
    void operator()(cooperative_matrix_scale_inst const &in);
    void operator()(cooperative_matrix_store_inst const &in);
    void operator()(expand_inst const &in);
    void operator()(for_inst const &in);
    void operator()(fuse_inst const &in);
    void operator()(if_inst const &in);
    void operator()(lifetime_stop_inst const &in);
    void operator()(load_inst const &in);
    void operator()(parallel_inst const &in);
    void operator()(size_inst const &in);
    void operator()(subgroup_broadcast_inst const &in);
    void operator()(store_inst const &in);
    void operator()(subview_inst const &in);
    void operator()(work_group_inst const &in);
    void operator()(yield_inst const &in);

    void run_on_region(tinytc_region const &reg);
    auto run_on_region_with_yield(region_node const &reg, std::int64_t num_results)
        -> std::vector<spv_inst *>;
    void run_on_function(tinytc_func const &fn);

    inline auto unique() -> uniquifier & { return unique_; }

  private:
    auto get_last_label() -> spv_inst *;
    auto get_dope_vector(tinytc_value const &v) -> dope_vector *;
    auto load_builtin(BuiltIn b) -> spv_inst *;
    auto declare(tinytc_value const &v, spv_inst *in);
    auto val(tinytc_value const &v) -> spv_inst *;
    auto make_binary_op(scalar_type sty, arithmetic op, spv_inst *ty, spv_inst *a, spv_inst *b,
                        location const &loc) -> spv_inst *;
    auto make_cast(scalar_type to_ty, scalar_type a_ty, spv_inst *spv_to_ty, spv_inst *a,
                   location const &loc) -> spv_inst *;
    auto make_complex_mul(spv_inst *ty, spv_inst *a, spv_inst *b, bool conj_b = false)
        -> spv_inst *;
    auto make_conditional_execution(spv_inst *returned_element_ty, spv_inst *condition,
                                    std::function<std::vector<spv_inst *>()> conditional_code,
                                    location const &loc) -> std::vector<spv_inst *>;
    auto make_constant(scalar_type sty, spv_inst *spv_ty, constant_inst::value_type const &val)
        -> spv_inst *;
    auto make_dope_vector(tinytc_value const &v) -> dope_vector *;
    void make_store(store_flag flag, scalar_type sty, address_space as, spv_inst *pointer,
                    spv_inst *value, location const &loc);

    tinytc_spv_mod_t mod_;
    tinytc_core_info const *info_;
    uniquifier unique_;
    coopmatrix_diy diy_;
    std::unordered_map<const_tinytc_value_t, dope_vector> dope_vec_;
    std::unordered_map<const_tinytc_value_t, spv_inst *> vals_;
    std::stack<std::vector<spv_inst *>> yielded_vals_;
    std::vector<spv_inst *> vars_used_by_function_;
    spv_inst *stack_ = nullptr;
    core_config core_cfg_ = {};
};

} // namespace tinytc::spv

#endif // CONVERTER_20241111_HPP
