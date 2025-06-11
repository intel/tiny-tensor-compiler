// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CONVERTER_20241111_HPP
#define CONVERTER_20241111_HPP

#include "device_info.hpp"
#include "node/inst_node.hpp"
#include "node/region_node.hpp"
#include "spv/coopmatrix_impl.hpp"
#include "spv/defs.hpp"
#include "spv/dope_vector.hpp"
#include "spv/uniquifier.hpp"
#include "tiling.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"

#include <cstdint>
#include <memory>
#include <stack>
#include <unordered_map>
#include <vector>

namespace tinytc::spv {

auto convert_prog_to_spirv(tinytc_prog const &p, tinytc_core_info const &info) -> ::tinytc::spv_mod;

class inst_converter {
  public:
    inst_converter(tinytc_spv_mod &m, tinytc_core_info const &info);

    void add_memory_model();
    void add_debug_info(tinytc_compiler_context_t ctx, location const &loc);

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
    void operator()(cooperative_matrix_extract_inst const &in);
    void operator()(cooperative_matrix_insert_inst const &in);
    void operator()(cooperative_matrix_load_inst const &in);
    void operator()(cooperative_matrix_mul_add_inst const &in);
    void operator()(cooperative_matrix_prefetch_inst const &in);
    void operator()(cooperative_matrix_reduce_inst const &in);
    void operator()(cooperative_matrix_scale_inst const &in);
    void operator()(cooperative_matrix_store_inst const &in);
    void operator()(expand_inst const &in);
    void operator()(for_inst const &in);
    void operator()(fuse_inst const &in);
    void operator()(if_inst const &in);
    void operator()(lifetime_stop_inst const &in);
    void operator()(load_inst const &in);
    void operator()(math_unary_inst const &in);
    void operator()(parallel_inst const &in);
    void operator()(size_inst const &in);
    void operator()(subgroup_broadcast_inst const &in);
    void operator()(subgroup_operation_inst const &in);
    void operator()(store_inst const &in);
    void operator()(subview_inst const &in);
    void operator()(yield_inst const &in);

    void run_on_region(tinytc_region const &reg);
    auto run_on_region_with_yield(region_node const &reg, std::int64_t num_results)
        -> std::vector<spv_inst *>;
    void run_on_function(tinytc_func const &fn);

    inline auto unique() -> uniquifier & { return unique_; }

  private:
    auto get_dope_vector(tinytc_value const &v) -> dope_vector *;
    auto declare(tinytc_value const &v, spv_inst *in);
    auto val(tinytc_value const &v) -> spv_inst *;
    auto spv_ty(const_tinytc_data_type_t ty) -> spv_inst *;
    auto make_dope_vector(tinytc_value const &v) -> dope_vector *;
    auto matrix_impl() -> coopmatrix_impl &;
    auto make_matrix_impl() -> std::unique_ptr<coopmatrix_impl>;

    tinytc_spv_mod_t mod_;
    tinytc_core_info const *info_;
    uniquifier unique_;
    spv_inst *debug_source_ = nullptr;
    spv_inst *compilation_unit_ = nullptr;
    std::unique_ptr<coopmatrix_impl> matrix_impl_ = nullptr;
    std::unordered_map<const_tinytc_value_t, dope_vector> dope_vec_;
    std::unordered_map<const_tinytc_value_t, spv_inst *> vals_;
    std::stack<std::vector<spv_inst *>> yielded_vals_;
    spv_inst *stack_ = nullptr;
    core_config core_cfg_ = {};
    local_tiling tiling_ = {};
};

} // namespace tinytc::spv

#endif // CONVERTER_20241111_HPP
