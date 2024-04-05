// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DUMP_IR_20230330_HPP
#define DUMP_IR_20230330_HPP

#include "tinytc/export.hpp"
#include "tinytc/ir/internal/data_type_node.hpp"
#include "tinytc/ir/internal/function_node.hpp"
#include "tinytc/ir/internal/inst_node.hpp"
#include "tinytc/ir/internal/program_node.hpp"
#include "tinytc/ir/internal/region_node.hpp"
#include "tinytc/ir/internal/value_node.hpp"

#include <ostream>
#include <string>

namespace tinytc::ir::internal {

class TINYTC_EXPORT ir_dumper {
  public:
    ir_dumper(std::ostream &os);

    /* Data type nodes */
    void operator()(void_data_type &);
    void operator()(group_data_type &g);
    void operator()(memref_data_type &m);
    void operator()(scalar_data_type &s);

    /* Var nodes */
    void operator()(float_imm &v);
    void operator()(int_imm &v);
    void operator()(val &v);

    /* Inst nodes */
    void operator()(alloca_inst &a);
    void operator()(axpby_inst &a);
    void operator()(barrier_inst &b);
    void operator()(binary_op_inst &b);
    void operator()(cast_inst &c);
    void operator()(compare_inst &c);
    void operator()(expand_inst &e);
    void operator()(fuse_inst &f);
    void operator()(load_inst &e);
    void operator()(group_id_inst &g);
    void operator()(group_size_inst &g);
    void operator()(lifetime_stop_inst &l);
    void operator()(gemm_inst &g);
    void operator()(gemv_inst &g);
    void operator()(ger_inst &g);
    void operator()(for_inst &p);
    void operator()(foreach_inst &p);
    void operator()(hadamard_inst &g);
    void operator()(if_inst &in);
    void operator()(neg_inst &n);
    void operator()(size_inst &s);
    void operator()(subview_inst &s);
    void operator()(store_inst &s);
    void operator()(sum_inst &s);
    void operator()(yield_inst &y);

    /* Region nodes */
    void operator()(rgn &b);

    /* Func nodes */
    void operator()(prototype &p);
    void operator()(function &fn);

    /* Program nodes */
    void operator()(program &p);

  private:
    void dump_blas_a2(blas_a2_inst &g);
    void dump_blas_a3(blas_a3_inst &g);

    template <typename Iterator, typename Action>
    TINYTC_NO_EXPORT void do_with_infix(Iterator begin, Iterator end, Action a,
                                        std::string const &infix = ",") {
        for (auto it = begin; it != end; ++it) {
            if (it != begin) {
                os_ << infix;
            }
            a(*it);
        }
    }
    inline TINYTC_NO_EXPORT auto indent() { return std::string(2 * lvl_, ' '); }
    std::ostream &os_;
    int lvl_ = 0;
};

} // namespace tinytc::ir::internal

#endif // DUMP_IR_20230330_HPP
