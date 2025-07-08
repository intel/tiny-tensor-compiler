.. Copyright (C) 2025 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

.. _Builder C-API:

=============
Builder C-API
=============

Common
======

* Definitions

  * :ref:`TINYTC_DYNAMIC`

* Structures

  * :ref:`tinytc_named_attr`

  * :ref:`tinytc_location`

  * :ref:`tinytc_position`

* Typedefs

  * :ref:`tinytc_attr_t`

  * :ref:`tinytc_func_t`

  * :ref:`tinytc_named_attr_t`

  * :ref:`tinytc_location_t`

  * :ref:`tinytc_position_t`

  * :ref:`tinytc_inst_t`

  * :ref:`tinytc_inst_iterator_t`

  * :ref:`tinytc_region_t`

  * :ref:`tinytc_type_t`

  * :ref:`tinytc_value_t`

  * :ref:`const_tinytc_attr_t`

  * :ref:`const_tinytc_func_t`

  * :ref:`const_tinytc_inst_t`

  * :ref:`const_tinytc_region_t`

  * :ref:`const_tinytc_type_t`

  * :ref:`const_tinytc_value_t`

Common Definitions
------------------

.. _TINYTC_DYNAMIC:

TINYTC_DYNAMIC
..............

.. doxygendefine:: TINYTC_DYNAMIC

Common Structures
-----------------

.. _tinytc_named_attr:

tinytc_named_attr
.................

.. doxygenstruct:: tinytc_named_attr

.. _tinytc_location:

tinytc_location
...............

.. doxygenstruct:: tinytc_location

.. _tinytc_position:

tinytc_position
...............

.. doxygenstruct:: tinytc_position

Common Typedefs
---------------

.. _tinytc_attr_t:

tinytc_attr_t
.............

.. doxygentypedef:: tinytc_attr_t

.. _tinytc_func_t:

tinytc_func_t
.............

.. doxygentypedef:: tinytc_func_t

.. _tinytc_named_attr_t:

tinytc_named_attr_t
...................

.. doxygentypedef:: tinytc_named_attr_t

.. _tinytc_location_t:

tinytc_location_t
.................

.. doxygentypedef:: tinytc_location_t

.. _tinytc_position_t:

tinytc_position_t
.................

.. doxygentypedef:: tinytc_position_t

.. _tinytc_inst_t:

tinytc_inst_t
.............

.. doxygentypedef:: tinytc_inst_t

.. _tinytc_inst_iterator_t:

tinytc_inst_iterator_t
......................

.. doxygentypedef:: tinytc_inst_iterator_t

.. _tinytc_region_t:

tinytc_region_t
...............

.. doxygentypedef:: tinytc_region_t

.. _tinytc_type_t:

tinytc_type_t
.............

.. doxygentypedef:: tinytc_type_t

.. _tinytc_value_t:

tinytc_value_t
..............

.. doxygentypedef:: tinytc_value_t

.. _const_tinytc_attr_t:

const_tinytc_attr_t
...................

.. doxygentypedef:: const_tinytc_attr_t

.. _const_tinytc_func_t:

const_tinytc_func_t
...................

.. doxygentypedef:: const_tinytc_func_t

.. _const_tinytc_inst_t:

const_tinytc_inst_t
...................

.. doxygentypedef:: const_tinytc_inst_t

.. _const_tinytc_region_t:

const_tinytc_region_t
.....................

.. doxygentypedef:: const_tinytc_region_t

.. _const_tinytc_type_t:

const_tinytc_type_t
...................

.. doxygentypedef:: const_tinytc_type_t

.. _const_tinytc_value_t:

const_tinytc_value_t
....................

.. doxygentypedef:: const_tinytc_value_t

Attribute
=========

* Functions

  * :ref:`tinytc_array_attr_get`

  * :ref:`tinytc_boolean_attr_get`

  * :ref:`tinytc_dictionary_attr_get`

  * :ref:`tinytc_dictionary_attr_get_with_sorted`

  * :ref:`tinytc_dictionary_attr_sort`

  * :ref:`tinytc_integer_attr_get`

  * :ref:`tinytc_string_attr_get`

