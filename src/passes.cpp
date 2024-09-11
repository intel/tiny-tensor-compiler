// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "passes.hpp"
#include "device_info.hpp"
#include "kernel_metadata.hpp"
#include "node/data_type_node.hpp"
// #include "node/function_node.hpp"
// #include "node/program_node.hpp"
#include "pass/check_ir.hpp"
// #include "pass/constant_propagation.hpp"
#include "pass/dump_ir.hpp"
#include "pass/equal.hpp"
// #include "pass/insert_barrier.hpp"
// #include "pass/lifetime_analysis.hpp"
// #include "pass/lower_linalg.hpp"
// #include "pass/metadata.hpp"
// #include "pass/opencl_ast.hpp"
// #include "pass/stack.hpp"
// #include "pass/work_group_size.hpp"
#include "support/visit.hpp"

namespace tinytc {

// void check_ir(tinytc_prog const &p) { return visit(ir_checker{}, p); }

// void dump_ir(std::ostream &os, tinytc_func const &f) { visit(ir_dumper{os}, f); }
void dump_ir(std::ostream &os, tinytc_prog const &p) { run_function_pass(dump_ir_pass{os}, p); }

/*clir::prog generate_opencl_ast(tinytc_prog const &p, ::tinytc_core_info const &info) {
    return visit(opencl_ast{&info}, p);
}

auto get_metadata(tinytc_prog const &p) -> std::unordered_map<std::string, kernel_metadata> {
    auto v = metadata{};
    visit(v, p);
    return v.get_result();
}

void insert_barriers(tinytc_func &f) { visit(insert_barrier{}, f); }
void insert_barriers(tinytc_prog &p) { visit(insert_barrier{}, p); }

void insert_lifetime_stop_inst(tinytc_func &f) { visit(lifetime_inserter{}, f); }
void insert_lifetime_stop_inst(tinytc_prog &p) { visit(lifetime_inserter{}, p); }*/

bool is_equal(tinytc_data_type const &a, tinytc_data_type const &b) { return visit(equal{}, a, b); }

/*void lower_linalg(tinytc_prog &p, ::tinytc_core_info const &info) {
    visit(lower_linalg_pass{&info}, p);
}

void propagate_constants(tinytc_prog &p) { visit(constant_propagation{}, p); }

void set_stack_ptrs(tinytc_func &f) { visit(stack_ptr{}, f); }
void set_stack_ptrs(tinytc_prog &p) { visit(stack_ptr{}, p); }

void set_work_group_size(tinytc_func &f, ::tinytc_core_info const &info) {
    visit(work_group_size{&info}, f);
}
void set_work_group_size(tinytc_prog &p, ::tinytc_core_info const &info) {
    visit(work_group_size{&info}, p);
}*/

} // namespace tinytc

