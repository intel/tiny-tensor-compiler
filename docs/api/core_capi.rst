.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

.. _Core C-API:

==========
Core C-API
==========

Common
======

* Enumerations

  * :ref:`tinytc_status_t`

  * :ref:`tinytc_support_level_t`

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

  * :ref:`tinytc_spv_mod_t`

  * :ref:`tinytc_compiler_context_t`

  * :ref:`const_tinytc_binary_t`

  * :ref:`const_tinytc_core_info_t`

  * :ref:`const_tinytc_recipe_t`

  * :ref:`const_tinytc_recipe_handler_t`

  * :ref:`const_tinytc_spv_mod_t`

  * :ref:`const_tinytc_compiler_context_t`

  * :ref:`tinytc_error_reporter_t`

Common Enumerations
-------------------

.. _tinytc_status_t:

tinytc_status_t
...............

.. doxygenenum:: tinytc_status_t

.. _tinytc_support_level_t:

tinytc_support_level_t
......................

.. doxygenenum:: tinytc_support_level_t

Common Definitions
------------------

.. _TINYTC_VERSION_MAJOR:

TINYTC_VERSION_MAJOR
....................

.. doxygendefine:: TINYTC_VERSION_MAJOR

.. _TINYTC_VERSION_MINOR:

TINYTC_VERSION_MINOR
....................

.. doxygendefine:: TINYTC_VERSION_MINOR

.. _TINYTC_VERSION_PATCH:

TINYTC_VERSION_PATCH
....................

.. doxygendefine:: TINYTC_VERSION_PATCH

.. _TINYTC_VERSION_HASH:

TINYTC_VERSION_HASH
...................

.. doxygendefine:: TINYTC_VERSION_HASH

.. _TINYTC_VERSION_NUMBER_OF_COMMITS_SINCE_RELEASE:

TINYTC_VERSION_NUMBER_OF_COMMITS_SINCE_RELEASE
..............................................

.. doxygendefine:: TINYTC_VERSION_NUMBER_OF_COMMITS_SINCE_RELEASE

.. _TINYTC_VERSION_DESCRIPTION:

TINYTC_VERSION_DESCRIPTION
..........................

.. doxygendefine:: TINYTC_VERSION_DESCRIPTION

Common Functions
----------------

.. _tinytc_error_string:

tinytc_error_string
...................

.. doxygenfunction:: tinytc_error_string

.. _tinytc_string_destroy:

tinytc_string_destroy
.....................

.. doxygenfunction:: tinytc_string_destroy

Common Typedefs
---------------

.. _tinytc_binary_t:

tinytc_binary_t
...............

.. doxygentypedef:: tinytc_binary_t

.. _tinytc_bool_t:

tinytc_bool_t
.............

.. doxygentypedef:: tinytc_bool_t

.. _tinytc_core_info_t:

tinytc_core_info_t
..................

.. doxygentypedef:: tinytc_core_info_t

.. _tinytc_recipe_t:

tinytc_recipe_t
...............

.. doxygentypedef:: tinytc_recipe_t

.. _tinytc_recipe_handler_t:

tinytc_recipe_handler_t
.......................

.. doxygentypedef:: tinytc_recipe_handler_t

.. _tinytc_spv_mod_t:

tinytc_spv_mod_t
................

.. doxygentypedef:: tinytc_spv_mod_t

.. _tinytc_compiler_context_t:

tinytc_compiler_context_t
.........................

.. doxygentypedef:: tinytc_compiler_context_t

.. _const_tinytc_binary_t:

const_tinytc_binary_t
.....................

.. doxygentypedef:: const_tinytc_binary_t

.. _const_tinytc_core_info_t:

const_tinytc_core_info_t
........................

.. doxygentypedef:: const_tinytc_core_info_t

.. _const_tinytc_recipe_t:

const_tinytc_recipe_t
.....................

.. doxygentypedef:: const_tinytc_recipe_t

.. _const_tinytc_recipe_handler_t:

const_tinytc_recipe_handler_t
.............................

.. doxygentypedef:: const_tinytc_recipe_handler_t

.. _const_tinytc_spv_mod_t:

const_tinytc_spv_mod_t
......................

.. doxygentypedef:: const_tinytc_spv_mod_t

.. _const_tinytc_compiler_context_t:

const_tinytc_compiler_context_t
...............................

.. doxygentypedef:: const_tinytc_compiler_context_t

.. _tinytc_error_reporter_t:

tinytc_error_reporter_t
.......................

.. doxygentypedef:: tinytc_error_reporter_t

Binary
======

* Functions

  * :ref:`tinytc_binary_create`

  * :ref:`tinytc_binary_get_compiler_context`

  * :ref:`tinytc_binary_get_core_features`

  * :ref:`tinytc_binary_get_raw`

  * :ref:`tinytc_binary_release`

  * :ref:`tinytc_binary_retain`

