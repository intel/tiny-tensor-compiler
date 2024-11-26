.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

.. _Builder C-API:

=============
Builder C-API
=============

Common
======

* Enumerations

  * :ref:`tinytc_address_space_t`

  * :ref:`tinytc_arithmetic_t`

  * :ref:`tinytc_arithmetic_unary_t`

  * :ref:`tinytc_builtin_t`

  * :ref:`tinytc_checked_flag_t`

  * :ref:`tinytc_cmp_condition_t`

  * :ref:`tinytc_matrix_use_t`

  * :ref:`tinytc_scalar_type_t`

  * :ref:`tinytc_store_flag_t`

  * :ref:`tinytc_transpose_t`

  * :ref:`tinytc_work_group_operation_t`

* Definitions

  * :ref:`TINYTC_DYNAMIC`

* Functions

  * :ref:`tinytc_address_space_to_string`

  * :ref:`tinytc_arithmetic_to_string`

  * :ref:`tinytc_arithmetic_unary_to_string`

  * :ref:`tinytc_builtin_to_string`

  * :ref:`tinytc_checked_flag_to_string`

  * :ref:`tinytc_cmp_condition_to_string`

  * :ref:`tinytc_matrix_use_to_string`

  * :ref:`tinytc_scalar_type_size`

  * :ref:`tinytc_scalar_type_to_string`

  * :ref:`tinytc_store_flag_to_string`

  * :ref:`tinytc_transpose_to_string`

  * :ref:`tinytc_work_group_operation_to_string`

* Structures

  * :ref:`tinytc_position`

  * :ref:`tinytc_location`

* Typedefs

  * :ref:`tinytc_data_type_t`

  * :ref:`tinytc_func_t`

  * :ref:`tinytc_location_t`

  * :ref:`tinytc_position_t`

  * :ref:`tinytc_prog_t`

  * :ref:`tinytc_inst_t`

  * :ref:`tinytc_region_t`

  * :ref:`tinytc_value_t`

  * :ref:`const_tinytc_data_type_t`

  * :ref:`const_tinytc_func_t`

  * :ref:`const_tinytc_inst_t`

  * :ref:`const_tinytc_prog_t`

  * :ref:`const_tinytc_region_t`

  * :ref:`const_tinytc_value_t`

Common Enumerations
-------------------

.. _tinytc_address_space_t:

tinytc_address_space_t
......................

.. doxygenenum:: tinytc_address_space_t

.. _tinytc_arithmetic_t:

tinytc_arithmetic_t
...................

.. doxygenenum:: tinytc_arithmetic_t

.. _tinytc_arithmetic_unary_t:

tinytc_arithmetic_unary_t
.........................

.. doxygenenum:: tinytc_arithmetic_unary_t

.. _tinytc_builtin_t:

tinytc_builtin_t
................

.. doxygenenum:: tinytc_builtin_t

.. _tinytc_checked_flag_t:

tinytc_checked_flag_t
.....................

.. doxygenenum:: tinytc_checked_flag_t

.. _tinytc_cmp_condition_t:

tinytc_cmp_condition_t
......................

.. doxygenenum:: tinytc_cmp_condition_t

.. _tinytc_matrix_use_t:

tinytc_matrix_use_t
...................

.. doxygenenum:: tinytc_matrix_use_t

.. _tinytc_scalar_type_t:

tinytc_scalar_type_t
....................

.. doxygenenum:: tinytc_scalar_type_t

.. _tinytc_store_flag_t:

tinytc_store_flag_t
...................

.. doxygenenum:: tinytc_store_flag_t

.. _tinytc_transpose_t:

tinytc_transpose_t
..................

.. doxygenenum:: tinytc_transpose_t

.. _tinytc_work_group_operation_t:

tinytc_work_group_operation_t
.............................

.. doxygenenum:: tinytc_work_group_operation_t

Common Definitions
------------------

.. _TINYTC_DYNAMIC:

TINYTC_DYNAMIC
..............

.. doxygendefine:: TINYTC_DYNAMIC

Common Functions
----------------

.. _tinytc_address_space_to_string:

tinytc_address_space_to_string
..............................

.. doxygenfunction:: tinytc_address_space_to_string