Attribute Functions
-------------------

.. _tinytc_array_attr_get:

tinytc_array_attr_get
.....................

.. doxygenfunction:: tinytc_array_attr_get

.. _tinytc_boolean_attr_get:

tinytc_boolean_attr_get
.......................

.. doxygenfunction:: tinytc_boolean_attr_get

.. _tinytc_dictionary_attr_get:

tinytc_dictionary_attr_get
..........................

.. doxygenfunction:: tinytc_dictionary_attr_get

.. _tinytc_dictionary_attr_get_with_sorted:

tinytc_dictionary_attr_get_with_sorted
......................................

.. doxygenfunction:: tinytc_dictionary_attr_get_with_sorted

.. _tinytc_dictionary_attr_sort:

tinytc_dictionary_attr_sort
...........................

.. doxygenfunction:: tinytc_dictionary_attr_sort

.. _tinytc_integer_attr_get:

tinytc_integer_attr_get
.......................

.. doxygenfunction:: tinytc_integer_attr_get

.. _tinytc_string_attr_get:

tinytc_string_attr_get
......................

.. doxygenfunction:: tinytc_string_attr_get

Data Type
=========

* Functions

  * :ref:`tinytc_type_get_compiler_context`

Data Type Functions
-------------------

.. _tinytc_type_get_compiler_context:

tinytc_type_get_compiler_context
................................

.. doxygenfunction:: tinytc_type_get_compiler_context

Data Type Builder
=================

* Functions

  * :ref:`tinytc_boolean_type_get`

  * :ref:`tinytc_i8_type_get`

  * :ref:`tinytc_i16_type_get`

  * :ref:`tinytc_i32_type_get`

  * :ref:`tinytc_i64_type_get`

  * :ref:`tinytc_index_type_get`

  * :ref:`tinytc_bf16_type_get`

  * :ref:`tinytc_f16_type_get`

  * :ref:`tinytc_f32_type_get`

  * :ref:`tinytc_f64_type_get`

  * :ref:`tinytc_c32_type_get`

  * :ref:`tinytc_c64_type_get`

  * :ref:`tinytc_coopmatrix_type_get`

  * :ref:`tinytc_group_type_get`

  * :ref:`tinytc_memref_type_get`

  * :ref:`tinytc_void_type_get`

Data Type Builder Functions
---------------------------

.. _tinytc_boolean_type_get:

tinytc_boolean_type_get
.......................

.. doxygenfunction:: tinytc_boolean_type_get

.. _tinytc_i8_type_get:

tinytc_i8_type_get
..................

.. doxygenfunction:: tinytc_i8_type_get

.. _tinytc_i16_type_get:

tinytc_i16_type_get
...................

.. doxygenfunction:: tinytc_i16_type_get

.. _tinytc_i32_type_get:

tinytc_i32_type_get
...................

.. doxygenfunction:: tinytc_i32_type_get

.. _tinytc_i64_type_get:

tinytc_i64_type_get
...................

.. doxygenfunction:: tinytc_i64_type_get

.. _tinytc_index_type_get:

tinytc_index_type_get
.....................

.. doxygenfunction:: tinytc_index_type_get

.. _tinytc_bf16_type_get:

tinytc_bf16_type_get
....................

.. doxygenfunction:: tinytc_bf16_type_get

.. _tinytc_f16_type_get:

tinytc_f16_type_get
...................

.. doxygenfunction:: tinytc_f16_type_get

.. _tinytc_f32_type_get:

tinytc_f32_type_get
...................

.. doxygenfunction:: tinytc_f32_type_get

.. _tinytc_f64_type_get:

tinytc_f64_type_get
...................

.. doxygenfunction:: tinytc_f64_type_get

.. _tinytc_c32_type_get:

tinytc_c32_type_get
...................

.. doxygenfunction:: tinytc_c32_type_get

.. _tinytc_c64_type_get:

tinytc_c64_type_get
...................

.. doxygenfunction:: tinytc_c64_type_get

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