Binary Functions
----------------

.. _tinytc_binary_create:

tinytc_binary_create
....................

.. doxygenfunction:: tinytc_binary_create

.. _tinytc_binary_get_compiler_context:

tinytc_binary_get_compiler_context
..................................

.. doxygenfunction:: tinytc_binary_get_compiler_context

.. _tinytc_binary_get_core_features:

tinytc_binary_get_core_features
...............................

.. doxygenfunction:: tinytc_binary_get_core_features

.. _tinytc_binary_get_raw:

tinytc_binary_get_raw
.....................

.. doxygenfunction:: tinytc_binary_get_raw

.. _tinytc_binary_release:

tinytc_binary_release
.....................

.. doxygenfunction:: tinytc_binary_release

.. _tinytc_binary_retain:

tinytc_binary_retain
....................

.. doxygenfunction:: tinytc_binary_retain

Compiler
========

* Enumerations

  * :ref:`tinytc_bundle_format_t`

  * :ref:`tinytc_optflag_t`

* Functions

  * :ref:`tinytc_run_function_pass`

  * :ref:`tinytc_list_function_passes`

  * :ref:`tinytc_prog_compile_to_spirv`

  * :ref:`tinytc_prog_compile_to_spirv_and_assemble`

  * :ref:`tinytc_spirv_assemble`

Compiler Enumerations
---------------------

.. _tinytc_bundle_format_t:

tinytc_bundle_format_t
......................

.. doxygenenum:: tinytc_bundle_format_t

.. _tinytc_optflag_t:

tinytc_optflag_t
................

.. doxygenenum:: tinytc_optflag_t

Compiler Functions
------------------

.. _tinytc_run_function_pass:

tinytc_run_function_pass
........................

.. doxygenfunction:: tinytc_run_function_pass

.. _tinytc_list_function_passes:

tinytc_list_function_passes
...........................

.. doxygenfunction:: tinytc_list_function_passes

.. _tinytc_prog_compile_to_spirv:

tinytc_prog_compile_to_spirv
............................

.. doxygenfunction:: tinytc_prog_compile_to_spirv

.. _tinytc_prog_compile_to_spirv_and_assemble:

tinytc_prog_compile_to_spirv_and_assemble
.........................................

.. doxygenfunction:: tinytc_prog_compile_to_spirv_and_assemble

.. _tinytc_spirv_assemble:

tinytc_spirv_assemble
.....................

.. doxygenfunction:: tinytc_spirv_assemble

Compiler Context
================

* Functions

  * :ref:`tinytc_compiler_context_create`

  * :ref:`tinytc_compiler_context_add_source`

  * :ref:`tinytc_compiler_context_set_error_reporter`

  * :ref:`tinytc_compiler_context_set_optimization_flag`

  * :ref:`tinytc_compiler_context_set_optimization_level`

  * :ref:`tinytc_compiler_context_report_error`

  * :ref:`tinytc_compiler_context_release`

  * :ref:`tinytc_compiler_context_retain`

Compiler Context Functions
--------------------------

.. _tinytc_compiler_context_create:

tinytc_compiler_context_create
..............................

.. doxygenfunction:: tinytc_compiler_context_create

.. _tinytc_compiler_context_add_source:

tinytc_compiler_context_add_source
..................................

.. doxygenfunction:: tinytc_compiler_context_add_source

.. _tinytc_compiler_context_set_error_reporter:

tinytc_compiler_context_set_error_reporter
..........................................

.. doxygenfunction:: tinytc_compiler_context_set_error_reporter

.. _tinytc_compiler_context_set_optimization_flag:

tinytc_compiler_context_set_optimization_flag
.............................................

.. doxygenfunction:: tinytc_compiler_context_set_optimization_flag

.. _tinytc_compiler_context_set_optimization_level:

tinytc_compiler_context_set_optimization_level
..............................................

.. doxygenfunction:: tinytc_compiler_context_set_optimization_level

.. _tinytc_compiler_context_report_error:

tinytc_compiler_context_report_error
....................................

.. doxygenfunction:: tinytc_compiler_context_report_error

.. _tinytc_compiler_context_release:

tinytc_compiler_context_release
...............................

.. doxygenfunction:: tinytc_compiler_context_release

.. _tinytc_compiler_context_retain:

tinytc_compiler_context_retain
..............................

.. doxygenfunction:: tinytc_compiler_context_retain

Device Info
===========

* Enumerations

  * :ref:`tinytc_core_feature_flag_t`

  * :ref:`tinytc_intel_gpu_architecture_t`

  * :ref:`tinytc_spirv_feature_t`

