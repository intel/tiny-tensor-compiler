.. Copyright (C) 2025 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

.. _Core C++-API:

============
Core C++-API
============

Common
======

* Classes

  * :ref:`tinytc::array_view_base`

  * :ref:`tinytc::array_view`

  * :ref:`tinytc::mutable_array_view`

  * :ref:`tinytc::shared_handle`

  * :ref:`tinytc::unique_handle`

* Enumerations

  * :ref:`tinytc::address_space`

  * :ref:`tinytc::bundle_format`

  * :ref:`tinytc::checked_flag`

  * :ref:`tinytc::comp3`

  * :ref:`tinytc::core_feature_flag`

  * :ref:`tinytc::intel_gpu_architecture`

  * :ref:`tinytc::matrix_use`

  * :ref:`tinytc::mem_type`

  * :ref:`tinytc::memory_scope`

  * :ref:`tinytc::memory_semantics`

  * :ref:`tinytc::optflag`

  * :ref:`tinytc::reduce_mode`

  * :ref:`tinytc::spirv_feature`

  * :ref:`tinytc::status`

  * :ref:`tinytc::support_level`

  * :ref:`tinytc::transpose`

* Functions

  * :ref:`tinytc::CHECK_STATUS`

  * :ref:`tinytc::CHECK_STATUS_LOC`

  * :ref:`tinytc::to_string(address_space)`

  * :ref:`tinytc::to_string(bundle_format)`

  * :ref:`tinytc::to_string(checked_flag)`

  * :ref:`tinytc::to_string(comp3)`

  * :ref:`tinytc::to_string(core_feature_flag)`

  * :ref:`tinytc::to_string(intel_gpu_architecture)`

  * :ref:`tinytc::to_string(matrix_use)`

  * :ref:`tinytc::to_string(mem_type)`

  * :ref:`tinytc::to_string(memory_scope)`

  * :ref:`tinytc::to_string(memory_semantics)`

  * :ref:`tinytc::to_string(optflag)`

  * :ref:`tinytc::to_string(reduce_mode)`

  * :ref:`tinytc::to_string(spirv_feature)`

  * :ref:`tinytc::to_string(status)`

  * :ref:`tinytc::to_string(support_level)`

  * :ref:`tinytc::to_string(transpose)`

* Structures

  * :ref:`tinytc::auto_mem_type`

  * :ref:`tinytc::auto_mem_type\< T, std::enable_if_t\< is_usm_pointer_type\< T \> \> \>`

  * :ref:`tinytc::mem`

* Variables

  * :ref:`tinytc::auto_mem_type_v`

  * :ref:`tinytc::is_supported_scalar_type`

  * :ref:`tinytc::is_usm_pointer_type`

Common Classes
--------------

.. _tinytc::array_view_base:

array_view_base
...............

.. doxygenclass:: tinytc::array_view_base

.. _tinytc::array_view:

array_view
..........

.. doxygenclass:: tinytc::array_view

.. _tinytc::mutable_array_view:

mutable_array_view
..................

.. doxygenclass:: tinytc::mutable_array_view

.. _tinytc::shared_handle:

shared_handle
.............

.. doxygenclass:: tinytc::shared_handle

.. _tinytc::unique_handle:

unique_handle
.............

.. doxygenclass:: tinytc::unique_handle

Common Enumerations
-------------------

.. _tinytc::address_space:

address_space
.............

.. doxygenenum:: tinytc::address_space

.. _tinytc::bundle_format:

bundle_format
.............

.. doxygenenum:: tinytc::bundle_format

.. _tinytc::checked_flag:

checked_flag
............

.. doxygenenum:: tinytc::checked_flag

.. _tinytc::comp3:

comp3
.....

.. doxygenenum:: tinytc::comp3

.. _tinytc::core_feature_flag:

core_feature_flag
.................

.. doxygenenum:: tinytc::core_feature_flag

.. _tinytc::intel_gpu_architecture:

intel_gpu_architecture
......................

.. doxygenenum:: tinytc::intel_gpu_architecture

.. _tinytc::matrix_use:

matrix_use
..........

.. doxygenenum:: tinytc::matrix_use

.. _tinytc::mem_type:

mem_type
........

.. doxygenenum:: tinytc::mem_type

.. _tinytc::memory_scope:

memory_scope
............

.. doxygenenum:: tinytc::memory_scope

.. _tinytc::memory_semantics:

memory_semantics
................

.. doxygenenum:: tinytc::memory_semantics

.. _tinytc::optflag:

optflag
.......

.. doxygenenum:: tinytc::optflag

.. _tinytc::reduce_mode:

reduce_mode
...........

.. doxygenenum:: tinytc::reduce_mode

.. _tinytc::spirv_feature:

spirv_feature
.............

