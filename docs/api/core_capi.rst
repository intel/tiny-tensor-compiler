.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

==========
Core C-API
==========

Common
======

* Enumerations

  * :ref:`tinytc_status_t`

* Definitions

  * :ref:`TINYTC_VERSION_MAJOR`

  * :ref:`TINYTC_VERSION_MINOR`

  * :ref:`TINYTC_VERSION_PATCH`

  * :ref:`TINYTC_VERSION_HASH`

  * :ref:`TINYTC_VERSION_NUMBER_OF_COMMITS_SINCE_RELEASE`

  * :ref:`TINYTC_VERSION_DESCRIPTION`

* Functions

  * :ref:`tinytc_error_string`

  * :ref:`tinytc_string_destroy`

* Typedefs

  * :ref:`tinytc_binary_t`

  * :ref:`tinytc_bool_t`

  * :ref:`tinytc_core_info_t`

  * :ref:`tinytc_recipe_t`

  * :ref:`tinytc_recipe_handler_t`

  * :ref:`tinytc_source_t`

  * :ref:`tinytc_source_context_t`

  * :ref:`const_tinytc_binary_t`

  * :ref:`const_tinytc_core_info_t`

  * :ref:`const_tinytc_recipe_t`

  * :ref:`const_tinytc_recipe_handler_t`

  * :ref:`const_tinytc_source_t`

  * :ref:`const_tinytc_source_context_t`

Common Enums
------------

tinytc_status_t
...............

.. doxygenenum:: tinytc_status_t

Common Defines
--------------

TINYTC_VERSION_MAJOR
....................

.. doxygendefine:: TINYTC_VERSION_MAJOR

TINYTC_VERSION_MINOR
....................

.. doxygendefine:: TINYTC_VERSION_MINOR

TINYTC_VERSION_PATCH
....................

.. doxygendefine:: TINYTC_VERSION_PATCH

TINYTC_VERSION_HASH
...................

.. doxygendefine:: TINYTC_VERSION_HASH

TINYTC_VERSION_NUMBER_OF_COMMITS_SINCE_RELEASE
..............................................

.. doxygendefine:: TINYTC_VERSION_NUMBER_OF_COMMITS_SINCE_RELEASE

TINYTC_VERSION_DESCRIPTION
..........................

.. doxygendefine:: TINYTC_VERSION_DESCRIPTION

Common Functions
----------------

tinytc_error_string
...................

.. doxygenfunction:: tinytc_error_string

tinytc_string_destroy
.....................

.. doxygenfunction:: tinytc_string_destroy

Common Typedefs
---------------

tinytc_binary_t
...............

.. doxygentypedef:: tinytc_binary_t

tinytc_bool_t
.............

.. doxygentypedef:: tinytc_bool_t

tinytc_core_info_t
..................

.. doxygentypedef:: tinytc_core_info_t

tinytc_recipe_t
...............

.. doxygentypedef:: tinytc_recipe_t

tinytc_recipe_handler_t
.......................

.. doxygentypedef:: tinytc_recipe_handler_t

tinytc_source_t
...............

.. doxygentypedef:: tinytc_source_t

tinytc_source_context_t
.......................

.. doxygentypedef:: tinytc_source_context_t

const_tinytc_binary_t
.....................

.. doxygentypedef:: const_tinytc_binary_t

const_tinytc_core_info_t
........................

.. doxygentypedef:: const_tinytc_core_info_t

const_tinytc_recipe_t
.....................

.. doxygentypedef:: const_tinytc_recipe_t

const_tinytc_recipe_handler_t
.............................

.. doxygentypedef:: const_tinytc_recipe_handler_t

const_tinytc_source_t
.....................

.. doxygentypedef:: const_tinytc_source_t

const_tinytc_source_context_t
.............................

.. doxygentypedef:: const_tinytc_source_context_t

Binary
======

* Functions

  * :ref:`tinytc_binary_get_core_features`

  * :ref:`tinytc_binary_get_raw`

  * :ref:`tinytc_binary_release`

  * :ref:`tinytc_binary_retain`

Binary Functions
----------------

