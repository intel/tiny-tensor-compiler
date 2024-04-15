// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "passes.hpp"
#include "device_info.hpp"
#include "tinytc/tinytc.hpp"
#include "visitor/check_ir.hpp"
#include "visitor/dump_ir.hpp"
#include "visitor/equal.hpp"
#include "visitor/insert_barrier.hpp"
#include "visitor/lifetime_analysis.hpp"
#include "visitor/metadata.hpp"
#include "visitor/opencl_ast.hpp"
#include "visitor/stack.hpp"
#include "visitor/work_group_size.hpp"

#include <clir/handle.hpp>
#include <clir/visit.hpp>

#include <utility>

using clir::visit;

namespace tinytc {

void check_ir(prog p) { return visit(ir_checker{}, *p); }

void dump_ir(std::ostream &os, func f) { visit(ir_dumper{os}, *f); }
void dump_ir(std::ostream &os, prog p) { visit(ir_dumper{os}, *p); }

clir::prog generate_opencl_ast(prog p, ::tinytc_core_info const &info) {
    return visit(opencl_ast{&info}, *p);
}

auto get_metadata(prog p) -> std::unordered_map<std::string, kernel_metadata> {
    auto v = metadata{};
    visit(v, *p);
    return v.get_result();
}

void insert_barriers(func f) { visit(insert_barrier{}, *f); }
void insert_barriers(prog p) { visit(insert_barrier{}, *p); }

void insert_lifetime_stop_inst(func f) { visit(lifetime_inserter{}, *f); }
void insert_lifetime_stop_inst(prog p) { visit(lifetime_inserter{}, *p); }

bool is_equal(data_type a, data_type b) { return visit(equal{}, *a, *b); }

void set_stack_ptrs(func f) { visit(stack_ptr{}, *f); }
void set_stack_ptrs(prog p) { visit(stack_ptr{}, *p); }

void set_work_group_size(func f, ::tinytc_core_info const &info) {
    visit(work_group_size{&info}, *f);
}
void set_work_group_size(prog p, ::tinytc_core_info const &info) {
    visit(work_group_size{&info}, *p);
}

} // namespace tinytc

