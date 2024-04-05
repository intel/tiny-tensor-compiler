// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef INSERT_BARRIER_20230310_HPP
#define INSERT_BARRIER_20230310_HPP

#include "tinytc/export.hpp"
#include "tinytc/ir/internal/data_type_node.hpp"
#include "tinytc/ir/internal/function_node.hpp"
#include "tinytc/ir/internal/inst_node.hpp"
#include "tinytc/ir/internal/program_node.hpp"
#include "tinytc/ir/internal/region_node.hpp"
#include "tinytc/ir/internal/value_node.hpp"
#include "tinytc/ir/visitor/aa_results.hpp"

#include <cstdint>
#include <unordered_set>

namespace tinytc::ir::internal {

class TINYTC_EXPORT insert_barrier {
  public:
    /* Data type nodes */
    bool operator()(void_data_type &);
    bool operator()(group_data_type &b);
    bool operator()(memref_data_type &m);
    bool operator()(scalar_data_type &s);

    /* Value nodes */
    std::uintptr_t operator()(int_imm &v);
    std::uintptr_t operator()(float_imm &v);
    std::uintptr_t operator()(val &v);

    /* Stmt nodes */
    std::unordered_set<std::uintptr_t> operator()(blas_a2_inst &inst);
    std::unordered_set<std::uintptr_t> operator()(blas_a3_inst &inst);
    std::unordered_set<std::uintptr_t> operator()(loop_inst &p);
    std::unordered_set<std::uintptr_t> operator()(scalar_inst &inst);
    std::unordered_set<std::uintptr_t> operator()(alloca_inst &a);
    std::unordered_set<std::uintptr_t> operator()(barrier_inst &b);
    std::unordered_set<std::uintptr_t> operator()(expand_inst &e);
    std::unordered_set<std::uintptr_t> operator()(fuse_inst &f);
    std::unordered_set<std::uintptr_t> operator()(load_inst &e);
    std::unordered_set<std::uintptr_t> operator()(if_inst &in);
    std::unordered_set<std::uintptr_t> operator()(lifetime_stop_inst &);
    std::unordered_set<std::uintptr_t> operator()(size_inst &s);
    std::unordered_set<std::uintptr_t> operator()(store_inst &s);
    std::unordered_set<std::uintptr_t> operator()(subview_inst &s);
    std::unordered_set<std::uintptr_t> operator()(yield_inst &y);

    /* Region nodes */
    std::unordered_set<std::uintptr_t> operator()(rgn &b);

    /* Func nodes */
    void operator()(prototype &p);
    void operator()(function &fn);

    /* Program nodes */
    void operator()(program &p);

  private:
    aa_results aa_;
    bool last_instruction_was_barrier_ = false;
};

} // namespace tinytc::ir::internal

#endif // INSERT_BARRIER_20230310_HPP