.. _tinytc_arithmetic_to_string:

tinytc_arithmetic_to_string
...........................

.. doxygenfunction:: tinytc_arithmetic_to_string

.. _tinytc_arithmetic_unary_to_string:

tinytc_arithmetic_unary_to_string
.................................

.. doxygenfunction:: tinytc_arithmetic_unary_to_string

.. _tinytc_builtin_to_string:

tinytc_builtin_to_string
........................

.. doxygenfunction:: tinytc_builtin_to_string

.. _tinytc_checked_flag_to_string:

tinytc_checked_flag_to_string
.............................

.. doxygenfunction:: tinytc_checked_flag_to_string

.. _tinytc_cmp_condition_to_string:

tinytc_cmp_condition_to_string
..............................

.. doxygenfunction:: tinytc_cmp_condition_to_string

.. _tinytc_matrix_use_to_string:

tinytc_matrix_use_to_string
...........................

.. doxygenfunction:: tinytc_matrix_use_to_string

.. _tinytc_scalar_type_size:

tinytc_scalar_type_size
.......................

.. doxygenfunction:: tinytc_scalar_type_size

.. _tinytc_scalar_type_to_string:

tinytc_scalar_type_to_string
............................

.. doxygenfunction:: tinytc_scalar_type_to_string

.. _tinytc_store_flag_to_string:

tinytc_store_flag_to_string
...........................

.. doxygenfunction:: tinytc_store_flag_to_string

.. _tinytc_transpose_to_string:

tinytc_transpose_to_string
..........................

.. doxygenfunction:: tinytc_transpose_to_string

.. _tinytc_work_group_operation_to_string:

tinytc_work_group_operation_to_string
.....................................

.. doxygenfunction:: tinytc_work_group_operation_to_string

Common Structures
-----------------

.. _tinytc_position:

tinytc_position
...............

.. doxygenstruct:: tinytc_position

.. _tinytc_location:

tinytc_location
...............

.. doxygenstruct:: tinytc_location

Common Typedefs
---------------

.. _tinytc_data_type_t:

tinytc_data_type_t
..................

.. doxygentypedef:: tinytc_data_type_t

.. _tinytc_func_t:

tinytc_func_t
.............

.. doxygentypedef:: tinytc_func_t

.. _tinytc_location_t:

tinytc_location_t
.................

.. doxygentypedef:: tinytc_location_t

.. _tinytc_position_t:

tinytc_position_t
.................

.. doxygentypedef:: tinytc_position_t

.. _tinytc_prog_t:

tinytc_prog_t
.............

.. doxygentypedef:: tinytc_prog_t

.. _tinytc_inst_t:

tinytc_inst_t
.............

.. doxygentypedef:: tinytc_inst_t

.. _tinytc_region_t:

tinytc_region_t
...............

.. doxygentypedef:: tinytc_region_t

.. _tinytc_value_t:

tinytc_value_t
..............

.. doxygentypedef:: tinytc_value_t

.. _const_tinytc_data_type_t:

const_tinytc_data_type_t
........................

.. doxygentypedef:: const_tinytc_data_type_t

.. _const_tinytc_func_t:

const_tinytc_func_t
...................

.. doxygentypedef:: const_tinytc_func_t

.. _const_tinytc_inst_t:

const_tinytc_inst_t
...................

.. doxygentypedef:: const_tinytc_inst_t

.. _const_tinytc_prog_t:

const_tinytc_prog_t
...................

.. doxygentypedef:: const_tinytc_prog_t

.. _const_tinytc_region_t:

const_tinytc_region_t
.....................

.. doxygentypedef:: const_tinytc_region_t

.. _const_tinytc_value_t:

const_tinytc_value_t
....................

.. doxygentypedef:: const_tinytc_value_t

Data Type
=========

* Functions

  * :ref:`tinytc_boolean_type_get`

  * :ref:`tinytc_coopmatrix_type_get`

  * :ref:`tinytc_group_type_get`

  * :ref:`tinytc_memref_type_get`

  * :ref:`tinytc_scalar_type_get`

Data Type Functions
-------------------

.. _tinytc_boolean_type_get:

