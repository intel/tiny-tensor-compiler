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

  * :ref:`tinytc::error_string`

  * :ref:`tinytc::CHECK_STATUS`

  * :ref:`tinytc::CHECK_STATUS_LOC`

* Classes

  * :ref:`tinytc::array_view_base`

  * :ref:`tinytc::array_view`

  * :ref:`tinytc::mutable_array_view`

  * :ref:`tinytc::handle`

  * :ref:`tinytc::shared_handle`

  * :ref:`tinytc::unique_handle`

* Typedefs

  * :ref:`tinytc::error_reporter_t`

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

.. _tinytc::error_string:

error_string
............

.. doxygenfunction:: tinytc::error_string

.. _tinytc::CHECK_STATUS:

CHECK_STATUS
............

.. doxygenfunction:: tinytc::CHECK_STATUS

.. _tinytc::CHECK_STATUS_LOC:

CHECK_STATUS_LOC
................

.. doxygenfunction:: tinytc::CHECK_STATUS_LOC

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

.. _tinytc::handle:

handle
......

.. doxygenclass:: tinytc::handle

.. _tinytc::shared_handle:

shared_handle
.............

.. doxygenclass:: tinytc::shared_handle

.. _tinytc::unique_handle:

unique_handle
.............

.. doxygenclass:: tinytc::unique_handle

Common Typedefs
---------------

.. _tinytc::error_reporter_t:

error_reporter_t
................

.. doxygentypedef:: tinytc::error_reporter_t

Binary
======

* Enumerations

  * :ref:`tinytc::bundle_format`

* Functions

  * :ref:`tinytc::make_binary`

* Classes

  * :ref:`tinytc::binary`

Binary Enumerations
-------------------

.. _tinytc::bundle_format:

bundle_format
.............

.. doxygenenum:: tinytc::bundle_format

Binary Functions
----------------

.. _tinytc::make_binary:

make_binary
...........

.. doxygenfunction:: tinytc::make_binary

Binary Classes
--------------

.. _tinytc::binary:

binary
......

.. doxygenclass:: tinytc::binary

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

  * :ref:`tinytc::make_compiler_context`

* Classes

  * :ref:`tinytc::compiler_context`

Compiler Context Functions
--------------------------

.. _tinytc::make_compiler_context:

make_compiler_context
.....................

.. doxygenfunction:: tinytc::make_compiler_context

Compiler Context Classes
------------------------

.. _tinytc::compiler_context:

compiler_context
................

.. doxygenclass:: tinytc::compiler_context

Device Info
===========

* Enumerations

  * :ref:`tinytc::core_feature_flag`

  * :ref:`tinytc::intel_gpu_architecture`

* Functions

  * :ref:`tinytc::make_core_info_generic`

  * :ref:`tinytc::make_core_info_intel`

  * :ref:`tinytc::make_core_info_intel_from_arch`

  * :ref:`tinytc::make_core_info_intel_from_name`

* Classes

  * :ref:`tinytc::core_info`

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

Device Info Functions
---------------------

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

Device Info Classes
-------------------

.. _tinytc::core_info:

core_info
.........

.. doxygenclass:: tinytc::core_info

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

Recipe
======

* Enumerations

  * :ref:`tinytc::mem_type`

* Functions

  * :ref:`tinytc::make_small_gemm_batched`

  * :ref:`tinytc::make_tall_and_skinny`

  * :ref:`tinytc::make_tall_and_skinny_specialized`

* Classes

  * :ref:`tinytc::recipe`

  * :ref:`tinytc::recipe_handler`

  * :ref:`tinytc::small_gemm_batched`

  * :ref:`tinytc::tall_and_skinny`

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

Recipe Classes
--------------

.. _tinytc::recipe:

recipe
......

.. doxygenclass:: tinytc::recipe

.. _tinytc::recipe_handler:

recipe_handler
..............

.. doxygenclass:: tinytc::recipe_handler

.. _tinytc::small_gemm_batched:

small_gemm_batched
..................

.. doxygenclass:: tinytc::small_gemm_batched

.. _tinytc::tall_and_skinny:

tall_and_skinny
...............

.. doxygenclass:: tinytc::tall_and_skinny

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

* Classes

  * :ref:`tinytc::spv_mod`

SPIR-V module Classes
---------------------

.. _tinytc::spv_mod:

spv_mod
.......

.. doxygenclass:: tinytc::spv_mod