.. _tinytc_void_type_get:

tinytc_void_type_get
....................

.. doxygenfunction:: tinytc_void_type_get

Function
========

* Functions

  * :ref:`tinytc_func_create`

  * :ref:`tinytc_func_destroy`

  * :ref:`tinytc_func_get_body`

  * :ref:`tinytc_func_set_attr`

  * :ref:`tinytc_func_set_parameter_attr`

Function Functions
------------------

.. _tinytc_func_create:

tinytc_func_create
..................

.. doxygenfunction:: tinytc_func_create

.. _tinytc_func_destroy:

tinytc_func_destroy
...................

.. doxygenfunction:: tinytc_func_destroy

.. _tinytc_func_get_body:

tinytc_func_get_body
....................

.. doxygenfunction:: tinytc_func_get_body

.. _tinytc_func_set_attr:

tinytc_func_set_attr
....................

.. doxygenfunction:: tinytc_func_set_attr

.. _tinytc_func_set_parameter_attr:

tinytc_func_set_parameter_attr
..............................

.. doxygenfunction:: tinytc_func_set_parameter_attr

Instruction
===========

* Functions

  * :ref:`tinytc_inst_get_parent_region`

  * :ref:`tinytc_inst_get_regions`

  * :ref:`tinytc_inst_get_values`

  * :ref:`tinytc_inst_destroy`

  * :ref:`tinytc_inst_set_attr`

Instruction Functions
---------------------

.. _tinytc_inst_get_parent_region:

tinytc_inst_get_parent_region
.............................

.. doxygenfunction:: tinytc_inst_get_parent_region

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

.. _tinytc_inst_set_attr:

tinytc_inst_set_attr
....................

.. doxygenfunction:: tinytc_inst_set_attr

Instruction Builder
===================