* Functions

  * :ref:`tinytc_core_info_generic_create`

  * :ref:`tinytc_core_info_get_core_features`

  * :ref:`tinytc_core_info_get_register_space`

  * :ref:`tinytc_core_info_get_subgroup_sizes`

  * :ref:`tinytc_core_info_have_spirv_feature`

  * :ref:`tinytc_core_info_intel_create`

  * :ref:`tinytc_core_info_intel_create_from_arch`

  * :ref:`tinytc_core_info_intel_create_from_name`

  * :ref:`tinytc_core_info_release`

  * :ref:`tinytc_core_info_retain`

  * :ref:`tinytc_core_info_set_core_features`

  * :ref:`tinytc_core_info_set_spirv_feature`

  * :ref:`tinytc_spirv_feature_to_string`

* Typedefs

  * :ref:`tinytc_core_feature_flags_t`

Device Info Enumerations
------------------------

.. _tinytc_core_feature_flag_t:

tinytc_core_feature_flag_t
..........................

.. doxygenenum:: tinytc_core_feature_flag_t

.. _tinytc_intel_gpu_architecture_t:

tinytc_intel_gpu_architecture_t
...............................

.. doxygenenum:: tinytc_intel_gpu_architecture_t

.. _tinytc_spirv_feature_t:

tinytc_spirv_feature_t
......................

.. doxygenenum:: tinytc_spirv_feature_t

Device Info Functions
---------------------

.. _tinytc_core_info_generic_create:

tinytc_core_info_generic_create
...............................

.. doxygenfunction:: tinytc_core_info_generic_create

.. _tinytc_core_info_get_core_features:

tinytc_core_info_get_core_features
..................................

.. doxygenfunction:: tinytc_core_info_get_core_features

.. _tinytc_core_info_get_register_space:

tinytc_core_info_get_register_space
...................................

.. doxygenfunction:: tinytc_core_info_get_register_space

.. _tinytc_core_info_get_subgroup_sizes:

tinytc_core_info_get_subgroup_sizes
...................................

.. doxygenfunction:: tinytc_core_info_get_subgroup_sizes

.. _tinytc_core_info_have_spirv_feature:

tinytc_core_info_have_spirv_feature
...................................

.. doxygenfunction:: tinytc_core_info_have_spirv_feature

.. _tinytc_core_info_intel_create:

tinytc_core_info_intel_create
.............................

.. doxygenfunction:: tinytc_core_info_intel_create

.. _tinytc_core_info_intel_create_from_arch:

tinytc_core_info_intel_create_from_arch
.......................................

.. doxygenfunction:: tinytc_core_info_intel_create_from_arch

.. _tinytc_core_info_intel_create_from_name:

tinytc_core_info_intel_create_from_name
.......................................

.. doxygenfunction:: tinytc_core_info_intel_create_from_name

.. _tinytc_core_info_release:

tinytc_core_info_release
........................

.. doxygenfunction:: tinytc_core_info_release

.. _tinytc_core_info_retain:

tinytc_core_info_retain
.......................

.. doxygenfunction:: tinytc_core_info_retain

.. _tinytc_core_info_set_core_features:

tinytc_core_info_set_core_features
..................................

.. doxygenfunction:: tinytc_core_info_set_core_features

.. _tinytc_core_info_set_spirv_feature:

tinytc_core_info_set_spirv_feature
..................................

.. doxygenfunction:: tinytc_core_info_set_spirv_feature

.. _tinytc_spirv_feature_to_string:

tinytc_spirv_feature_to_string
..............................

.. doxygenfunction:: tinytc_spirv_feature_to_string

Device Info Typedefs
--------------------

.. _tinytc_core_feature_flags_t:

tinytc_core_feature_flags_t
...........................

.. doxygentypedef:: tinytc_core_feature_flags_t

FP math
=======

* Functions

  * :ref:`tinytc_f32_to_bf16_as_ui16`

  * :ref:`tinytc_f32_to_f16_as_ui16`

  * :ref:`tinytc_f16_as_ui16_to_f32`

  * :ref:`tinytc_bf16_as_ui16_to_f32`

FP math Functions
-----------------

.. _tinytc_f32_to_bf16_as_ui16:

tinytc_f32_to_bf16_as_ui16
..........................

.. doxygenfunction:: tinytc_f32_to_bf16_as_ui16

.. _tinytc_f32_to_f16_as_ui16:

tinytc_f32_to_f16_as_ui16
.........................

.. doxygenfunction:: tinytc_f32_to_f16_as_ui16

.. _tinytc_f16_as_ui16_to_f32:

tinytc_f16_as_ui16_to_f32
.........................

.. doxygenfunction:: tinytc_f16_as_ui16_to_f32

.. _tinytc_bf16_as_ui16_to_f32:

tinytc_bf16_as_ui16_to_f32
..........................