tinytc_boolean_type_get
.......................

.. doxygenfunction:: tinytc_boolean_type_get

.. _tinytc_coopmatrix_type_get:

tinytc_coopmatrix_type_get
..........................

.. doxygenfunction:: tinytc_coopmatrix_type_get

.. _tinytc_group_type_get:

tinytc_group_type_get
.....................

.. doxygenfunction:: tinytc_group_type_get

.. _tinytc_memref_type_get:

tinytc_memref_type_get
......................

.. doxygenfunction:: tinytc_memref_type_get

.. _tinytc_scalar_type_get:

tinytc_scalar_type_get
......................

.. doxygenfunction:: tinytc_scalar_type_get

Function
========

* Functions

  * :ref:`tinytc_func_create`

  * :ref:`tinytc_func_set_subgroup_size`

  * :ref:`tinytc_func_set_work_group_size`

  * :ref:`tinytc_func_get_body`

  * :ref:`tinytc_func_destroy`

Function Functions
------------------

.. _tinytc_func_create:

tinytc_func_create
..................

.. doxygenfunction:: tinytc_func_create

.. _tinytc_func_set_subgroup_size:

tinytc_func_set_subgroup_size
.............................

.. doxygenfunction:: tinytc_func_set_subgroup_size

.. _tinytc_func_set_work_group_size:

tinytc_func_set_work_group_size
...............................

.. doxygenfunction:: tinytc_func_set_work_group_size

.. _tinytc_func_get_body:

tinytc_func_get_body
....................

.. doxygenfunction:: tinytc_func_get_body

.. _tinytc_func_destroy:

tinytc_func_destroy
...................

.. doxygenfunction:: tinytc_func_destroy

Instruction
===========

* Functions

  * :ref:`tinytc_alloca_inst_create`

  * :ref:`tinytc_axpby_inst_create`

  * :ref:`tinytc_arith_inst_create`

  * :ref:`tinytc_arith_unary_inst_create`

  * :ref:`tinytc_builtin_inst_create`

  * :ref:`tinytc_cast_inst_create`

  * :ref:`tinytc_cmp_inst_create`

  * :ref:`tinytc_constant_inst_create_boolean`

  * :ref:`tinytc_constant_inst_create_complex`

  * :ref:`tinytc_constant_inst_create_float`

  * :ref:`tinytc_constant_inst_create_int`

  * :ref:`tinytc_constant_inst_create_one`

  * :ref:`tinytc_constant_inst_create_zero`

  * :ref:`tinytc_cooperative_matrix_load_inst_create`

  * :ref:`tinytc_cooperative_matrix_mul_add_inst_create`

  * :ref:`tinytc_cooperative_matrix_scale_inst_create`

  * :ref:`tinytc_cooperative_matrix_store_inst_create`

  * :ref:`tinytc_expand_inst_create`

  * :ref:`tinytc_for_inst_create`

  * :ref:`tinytc_foreach_inst_create`

  * :ref:`tinytc_fuse_inst_create`

  * :ref:`tinytc_gemm_inst_create`

  * :ref:`tinytc_gemv_inst_create`

  * :ref:`tinytc_ger_inst_create`

  * :ref:`tinytc_hadamard_inst_create`

  * :ref:`tinytc_if_inst_create`

  * :ref:`tinytc_load_inst_create`

  * :ref:`tinytc_parallel_inst_create`

  * :ref:`tinytc_size_inst_create`

  * :ref:`tinytc_store_inst_create`

  * :ref:`tinytc_subview_inst_create`

  * :ref:`tinytc_sum_inst_create`

  * :ref:`tinytc_work_group_inst_create`

  * :ref:`tinytc_yield_inst_create`

  * :ref:`tinytc_inst_get_regions`

  * :ref:`tinytc_inst_get_values`

  * :ref:`tinytc_inst_destroy`

Instruction Functions
---------------------

.. _tinytc_alloca_inst_create:

tinytc_alloca_inst_create
.........................

.. doxygenfunction:: tinytc_alloca_inst_create

.. _tinytc_axpby_inst_create:

tinytc_axpby_inst_create
........................

.. doxygenfunction:: tinytc_axpby_inst_create

