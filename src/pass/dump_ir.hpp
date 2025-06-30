// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DUMP_IR_20230330_HPP
#define DUMP_IR_20230330_HPP

#include "node/attr.hpp" // IWYU pragma: keep
#include "node/inst_view.hpp"
#include "node/type.hpp"
#include "pass/slot_tracker.hpp"
#include "tinytc/types.h"

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
    void operator()(alloca_inst a);
    void operator()(axpby_inst a);
    void operator()(arith_inst a);
    void operator()(arith_unary_inst a);
    void operator()(barrier_inst b);
    void operator()(cast_inst c);
    void operator()(compare_inst c);
    void operator()(constant_inst c);
    void operator()(cooperative_matrix_apply_inst c);
    void operator()(cooperative_matrix_extract_inst c);
    void operator()(cooperative_matrix_insert_inst c);
    void operator()(cooperative_matrix_load_inst c);
    void operator()(cooperative_matrix_mul_add_inst c);
    void operator()(cooperative_matrix_prefetch_inst c);
    void operator()(cooperative_matrix_reduce_inst c);
    void operator()(cooperative_matrix_scale_inst c);
    void operator()(cooperative_matrix_store_inst c);
    void operator()(cumsum_inst a);
    void operator()(expand_inst e);
    void operator()(fuse_inst f);
    void operator()(load_inst e);
    void operator()(lifetime_stop_inst l);
    void operator()(gemm_inst g);
    void operator()(gemv_inst g);
    void operator()(ger_inst g);
    void operator()(for_inst p);
    void operator()(foreach_inst p);
    void operator()(hadamard_inst g);
    void operator()(if_inst in);
    void operator()(math_unary_inst in);
    void operator()(parallel_inst p);
    void operator()(size_inst s);
    void operator()(subgroup_broadcast_inst in);
    void operator()(subgroup_operation_inst in);
    void operator()(subview_inst s);
    void operator()(store_inst s);
    void operator()(sum_inst s);
    void operator()(yield_inst y);
    void operator()(group_id_inst in);
    void operator()(num_groups_inst in);
    void operator()(num_subgroups_inst in);
    void operator()(subgroup_size_inst in);
    void operator()(subgroup_id_inst in);
    void operator()(subgroup_linear_id_inst in);
    void operator()(subgroup_local_id_inst in);

    void run_on_function(tinytc_func &fn);
    void run_on_region(tinytc_region &reg);
    void run_on_instruction(tinytc_inst &in);

    void dump_val(tinytc_value const &v);
    void init_slot_tracker(tinytc_func &fn);

  private:
    void dump_region(tinytc_region &reg);
    void dump_blas_a2(blas_a2_inst g);
    void dump_blas_a3(blas_a3_inst g);

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