* Functions

  * :ref:`tinytc_abs_inst_create`

  * :ref:`tinytc_add_inst_create`

  * :ref:`tinytc_alloca_inst_create`

  * :ref:`tinytc_and_inst_create`

  * :ref:`tinytc_atomic_add_inst_create`

  * :ref:`tinytc_atomic_load_inst_create`

  * :ref:`tinytc_atomic_max_inst_create`

  * :ref:`tinytc_atomic_min_inst_create`

  * :ref:`tinytc_atomic_store_inst_create`

  * :ref:`tinytc_axpby_inst_create`

  * :ref:`tinytc_barrier_inst_create`

  * :ref:`tinytc_cast_inst_create`

  * :ref:`tinytc_conj_inst_create`

  * :ref:`tinytc_constant_inst_create_boolean`

  * :ref:`tinytc_constant_inst_create_complex`

  * :ref:`tinytc_constant_inst_create_float`

  * :ref:`tinytc_constant_inst_create_int`

  * :ref:`tinytc_constant_inst_create_one`

  * :ref:`tinytc_constant_inst_create_zero`

  * :ref:`tinytc_cooperative_matrix_apply_inst_create`

  * :ref:`tinytc_cooperative_matrix_atomic_add_inst_create`

  * :ref:`tinytc_cooperative_matrix_atomic_load_inst_create`

  * :ref:`tinytc_cooperative_matrix_atomic_max_inst_create`

  * :ref:`tinytc_cooperative_matrix_atomic_min_inst_create`

  * :ref:`tinytc_cooperative_matrix_atomic_store_inst_create`

  * :ref:`tinytc_cooperative_matrix_extract_inst_create`

  * :ref:`tinytc_cooperative_matrix_insert_inst_create`

  * :ref:`tinytc_cooperative_matrix_load_inst_create`

  * :ref:`tinytc_cooperative_matrix_mul_add_inst_create`

  * :ref:`tinytc_cooperative_matrix_prefetch_inst_create`

  * :ref:`tinytc_cooperative_matrix_reduce_add_inst_create`

  * :ref:`tinytc_cooperative_matrix_reduce_max_inst_create`

  * :ref:`tinytc_cooperative_matrix_reduce_min_inst_create`

  * :ref:`tinytc_cooperative_matrix_scale_inst_create`

  * :ref:`tinytc_cooperative_matrix_store_inst_create`

  * :ref:`tinytc_cos_inst_create`

  * :ref:`tinytc_cumsum_inst_create`

  * :ref:`tinytc_div_inst_create`

  * :ref:`tinytc_equal_inst_create`

  * :ref:`tinytc_exp2_inst_create`

  * :ref:`tinytc_exp_inst_create`

  * :ref:`tinytc_expand_inst_create`

  * :ref:`tinytc_for_inst_create`

  * :ref:`tinytc_foreach_inst_create`

  * :ref:`tinytc_fuse_inst_create`

  * :ref:`tinytc_gemm_inst_create`

  * :ref:`tinytc_gemv_inst_create`

  * :ref:`tinytc_ger_inst_create`

  * :ref:`tinytc_greater_than_equal_inst_create`

  * :ref:`tinytc_greater_than_inst_create`

  * :ref:`tinytc_group_id_inst_create`

  * :ref:`tinytc_hadamard_inst_create`

  * :ref:`tinytc_if_inst_create`

  * :ref:`tinytc_im_inst_create`

  * :ref:`tinytc_less_than_equal_inst_create`

  * :ref:`tinytc_less_than_inst_create`

  * :ref:`tinytc_lifetime_stop_inst_create`

  * :ref:`tinytc_load_inst_create`

  * :ref:`tinytc_max_inst_create`

  * :ref:`tinytc_min_inst_create`

  * :ref:`tinytc_mul_inst_create`

  * :ref:`tinytc_native_cos_inst_create`

  * :ref:`tinytc_native_exp2_inst_create`

  * :ref:`tinytc_native_exp_inst_create`

  * :ref:`tinytc_native_sin_inst_create`

  * :ref:`tinytc_neg_inst_create`

  * :ref:`tinytc_not_equal_inst_create`

  * :ref:`tinytc_not_inst_create`

  * :ref:`tinytc_num_groups_inst_create`

  * :ref:`tinytc_num_subgroups_inst_create`

  * :ref:`tinytc_or_inst_create`

  * :ref:`tinytc_parallel_inst_create`

  * :ref:`tinytc_re_inst_create`

  * :ref:`tinytc_rem_inst_create`

  * :ref:`tinytc_shl_inst_create`

  * :ref:`tinytc_shr_inst_create`

  * :ref:`tinytc_sin_inst_create`

  * :ref:`tinytc_size_inst_create`

  * :ref:`tinytc_store_inst_create`

  * :ref:`tinytc_sub_inst_create`

  * :ref:`tinytc_subgroup_broadcast_inst_create`

  * :ref:`tinytc_subgroup_exclusive_scan_add_inst_create`

  * :ref:`tinytc_subgroup_exclusive_scan_max_inst_create`

  * :ref:`tinytc_subgroup_exclusive_scan_min_inst_create`

  * :ref:`tinytc_subgroup_id_inst_create`

  * :ref:`tinytc_subgroup_inclusive_scan_add_inst_create`

  * :ref:`tinytc_subgroup_inclusive_scan_max_inst_create`

  * :ref:`tinytc_subgroup_inclusive_scan_min_inst_create`

  * :ref:`tinytc_subgroup_linear_id_inst_create`

  * :ref:`tinytc_subgroup_local_id_inst_create`

  * :ref:`tinytc_subgroup_reduce_add_inst_create`

  * :ref:`tinytc_subgroup_reduce_max_inst_create`

  * :ref:`tinytc_subgroup_reduce_min_inst_create`

  * :ref:`tinytc_subgroup_size_inst_create`

  * :ref:`tinytc_subview_inst_create`

  * :ref:`tinytc_sum_inst_create`

  * :ref:`tinytc_xor_inst_create`

  * :ref:`tinytc_yield_inst_create`

Instruction Builder Functions
-----------------------------

.. _tinytc_abs_inst_create:

tinytc_abs_inst_create
......................

.. doxygenfunction:: tinytc_abs_inst_create

.. _tinytc_add_inst_create:

tinytc_add_inst_create
......................

.. doxygenfunction:: tinytc_add_inst_create

.. _tinytc_alloca_inst_create:

tinytc_alloca_inst_create
.........................

.. doxygenfunction:: tinytc_alloca_inst_create

.. _tinytc_and_inst_create:

tinytc_and_inst_create
......................

.. doxygenfunction:: tinytc_and_inst_create

.. _tinytc_atomic_add_inst_create:

tinytc_atomic_add_inst_create
.............................

.. doxygenfunction:: tinytc_atomic_add_inst_create

.. _tinytc_atomic_load_inst_create:

tinytc_atomic_load_inst_create
..............................

.. doxygenfunction:: tinytc_atomic_load_inst_create

.. _tinytc_atomic_max_inst_create:

tinytc_atomic_max_inst_create
.............................

.. doxygenfunction:: tinytc_atomic_max_inst_create

.. _tinytc_atomic_min_inst_create:

tinytc_atomic_min_inst_create
.............................

.. doxygenfunction:: tinytc_atomic_min_inst_create

.. _tinytc_atomic_store_inst_create:

tinytc_atomic_store_inst_create
...............................

.. doxygenfunction:: tinytc_atomic_store_inst_create

.. _tinytc_axpby_inst_create:

tinytc_axpby_inst_create
........................

.. doxygenfunction:: tinytc_axpby_inst_create

.. _tinytc_barrier_inst_create:

tinytc_barrier_inst_create
..........................

.. doxygenfunction:: tinytc_barrier_inst_create

.. _tinytc_cast_inst_create:

tinytc_cast_inst_create
.......................

.. doxygenfunction:: tinytc_cast_inst_create

.. _tinytc_conj_inst_create:

tinytc_conj_inst_create
.......................

.. doxygenfunction:: tinytc_conj_inst_create

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

.. _tinytc_cooperative_matrix_apply_inst_create:

tinytc_cooperative_matrix_apply_inst_create
...........................................

.. doxygenfunction:: tinytc_cooperative_matrix_apply_inst_create

.. _tinytc_cooperative_matrix_atomic_add_inst_create:

tinytc_cooperative_matrix_atomic_add_inst_create
................................................

.. doxygenfunction:: tinytc_cooperative_matrix_atomic_add_inst_create

.. _tinytc_cooperative_matrix_atomic_load_inst_create:

tinytc_cooperative_matrix_atomic_load_inst_create
.................................................

.. doxygenfunction:: tinytc_cooperative_matrix_atomic_load_inst_create

.. _tinytc_cooperative_matrix_atomic_max_inst_create:

tinytc_cooperative_matrix_atomic_max_inst_create
................................................

.. doxygenfunction:: tinytc_cooperative_matrix_atomic_max_inst_create

.. _tinytc_cooperative_matrix_atomic_min_inst_create:

tinytc_cooperative_matrix_atomic_min_inst_create
................................................

.. doxygenfunction:: tinytc_cooperative_matrix_atomic_min_inst_create

.. _tinytc_cooperative_matrix_atomic_store_inst_create:

tinytc_cooperative_matrix_atomic_store_inst_create
..................................................

.. doxygenfunction:: tinytc_cooperative_matrix_atomic_store_inst_create

.. _tinytc_cooperative_matrix_extract_inst_create:

tinytc_cooperative_matrix_extract_inst_create
.............................................

.. doxygenfunction:: tinytc_cooperative_matrix_extract_inst_create

.. _tinytc_cooperative_matrix_insert_inst_create:

tinytc_cooperative_matrix_insert_inst_create
............................................

.. doxygenfunction:: tinytc_cooperative_matrix_insert_inst_create

.. _tinytc_cooperative_matrix_load_inst_create:

tinytc_cooperative_matrix_load_inst_create
..........................................

.. doxygenfunction:: tinytc_cooperative_matrix_load_inst_create

.. _tinytc_cooperative_matrix_mul_add_inst_create:

tinytc_cooperative_matrix_mul_add_inst_create
.............................................

