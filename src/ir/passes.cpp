// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/ir/passes.hpp"
#include "tinytc/device_info.hpp"
#include "tinytc/ir/data_type.hpp"
#include "tinytc/ir/func.hpp"
#include "tinytc/ir/prog.hpp"
#include "tinytc/ir/visitor/alias_analysis.hpp"
#include "tinytc/ir/visitor/check_ir.hpp"
#include "tinytc/ir/visitor/dump_ir.hpp"
#include "tinytc/ir/visitor/equal.hpp"
#include "tinytc/ir/visitor/insert_barrier.hpp"
#include "tinytc/ir/visitor/lifetime_analysis.hpp"
#include "tinytc/ir/visitor/opencl_ast.hpp"
#include "tinytc/ir/visitor/stack.hpp"
#include "tinytc/ir/visitor/work_group_size.hpp"

#include <clir/handle.hpp>
#include <clir/visit.hpp>

#include <utility>

using clir::visit;

namespace tinytc::ir {

internal::aa_results alias_analysis(func f) {
    auto aa = internal::alias_analyser{};
    visit(aa, *f);
    return aa.get_result();
}

bool check_ir(prog p, error_reporter_function reporter) {
    return visit(internal::ir_checker{std::move(reporter)}, *p);
}

void dump_ir(std::ostream &os, func f) { visit(internal::ir_dumper{os}, *f); }
void dump_ir(std::ostream &os, prog p) { visit(internal::ir_dumper{os}, *p); }

clir::prog generate_opencl_ast(prog p, std::shared_ptr<core_info> info) {
    return visit(internal::opencl_ast{std::move(info)}, *p);
}

void insert_barriers(func f) { visit(internal::insert_barrier{}, *f); }
void insert_barriers(prog p) { visit(internal::insert_barrier{}, *p); }

void insert_lifetime_stop_inst(func f) { visit(internal::lifetime_inserter{}, *f); }
void insert_lifetime_stop_inst(prog p) { visit(internal::lifetime_inserter{}, *p); }

bool is_equal(data_type a, data_type b) { return visit(internal::equal{}, *a, *b); }

void set_stack_ptrs(func f) { visit(internal::stack_ptr{}, *f); }
void set_stack_ptrs(prog p) { visit(internal::stack_ptr{}, *p); }

void set_work_group_size(func f, std::shared_ptr<core_info> info) {
    visit(internal::work_group_size{std::move(info)}, *f);
}
void set_work_group_size(prog p, std::shared_ptr<core_info> info) {
    visit(internal::work_group_size{std::move(info)}, *p);
}

} // namespace tinytc::ir