.. doxygenenum:: tinytc::spirv_feature

.. _tinytc::status:

status
......

.. doxygenenum:: tinytc::status

.. _tinytc::support_level:

support_level
.............

.. doxygenenum:: tinytc::support_level

.. _tinytc::transpose:

transpose
.........

.. doxygenenum:: tinytc::transpose

Common Functions
----------------

.. _tinytc::CHECK_STATUS:

CHECK_STATUS
............

.. doxygenfunction:: tinytc::CHECK_STATUS

.. _tinytc::CHECK_STATUS_LOC:

CHECK_STATUS_LOC
................

.. doxygenfunction:: tinytc::CHECK_STATUS_LOC

.. _tinytc::to_string(address_space):

to_string(address_space)
........................

.. doxygenfunction:: tinytc::to_string(address_space)

.. _tinytc::to_string(bundle_format):

to_string(bundle_format)
........................

.. doxygenfunction:: tinytc::to_string(bundle_format)

.. _tinytc::to_string(checked_flag):

to_string(checked_flag)
.......................

.. doxygenfunction:: tinytc::to_string(checked_flag)

.. _tinytc::to_string(comp3):

to_string(comp3)
................

.. doxygenfunction:: tinytc::to_string(comp3)

.. _tinytc::to_string(core_feature_flag):

to_string(core_feature_flag)
............................

.. doxygenfunction:: tinytc::to_string(core_feature_flag)

.. _tinytc::to_string(intel_gpu_architecture):

to_string(intel_gpu_architecture)
.................................

.. doxygenfunction:: tinytc::to_string(intel_gpu_architecture)

.. _tinytc::to_string(matrix_use):

to_string(matrix_use)
.....................

.. doxygenfunction:: tinytc::to_string(matrix_use)

.. _tinytc::to_string(mem_type):

to_string(mem_type)
...................

.. doxygenfunction:: tinytc::to_string(mem_type)

.. _tinytc::to_string(memory_scope):

to_string(memory_scope)
.......................

.. doxygenfunction:: tinytc::to_string(memory_scope)

.. _tinytc::to_string(memory_semantics):

to_string(memory_semantics)
...........................

.. doxygenfunction:: tinytc::to_string(memory_semantics)

.. _tinytc::to_string(optflag):

to_string(optflag)
..................

.. doxygenfunction:: tinytc::to_string(optflag)

.. _tinytc::to_string(reduce_mode):

to_string(reduce_mode)
......................

.. doxygenfunction:: tinytc::to_string(reduce_mode)

.. _tinytc::to_string(spirv_feature):

to_string(spirv_feature)
........................

.. doxygenfunction:: tinytc::to_string(spirv_feature)

.. _tinytc::to_string(status):

to_string(status)
.................

.. doxygenfunction:: tinytc::to_string(status)

.. _tinytc::to_string(support_level):

to_string(support_level)
........................

.. doxygenfunction:: tinytc::to_string(support_level)

.. _tinytc::to_string(transpose):

to_string(transpose)
....................

.. doxygenfunction:: tinytc::to_string(transpose)

Common Structures
-----------------

.. _tinytc::auto_mem_type:

auto_mem_type
.............

.. doxygenstruct:: tinytc::auto_mem_type

.. _tinytc::auto_mem_type\< T, std::enable_if_t\< is_usm_pointer_type\< T \> \> \>:

auto_mem_type<T, std::enable_if_t<is_usm_pointer_type<T>>>
..........................................................

.. doxygenstruct:: tinytc::auto_mem_type< T, std::enable_if_t< is_usm_pointer_type< T > > >

.. _tinytc::mem:

mem
...

.. doxygenstruct:: tinytc::mem

Common Variables
----------------

.. _tinytc::auto_mem_type_v:

auto_mem_type_v
...............

.. doxygenvariable:: tinytc::auto_mem_type_v

.. _tinytc::is_supported_scalar_type:

is_supported_scalar_type
........................

.. doxygenvariable:: tinytc::is_supported_scalar_type

.. _tinytc::is_usm_pointer_type:

is_usm_pointer_type
...................

.. doxygenvariable:: tinytc::is_usm_pointer_type

Binary
======

* Functions

  * :ref:`tinytc::create_binary`

  * :ref:`tinytc::get_compiler_context(const_tinytc_binary_t)`

  * :ref:`tinytc::get_core_features(const_tinytc_binary_t)`

  * :ref:`tinytc::get_raw`

* Structures

  * :ref:`tinytc::raw_binary`

Binary Functions
----------------

.. _tinytc::create_binary:

create_binary
.............

.. doxygenfunction:: tinytc::create_binary

.. _tinytc::get_compiler_context(const_tinytc_binary_t):

get_compiler_context(const_tinytc_binary_t)
...........................................

