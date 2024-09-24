.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

============
Core C++-API
============

Common
======

* Enumerations

  * :ref:`status`

  * :ref:`support_level`

* Functions

  * :ref:`error_string`

  * :ref:`CHECK_STATUS`

  * :ref:`CHECK_STATUS_LOC`

* Classes

  * :ref:`shared_handle`

  * :ref:`unique_handle`

* Typedefs

  * :ref:`error_reporter_t`

Common Enumerations
-------------------

status
......

.. doxygenenum:: tinytc::status

support_level
.............

.. doxygenenum:: tinytc::support_level

Common Functions
----------------

error_string
............

.. doxygenfunction:: tinytc::error_string

CHECK_STATUS
............

.. doxygenfunction:: tinytc::CHECK_STATUS

CHECK_STATUS_LOC
................

.. doxygenfunction:: tinytc::CHECK_STATUS_LOC

Common Classes
--------------

shared_handle
.............

.. doxygenclass:: tinytc::shared_handle

unique_handle
.............

.. doxygenclass:: tinytc::unique_handle

Common Typedefs
---------------

error_reporter_t
................

.. doxygentypedef:: tinytc::error_reporter_t

Binary
======

* Enumerations

  * :ref:`bundle_format`

* Functions

  * :ref:`make_binary`

* Classes

  * :ref:`binary`

Binary Enumerations
-------------------

bundle_format
.............

.. doxygenenum:: tinytc::bundle_format

Binary Functions
----------------

make_binary
...........

.. doxygenfunction:: tinytc::make_binary

Binary Classes
--------------

binary
......

.. doxygenclass:: tinytc::binary

Compiler
========

* Functions

  * :ref:`run_function_pass`

  * :ref:`list_function_passes`

  * :ref:`compile_to_opencl`

Compiler Functions
------------------

run_function_pass
.................

.. doxygenfunction:: tinytc::run_function_pass

list_function_passes
....................

.. doxygenfunction:: tinytc::list_function_passes

compile_to_opencl
.................

.. doxygenfunction:: tinytc::compile_to_opencl

Compiler Context
================

* Functions

  * :ref:`make_compiler_context`

* Classes

  * :ref:`compiler_context`

Compiler Context Functions
--------------------------

make_compiler_context
.....................

.. doxygenfunction:: tinytc::make_compiler_context

Compiler Context Classes
------------------------

compiler_context
................

.. doxygenclass:: tinytc::compiler_context

Device Info
===========

* Enumerations

  * :ref:`core_feature_flag`

  * :ref:`intel_gpu_architecture`

* Functions

  * :ref:`make_core_info_generic`

  * :ref:`make_core_info_intel`

  * :ref:`make_core_info_intel_from_arch`

* Classes

  * :ref:`core_info`

Device Info Enumerations
------------------------

core_feature_flag
.................

.. doxygenenum:: tinytc::core_feature_flag

intel_gpu_architecture
......................

.. doxygenenum:: tinytc::intel_gpu_architecture

Device Info Functions
---------------------

make_core_info_generic
......................

.. doxygenfunction:: tinytc::make_core_info_generic

make_core_info_intel
....................

.. doxygenfunction:: tinytc::make_core_info_intel

make_core_info_intel_from_arch
..............................

.. doxygenfunction:: tinytc::make_core_info_intel_from_arch

Device Info Classes
-------------------

core_info
.........

.. doxygenclass:: tinytc::core_info

Parser
======

* Functions

  * :ref:`parse_file`

  * :ref:`parse_stdin`

  * :ref:`parse_string`

Parser Functions
----------------

parse_file
..........

.. doxygenfunction:: tinytc::parse_file

parse_stdin
...........

.. doxygenfunction:: tinytc::parse_stdin

parse_string
............

.. doxygenfunction:: tinytc::parse_string

Recipe
======

* Enumerations

  * :ref:`mem_type`

* Functions

  * :ref:`make_small_gemm_batched`

  * :ref:`make_tall_and_skinny`

  * :ref:`make_tall_and_skinny_specialized`

* Classes

  * :ref:`recipe`

  * :ref:`recipe_handler`

  * :ref:`small_gemm_batched`

  * :ref:`tall_and_skinny`

* Structures

  * :ref:`auto_mem_type`

  * :ref:`auto_mem_type\<T, std::enable_if_t\<usm_pointer_type\<T\>\>\>`

  * :ref:`mem`

* Variables

  * :ref:`auto_mem_type_v`

  * :ref:`usm_pointer_type`

Recipe Enumerations
-------------------

mem_type
........

.. doxygenenum:: tinytc::mem_type

Recipe Functions
----------------

make_small_gemm_batched
.......................

.. doxygenfunction:: tinytc::make_small_gemm_batched

make_tall_and_skinny
....................

.. doxygenfunction:: tinytc::make_tall_and_skinny

make_tall_and_skinny_specialized
................................

.. doxygenfunction:: tinytc::make_tall_and_skinny_specialized

Recipe Classes
--------------

recipe
......

.. doxygenclass:: tinytc::recipe

recipe_handler
..............

.. doxygenclass:: tinytc::recipe_handler

small_gemm_batched
..................

.. doxygenclass:: tinytc::small_gemm_batched

tall_and_skinny
...............

.. doxygenclass:: tinytc::tall_and_skinny

Recipe Structures
-----------------

auto_mem_type
.............

.. doxygenstruct:: tinytc::auto_mem_type

auto_mem_type<T, std::enable_if_t<usm_pointer_type<T>>>
.......................................................

.. doxygenstruct:: tinytc::auto_mem_type< T, std::enable_if_t< usm_pointer_type< T > > >

mem
...

.. doxygenstruct:: tinytc::mem

Recipe Variables
----------------

auto_mem_type_v
...............

.. doxygenvariable:: tinytc::auto_mem_type_v

usm_pointer_type
................

.. doxygenvariable:: tinytc::usm_pointer_type

Source
======

* Classes

  * :ref:`source`

Source Classes
--------------

source
......

.. doxygenclass:: tinytc::source