tinytc_binary_get_core_features
...............................

.. doxygenfunction:: tinytc_binary_get_core_features

tinytc_binary_get_raw
.....................

.. doxygenfunction:: tinytc_binary_get_raw

tinytc_binary_release
.....................

.. doxygenfunction:: tinytc_binary_release

tinytc_binary_retain
....................

.. doxygenfunction:: tinytc_binary_retain

Compiler
========

* Enumerations

  * :ref:`tinytc_bundle_format_t`

* Functions

  * :ref:`tinytc_prog_compile_to_binary`

  * :ref:`tinytc_prog_compile_to_opencl`

  * :ref:`tinytc_source_compile_to_binary`

Compiler Enums
--------------

tinytc_bundle_format_t
......................

.. doxygenenum:: tinytc_bundle_format_t

Compiler Functions
------------------

tinytc_prog_compile_to_binary
.............................

.. doxygenfunction:: tinytc_prog_compile_to_binary

tinytc_prog_compile_to_opencl
.............................

.. doxygenfunction:: tinytc_prog_compile_to_opencl

tinytc_source_compile_to_binary
...............................

.. doxygenfunction:: tinytc_source_compile_to_binary

Device Info
===========

* Enumerations

  * :ref:`tinytc_core_feature_flag_t`

  * :ref:`tinytc_intel_gpu_architecture_t`

* Functions

  * :ref:`tinytc_core_info_get_ip_version`

  * :ref:`tinytc_core_info_get_num_registers_per_thread`

  * :ref:`tinytc_core_info_get_register_size`

  * :ref:`tinytc_core_info_get_subgroup_sizes`

  * :ref:`tinytc_core_info_clear_core_feature`

  * :ref:`tinytc_core_info_set_core_feature`

  * :ref:`tinytc_core_info_intel_create`

  * :ref:`tinytc_core_info_intel_create_from_arch`

  * :ref:`tinytc_core_info_release`

  * :ref:`tinytc_core_info_retain`

Device Info Enums
-----------------

tinytc_core_feature_flag_t
..........................

.. doxygenenum:: tinytc_core_feature_flag_t

tinytc_intel_gpu_architecture_t
...............................

.. doxygenenum:: tinytc_intel_gpu_architecture_t

Device Info Functions
---------------------

tinytc_core_info_get_ip_version
...............................

.. doxygenfunction:: tinytc_core_info_get_ip_version

tinytc_core_info_get_num_registers_per_thread
.............................................

.. doxygenfunction:: tinytc_core_info_get_num_registers_per_thread

tinytc_core_info_get_register_size
..................................

.. doxygenfunction:: tinytc_core_info_get_register_size

tinytc_core_info_get_subgroup_sizes
...................................

.. doxygenfunction:: tinytc_core_info_get_subgroup_sizes

tinytc_core_info_clear_core_feature
...................................

.. doxygenfunction:: tinytc_core_info_clear_core_feature

tinytc_core_info_set_core_feature
.................................

.. doxygenfunction:: tinytc_core_info_set_core_feature

tinytc_core_info_intel_create
.............................

.. doxygenfunction:: tinytc_core_info_intel_create

tinytc_core_info_intel_create_from_arch
.......................................

.. doxygenfunction:: tinytc_core_info_intel_create_from_arch

tinytc_core_info_release
........................

.. doxygenfunction:: tinytc_core_info_release

tinytc_core_info_retain
.......................

.. doxygenfunction:: tinytc_core_info_retain

Parser
======

* Functions

  * :ref:`tinytc_parse_file`

  * :ref:`tinytc_parse_stdin`

  * :ref:`tinytc_parse_string`

Parser Functions
----------------

tinytc_parse_file
.................

.. doxygenfunction:: tinytc_parse_file

tinytc_parse_stdin
..................

.. doxygenfunction:: tinytc_parse_stdin

tinytc_parse_string
...................

.. doxygenfunction:: tinytc_parse_string

Recipe
======

* Enumerations

  * :ref:`tinytc_mem_type_t`

