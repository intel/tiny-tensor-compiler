// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/ir/passes.hpp"
#include "device_info.hpp"
#include "ir/visitor/check_ir.hpp"
#include "ir/visitor/dump_ir.hpp"
#include "ir/visitor/equal.hpp"
#include "ir/visitor/insert_barrier.hpp"
#include "ir/visitor/lifetime_analysis.hpp"
#include "ir/visitor/opencl_ast.hpp"
#include "ir/visitor/stack.hpp"
#include "ir/visitor/work_group_size.hpp"
#include "tinytc/tinytc.hpp"

#include <clir/handle.hpp>
#include <clir/visit.hpp>

#include <utility>

using clir::visit;

namespace tinytc {

bool check_ir(prog p, error_reporter_function reporter) {
    return visit(ir_checker{std::move(reporter)}, *p);
}

void dump_ir(std::ostream &os, func f) { visit(ir_dumper{os}, *f); }
void dump_ir(std::ostream &os, prog p) { visit(ir_dumper{os}, *p); }

clir::prog generate_opencl_ast(prog p, core_info const &info) {
    return visit(opencl_ast{&info}, *p);
}

void insert_barriers(func f) { visit(insert_barrier{}, *f); }
void insert_barriers(prog p) { visit(insert_barrier{}, *p); }

void insert_lifetime_stop_inst(func f) { visit(lifetime_inserter{}, *f); }
void insert_lifetime_stop_inst(prog p) { visit(lifetime_inserter{}, *p); }

bool is_equal(data_type a, data_type b) { return visit(equal{}, *a, *b); }

void set_stack_ptrs(func f) { visit(stack_ptr{}, *f); }
void set_stack_ptrs(prog p) { visit(stack_ptr{}, *p); }

void set_work_group_size(func f, core_info const &info) { visit(work_group_size{&info}, *f); }
void set_work_group_size(prog p, core_info const &info) { visit(work_group_size{&info}, *p); }

} // namespace tinytc

