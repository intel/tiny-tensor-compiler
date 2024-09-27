// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CONVERT_TO_OPENCL_20240913_HPP
#define CONVERT_TO_OPENCL_20240913_HPP

#include "device_info.hpp"
#include "node/data_type_node.hpp"
#include "node/function_node.hpp"
#include "node/inst_node.hpp"
#include "node/program_node.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "tiling.hpp"
#include "tinytc/types.hpp"

#include <clir/builder.hpp>
#include <clir/data_type.hpp>
#include <clir/expr.hpp>
#include <clir/func.hpp>
#include <clir/prog.hpp>
#include <clir/stmt.hpp>
#include <clir/var.hpp>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace tinytc {

class dope_vector {
  public:
    enum class type { shape, stride, offset };
    using decl_fun_t = std::function<void(clir::data_type, clir::var, type, std::int64_t)>;
    static dope_vector from_value(value_node const &v, decl_fun_t declare);

    inline dope_vector() {}
    inline dope_vector(std::vector<clir::expr> shape, std::vector<clir::expr> stride)
        : shape_(std::move(shape)), stride_(std::move(stride)) {}

    inline auto shape(std::int64_t i) { return shape_[i]; }
    inline auto stride(std::int64_t i) { return stride_[i]; }
    inline auto offset() { return offset_; }
    inline auto offset(clir::expr offset) { offset_ = std::move(offset); }

  private:
    static dope_vector from_memref_type(std::string const &prefix, memref_data_type const &m,
                                        clir::data_type dt, decl_fun_t declare);
    std::vector<clir::expr> shape_, stride_;
    clir::expr offset_ = clir::expr(std::int64_t(0));
};

class convert_to_opencl_pass {
  public:
    convert_to_opencl_pass(::tinytc_core_info const *info);

    /* Data type nodes */
    clir::data_type operator()(void_data_type const &);
    clir::data_type operator()(group_data_type const &g);
    clir::data_type operator()(memref_data_type const &m);
    clir::data_type operator()(scalar_data_type const &s);

    /* Inst nodes */
    std::vector<clir::stmt> operator()(alloca_inst const &a);
    std::vector<clir::stmt> operator()(axpby_inst const &a);
    std::vector<clir::stmt> operator()(barrier_inst const &b);
    std::vector<clir::stmt> operator()(arith_inst const &a);
    std::vector<clir::stmt> operator()(arith_unary_inst const &a);
    std::vector<clir::stmt> operator()(cast_inst const &c);
    std::vector<clir::stmt> operator()(compare_inst const &c);
    std::vector<clir::stmt> operator()(constant_inst const &c);
    std::vector<clir::stmt> operator()(expand_inst const &e);
    std::vector<clir::stmt> operator()(fuse_inst const &f);
    std::vector<clir::stmt> operator()(load_inst const &e);
    std::vector<clir::stmt> operator()(group_id_inst const &g);
    std::vector<clir::stmt> operator()(group_size_inst const &g);
    std::vector<clir::stmt> operator()(lifetime_stop_inst const &l);
    std::vector<clir::stmt> operator()(gemm_inst const &g);
    std::vector<clir::stmt> operator()(gemv_inst const &g);
    std::vector<clir::stmt> operator()(ger_inst const &g);
    std::vector<clir::stmt> operator()(for_inst const &p);
    std::vector<clir::stmt> operator()(foreach_inst const &in);
    std::vector<clir::stmt> operator()(hadamard_inst const &g);
    std::vector<clir::stmt> operator()(if_inst const &in);
    std::vector<clir::stmt> operator()(num_subgroups_inst const &sg);
    std::vector<clir::stmt> operator()(parallel_inst const &p);
    std::vector<clir::stmt> operator()(size_inst const &s);
    std::vector<clir::stmt> operator()(subgroup_id_inst const &sg);
    std::vector<clir::stmt> operator()(subgroup_local_id_inst const &sg);
    std::vector<clir::stmt> operator()(subgroup_size_inst const &sg);
    std::vector<clir::stmt> operator()(subview_inst const &s);
    std::vector<clir::stmt> operator()(store_inst const &s);
    std::vector<clir::stmt> operator()(sum_inst const &s);
    std::vector<clir::stmt> operator()(yield_inst const &in);

    auto run_on_program(program_node const &p) -> clir::prog;

  private:
    auto run_on_region(region_node const &reg) -> clir::stmt;
    auto run_on_function(function_node const &fn) -> clir::func;
    auto val(value_node const &v) -> clir::expr;

    auto get_dope_vector(value_node *v) -> dope_vector &;
    void set_dope_vector(value_node *v, dope_vector dv);
    clir::var declare(value_node const &v);
    auto get_memref_type(value_node const &v) const -> const memref_data_type *;
    static auto get_scalar_type(value_node const &v) -> scalar_type;

    ::tinytc_core_info const *info_;
    clir::program_builder prog_builder_;
    std::vector<std::unordered_map<uintptr_t, clir::var>> declared_vars_;
    std::vector<std::vector<clir::var>> yielded_vars_;
    std::unordered_map<uintptr_t, dope_vector> dope_vector_;
    std::unordered_set<std::string> reserved_names_;
    std::unordered_set<std::string> has_gemm_;
    clir::var stack_;
    std::size_t stack_high_water_mark_ = 0;
    local_tiling tiling_ = {};
    core_config core_cfg_ = {};
};

} // namespace tinytc

#endif // CONVERT_TO_OPENCL_20240913_HPP
