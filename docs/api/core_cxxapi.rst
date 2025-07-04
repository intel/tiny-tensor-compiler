.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

.. _Core C++-API:

============
Core C++-API
============

Common
======

* Enumerations

  * :ref:`tinytc::status`

  * :ref:`tinytc::support_level`

* Functions

  * :ref:`tinytc::CHECK_STATUS`

  * :ref:`tinytc::CHECK_STATUS_LOC`

  * :ref:`tinytc::to_string(status)`

  * :ref:`tinytc::to_string(support_level)`

* Classes

  * :ref:`tinytc::array_view_base`

  * :ref:`tinytc::array_view`

  * :ref:`tinytc::mutable_array_view`

  * :ref:`tinytc::shared_handle`

  * :ref:`tinytc::unique_handle`

Common Enumerations
-------------------

.. _tinytc::status:

status
......

.. doxygenenum:: tinytc::status

.. _tinytc::support_level:

support_level
.............

.. doxygenenum:: tinytc::support_level

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

.. _tinytc::to_string(status):

to_string(status)
.................

.. doxygenfunction:: tinytc::to_string(status)

.. _tinytc::to_string(support_level):

to_string(support_level)
........................

.. doxygenfunction:: tinytc::to_string(support_level)

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

Binary
======

* Enumerations

  * :ref:`tinytc::bundle_format`

* Functions

  * :ref:`tinytc::get_compiler_context(const_tinytc_binary_t)`

  * :ref:`tinytc::get_core_features(const_tinytc_binary_t)`

  * :ref:`tinytc::get_raw`

  * :ref:`tinytc::make_binary`

  * :ref:`tinytc::to_string(bundle_format)`

* Structures

  * :ref:`tinytc::raw_binary`

Binary Enumerations
-------------------

.. _tinytc::bundle_format:

bundle_format
.............

.. doxygenenum:: tinytc::bundle_format

Binary Functions
----------------

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

.. _tinytc::make_binary:

make_binary
...........

.. doxygenfunction:: tinytc::make_binary

.. _tinytc::to_string(bundle_format):

to_string(bundle_format)
........................

.. doxygenfunction:: tinytc::to_string(bundle_format)

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

* Enumerations

  * :ref:`tinytc::optflag`

* Functions

  * :ref:`tinytc::add_source`

  * :ref:`tinytc::make_compiler_context`

  * :ref:`tinytc::set_error_reporter`

  * :ref:`tinytc::set_optimization_flag`

  * :ref:`tinytc::set_optimization_level`

  * :ref:`tinytc::report_error`

  * :ref:`tinytc::to_string(optflag)`

Compiler Context Enumerations
-----------------------------

.. _tinytc::optflag:

optflag
.......

.. doxygenenum:: tinytc::optflag

Compiler Context Functions
--------------------------

.. _tinytc::add_source:

add_source
..........

.. doxygenfunction:: tinytc::add_source

.. _tinytc::make_compiler_context:

make_compiler_context
.....................

.. doxygenfunction:: tinytc::make_compiler_context

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

.. _tinytc::to_string(optflag):

to_string(optflag)
..................

.. doxygenfunction:: tinytc::to_string(optflag)

Device Info
===========

* Enumerations

  * :ref:`tinytc::core_feature_flag`

  * :ref:`tinytc::intel_gpu_architecture`

  * :ref:`tinytc::spirv_feature`

* Functions

  * :ref:`tinytc::get_core_features(const_tinytc_core_info_t)`

  * :ref:`tinytc::get_subgroup_sizes`

  * :ref:`tinytc::get_register_space`

  * :ref:`tinytc::have_spirv_feature`

  * :ref:`tinytc::make_core_info_generic`

  * :ref:`tinytc::make_core_info_intel`

  * :ref:`tinytc::make_core_info_intel_from_arch`

  * :ref:`tinytc::make_core_info_intel_from_name`

  * :ref:`tinytc::set_core_features`

  * :ref:`tinytc::set_default_alignment`

  * :ref:`tinytc::set_spirv_feature`

  * :ref:`tinytc::to_string(core_feature_flag)`

  * :ref:`tinytc::to_string(intel_gpu_architecture)`

  * :ref:`tinytc::to_string(spirv_feature)`

Device Info Enumerations
------------------------

.. _tinytc::core_feature_flag:

core_feature_flag
.................

.. doxygenenum:: tinytc::core_feature_flag

.. _tinytc::intel_gpu_architecture:

intel_gpu_architecture
......................

.. doxygenenum:: tinytc::intel_gpu_architecture