.. doxygenfunction:: tinytc_bf16_as_ui16_to_f32

Parser
======

* Functions

  * :ref:`tinytc_parse_file`

  * :ref:`tinytc_parse_stdin`

  * :ref:`tinytc_parse_string`

Parser Functions
----------------

.. _tinytc_parse_file:

tinytc_parse_file
.................

.. doxygenfunction:: tinytc_parse_file

.. _tinytc_parse_stdin:

tinytc_parse_stdin
..................

.. doxygenfunction:: tinytc_parse_stdin

.. _tinytc_parse_string:

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

  * :ref:`tinytc_recipe_tall_and_skinny_create_specialized`

  * :ref:`tinytc_recipe_tall_and_skinny_set_args`

  * :ref:`tinytc_recipe_tall_and_skinny_suggest_block_size`

  * :ref:`tinytc_recipe_release`

  * :ref:`tinytc_recipe_retain`

  * :ref:`tinytc_recipe_handler_release`

  * :ref:`tinytc_recipe_handler_retain`

Recipe Enumerations
-------------------

.. _tinytc_mem_type_t:

tinytc_mem_type_t
.................

.. doxygenenum:: tinytc_mem_type_t

Recipe Functions
----------------

.. _tinytc_recipe_get_binary:

tinytc_recipe_get_binary
........................

.. doxygenfunction:: tinytc_recipe_get_binary

.. _tinytc_recipe_get_prog:

tinytc_recipe_get_prog
......................

.. doxygenfunction:: tinytc_recipe_get_prog

.. _tinytc_recipe_handler_get_recipe:

tinytc_recipe_handler_get_recipe
................................

.. doxygenfunction:: tinytc_recipe_handler_get_recipe

.. _tinytc_recipe_small_gemm_batched_create:

tinytc_recipe_small_gemm_batched_create
.......................................

.. doxygenfunction:: tinytc_recipe_small_gemm_batched_create

.. _tinytc_recipe_small_gemm_batched_set_args:

tinytc_recipe_small_gemm_batched_set_args
.........................................

.. doxygenfunction:: tinytc_recipe_small_gemm_batched_set_args

.. _tinytc_recipe_tall_and_skinny_create:

tinytc_recipe_tall_and_skinny_create
....................................

.. doxygenfunction:: tinytc_recipe_tall_and_skinny_create

.. _tinytc_recipe_tall_and_skinny_create_specialized:

tinytc_recipe_tall_and_skinny_create_specialized
................................................

.. doxygenfunction:: tinytc_recipe_tall_and_skinny_create_specialized

.. _tinytc_recipe_tall_and_skinny_set_args:

tinytc_recipe_tall_and_skinny_set_args
......................................

.. doxygenfunction:: tinytc_recipe_tall_and_skinny_set_args

.. _tinytc_recipe_tall_and_skinny_suggest_block_size:

tinytc_recipe_tall_and_skinny_suggest_block_size
................................................

.. doxygenfunction:: tinytc_recipe_tall_and_skinny_suggest_block_size

.. _tinytc_recipe_release:

tinytc_recipe_release
.....................

.. doxygenfunction:: tinytc_recipe_release

.. _tinytc_recipe_retain:

tinytc_recipe_retain
....................

.. doxygenfunction:: tinytc_recipe_retain

.. _tinytc_recipe_handler_release:

tinytc_recipe_handler_release
.............................

.. doxygenfunction:: tinytc_recipe_handler_release

.. _tinytc_recipe_handler_retain:

tinytc_recipe_handler_retain
............................

.. doxygenfunction:: tinytc_recipe_handler_retain

SPIR-V module
=============

* Functions

  * :ref:`tinytc_spv_mod_dump`

  * :ref:`tinytc_spv_mod_print_to_file`

  * :ref:`tinytc_spv_mod_print_to_string`

  * :ref:`tinytc_spv_mod_release`

  * :ref:`tinytc_spv_mod_retain`

SPIR-V module Functions
-----------------------

.. _tinytc_spv_mod_dump:

tinytc_spv_mod_dump
...................

.. doxygenfunction:: tinytc_spv_mod_dump

.. _tinytc_spv_mod_print_to_file:

tinytc_spv_mod_print_to_file
............................

.. doxygenfunction:: tinytc_spv_mod_print_to_file

.. _tinytc_spv_mod_print_to_string:

tinytc_spv_mod_print_to_string
..............................

.. doxygenfunction:: tinytc_spv_mod_print_to_string

.. _tinytc_spv_mod_release:

tinytc_spv_mod_release
......................

.. doxygenfunction:: tinytc_spv_mod_release

.. _tinytc_spv_mod_retain:

tinytc_spv_mod_retain
.....................

.. doxygenfunction:: tinytc_spv_mod_retain