.. _tinytc_arith_inst_create:

tinytc_arith_inst_create
........................

.. doxygenfunction:: tinytc_arith_inst_create

.. _tinytc_arith_unary_inst_create:

tinytc_arith_unary_inst_create
..............................

.. doxygenfunction:: tinytc_arith_unary_inst_create

.. _tinytc_builtin_inst_create:

tinytc_builtin_inst_create
..........................

.. doxygenfunction:: tinytc_builtin_inst_create

.. _tinytc_cast_inst_create:

tinytc_cast_inst_create
.......................

.. doxygenfunction:: tinytc_cast_inst_create

.. _tinytc_cmp_inst_create:

tinytc_cmp_inst_create
......................

.. doxygenfunction:: tinytc_cmp_inst_create

.. _tinytc_constant_inst_create_boolean:

tinytc_constant_inst_create_boolean
...................................

.. doxygenfunction:: tinytc_constant_inst_create_boolean

.. _tinytc_constant_inst_create_complex:

tinytc_constant_inst_create_complex
...................................

.. doxygenfunction:: tinytc_constant_inst_create_complex

.. _tinytc_constant_inst_create_float:

tinytc_constant_inst_create_float
.................................

.. doxygenfunction:: tinytc_constant_inst_create_float

.. _tinytc_constant_inst_create_int:

tinytc_constant_inst_create_int
...............................

.. doxygenfunction:: tinytc_constant_inst_create_int

.. _tinytc_constant_inst_create_one:

tinytc_constant_inst_create_one
...............................

.. doxygenfunction:: tinytc_constant_inst_create_one

.. _tinytc_constant_inst_create_zero:

tinytc_constant_inst_create_zero
................................

.. doxygenfunction:: tinytc_constant_inst_create_zero

.. _tinytc_cooperative_matrix_load_inst_create:

tinytc_cooperative_matrix_load_inst_create
..........................................

.. doxygenfunction:: tinytc_cooperative_matrix_load_inst_create

.. _tinytc_cooperative_matrix_mul_add_inst_create:

tinytc_cooperative_matrix_mul_add_inst_create
.............................................

.. doxygenfunction:: tinytc_cooperative_matrix_mul_add_inst_create

.. _tinytc_cooperative_matrix_scale_inst_create:

tinytc_cooperative_matrix_scale_inst_create
...........................................

.. doxygenfunction:: tinytc_cooperative_matrix_scale_inst_create

.. _tinytc_cooperative_matrix_store_inst_create:

tinytc_cooperative_matrix_store_inst_create
...........................................

.. doxygenfunction:: tinytc_cooperative_matrix_store_inst_create

.. _tinytc_expand_inst_create:

tinytc_expand_inst_create
.........................

.. doxygenfunction:: tinytc_expand_inst_create

.. _tinytc_for_inst_create:

tinytc_for_inst_create
......................

.. doxygenfunction:: tinytc_for_inst_create

.. _tinytc_foreach_inst_create:

tinytc_foreach_inst_create
..........................

.. doxygenfunction:: tinytc_foreach_inst_create

.. _tinytc_fuse_inst_create:

tinytc_fuse_inst_create
.......................

.. doxygenfunction:: tinytc_fuse_inst_create

.. _tinytc_gemm_inst_create:

tinytc_gemm_inst_create
.......................

.. doxygenfunction:: tinytc_gemm_inst_create

.. _tinytc_gemv_inst_create:

tinytc_gemv_inst_create
.......................

.. doxygenfunction:: tinytc_gemv_inst_create

.. _tinytc_ger_inst_create:

tinytc_ger_inst_create
......................

.. doxygenfunction:: tinytc_ger_inst_create

.. _tinytc_hadamard_inst_create:

tinytc_hadamard_inst_create
...........................

.. doxygenfunction:: tinytc_hadamard_inst_create

.. _tinytc_if_inst_create:

tinytc_if_inst_create
.....................

.. doxygenfunction:: tinytc_if_inst_create

.. _tinytc_load_inst_create:

tinytc_load_inst_create
.......................

.. doxygenfunction:: tinytc_load_inst_create

.. _tinytc_parallel_inst_create:

tinytc_parallel_inst_create
...........................