.. _tinytc::spirv_feature:

spirv_feature
.............

.. doxygenenum:: tinytc::spirv_feature

Device Info Functions
---------------------

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

.. _tinytc::make_core_info_generic:

make_core_info_generic
......................

.. doxygenfunction:: tinytc::make_core_info_generic

.. _tinytc::make_core_info_intel:

make_core_info_intel
....................

.. doxygenfunction:: tinytc::make_core_info_intel

.. _tinytc::make_core_info_intel_from_arch:

make_core_info_intel_from_arch
..............................

.. doxygenfunction:: tinytc::make_core_info_intel_from_arch

.. _tinytc::make_core_info_intel_from_name:

make_core_info_intel_from_name
..............................

.. doxygenfunction:: tinytc::make_core_info_intel_from_name

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

.. _tinytc::to_string(core_feature_flag):

to_string(core_feature_flag)
............................

.. doxygenfunction:: tinytc::to_string(core_feature_flag)

.. _tinytc::to_string(intel_gpu_architecture):

to_string(intel_gpu_architecture)
.................................

.. doxygenfunction:: tinytc::to_string(intel_gpu_architecture)

.. _tinytc::to_string(spirv_feature):

to_string(spirv_feature)
........................

.. doxygenfunction:: tinytc::to_string(spirv_feature)

FP math
=======

* Functions

  * :ref:`tinytc::ieee754_extend`

  * :ref:`tinytc::ieee754_truncate`

* Classes

  * :ref:`tinytc::lp_float`

* Structures

  * :ref:`tinytc::ieee754_format`

* Typedefs

  * :ref:`tinytc::bf16_format`

  * :ref:`tinytc::bfloat16`

  * :ref:`tinytc::f16_format`

  * :ref:`tinytc::f32_format`

  * :ref:`tinytc::half`

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

FP math Classes
---------------

.. _tinytc::lp_float:

lp_float
........

.. doxygenclass:: tinytc::lp_float

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

Recipe
======

* Enumerations

  * :ref:`tinytc::mem_type`

* Functions

  * :ref:`tinytc::get_prog`

  * :ref:`tinytc::get_binary`

  * :ref:`tinytc::get_recipe`

  * :ref:`tinytc::make_small_gemm_batched`

  * :ref:`tinytc::make_tall_and_skinny`

  * :ref:`tinytc::make_tall_and_skinny_specialized`

  * :ref:`tinytc::set_small_gemm_batched_args`

  * :ref:`tinytc::set_tall_and_skinny_args`

  * :ref:`tinytc::to_string(mem_type)`

* Structures

  * :ref:`tinytc::auto_mem_type`

  * :ref:`tinytc::auto_mem_type\< T, std::enable_if_t\< is_usm_pointer_type\< T \> \> \>`

  * :ref:`tinytc::mem`

* Variables

  * :ref:`tinytc::auto_mem_type_v`

  * :ref:`tinytc::is_supported_scalar_type`

  * :ref:`tinytc::is_usm_pointer_type`

Recipe Enumerations
-------------------

.. _tinytc::mem_type:

mem_type
........

.. doxygenenum:: tinytc::mem_type

Recipe Functions
----------------

.. _tinytc::get_prog:

get_prog
........

.. doxygenfunction:: tinytc::get_prog

.. _tinytc::get_binary:

get_binary
..........

.. doxygenfunction:: tinytc::get_binary

.. _tinytc::get_recipe:

get_recipe
..........

.. doxygenfunction:: tinytc::get_recipe

.. _tinytc::make_small_gemm_batched:

make_small_gemm_batched
.......................

.. doxygenfunction:: tinytc::make_small_gemm_batched

.. _tinytc::make_tall_and_skinny:

make_tall_and_skinny
....................

.. doxygenfunction:: tinytc::make_tall_and_skinny

.. _tinytc::make_tall_and_skinny_specialized:

make_tall_and_skinny_specialized
................................

.. doxygenfunction:: tinytc::make_tall_and_skinny_specialized

.. _tinytc::set_small_gemm_batched_args:

set_small_gemm_batched_args
...........................

.. doxygenfunction:: tinytc::set_small_gemm_batched_args

.. _tinytc::set_tall_and_skinny_args:

set_tall_and_skinny_args
........................

.. doxygenfunction:: tinytc::set_tall_and_skinny_args

.. _tinytc::to_string(mem_type):

to_string(mem_type)
...................

.. doxygenfunction:: tinytc::to_string(mem_type)

Recipe Structures
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

Recipe Variables
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

