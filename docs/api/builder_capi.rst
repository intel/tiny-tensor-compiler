.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

=============
Builder C-API
=============

Common
======

* Enumerations

  * :ref:`tinytc_address_space_t`

  * :ref:`tinytc_arithmetic_t`

  * :ref:`tinytc_arithmetic_unary_t`

  * :ref:`tinytc_cmp_condition_t`

  * :ref:`tinytc_scalar_type_t`

  * :ref:`tinytc_transpose_t`

* Definitions

  * :ref:`TINYTC_DYNAMIC`

* Functions

  * :ref:`tinytc_address_space_to_string`

  * :ref:`tinytc_arithmetic_to_string`

  * :ref:`tinytc_arithmetic_unary_to_string`

  * :ref:`tinytc_cmp_condition_to_string`

  * :ref:`tinytc_scalar_type_size`

  * :ref:`tinytc_scalar_type_to_string`

  * :ref:`tinytc_transpose_to_string`

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

Common Enumerations
-------------------

tinytc_address_space_t
......................

.. doxygenenum:: tinytc_address_space_t

tinytc_arithmetic_t
...................

.. doxygenenum:: tinytc_arithmetic_t

tinytc_arithmetic_unary_t
.........................

.. doxygenenum:: tinytc_arithmetic_unary_t

tinytc_cmp_condition_t
......................

.. doxygenenum:: tinytc_cmp_condition_t

tinytc_scalar_type_t
....................

.. doxygenenum:: tinytc_scalar_type_t

tinytc_transpose_t
..................

.. doxygenenum:: tinytc_transpose_t

Common Definitions
------------------

TINYTC_DYNAMIC
..............

.. doxygendefine:: TINYTC_DYNAMIC

Common Functions
----------------

tinytc_address_space_to_string
..............................

.. doxygenfunction:: tinytc_address_space_to_string

tinytc_arithmetic_to_string
...........................

.. doxygenfunction:: tinytc_arithmetic_to_string

tinytc_arithmetic_unary_to_string
.................................

.. doxygenfunction:: tinytc_arithmetic_unary_to_string

tinytc_cmp_condition_to_string
..............................

.. doxygenfunction:: tinytc_cmp_condition_to_string

tinytc_scalar_type_size
.......................

.. doxygenfunction:: tinytc_scalar_type_size

tinytc_scalar_type_to_string
............................

.. doxygenfunction:: tinytc_scalar_type_to_string

tinytc_transpose_to_string
..........................

.. doxygenfunction:: tinytc_transpose_to_string

Common Structures
-----------------

tinytc_position
...............

.. doxygenstruct:: tinytc_position

tinytc_location
...............

.. doxygenstruct:: tinytc_location

Common Typedefs
---------------

tinytc_data_type_t
..................

.. doxygentypedef:: tinytc_data_type_t

tinytc_func_t
.............

.. doxygentypedef:: tinytc_func_t

tinytc_location_t
.................

.. doxygentypedef:: tinytc_location_t

tinytc_position_t
.................

.. doxygentypedef:: tinytc_position_t

tinytc_prog_t
.............

.. doxygentypedef:: tinytc_prog_t

tinytc_inst_t
.............

.. doxygentypedef:: tinytc_inst_t

tinytc_region_t
...............

.. doxygentypedef:: tinytc_region_t

tinytc_value_t
..............

.. doxygentypedef:: tinytc_value_t

const_tinytc_data_type_t
........................

.. doxygentypedef:: const_tinytc_data_type_t

const_tinytc_func_t
...................

.. doxygentypedef:: const_tinytc_func_t

const_tinytc_inst_t
...................

.. doxygentypedef:: const_tinytc_inst_t

const_tinytc_prog_t
...................

.. doxygentypedef:: const_tinytc_prog_t

const_tinytc_region_t
.....................

.. doxygentypedef:: const_tinytc_region_t

Data Type
=========

* Functions

  * :ref:`tinytc_group_type_get`

  * :ref:`tinytc_memref_type_get`

  * :ref:`tinytc_scalar_type_get`