.. doxygenfunction:: tinytc_parallel_inst_create

.. _tinytc_size_inst_create:

tinytc_size_inst_create
.......................

.. doxygenfunction:: tinytc_size_inst_create

.. _tinytc_store_inst_create:

tinytc_store_inst_create
........................

.. doxygenfunction:: tinytc_store_inst_create

.. _tinytc_subview_inst_create:

tinytc_subview_inst_create
..........................

.. doxygenfunction:: tinytc_subview_inst_create

.. _tinytc_sum_inst_create:

tinytc_sum_inst_create
......................

.. doxygenfunction:: tinytc_sum_inst_create

.. _tinytc_work_group_inst_create:

tinytc_work_group_inst_create
.............................

.. doxygenfunction:: tinytc_work_group_inst_create

.. _tinytc_yield_inst_create:

tinytc_yield_inst_create
........................

.. doxygenfunction:: tinytc_yield_inst_create

.. _tinytc_inst_get_regions:

tinytc_inst_get_regions
.......................

.. doxygenfunction:: tinytc_inst_get_regions

.. _tinytc_inst_get_values:

tinytc_inst_get_values
......................

.. doxygenfunction:: tinytc_inst_get_values

.. _tinytc_inst_destroy:

tinytc_inst_destroy
...................

.. doxygenfunction:: tinytc_inst_destroy

Program
=======

* Functions

  * :ref:`tinytc_prog_create`

  * :ref:`tinytc_prog_add_function`

  * :ref:`tinytc_prog_dump`

  * :ref:`tinytc_prog_get_compiler_context`

  * :ref:`tinytc_prog_print_to_file`

  * :ref:`tinytc_prog_print_to_string`

  * :ref:`tinytc_prog_release`

  * :ref:`tinytc_prog_retain`

Program Functions
-----------------

.. _tinytc_prog_create:

tinytc_prog_create
..................

.. doxygenfunction:: tinytc_prog_create

.. _tinytc_prog_add_function:

tinytc_prog_add_function
........................

.. doxygenfunction:: tinytc_prog_add_function

.. _tinytc_prog_dump:

tinytc_prog_dump
................

.. doxygenfunction:: tinytc_prog_dump

.. _tinytc_prog_get_compiler_context:

tinytc_prog_get_compiler_context
................................

.. doxygenfunction:: tinytc_prog_get_compiler_context

.. _tinytc_prog_print_to_file:

tinytc_prog_print_to_file
.........................

.. doxygenfunction:: tinytc_prog_print_to_file

.. _tinytc_prog_print_to_string:

tinytc_prog_print_to_string
...........................

.. doxygenfunction:: tinytc_prog_print_to_string

.. _tinytc_prog_release:

tinytc_prog_release
...................

.. doxygenfunction:: tinytc_prog_release

.. _tinytc_prog_retain:

tinytc_prog_retain
..................

.. doxygenfunction:: tinytc_prog_retain

Region
======

* Functions

  * :ref:`tinytc_region_add_instruction`

  * :ref:`tinytc_region_get_parameters`

Region Functions
----------------

.. _tinytc_region_add_instruction:

tinytc_region_add_instruction
.............................

.. doxygenfunction:: tinytc_region_add_instruction

.. _tinytc_region_get_parameters:

tinytc_region_get_parameters
............................

.. doxygenfunction:: tinytc_region_get_parameters

Value
=====

* Functions

  * :ref:`tinytc_value_get_name`

  * :ref:`tinytc_value_get_type`

  * :ref:`tinytc_value_set_name`

  * :ref:`tinytc_value_set_name_n`

Value Functions
---------------

.. _tinytc_value_get_name:

tinytc_value_get_name
.....................

.. doxygenfunction:: tinytc_value_get_name

.. _tinytc_value_get_type:

tinytc_value_get_type
.....................

.. doxygenfunction:: tinytc_value_get_type

.. _tinytc_value_set_name:

tinytc_value_set_name
.....................

.. doxygenfunction:: tinytc_value_set_name

.. _tinytc_value_set_name_n:

tinytc_value_set_name_n
.......................

.. doxygenfunction:: tinytc_value_set_name_n

