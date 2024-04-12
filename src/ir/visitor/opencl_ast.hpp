// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef OPENCL_AST_20230309_HPP
#define OPENCL_AST_20230309_HPP

#include "device_info.hpp"
#include "ir/node/data_type_node.hpp"
#include "ir/node/function_node.hpp"
#include "ir/node/inst_node.hpp"
#include "ir/node/program_node.hpp"
#include "ir/node/region_node.hpp"
#include "ir/node/value_node.hpp"
#include "tiling.hpp"

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
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace tinytc {

enum class scalar_type;

class dope_vector {
  public:
    enum class type { shape, stride };
    using decl_fun_t = std::function<void(clir::data_type, clir::var, type, std::int64_t)>;
    static dope_vector from_value(value_node &v, decl_fun_t declare);

    inline dope_vector() {}
    inline dope_vector(std::vector<clir::expr> shape, std::vector<clir::expr> stride)
        : shape_(std::move(shape)), stride_(std::move(stride)) {}

    inline auto shape(std::int64_t i) { return shape_[i]; }
    inline auto stride(std::int64_t i) { return stride_[i]; }

  private:
    static dope_vector from_memref_type(std::string const &prefix, memref_data_type &m,
                                        clir::data_type dt, decl_fun_t declare);
    std::vector<clir::expr> shape_, stride_;
};

class opencl_ast {
  public:
    opencl_ast(core_info const *info);

    /* Data type nodes */
    clir::data_type operator()(void_data_type &);
    clir::data_type operator()(group_data_type &g);
    clir::data_type operator()(memref_data_type &m);
    clir::data_type operator()(scalar_data_type &s);

    /* Var nodes */
    clir::expr operator()(float_imm &v);
    clir::expr operator()(int_imm &v);
    clir::expr operator()(val &v);

    /* Inst nodes */
    std::vector<clir::stmt> operator()(alloca_inst &a);
    std::vector<clir::stmt> operator()(axpby_inst &a);
    std::vector<clir::stmt> operator()(barrier_inst &b);
    std::vector<clir::stmt> operator()(binary_op_inst &b);
    std::vector<clir::stmt> operator()(cast_inst &c);
    std::vector<clir::stmt> operator()(compare_inst &c);
    std::vector<clir::stmt> operator()(expand_inst &e);
    std::vector<clir::stmt> operator()(fuse_inst &f);
    std::vector<clir::stmt> operator()(load_inst &e);
    std::vector<clir::stmt> operator()(group_id_inst &g);
    std::vector<clir::stmt> operator()(group_size_inst &g);
    std::vector<clir::stmt> operator()(lifetime_stop_inst &l);
    std::vector<clir::stmt> operator()(gemm_inst &g);
    std::vector<clir::stmt> operator()(gemv_inst &g);
    std::vector<clir::stmt> operator()(ger_inst &g);
    std::vector<clir::stmt> operator()(for_inst &p);
    std::vector<clir::stmt> operator()(foreach_inst &in);
    std::vector<clir::stmt> operator()(hadamard_inst &g);
    std::vector<clir::stmt> operator()(if_inst &in);
    std::vector<clir::stmt> operator()(neg_inst &n);
    std::vector<clir::stmt> operator()(size_inst &s);
    std::vector<clir::stmt> operator()(subview_inst &s);
    std::vector<clir::stmt> operator()(store_inst &s);
    std::vector<clir::stmt> operator()(sum_inst &s);
    std::vector<clir::stmt> operator()(yield_inst &in);

    /* Region nodes */
    clir::stmt operator()(rgn &b);

    /* Func nodes */
    clir::func operator()(prototype &p);
    clir::func operator()(function &fn);

    /* Program nodes */
    clir::prog operator()(program &p);

  private:
    auto get_dope_vector(value_node *v) -> dope_vector &;
    void set_dope_vector(value_node *v, dope_vector dv);
    clir::var declare(value_node &v);
    memref_data_type *get_memref_type(value_node &v);
    static scalar_type get_scalar_type(data_type_node &ty);

    core_info const *info_;
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

#endif // OPENCL_AST_20230309_HPP