.. doxygenfunction:: tinytc::get_compiler_context(const_tinytc_binary_t)

.. _tinytc::get_core_features(const_tinytc_binary_t):

get_core_features(const_tinytc_binary_t)
........................................

.. doxygenfunction:: tinytc::get_core_features(const_tinytc_binary_t)

.. _tinytc::get_raw:

get_raw
.......

.. doxygenfunction:: tinytc::get_raw

Binary Structures
-----------------

.. _tinytc::raw_binary:

raw_binary
..........

.. doxygenstruct:: tinytc::raw_binary

Compiler
========

* Functions

  * :ref:`tinytc::run_function_pass`

  * :ref:`tinytc::list_function_passes`

  * :ref:`tinytc::compile_to_spirv`

  * :ref:`tinytc::compile_to_spirv_and_assemble`

  * :ref:`tinytc::spirv_assemble`

Compiler Functions
------------------

.. _tinytc::run_function_pass:

run_function_pass
.................

.. doxygenfunction:: tinytc::run_function_pass

.. _tinytc::list_function_passes:

list_function_passes
....................

.. doxygenfunction:: tinytc::list_function_passes

.. _tinytc::compile_to_spirv:

compile_to_spirv
................

.. doxygenfunction:: tinytc::compile_to_spirv

.. _tinytc::compile_to_spirv_and_assemble:

compile_to_spirv_and_assemble
.............................

.. doxygenfunction:: tinytc::compile_to_spirv_and_assemble

.. _tinytc::spirv_assemble:

spirv_assemble
..............

.. doxygenfunction:: tinytc::spirv_assemble

Compiler Context
================

* Functions

  * :ref:`tinytc::add_source`

  * :ref:`tinytc::create_compiler_context`

  * :ref:`tinytc::set_error_reporter`

  * :ref:`tinytc::set_optimization_flag`

  * :ref:`tinytc::set_optimization_level`

  * :ref:`tinytc::report_error`

Compiler Context Functions
--------------------------

.. _tinytc::add_source:

add_source
..........

.. doxygenfunction:: tinytc::add_source

.. _tinytc::create_compiler_context:

create_compiler_context
.......................

.. doxygenfunction:: tinytc::create_compiler_context

.. _tinytc::set_error_reporter:

set_error_reporter
..................

.. doxygenfunction:: tinytc::set_error_reporter

.. _tinytc::set_optimization_flag:

set_optimization_flag
.....................

.. doxygenfunction:: tinytc::set_optimization_flag

.. _tinytc::set_optimization_level:

set_optimization_level
......................

.. doxygenfunction:: tinytc::set_optimization_level

.. _tinytc::report_error:

report_error
............

.. doxygenfunction:: tinytc::report_error

Device Info
===========

* Functions

  * :ref:`tinytc::create_core_info_generic`

  * :ref:`tinytc::create_core_info_intel`

  * :ref:`tinytc::create_core_info_intel_from_arch`

  * :ref:`tinytc::create_core_info_intel_from_name`

  * :ref:`tinytc::get_core_features(const_tinytc_core_info_t)`

  * :ref:`tinytc::get_subgroup_sizes`

  * :ref:`tinytc::get_register_space`

  * :ref:`tinytc::have_spirv_feature`

  * :ref:`tinytc::set_core_features`

  * :ref:`tinytc::set_default_alignment`

  * :ref:`tinytc::set_spirv_feature`

Device Info Functions
---------------------

.. _tinytc::create_core_info_generic:

create_core_info_generic
........................

.. doxygenfunction:: tinytc::create_core_info_generic

.. _tinytc::create_core_info_intel:

create_core_info_intel
......................

.. doxygenfunction:: tinytc::create_core_info_intel

.. _tinytc::create_core_info_intel_from_arch:

create_core_info_intel_from_arch
................................

.. doxygenfunction:: tinytc::create_core_info_intel_from_arch

.. _tinytc::create_core_info_intel_from_name:

create_core_info_intel_from_name
................................

.. doxygenfunction:: tinytc::create_core_info_intel_from_name

.. _tinytc::get_core_features(const_tinytc_core_info_t):

get_core_features(const_tinytc_core_info_t)
...........................................

.. doxygenfunction:: tinytc::get_core_features(const_tinytc_core_info_t)

.. _tinytc::get_subgroup_sizes:

get_subgroup_sizes
..................

.. doxygenfunction:: tinytc::get_subgroup_sizes

.. _tinytc::get_register_space:

get_register_space
..................

.. doxygenfunction:: tinytc::get_register_space

.. _tinytc::have_spirv_feature:

have_spirv_feature
..................

.. doxygenfunction:: tinytc::have_spirv_feature

.. _tinytc::set_core_features:

set_core_features
.................

.. doxygenfunction:: tinytc::set_core_features