Data Type Functions
-------------------

tinytc_group_type_get
.....................

.. doxygenfunction:: tinytc_group_type_get

tinytc_memref_type_get
......................

.. doxygenfunction:: tinytc_memref_type_get

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

tinytc_func_create
..................

.. doxygenfunction:: tinytc_func_create

tinytc_func_set_subgroup_size
.............................

.. doxygenfunction:: tinytc_func_set_subgroup_size

tinytc_func_set_work_group_size
...............................

.. doxygenfunction:: tinytc_func_set_work_group_size

tinytc_func_get_body
....................

.. doxygenfunction:: tinytc_func_get_body

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

  * :ref:`tinytc_cast_inst_create`

  * :ref:`tinytc_cmp_inst_create`

  * :ref:`tinytc_constant_inst_create_complex`

  * :ref:`tinytc_constant_inst_create_float`

  * :ref:`tinytc_constant_inst_create_int`

  * :ref:`tinytc_expand_inst_create`

  * :ref:`tinytc_for_inst_create`

  * :ref:`tinytc_foreach_inst_create`

  * :ref:`tinytc_fuse_inst_create`

  * :ref:`tinytc_gemm_inst_create`

  * :ref:`tinytc_gemv_inst_create`

  * :ref:`tinytc_ger_inst_create`

  * :ref:`tinytc_group_id_inst_create`

  * :ref:`tinytc_group_size_inst_create`

  * :ref:`tinytc_hadamard_inst_create`

  * :ref:`tinytc_if_inst_create`

  * :ref:`tinytc_load_inst_create`

  * :ref:`tinytc_num_subgroups_inst_create`

  * :ref:`tinytc_parallel_inst_create`

  * :ref:`tinytc_size_inst_create`

  * :ref:`tinytc_store_inst_create`

  * :ref:`tinytc_subgroup_id_inst_create`

  * :ref:`tinytc_subgroup_local_id_inst_create`

  * :ref:`tinytc_subgroup_size_inst_create`

  * :ref:`tinytc_subview_inst_create`

  * :ref:`tinytc_sum_inst_create`

  * :ref:`tinytc_yield_inst_create`

  * :ref:`tinytc_inst_get_region`

  * :ref:`tinytc_inst_get_regions`

  * :ref:`tinytc_inst_get_value`

  * :ref:`tinytc_inst_get_values`

  * :ref:`tinytc_inst_destroy`

Instruction Functions
---------------------

tinytc_alloca_inst_create
.........................

.. doxygenfunction:: tinytc_alloca_inst_create

tinytc_axpby_inst_create
........................

.. doxygenfunction:: tinytc_axpby_inst_create

tinytc_arith_inst_create
........................

.. doxygenfunction:: tinytc_arith_inst_create

tinytc_arith_unary_inst_create
..............................

.. doxygenfunction:: tinytc_arith_unary_inst_create

tinytc_cast_inst_create
.......................

.. doxygenfunction:: tinytc_cast_inst_create

tinytc_cmp_inst_create
......................

.. doxygenfunction:: tinytc_cmp_inst_create

tinytc_constant_inst_create_complex
...................................

.. doxygenfunction:: tinytc_constant_inst_create_complex

tinytc_constant_inst_create_float
.................................

.. doxygenfunction:: tinytc_constant_inst_create_float

tinytc_constant_inst_create_int
...............................

.. doxygenfunction:: tinytc_constant_inst_create_int

tinytc_expand_inst_create
.........................

.. doxygenfunction:: tinytc_expand_inst_create

tinytc_for_inst_create
......................

.. doxygenfunction:: tinytc_for_inst_create

tinytc_foreach_inst_create
..........................

.. doxygenfunction:: tinytc_foreach_inst_create

tinytc_fuse_inst_create
.......................

.. doxygenfunction:: tinytc_fuse_inst_create

tinytc_gemm_inst_create
.......................

.. doxygenfunction:: tinytc_gemm_inst_create

