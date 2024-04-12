// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PASSES_20240314_HPP
#define PASSES_20240314_HPP

#include "tinytc/export.h"
#include "tinytc/ir/error.hpp"

#include <clir/prog.hpp>
#include <iosfwd>
#include <memory>
#include <string>

struct tinytc_core_info;

namespace tinytc {

class data_type;
class func;
class prog;

//! Check whether some IR rules are respected
TINYTC_EXPORT bool check_ir(prog p, error_reporter_function reporter = null_error_reporter());
//! Dump IR to ostream
TINYTC_EXPORT void dump_ir(std::ostream &os, func f);
//! Dump IR to ostream
TINYTC_EXPORT void dump_ir(std::ostream &os, prog p);
//! Generate OpenCL AST
TINYTC_EXPORT clir::prog generate_opencl_ast(prog p, tinytc_core_info const &info);
//! Insert barriers where necessary
TINYTC_EXPORT void insert_barriers(func f);
//! Insert barriers where necessary
TINYTC_EXPORT void insert_barriers(prog p);
//! Insert lifetime stop instructions for set_stack_ptrs pass
TINYTC_EXPORT void insert_lifetime_stop_inst(func f);
//! Insert lifetime stop instructions for set_stack_ptrs pass
TINYTC_EXPORT void insert_lifetime_stop_inst(prog p);
//! Check whether data types a and b are equal
TINYTC_EXPORT bool is_equal(data_type a, data_type b);
//! Manage temporary memory requested by alloca
TINYTC_EXPORT void set_stack_ptrs(func f);
//! Manage temporary memory requested by alloca
TINYTC_EXPORT void set_stack_ptrs(prog p);
//! Choose work group and subgroup size heuristically if not given explicitly
TINYTC_EXPORT void set_work_group_size(func f, tinytc_core_info const &info);
//! Choose work group and subgroup size heuristically if not given explicitly
TINYTC_EXPORT void set_work_group_size(prog p, tinytc_core_info const &info);

} // namespace tinytc

#endif // PASSES_20240314_HPP