.. doxygenfunction:: tinytc_cooperative_matrix_mul_add_inst_create

.. _tinytc_cooperative_matrix_prefetch_inst_create:

tinytc_cooperative_matrix_prefetch_inst_create
..............................................

.. doxygenfunction:: tinytc_cooperative_matrix_prefetch_inst_create

.. _tinytc_cooperative_matrix_reduce_add_inst_create:

tinytc_cooperative_matrix_reduce_add_inst_create
................................................

.. doxygenfunction:: tinytc_cooperative_matrix_reduce_add_inst_create

.. _tinytc_cooperative_matrix_reduce_max_inst_create:

tinytc_cooperative_matrix_reduce_max_inst_create
................................................

.. doxygenfunction:: tinytc_cooperative_matrix_reduce_max_inst_create

.. _tinytc_cooperative_matrix_reduce_min_inst_create:

tinytc_cooperative_matrix_reduce_min_inst_create
................................................

.. doxygenfunction:: tinytc_cooperative_matrix_reduce_min_inst_create

.. _tinytc_cooperative_matrix_scale_inst_create:

tinytc_cooperative_matrix_scale_inst_create
...........................................

.. doxygenfunction:: tinytc_cooperative_matrix_scale_inst_create

.. _tinytc_cooperative_matrix_store_inst_create:

tinytc_cooperative_matrix_store_inst_create
...........................................

.. doxygenfunction:: tinytc_cooperative_matrix_store_inst_create

.. _tinytc_cos_inst_create:

tinytc_cos_inst_create
......................

.. doxygenfunction:: tinytc_cos_inst_create

.. _tinytc_cumsum_inst_create:

tinytc_cumsum_inst_create
.........................

.. doxygenfunction:: tinytc_cumsum_inst_create

.. _tinytc_div_inst_create:

tinytc_div_inst_create
......................

.. doxygenfunction:: tinytc_div_inst_create

.. _tinytc_equal_inst_create:

tinytc_equal_inst_create
........................

.. doxygenfunction:: tinytc_equal_inst_create

.. _tinytc_exp2_inst_create:

tinytc_exp2_inst_create
.......................

.. doxygenfunction:: tinytc_exp2_inst_create

.. _tinytc_exp_inst_create:

tinytc_exp_inst_create
......................

.. doxygenfunction:: tinytc_exp_inst_create

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

.. _tinytc_greater_than_equal_inst_create:

tinytc_greater_than_equal_inst_create
.....................................

.. doxygenfunction:: tinytc_greater_than_equal_inst_create

.. _tinytc_greater_than_inst_create:

tinytc_greater_than_inst_create
...............................

.. doxygenfunction:: tinytc_greater_than_inst_create

.. _tinytc_group_id_inst_create:

tinytc_group_id_inst_create
...........................

.. doxygenfunction:: tinytc_group_id_inst_create

.. _tinytc_hadamard_inst_create:

tinytc_hadamard_inst_create
...........................

.. doxygenfunction:: tinytc_hadamard_inst_create

.. _tinytc_if_inst_create:

tinytc_if_inst_create
.....................

.. doxygenfunction:: tinytc_if_inst_create

.. _tinytc_im_inst_create:

tinytc_im_inst_create
.....................

.. doxygenfunction:: tinytc_im_inst_create

.. _tinytc_less_than_equal_inst_create:

tinytc_less_than_equal_inst_create
..................................

.. doxygenfunction:: tinytc_less_than_equal_inst_create

.. _tinytc_less_than_inst_create:

tinytc_less_than_inst_create
............................

.. doxygenfunction:: tinytc_less_than_inst_create

.. _tinytc_lifetime_stop_inst_create:

tinytc_lifetime_stop_inst_create
................................

.. doxygenfunction:: tinytc_lifetime_stop_inst_create

.. _tinytc_load_inst_create:

tinytc_load_inst_create
.......................

.. doxygenfunction:: tinytc_load_inst_create

.. _tinytc_max_inst_create:

tinytc_max_inst_create
......................

.. doxygenfunction:: tinytc_max_inst_create

