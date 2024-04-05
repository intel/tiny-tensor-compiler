.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

===============
Compiler passes
===============

Compiler passes insert barriers, analysis memref aliasing, manage a stack for alloca,
dump the IR, and generate code.
Compiler passes may depend on each other, use with care.

Passes
======

.. doxygenfunction:: tinytc::ir::alias_analysis(func)
.. doxygenfunction:: tinytc::ir::check_ir(prog,error_reporter_function)
.. doxygenfunction:: tinytc::ir::dump_ir(std::ostream&,func)
.. doxygenfunction:: tinytc::ir::dump_ir(std::ostream&,prog)
.. doxygenfunction:: tinytc::ir::generate_opencl_ast(prog,std::shared_ptr<core_info>)
.. doxygenfunction:: tinytc::ir::insert_barriers(func)
.. doxygenfunction:: tinytc::ir::insert_barriers(prog)
.. doxygenfunction:: tinytc::ir::insert_lifetime_stop_inst(func)
.. doxygenfunction:: tinytc::ir::insert_lifetime_stop_inst(prog)
.. doxygenfunction:: tinytc::ir::is_equal(data_type,data_type)
.. doxygenfunction:: tinytc::ir::set_stack_ptrs(func)
.. doxygenfunction:: tinytc::ir::set_stack_ptrs(prog)
.. doxygenfunction:: tinytc::ir::set_work_group_size(func,std::shared_ptr<core_info>)
.. doxygenfunction:: tinytc::ir::set_work_group_size(prog,std::shared_ptr<core_info>)
