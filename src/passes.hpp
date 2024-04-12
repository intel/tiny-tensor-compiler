// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PASSES_20240314_HPP
#define PASSES_20240314_HPP

#include "kernel_metadata.hpp"
#include "tinytc/ir/error.hpp"

#include <clir/prog.hpp>
#include <iosfwd>
#include <memory>
#include <string>
#include <unordered_map>

struct tinytc_core_info;

namespace tinytc {

class data_type;
class func;
class prog;

//! Check whether some IR rules are respected
bool check_ir(prog p, error_reporter_function reporter = null_error_reporter());
//! Dump IR to ostream
void dump_ir(std::ostream &os, func f);
//! Dump IR to ostream
void dump_ir(std::ostream &os, prog p);
//! Generate OpenCL AST
clir::prog generate_opencl_ast(prog p, tinytc_core_info const &info);
//! Get kernel metadata
auto get_metadata(prog p) -> std::unordered_map<std::string, kernel_metadata>;
//! Insert barriers where necessary
void insert_barriers(func f);
//! Insert barriers where necessary
void insert_barriers(prog p);
//! Insert lifetime stop instructions for set_stack_ptrs pass
void insert_lifetime_stop_inst(func f);
//! Insert lifetime stop instructions for set_stack_ptrs pass
void insert_lifetime_stop_inst(prog p);
//! Check whether data types a and b are equal
bool is_equal(data_type a, data_type b);
//! Manage temporary memory requested by alloca
void set_stack_ptrs(func f);
//! Manage temporary memory requested by alloca
void set_stack_ptrs(prog p);
//! Choose work group and subgroup size heuristically if not given explicitly
void set_work_group_size(func f, tinytc_core_info const &info);
//! Choose work group and subgroup size heuristically if not given explicitly
void set_work_group_size(prog p, tinytc_core_info const &info);

} // namespace tinytc

#endif // PASSES_20240314_HPP