.. _tinytc_min_inst_create:

tinytc_min_inst_create
......................

.. doxygenfunction:: tinytc_min_inst_create

.. _tinytc_mul_inst_create:

tinytc_mul_inst_create
......................

.. doxygenfunction:: tinytc_mul_inst_create

.. _tinytc_native_cos_inst_create:

tinytc_native_cos_inst_create
.............................

.. doxygenfunction:: tinytc_native_cos_inst_create

.. _tinytc_native_exp2_inst_create:

tinytc_native_exp2_inst_create
..............................

.. doxygenfunction:: tinytc_native_exp2_inst_create

.. _tinytc_native_exp_inst_create:

tinytc_native_exp_inst_create
.............................

.. doxygenfunction:: tinytc_native_exp_inst_create

.. _tinytc_native_sin_inst_create:

tinytc_native_sin_inst_create
.............................

.. doxygenfunction:: tinytc_native_sin_inst_create

.. _tinytc_neg_inst_create:

tinytc_neg_inst_create
......................

.. doxygenfunction:: tinytc_neg_inst_create

.. _tinytc_not_equal_inst_create:

tinytc_not_equal_inst_create
............................

.. doxygenfunction:: tinytc_not_equal_inst_create

.. _tinytc_not_inst_create:

tinytc_not_inst_create
......................

.. doxygenfunction:: tinytc_not_inst_create

.. _tinytc_num_groups_inst_create:

tinytc_num_groups_inst_create
.............................

.. doxygenfunction:: tinytc_num_groups_inst_create

.. _tinytc_num_subgroups_inst_create:

tinytc_num_subgroups_inst_create
................................

.. doxygenfunction:: tinytc_num_subgroups_inst_create

.. _tinytc_or_inst_create:

tinytc_or_inst_create
.....................

.. doxygenfunction:: tinytc_or_inst_create

.. _tinytc_parallel_inst_create:

tinytc_parallel_inst_create
...........................

.. doxygenfunction:: tinytc_parallel_inst_create

.. _tinytc_re_inst_create:

tinytc_re_inst_create
.....................

.. doxygenfunction:: tinytc_re_inst_create

.. _tinytc_rem_inst_create:

tinytc_rem_inst_create
......................

.. doxygenfunction:: tinytc_rem_inst_create

.. _tinytc_shl_inst_create:

tinytc_shl_inst_create
......................

.. doxygenfunction:: tinytc_shl_inst_create

.. _tinytc_shr_inst_create:

tinytc_shr_inst_create
......................

.. doxygenfunction:: tinytc_shr_inst_create

.. _tinytc_sin_inst_create:

tinytc_sin_inst_create
......................

.. doxygenfunction:: tinytc_sin_inst_create

.. _tinytc_size_inst_create:

tinytc_size_inst_create
.......................

.. doxygenfunction:: tinytc_size_inst_create

.. _tinytc_store_inst_create:

tinytc_store_inst_create
........................

.. doxygenfunction:: tinytc_store_inst_create

.. _tinytc_sub_inst_create:

tinytc_sub_inst_create
......................

.. doxygenfunction:: tinytc_sub_inst_create

.. _tinytc_subgroup_broadcast_inst_create:

tinytc_subgroup_broadcast_inst_create
.....................................

.. doxygenfunction:: tinytc_subgroup_broadcast_inst_create

.. _tinytc_subgroup_exclusive_scan_add_inst_create:

tinytc_subgroup_exclusive_scan_add_inst_create
..............................................

.. doxygenfunction:: tinytc_subgroup_exclusive_scan_add_inst_create

.. _tinytc_subgroup_exclusive_scan_max_inst_create:

tinytc_subgroup_exclusive_scan_max_inst_create
..............................................

.. doxygenfunction:: tinytc_subgroup_exclusive_scan_max_inst_create

.. _tinytc_subgroup_exclusive_scan_min_inst_create:

tinytc_subgroup_exclusive_scan_min_inst_create
..............................................

.. doxygenfunction:: tinytc_subgroup_exclusive_scan_min_inst_create