tinytc_gemv_inst_create
.......................

.. doxygenfunction:: tinytc_gemv_inst_create

tinytc_ger_inst_create
......................

.. doxygenfunction:: tinytc_ger_inst_create

tinytc_group_id_inst_create
...........................

.. doxygenfunction:: tinytc_group_id_inst_create

tinytc_group_size_inst_create
.............................

.. doxygenfunction:: tinytc_group_size_inst_create

tinytc_hadamard_inst_create
...........................

.. doxygenfunction:: tinytc_hadamard_inst_create

tinytc_if_inst_create
.....................

.. doxygenfunction:: tinytc_if_inst_create

tinytc_load_inst_create
.......................

.. doxygenfunction:: tinytc_load_inst_create

tinytc_num_subgroups_inst_create
................................

.. doxygenfunction:: tinytc_num_subgroups_inst_create

tinytc_parallel_inst_create
...........................

.. doxygenfunction:: tinytc_parallel_inst_create

tinytc_size_inst_create
.......................

.. doxygenfunction:: tinytc_size_inst_create

tinytc_store_inst_create
........................

.. doxygenfunction:: tinytc_store_inst_create

tinytc_subgroup_id_inst_create
..............................

.. doxygenfunction:: tinytc_subgroup_id_inst_create

tinytc_subgroup_local_id_inst_create
....................................

.. doxygenfunction:: tinytc_subgroup_local_id_inst_create

tinytc_subgroup_size_inst_create
................................

.. doxygenfunction:: tinytc_subgroup_size_inst_create

tinytc_subview_inst_create
..........................

.. doxygenfunction:: tinytc_subview_inst_create

tinytc_sum_inst_create
......................

.. doxygenfunction:: tinytc_sum_inst_create

tinytc_yield_inst_create
........................

.. doxygenfunction:: tinytc_yield_inst_create

tinytc_inst_get_region
......................

.. doxygenfunction:: tinytc_inst_get_region

tinytc_inst_get_regions
.......................

.. doxygenfunction:: tinytc_inst_get_regions

tinytc_inst_get_value
.....................

.. doxygenfunction:: tinytc_inst_get_value

tinytc_inst_get_values
......................

.. doxygenfunction:: tinytc_inst_get_values

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

tinytc_prog_create
..................

.. doxygenfunction:: tinytc_prog_create

tinytc_prog_add_function
........................

.. doxygenfunction:: tinytc_prog_add_function

tinytc_prog_dump
................

.. doxygenfunction:: tinytc_prog_dump

tinytc_prog_get_compiler_context
................................

.. doxygenfunction:: tinytc_prog_get_compiler_context

tinytc_prog_print_to_file
.........................

.. doxygenfunction:: tinytc_prog_print_to_file

tinytc_prog_print_to_string
...........................

.. doxygenfunction:: tinytc_prog_print_to_string

tinytc_prog_release
...................

.. doxygenfunction:: tinytc_prog_release

tinytc_prog_retain
..................

.. doxygenfunction:: tinytc_prog_retain

Region
======

* Functions

  * :ref:`tinytc_region_add_instruction`

  * :ref:`tinytc_region_get_parameter`

  * :ref:`tinytc_region_get_parameters`

Region Functions
----------------

tinytc_region_add_instruction
.............................

.. doxygenfunction:: tinytc_region_add_instruction

tinytc_region_get_parameter
...........................

.. doxygenfunction:: tinytc_region_get_parameter

tinytc_region_get_parameters
............................

.. doxygenfunction:: tinytc_region_get_parameters

Value
=====

* Functions

  * :ref:`tinytc_value_get_name`

  * :ref:`tinytc_value_set_name`

  * :ref:`tinytc_value_set_name_n`

Value Functions
---------------

tinytc_value_get_name
.....................

.. doxygenfunction:: tinytc_value_get_name

tinytc_value_set_name
.....................

.. doxygenfunction:: tinytc_value_set_name

tinytc_value_set_name_n
.......................

.. doxygenfunction:: tinytc_value_set_name_n

