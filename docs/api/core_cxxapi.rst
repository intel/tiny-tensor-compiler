.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

============
Core C++-API
============

Common
======

* Functions

  * :ref:`error_string`

  * :ref:`CHECK_STATUS`

  * :ref:`CHECK_STATUS_LOC`

* Classes

  * :ref:`shared_handle`

  * :ref:`unique_handle`

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

Common Classs
-------------

shared_handle
.............

.. doxygenclass:: tinytc::shared_handle

unique_handle
.............

.. doxygenclass:: tinytc::unique_handle

Binary
======

* Classes

  * :ref:`binary`

Binary Classs
-------------

binary
......

.. doxygenclass:: tinytc::binary

Compiler
========

* Functions

  * :ref:`compile_to_opencl`

  * :ref:`compile_to_binary`

  * :ref:`compile_to_binary`

Compiler Functions
------------------

compile_to_opencl
.................

.. doxygenfunction:: tinytc::compile_to_opencl

compile_to_binary
.................

.. doxygenfunction:: tinytc::compile_to_binary

compile_to_binary
.................

.. doxygenfunction:: tinytc::compile_to_binary

Device Info
===========

* Functions

  * :ref:`create_core_info_intel`

  * :ref:`create_core_info_intel_from_arch`

* Classes

  * :ref:`core_info`

Device Info Functions
---------------------

create_core_info_intel
......................

.. doxygenfunction:: tinytc::create_core_info_intel

create_core_info_intel_from_arch
................................

.. doxygenfunction:: tinytc::create_core_info_intel_from_arch

Device Info Classs
------------------

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

* Classes

  * :ref:`recipe`

  * :ref:`recipe_handler`

  * :ref:`small_gemm_batched`

  * :ref:`tall_and_skinny`

* Structures

  * :ref:`auto_mem_type`

  * :ref:`mem`

* Variables

  * :ref:`auto_mem_type_v`

* Concepts

  * :ref:`pointer_to_scalar`

Recipe Classs
-------------

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

Recipe Structs
--------------

auto_mem_type
.............

.. doxygenstruct:: tinytc::auto_mem_type

mem
...

.. doxygenstruct:: tinytc::mem

Recipe Variables
----------------

auto_mem_type_v
...............

.. doxygenvariable:: tinytc::auto_mem_type_v

Recipe Concepts
---------------

pointer_to_scalar
.................

.. doxygenconcept:: tinytc::pointer_to_scalar

Source
======

* Classes

  * :ref:`source`

Source Classs
-------------

source
......

.. doxygenclass:: tinytc::source

Source Context
==============

* Functions

  * :ref:`create_source_context`

* Classes

  * :ref:`source_context`

Source Context Functions
------------------------

create_source_context
.....................

.. doxygenfunction:: tinytc::create_source_context

Source Context Classs
---------------------

source_context
..............

.. doxygenclass:: tinytc::source_context