.. _tinytc_subgroup_id_inst_create:

tinytc_subgroup_id_inst_create
..............................

.. doxygenfunction:: tinytc_subgroup_id_inst_create

.. _tinytc_subgroup_inclusive_scan_add_inst_create:

tinytc_subgroup_inclusive_scan_add_inst_create
..............................................

.. doxygenfunction:: tinytc_subgroup_inclusive_scan_add_inst_create

.. _tinytc_subgroup_inclusive_scan_max_inst_create:

tinytc_subgroup_inclusive_scan_max_inst_create
..............................................

.. doxygenfunction:: tinytc_subgroup_inclusive_scan_max_inst_create

.. _tinytc_subgroup_inclusive_scan_min_inst_create:

tinytc_subgroup_inclusive_scan_min_inst_create
..............................................

.. doxygenfunction:: tinytc_subgroup_inclusive_scan_min_inst_create

.. _tinytc_subgroup_linear_id_inst_create:

tinytc_subgroup_linear_id_inst_create
.....................................

.. doxygenfunction:: tinytc_subgroup_linear_id_inst_create

.. _tinytc_subgroup_local_id_inst_create:

tinytc_subgroup_local_id_inst_create
....................................

.. doxygenfunction:: tinytc_subgroup_local_id_inst_create

.. _tinytc_subgroup_reduce_add_inst_create:

tinytc_subgroup_reduce_add_inst_create
......................................

.. doxygenfunction:: tinytc_subgroup_reduce_add_inst_create

.. _tinytc_subgroup_reduce_max_inst_create:

tinytc_subgroup_reduce_max_inst_create
......................................

.. doxygenfunction:: tinytc_subgroup_reduce_max_inst_create

.. _tinytc_subgroup_reduce_min_inst_create:

tinytc_subgroup_reduce_min_inst_create
......................................

.. doxygenfunction:: tinytc_subgroup_reduce_min_inst_create

.. _tinytc_subgroup_size_inst_create:

tinytc_subgroup_size_inst_create
................................

.. doxygenfunction:: tinytc_subgroup_size_inst_create

.. _tinytc_subview_inst_create:

tinytc_subview_inst_create
..........................

.. doxygenfunction:: tinytc_subview_inst_create

.. _tinytc_sum_inst_create:

tinytc_sum_inst_create
......................

.. doxygenfunction:: tinytc_sum_inst_create

.. _tinytc_xor_inst_create:

tinytc_xor_inst_create
......................

.. doxygenfunction:: tinytc_xor_inst_create

.. _tinytc_yield_inst_create:

tinytc_yield_inst_create
........................

.. doxygenfunction:: tinytc_yield_inst_create

Program
=======

* Functions

  * :ref:`tinytc_prog_create`

  * :ref:`tinytc_prog_add_function`

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

Recipe
======

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

Region
======

* Functions

  * :ref:`tinytc_region_append`

  * :ref:`tinytc_region_begin`

  * :ref:`tinytc_region_end`

  * :ref:`tinytc_region_erase`

  * :ref:`tinytc_region_insert`

  * :ref:`tinytc_next_inst`

  * :ref:`tinytc_prev_inst`

  * :ref:`tinytc_region_get_parameters`

Region Functions
----------------

.. _tinytc_region_append:

tinytc_region_append
....................

.. doxygenfunction:: tinytc_region_append

.. _tinytc_region_begin:

tinytc_region_begin
...................

.. doxygenfunction:: tinytc_region_begin

.. _tinytc_region_end:

tinytc_region_end
.................

.. doxygenfunction:: tinytc_region_end

.. _tinytc_region_erase:

tinytc_region_erase
...................

.. doxygenfunction:: tinytc_region_erase

.. _tinytc_region_insert:

tinytc_region_insert
....................

.. doxygenfunction:: tinytc_region_insert

.. _tinytc_next_inst:

tinytc_next_inst
................

.. doxygenfunction:: tinytc_next_inst

.. _tinytc_prev_inst:

tinytc_prev_inst
................

.. doxygenfunction:: tinytc_prev_inst

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