* Functions

  * :ref:`tinytc_recipe_get_binary`

  * :ref:`tinytc_recipe_get_prog`

  * :ref:`tinytc_recipe_handler_get_recipe`

  * :ref:`tinytc_recipe_small_gemm_batched_create`

  * :ref:`tinytc_recipe_small_gemm_batched_set_args`

  * :ref:`tinytc_recipe_tall_and_skinny_create`

  * :ref:`tinytc_recipe_tall_and_skinny_set_args`

  * :ref:`tinytc_recipe_tall_and_skinny_suggest_block_size`

  * :ref:`tinytc_recipe_release`

  * :ref:`tinytc_recipe_retain`

  * :ref:`tinytc_recipe_handler_release`

  * :ref:`tinytc_recipe_handler_retain`

* Structures

  * :ref:`tinytc_mem`

* Typedefs

  * :ref:`tinytc_mem_t`

Recipe Enums
------------

tinytc_mem_type_t
.................

.. doxygenenum:: tinytc_mem_type_t

Recipe Functions
----------------

tinytc_recipe_get_binary
........................

.. doxygenfunction:: tinytc_recipe_get_binary

tinytc_recipe_get_prog
......................

.. doxygenfunction:: tinytc_recipe_get_prog

tinytc_recipe_handler_get_recipe
................................

.. doxygenfunction:: tinytc_recipe_handler_get_recipe

tinytc_recipe_small_gemm_batched_create
.......................................

.. doxygenfunction:: tinytc_recipe_small_gemm_batched_create

tinytc_recipe_small_gemm_batched_set_args
.........................................

.. doxygenfunction:: tinytc_recipe_small_gemm_batched_set_args

tinytc_recipe_tall_and_skinny_create
....................................

.. doxygenfunction:: tinytc_recipe_tall_and_skinny_create

tinytc_recipe_tall_and_skinny_set_args
......................................

.. doxygenfunction:: tinytc_recipe_tall_and_skinny_set_args

tinytc_recipe_tall_and_skinny_suggest_block_size
................................................

.. doxygenfunction:: tinytc_recipe_tall_and_skinny_suggest_block_size

tinytc_recipe_release
.....................

.. doxygenfunction:: tinytc_recipe_release

tinytc_recipe_retain
....................

.. doxygenfunction:: tinytc_recipe_retain

tinytc_recipe_handler_release
.............................

.. doxygenfunction:: tinytc_recipe_handler_release

tinytc_recipe_handler_retain
............................

.. doxygenfunction:: tinytc_recipe_handler_retain

Recipe Structs
--------------

tinytc_mem
..........

.. doxygenstruct:: tinytc_mem

Recipe Typedefs
---------------

tinytc_mem_t
............

.. doxygentypedef:: tinytc_mem_t

Source
======

* Functions

  * :ref:`tinytc_source_get_code`

  * :ref:`tinytc_source_release`

  * :ref:`tinytc_source_retain`

Source Functions
----------------

tinytc_source_get_code
......................

.. doxygenfunction:: tinytc_source_get_code

tinytc_source_release
.....................

.. doxygenfunction:: tinytc_source_release

tinytc_source_retain
....................

.. doxygenfunction:: tinytc_source_retain

Source Context
==============

* Functions

  * :ref:`tinytc_source_context_create`

  * :ref:`tinytc_source_context_add_source`

  * :ref:`tinytc_source_context_get_error_log`

  * :ref:`tinytc_source_context_report_error`

  * :ref:`tinytc_source_context_release`

  * :ref:`tinytc_source_context_retain`

Source Context Functions
------------------------

tinytc_source_context_create
............................

.. doxygenfunction:: tinytc_source_context_create

tinytc_source_context_add_source
................................

.. doxygenfunction:: tinytc_source_context_add_source

tinytc_source_context_get_error_log
...................................

.. doxygenfunction:: tinytc_source_context_get_error_log

tinytc_source_context_report_error
..................................

.. doxygenfunction:: tinytc_source_context_report_error

tinytc_source_context_release
.............................

.. doxygenfunction:: tinytc_source_context_release

tinytc_source_context_retain
............................

.. doxygenfunction:: tinytc_source_context_retain

