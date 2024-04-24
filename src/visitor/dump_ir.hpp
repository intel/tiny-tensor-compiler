// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DUMP_IR_20230330_HPP
#define DUMP_IR_20230330_HPP

#include "node/data_type_node.hpp"
#include "node/function_node.hpp"
#include "node/inst_node.hpp"
#include "node/program_node.hpp"
#include "node/region_node.hpp"
#include "node/value_node.hpp"
#include "visitor/slot_tracker.hpp"

#include <ostream>
#include <string>

namespace tinytc {

class ir_dumper {
  public:
    ir_dumper(std::ostream &os);

    /* Data type nodes */
    void operator()(void_data_type const &);
    void operator()(group_data_type const &g);
    void operator()(memref_data_type const &m);
    void operator()(scalar_data_type const &s);

    /* Var nodes */
    void operator()(float_imm const &v);
    void operator()(int_imm const &v);
    void operator()(val const &v);

    /* Inst nodes */
    void operator()(alloca_inst const &a);
    void operator()(axpby_inst const &a);
    void operator()(barrier_inst const &b);
    void operator()(binary_op_inst const &b);
    void operator()(cast_inst const &c);
    void operator()(compare_inst const &c);
    void operator()(expand_inst const &e);
    void operator()(fuse_inst const &f);
    void operator()(load_inst const &e);
    void operator()(group_id_inst const &g);
    void operator()(group_size_inst const &g);
    void operator()(lifetime_stop_inst const &l);
    void operator()(gemm_inst const &g);
    void operator()(gemv_inst const &g);
    void operator()(ger_inst const &g);
    void operator()(for_inst const &p);
    void operator()(foreach_inst const &p);
    void operator()(hadamard_inst const &g);
    void operator()(if_inst const &in);
    void operator()(neg_inst const &n);
    void operator()(size_inst const &s);
    void operator()(subview_inst const &s);
    void operator()(store_inst const &s);
    void operator()(sum_inst const &s);
    void operator()(yield_inst const &y);

    /* Region nodes */
    void operator()(rgn const &b);

    /* Func nodes */
    void operator()(prototype const &p);
    void operator()(function const &fn);

    /* Program nodes */
    void operator()(program const &p);

  private:
    void dump_blas_a2(blas_a2_inst const &g);
    void dump_blas_a3(blas_a3_inst const &g);

    template <typename Iterator, typename Action>
    void do_with_infix(Iterator begin, Iterator end, Action a, std::string const &infix = ",") {
        for (auto it = begin; it != end; ++it) {
            if (it != begin) {
                os_ << infix;
            }
            a(*it);
        }
    }
    inline auto indent() { return std::string(2 * lvl_, ' '); }
    std::ostream &os_;
    int lvl_ = 0;

    slot_tracker tracker_;
};

} // namespace tinytc

#endif // DUMP_IR_20230330_HPP
