// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "compiler_context.hpp"
#include "device_info.hpp"
#include "node/inst_node.hpp"
#include "spv/module.hpp"
#include "spv/uniquifier.hpp"
#include "support/casting.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <memory>
#include <stack>
#include <unordered_map>
#include <vector>

namespace tinytc::spv {

class spv_inst;

auto convert_prog_to_spirv(tinytc_prog const &p,
                           tinytc_core_info const &info) -> std::unique_ptr<mod>;

class inst_converter {
  public:
    inst_converter(tinytc_compiler_context_t ctx, mod &m);

    // Instruction nodes
    void operator()(inst_node const &in);
    void operator()(arith_inst const &in);
    void operator()(arith_unary_inst const &in);
    void operator()(barrier_inst const &in);
    void operator()(cast_inst const &in);
    void operator()(compare_inst const &in);
    void operator()(constant_inst const &in);
    void operator()(for_inst const &in);
    void operator()(group_id_inst const &in);
    void operator()(group_size_inst const &in);
    void operator()(if_inst const &in);
    void operator()(num_subgroups_inst const &in);
    void operator()(parallel_inst const &in);
    void operator()(subgroup_id_inst const &in);
    void operator()(subgroup_local_id_inst const &in);
    void operator()(subgroup_size_inst const &in);
    void operator()(work_group_inst const &in);
    void operator()(yield_inst const &in);

    void run_on_region(tinytc_region const &reg);
    auto run_on_region_with_yield(region_node const &reg,
                                  std::int64_t num_results) -> std::vector<spv_inst *>;
    void run_on_function(tinytc_func const &fn, core_config const &core_cfg);

    inline auto unique() -> uniquifier & { return unique_; }

  private:
    template <typename Iterator>
    auto num_yielded_vals(Iterator begin, Iterator end) -> std::int64_t {
        std::int64_t num_results = 0;
        for (; begin != end; ++begin) {
            if (auto ct = dyn_cast<coopmatrix_data_type>(begin->ty()); ct) {
                num_results += ct->length(core_cfg_.subgroup_size);
            } else {
                ++num_results;
            }
        }
        return num_results;
    }
    auto get_last_label() -> spv_inst *;
    auto get_scalar_type(tinytc_value const &v) -> scalar_type;
    auto get_coopmatrix_type(tinytc_value const &v) -> scalar_type;
    auto load_builtin(BuiltIn b) -> spv_inst *;
    auto declare(tinytc_value const &v, spv_inst *in);
    auto val(tinytc_value const &v) -> spv_inst *;
    auto multi_declare(tinytc_value const &v, std::vector<spv_inst *> insts);
    auto multi_val(tinytc_value const &v) -> std::vector<spv_inst *> &;
    auto make_constant(scalar_type sty, spv_inst *spv_ty,
                       constant_inst::value_type const &val) -> spv_inst *;

    tinytc_compiler_context_t ctx_;
    mod *mod_;
    uniquifier unique_;
    std::unordered_map<const_tinytc_value_t, spv_inst *> vals_;
    std::unordered_map<const_tinytc_value_t, std::vector<spv_inst *>> multi_vals_;
    std::stack<std::vector<spv_inst *>> yielded_vals_;
    std::vector<spv_inst *> builtins_used_by_function_;
    core_config core_cfg_ = {};
};

template <typename T>
concept spv_inst_with_required_capabilities = requires() { T::required_capabilities; };

} // namespace tinytc::spv