.. _tinytc::set_default_alignment:

set_default_alignment
.....................

.. doxygenfunction:: tinytc::set_default_alignment

.. _tinytc::set_spirv_feature:

set_spirv_feature
.................

.. doxygenfunction:: tinytc::set_spirv_feature

FP math
=======

* Classes

  * :ref:`tinytc::lp_float`

* Functions

  * :ref:`tinytc::ieee754_extend`

  * :ref:`tinytc::ieee754_truncate`

* Structures

  * :ref:`tinytc::ieee754_format`

* Typedefs

  * :ref:`tinytc::bf16_format`

  * :ref:`tinytc::bfloat16`

  * :ref:`tinytc::f16_format`

  * :ref:`tinytc::f32_format`

  * :ref:`tinytc::half`

FP math Classes
---------------

.. _tinytc::lp_float:

lp_float
........

.. doxygenclass:: tinytc::lp_float

FP math Functions
-----------------

.. _tinytc::ieee754_extend:

ieee754_extend
..............

.. doxygenfunction:: tinytc::ieee754_extend

.. _tinytc::ieee754_truncate:

ieee754_truncate
................

.. doxygenfunction:: tinytc::ieee754_truncate

FP math Structures
------------------

.. _tinytc::ieee754_format:

ieee754_format
..............

.. doxygenstruct:: tinytc::ieee754_format

FP math Typedefs
----------------

.. _tinytc::bf16_format:

bf16_format
...........

.. doxygentypedef:: tinytc::bf16_format

.. _tinytc::bfloat16:

bfloat16
........

.. doxygentypedef:: tinytc::bfloat16

.. _tinytc::f16_format:

f16_format
..........

.. doxygentypedef:: tinytc::f16_format

.. _tinytc::f32_format:

f32_format
..........

.. doxygentypedef:: tinytc::f32_format

.. _tinytc::half:

half
....

.. doxygentypedef:: tinytc::half

Parser
======

* Functions

  * :ref:`tinytc::parse_file`

  * :ref:`tinytc::parse_stdin`

  * :ref:`tinytc::parse_string`

Parser Functions
----------------

.. _tinytc::parse_file:

parse_file
..........

.. doxygenfunction:: tinytc::parse_file

.. _tinytc::parse_stdin:

parse_stdin
...........

.. doxygenfunction:: tinytc::parse_stdin

.. _tinytc::parse_string:

parse_string
............

.. doxygenfunction:: tinytc::parse_string

Program
=======

* Functions

  * :ref:`tinytc::dump(tinytc_prog_t)`

  * :ref:`tinytc::get_compiler_context(const_tinytc_prog_t)`

  * :ref:`tinytc::print_to_file(tinytc_prog_t, char const\*)`

  * :ref:`tinytc::print_to_string(tinytc_prog_t)`

Program Functions
-----------------

.. _tinytc::dump(tinytc_prog_t):

dump(tinytc_prog_t)
...................

.. doxygenfunction:: tinytc::dump(tinytc_prog_t)

.. _tinytc::get_compiler_context(const_tinytc_prog_t):

get_compiler_context(const_tinytc_prog_t)
.........................................

.. doxygenfunction:: tinytc::get_compiler_context(const_tinytc_prog_t)

.. _tinytc::print_to_file(tinytc_prog_t, char const\*):

print_to_file(tinytc_prog_t, char const\*)
..........................................

.. doxygenfunction:: tinytc::print_to_file(tinytc_prog_t, char const*)

.. _tinytc::print_to_string(tinytc_prog_t):

print_to_string(tinytc_prog_t)
..............................

.. doxygenfunction:: tinytc::print_to_string(tinytc_prog_t)

SPIR-V module
=============

* Functions

  * :ref:`tinytc::dump(const_tinytc_spv_mod_t)`

  * :ref:`tinytc::print_to_file(const_tinytc_spv_mod_t, char const\*)`

  * :ref:`tinytc::print_to_string(const_tinytc_spv_mod_t)`

SPIR-V module Functions
-----------------------

.. _tinytc::dump(const_tinytc_spv_mod_t):

dump(const_tinytc_spv_mod_t)
............................

.. doxygenfunction:: tinytc::dump(const_tinytc_spv_mod_t)

.. _tinytc::print_to_file(const_tinytc_spv_mod_t, char const\*):

print_to_file(const_tinytc_spv_mod_t, char const\*)
...................................................

.. doxygenfunction:: tinytc::print_to_file(const_tinytc_spv_mod_t, char const*)

.. _tinytc::print_to_string(const_tinytc_spv_mod_t):

print_to_string(const_tinytc_spv_mod_t)
.......................................

.. doxygenfunction:: tinytc::print_to_string(const_tinytc_spv_mod_t)

