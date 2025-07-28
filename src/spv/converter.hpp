// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CONVERTER_20241111_HPP
#define CONVERTER_20241111_HPP

#include "device_info.hpp"
#include "node/inst_view.hpp"
#include "spv/coopmatrix_impl.hpp"
#include "spv/defs.hpp"
#include "spv/dope_vector.hpp"
#include "spv/uniquifier.hpp"
#include "tiling.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <cstdint>
#include <memory>
#include <stack>
#include <unordered_map>
#include <vector>

namespace tinytc::spv {

auto convert_prog_to_spirv(tinytc_prog &p, tinytc_core_info const &info)
    -> shared_handle<tinytc_spv_mod_t>;

class inst_converter {
  public:
    inst_converter(tinytc_spv_mod &m, tinytc_core_info const &info);

    // Instruction nodes
    void operator()(inst_view in);
    void operator()(alloca_inst in);
    void operator()(arith_inst in);
    void operator()(arith_unary_inst in);
    void operator()(atomic_load_inst in);
    void operator()(atomic_store_inst in);
    void operator()(atomic_update_inst in);
    void operator()(barrier_inst in);
    void operator()(cast_inst in);
    void operator()(compare_inst in);
    void operator()(constant_inst in);
    void operator()(cooperative_matrix_atomic_load_inst in);
    void operator()(cooperative_matrix_atomic_store_inst in);
    void operator()(cooperative_matrix_atomic_update_inst in);
    void operator()(cooperative_matrix_construct_inst in);
    void operator()(cooperative_matrix_extract_inst in);
    void operator()(cooperative_matrix_insert_inst in);
    void operator()(cooperative_matrix_load_inst in);
    void operator()(cooperative_matrix_mul_add_inst in);
    void operator()(cooperative_matrix_prefetch_inst in);
    void operator()(cooperative_matrix_reduce_inst in);
    void operator()(cooperative_matrix_scale_inst in);
    void operator()(cooperative_matrix_store_inst in);
    void operator()(expand_inst in);
    void operator()(for_inst in);
    void operator()(fuse_inst in);
    void operator()(if_inst in);
    void operator()(lifetime_stop_inst in);
    void operator()(load_inst in);
    void operator()(math_unary_inst in);
    void operator()(parallel_inst in);
    void operator()(size_inst in);
    void operator()(subgroup_broadcast_inst in);
    void operator()(subgroup_operation_inst in);
    void operator()(store_inst in);
    void operator()(subview_inst in);
    void operator()(yield_inst in);
    void operator()(group_id_inst in);
    void operator()(num_groups_inst in);
    void operator()(num_subgroups_inst in);
    void operator()(subgroup_size_inst in);
    void operator()(subgroup_id_inst in);
    void operator()(subgroup_linear_id_inst in);
    void operator()(subgroup_local_id_inst in);

    void run_on_region(tinytc_region &reg);
    auto run_on_region_with_yield(tinytc_region &reg, std::int64_t num_results)
        -> std::vector<spv_inst *>;
    void run_on_function(tinytc_func &fn);

    inline auto unique() -> uniquifier & { return unique_; }

  private:
    auto get_dope_vector(tinytc_value const &v) -> dope_vector *;
    auto declare(tinytc_value const &v, spv_inst *in);
    auto val(tinytc_value const &v) -> spv_inst *;
    auto spv_ty(tinytc_type_t ty) -> spv_inst *;
    auto make_dope_vector(tinytc_value const &v) -> dope_vector *;
    auto matrix_impl() -> coopmatrix_impl &;
    auto make_matrix_impl() -> std::unique_ptr<coopmatrix_impl>;
    template <typename MemoryReadWriteInst>
    auto get_pointer_helper(MemoryReadWriteInst in) -> spv_inst *;
    auto get_pointer(memory_read_inst in) -> spv_inst *;
    auto get_pointer(memory_write_inst in) -> spv_inst *;

    tinytc_spv_mod_t mod_;
    tinytc_core_info const *info_;
    uniquifier unique_;
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
