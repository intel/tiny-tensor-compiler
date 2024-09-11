// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PASSES_20240314_HPP
#define PASSES_20240314_HPP

#include "kernel_metadata.hpp"
#include "node/program_node.hpp"
#include "tinytc/types.h"

#include <clir/prog.hpp>
#include <iosfwd>
#include <string>
#include <unordered_map>

namespace tinytc {

//! Check whether some IR rules are respected
// void check_ir(tinytc_prog const &p);
//! Dump IR to ostream
void dump_ir(std::ostream &os, tinytc_func const &f);
//! Dump IR to ostream
void dump_ir(std::ostream &os, tinytc_prog const &p);
//! Generate OpenCL AST
/*clir::prog generate_opencl_ast(tinytc_prog const &p, tinytc_core_info const &info);
//! Get kernel metadata
auto get_metadata(tinytc_prog const &p) -> std::unordered_map<std::string, kernel_metadata>;
//! Insert barriers where necessary
void insert_barriers(tinytc_func &f);
//! Insert barriers where necessary
void insert_barriers(tinytc_prog &p);
//! Insert lifetime stop instructions for set_stack_ptrs pass
void insert_lifetime_stop_inst(tinytc_func &f);
//! Insert lifetime stop instructions for set_stack_ptrs pass
void insert_lifetime_stop_inst(tinytc_prog &p);*/
//! Check whether data types a and b are equal
bool is_equal(tinytc_data_type const &a, tinytc_data_type const &b);
//! Implement linear algebra instructions
/*void lower_linalg(tinytc_prog &p, tinytc_core_info const &info);
//! Constant propagation
void propagate_constants(tinytc_prog &p);
//! Manage temporary memory requested by alloca
void set_stack_ptrs(tinytc_func &f);
//! Manage temporary memory requested by alloca
void set_stack_ptrs(tinytc_prog &p);
//! Choose work group and subgroup size heuristically if not given explicitly
void set_work_group_size(tinytc_func &f, tinytc_core_info const &info);
//! Choose work group and subgroup size heuristically if not given explicitly
void set_work_group_size(tinytc_prog &p, tinytc_core_info const &info);*/

template <typename FunctionPass> void run_function_pass(FunctionPass &&pass, tinytc_prog const &p) {
    for (auto const &func : p.functions()) {
        pass.run_on_function(*func);
    }
}

} // namespace tinytc

#endif // PASSES_20240314_HPP
