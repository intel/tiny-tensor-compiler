// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DUMP_IR_20230330_HPP
#define DUMP_IR_20230330_HPP

#include "node/attr_node.hpp" // IWYU pragma: keep
#include "node/data_type_node.hpp"
#include "node/function_node.hpp"
#include "node/inst_node.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "pass/slot_tracker.hpp"

#include <limits>
#include <ostream>
#include <string>

namespace tinytc {

class dump_ir_pass {
  public:
    dump_ir_pass(std::ostream &os, int level_limit = std::numeric_limits<int>::max());

    /* Attribute nodes */
    void operator()(array_attr const &a);
    void operator()(boolean_attr const &a);
    void operator()(dictionary_attr const &a);
    void operator()(integer_attr const &a);
    void operator()(string_attr const &a);

    /* Data type nodes */
    void operator()(void_data_type const &);
    void operator()(boolean_data_type const &);
    void operator()(coopmatrix_data_type const &ct);
    void operator()(group_data_type const &g);
    void operator()(memref_data_type const &m);
    void operator()(scalar_data_type const &s);

    /* Inst nodes */
    void operator()(alloca_inst const &a);
    void operator()(axpby_inst const &a);
    void operator()(arith_inst const &a);
    void operator()(arith_unary_inst const &a);
    void operator()(barrier_inst const &b);
    void operator()(builtin_inst const &in);
    void operator()(cast_inst const &c);
    void operator()(compare_inst const &c);
    void operator()(constant_inst const &c);
    void operator()(cooperative_matrix_apply_inst const &c);
    void operator()(cooperative_matrix_extract_inst const &c);
    void operator()(cooperative_matrix_insert_inst const &c);
    void operator()(cooperative_matrix_load_inst const &c);
    void operator()(cooperative_matrix_mul_add_inst const &c);
    void operator()(cooperative_matrix_prefetch_inst const &c);
    void operator()(cooperative_matrix_scale_inst const &c);
    void operator()(cooperative_matrix_store_inst const &c);
    void operator()(cumsum_inst const &a);
    void operator()(expand_inst const &e);
    void operator()(fuse_inst const &f);
    void operator()(load_inst const &e);
    void operator()(lifetime_stop_inst const &l);
    void operator()(gemm_inst const &g);
    void operator()(gemv_inst const &g);
    void operator()(ger_inst const &g);
    void operator()(for_inst const &p);
    void operator()(foreach_inst const &p);
    void operator()(hadamard_inst const &g);
    void operator()(if_inst const &in);
    void operator()(math_unary_inst const &in);
    void operator()(parallel_inst const &p);
    void operator()(size_inst const &s);
    void operator()(subgroup_add_inst const &in);
    void operator()(subgroup_broadcast_inst const &in);
    void operator()(subgroup_max_inst const &in);
    void operator()(subgroup_min_inst const &in);
    void operator()(subview_inst const &s);
    void operator()(store_inst const &s);
    void operator()(sum_inst const &s);
    void operator()(yield_inst const &y);

    void run_on_function(function_node const &fn);
    void run_on_region(region_node const &reg);
    void run_on_instruction(inst_node const &in);

    void dump_val(value_node const &v);
    void init_slot_tracker(function_node const &fn);

  private:
    void dump_region(region_node const &reg);
    void dump_blas_a2(blas_a2_inst const &g);
    void dump_blas_a3(blas_a3_inst const &g);

    template <typename Iterator, typename Action>
    void do_with_infix(Iterator begin, Iterator end, Action a, std::string const &infix = ",") {
        for (auto it = begin; it != end; ++it) {
            if (it != begin) {
                *os_ << infix;
            }
            a(*it);
        }
    }
    template <typename Iterator, typename Action>
    void do_with_infix_enumerated(Iterator begin, Iterator end, Action a,
                                  std::string const &infix = ",") {
        for (auto it = begin; it != end; ++it) {
            if (it != begin) {
                *os_ << infix;
            }
            a(it - begin, *it);
        }
    }

    inline auto indent() { return std::string(2 * lvl_, ' '); }
    std::ostream *os_;
    int lvl_limit_;
    int lvl_ = 0;

    slot_tracker tracker_;
};

} // namespace tinytc

#endif // DUMP_IR_20230330_HPP
