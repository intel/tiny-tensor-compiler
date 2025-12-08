.. Copyright (C) 2025 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

.. _Builder C++-API:

===============
Builder C++-API
===============

Common
======

* Classes

  * :ref:`tinytc::builder_error`

* Functions

  * :ref:`tinytc::is_dynamic_value`

* Typedefs

  * :ref:`tinytc::location`

  * :ref:`tinytc::position`

* Variables

  * :ref:`tinytc::dynamic`

Common Classes
--------------

.. _tinytc::builder_error:

builder_error
.............

.. doxygenclass:: tinytc::builder_error

Common Functions
----------------

.. _tinytc::is_dynamic_value:

is_dynamic_value
................

.. doxygenfunction:: tinytc::is_dynamic_value

Common Typedefs
---------------

.. _tinytc::location:

location
........

.. doxygentypedef:: tinytc::location

.. _tinytc::position:

position
........

.. doxygentypedef:: tinytc::position

Common Variables
----------------

.. _tinytc::dynamic:

dynamic
.......

.. doxygenvariable:: tinytc::dynamic

Attribute
=========

* Functions

  * :ref:`get_dictionary_attr_with_sorted`

  * :ref:`sort_items`

* Structures

  * :ref:`tinytc::getter\< array_attr \>`

  * :ref:`tinytc::getter\< boolean_attr \>`

  * :ref:`tinytc::getter\< dictionary_attr \>`

  * :ref:`tinytc::getter\< integer_attr \>`

  * :ref:`tinytc::getter\< string_attr \>`

Attribute Functions
-------------------

.. _get_dictionary_attr_with_sorted:

get_dictionary_attr_with_sorted
...............................

.. doxygenfunction:: get_dictionary_attr_with_sorted

.. _sort_items:

sort_items
..........

.. doxygenfunction:: sort_items

Attribute Structures
--------------------

.. _tinytc::getter\< array_attr \>:

getter<array_attr>
..................

.. doxygenstruct:: tinytc::getter< array_attr >

.. _tinytc::getter\< boolean_attr \>:

getter<boolean_attr>
....................

.. doxygenstruct:: tinytc::getter< boolean_attr >

.. _tinytc::getter\< dictionary_attr \>:

getter<dictionary_attr>
.......................

.. doxygenstruct:: tinytc::getter< dictionary_attr >

.. _tinytc::getter\< integer_attr \>:

getter<integer_attr>
....................

.. doxygenstruct:: tinytc::getter< integer_attr >

.. _tinytc::getter\< string_attr \>:

getter<string_attr>
...................

.. doxygenstruct:: tinytc::getter< string_attr >

Data Type
=========

* Functions

  * :ref:`tinytc::get_compiler_context(const_tinytc_type_t)`

  * :ref:`tinytc::to_type`

Data Type Functions
-------------------

.. _tinytc::get_compiler_context(const_tinytc_type_t):

get_compiler_context(const_tinytc_type_t)
.........................................

.. doxygenfunction:: tinytc::get_compiler_context(const_tinytc_type_t)

.. _tinytc::to_type:

to_type
.......

.. doxygenfunction:: tinytc::to_type

Data Type Builder
=================

* Functions

  * :ref:`tinytc::get`

* Structures

  * :ref:`tinytc::getter\< boolean_type \>`

  * :ref:`tinytc::getter\< i8_type \>`

  * :ref:`tinytc::getter\< i16_type \>`

  * :ref:`tinytc::getter\< i32_type \>`

  * :ref:`tinytc::getter\< i64_type \>`

  * :ref:`tinytc::getter\< index_type \>`

  * :ref:`tinytc::getter\< bf16_type \>`

  * :ref:`tinytc::getter\< f16_type \>`

  * :ref:`tinytc::getter\< f32_type \>`

  * :ref:`tinytc::getter\< f64_type \>`

  * :ref:`tinytc::getter\< c32_type \>`

  * :ref:`tinytc::getter\< c64_type \>`

  * :ref:`tinytc::getter\< coopmatrix_type \>`

  * :ref:`tinytc::getter\< group_type \>`

  * :ref:`tinytc::getter\< memref_type \>`

  * :ref:`tinytc::getter\< void_type \>`

Data Type Builder Functions
---------------------------

.. _tinytc::get:

get
...

.. doxygenfunction:: tinytc::get

Data Type Builder Structures
----------------------------

.. _tinytc::getter\< boolean_type \>:

getter<boolean_type>
....................

.. doxygenstruct:: tinytc::getter< boolean_type >

.. _tinytc::getter\< i8_type \>:

getter<i8_type>
...............

.. doxygenstruct:: tinytc::getter< i8_type >

.. _tinytc::getter\< i16_type \>:

getter<i16_type>
................

.. doxygenstruct:: tinytc::getter< i16_type >

.. _tinytc::getter\< i32_type \>:

getter<i32_type>
................

.. doxygenstruct:: tinytc::getter< i32_type >

.. _tinytc::getter\< i64_type \>:

getter<i64_type>
................

.. doxygenstruct:: tinytc::getter< i64_type >

.. _tinytc::getter\< index_type \>:

getter<index_type>
..................

.. doxygenstruct:: tinytc::getter< index_type >

.. _tinytc::getter\< bf16_type \>:

getter<bf16_type>
.................

.. doxygenstruct:: tinytc::getter< bf16_type >

.. _tinytc::getter\< f16_type \>:

getter<f16_type>
................

.. doxygenstruct:: tinytc::getter< f16_type >

.. _tinytc::getter\< f32_type \>:

getter<f32_type>
................

.. doxygenstruct:: tinytc::getter< f32_type >

.. _tinytc::getter\< f64_type \>:

getter<f64_type>
................

.. doxygenstruct:: tinytc::getter< f64_type >

.. _tinytc::getter\< c32_type \>:

getter<c32_type>
................

.. doxygenstruct:: tinytc::getter< c32_type >

.. _tinytc::getter\< c64_type \>:

getter<c64_type>
................

.. doxygenstruct:: tinytc::getter< c64_type >

.. _tinytc::getter\< coopmatrix_type \>:

getter<coopmatrix_type>
.......................

.. doxygenstruct:: tinytc::getter< coopmatrix_type >

.. _tinytc::getter\< group_type \>:

getter<group_type>
..................

.. doxygenstruct:: tinytc::getter< group_type >

.. _tinytc::getter\< memref_type \>:

getter<memref_type>
...................

.. doxygenstruct:: tinytc::getter< memref_type >

.. _tinytc::getter\< void_type \>:

getter<void_type>
.................

.. doxygenstruct:: tinytc::getter< void_type >

Function
========

* Functions

  * :ref:`tinytc::create_func`

  * :ref:`tinytc::get_body`

  * :ref:`tinytc::set_attr(tinytc_func_t,tinytc_attr_t)`

  * :ref:`tinytc::set_parameter_attr`

Function Functions
------------------

.. _tinytc::create_func:

create_func
...........

.. doxygenfunction:: tinytc::create_func

.. _tinytc::get_body:

get_body
........

.. doxygenfunction:: tinytc::get_body

.. _tinytc::set_attr(tinytc_func_t,tinytc_attr_t):

set_attr(tinytc_func_t,tinytc_attr_t)
.....................................

.. doxygenfunction:: tinytc::set_attr(tinytc_func_t,tinytc_attr_t)

.. _tinytc::set_parameter_attr:

set_parameter_attr
..................

.. doxygenfunction:: tinytc::set_parameter_attr

Instruction
===========

* Functions

  * :ref:`tinytc::get_parent_region`

  * :ref:`tinytc::get_regions`

  * :ref:`tinytc::get_values`

  * :ref:`tinytc::set_attr(tinytc_inst_t,tinytc_attr_t)`

Instruction Functions
---------------------

.. _tinytc::get_parent_region:

get_parent_region
.................

.. doxygenfunction:: tinytc::get_parent_region

.. _tinytc::get_regions:

get_regions
...........

.. doxygenfunction:: tinytc::get_regions

.. _tinytc::get_values:

get_values
..........

.. doxygenfunction:: tinytc::get_values

.. _tinytc::set_attr(tinytc_inst_t,tinytc_attr_t):

set_attr(tinytc_inst_t,tinytc_attr_t)
.....................................

.. doxygenfunction:: tinytc::set_attr(tinytc_inst_t,tinytc_attr_t)

Instruction Builder
===================

* Functions

  * :ref:`tinytc::create`

* Structures

  * :ref:`tinytc::creator\< abs_inst \>`

  * :ref:`tinytc::creator\< add_inst \>`

  * :ref:`tinytc::creator\< alloca_inst \>`

  * :ref:`tinytc::creator\< and_inst \>`

  * :ref:`tinytc::creator\< atomic_add_inst \>`

  * :ref:`tinytc::creator\< atomic_load_inst \>`

  * :ref:`tinytc::creator\< atomic_max_inst \>`

  * :ref:`tinytc::creator\< atomic_min_inst \>`

  * :ref:`tinytc::creator\< atomic_store_inst \>`

  * :ref:`tinytc::creator\< axpby_inst \>`

  * :ref:`tinytc::creator\< barrier_inst \>`

  * :ref:`tinytc::creator\< cast_inst \>`

  * :ref:`tinytc::creator\< conj_inst \>`

  * :ref:`tinytc::creator\< constant_inst \>`

  * :ref:`tinytc::creator\< cooperative_matrix_apply_inst \>`

  * :ref:`tinytc::creator\< cooperative_matrix_atomic_add_inst \>`

  * :ref:`tinytc::creator\< cooperative_matrix_atomic_load_inst \>`

  * :ref:`tinytc::creator\< cooperative_matrix_atomic_max_inst \>`

  * :ref:`tinytc::creator\< cooperative_matrix_atomic_min_inst \>`

  * :ref:`tinytc::creator\< cooperative_matrix_atomic_store_inst \>`

  * :ref:`tinytc::creator\< cooperative_matrix_extract_inst \>`

  * :ref:`tinytc::creator\< cooperative_matrix_insert_inst \>`

  * :ref:`tinytc::creator\< cooperative_matrix_load_inst \>`

  * :ref:`tinytc::creator\< cooperative_matrix_mul_add_inst \>`

  * :ref:`tinytc::creator\< cooperative_matrix_prefetch_inst \>`

  * :ref:`tinytc::creator\< cooperative_matrix_reduce_add_inst \>`

  * :ref:`tinytc::creator\< cooperative_matrix_reduce_max_inst \>`

  * :ref:`tinytc::creator\< cooperative_matrix_reduce_min_inst \>`

  * :ref:`tinytc::creator\< cooperative_matrix_scale_inst \>`

  * :ref:`tinytc::creator\< cooperative_matrix_store_inst \>`

  * :ref:`tinytc::creator\< cos_inst \>`

  * :ref:`tinytc::creator\< cumsum_inst \>`

  * :ref:`tinytc::creator\< div_inst \>`

  * :ref:`tinytc::creator\< equal_inst \>`

  * :ref:`tinytc::creator\< exp2_inst \>`

  * :ref:`tinytc::creator\< exp_inst \>`

  * :ref:`tinytc::creator\< expand_inst \>`

  * :ref:`tinytc::creator\< for_inst \>`

  * :ref:`tinytc::creator\< foreach_inst \>`

  * :ref:`tinytc::creator\< foreach_tile_inst \>`

  * :ref:`tinytc::creator\< fuse_inst \>`

  * :ref:`tinytc::creator\< gemm_inst \>`

  * :ref:`tinytc::creator\< gemv_inst \>`

  * :ref:`tinytc::creator\< ger_inst \>`

  * :ref:`tinytc::creator\< greater_than_equal_inst \>`

  * :ref:`tinytc::creator\< greater_than_inst \>`

  * :ref:`tinytc::creator\< group_id_inst \>`

  * :ref:`tinytc::creator\< hadamard_inst \>`

  * :ref:`tinytc::creator\< if_inst \>`

  * :ref:`tinytc::creator\< im_inst \>`

  * :ref:`tinytc::creator\< less_than_equal_inst \>`

  * :ref:`tinytc::creator\< less_than_inst \>`

  * :ref:`tinytc::creator\< lifetime_stop_inst \>`

  * :ref:`tinytc::creator\< load_inst \>`

  * :ref:`tinytc::creator\< max_inst \>`

  * :ref:`tinytc::creator\< min_inst \>`

  * :ref:`tinytc::creator\< mul_inst \>`

  * :ref:`tinytc::creator\< native_cos_inst \>`

  * :ref:`tinytc::creator\< native_exp2_inst \>`

  * :ref:`tinytc::creator\< native_exp_inst \>`

  * :ref:`tinytc::creator\< native_sin_inst \>`

  * :ref:`tinytc::creator\< neg_inst \>`

  * :ref:`tinytc::creator\< not_equal_inst \>`

  * :ref:`tinytc::creator\< not_inst \>`

  * :ref:`tinytc::creator\< num_groups_inst \>`

  * :ref:`tinytc::creator\< num_subgroups_inst \>`

  * :ref:`tinytc::creator\< or_inst \>`

  * :ref:`tinytc::creator\< parallel_inst \>`

  * :ref:`tinytc::creator\< re_inst \>`

  * :ref:`tinytc::creator\< rem_inst \>`

  * :ref:`tinytc::creator\< shl_inst \>`

  * :ref:`tinytc::creator\< shr_inst \>`

  * :ref:`tinytc::creator\< sin_inst \>`

  * :ref:`tinytc::creator\< size_inst \>`

  * :ref:`tinytc::creator\< store_inst \>`

  * :ref:`tinytc::creator\< sub_inst \>`

  * :ref:`tinytc::creator\< subgroup_broadcast_inst \>`

  * :ref:`tinytc::creator\< subgroup_exclusive_scan_add_inst \>`

  * :ref:`tinytc::creator\< subgroup_exclusive_scan_max_inst \>`

  * :ref:`tinytc::creator\< subgroup_exclusive_scan_min_inst \>`

  * :ref:`tinytc::creator\< subgroup_id_inst \>`

  * :ref:`tinytc::creator\< subgroup_inclusive_scan_add_inst \>`

  * :ref:`tinytc::creator\< subgroup_inclusive_scan_max_inst \>`

  * :ref:`tinytc::creator\< subgroup_inclusive_scan_min_inst \>`

  * :ref:`tinytc::creator\< subgroup_linear_id_inst \>`

  * :ref:`tinytc::creator\< subgroup_local_id_inst \>`

  * :ref:`tinytc::creator\< subgroup_reduce_add_inst \>`

  * :ref:`tinytc::creator\< subgroup_reduce_max_inst \>`

  * :ref:`tinytc::creator\< subgroup_reduce_min_inst \>`

  * :ref:`tinytc::creator\< subgroup_size_inst \>`

  * :ref:`tinytc::creator\< subview_inst \>`

  * :ref:`tinytc::creator\< sum_inst \>`

  * :ref:`tinytc::creator\< xor_inst \>`

  * :ref:`tinytc::creator\< yield_inst \>`

Instruction Builder Functions
-----------------------------

.. _tinytc::create:

create
......

.. doxygenfunction:: tinytc::create

Instruction Builder Structures
------------------------------

.. _tinytc::creator\< abs_inst \>:

creator<abs_inst>
.................

.. doxygenstruct:: tinytc::creator< abs_inst >

.. _tinytc::creator\< add_inst \>:

creator<add_inst>
.................

.. doxygenstruct:: tinytc::creator< add_inst >

.. _tinytc::creator\< alloca_inst \>:

creator<alloca_inst>
....................

.. doxygenstruct:: tinytc::creator< alloca_inst >

.. _tinytc::creator\< and_inst \>:

creator<and_inst>
.................

.. doxygenstruct:: tinytc::creator< and_inst >

.. _tinytc::creator\< atomic_add_inst \>:

creator<atomic_add_inst>
........................

.. doxygenstruct:: tinytc::creator< atomic_add_inst >

.. _tinytc::creator\< atomic_load_inst \>:

creator<atomic_load_inst>
.........................

.. doxygenstruct:: tinytc::creator< atomic_load_inst >

.. _tinytc::creator\< atomic_max_inst \>:

creator<atomic_max_inst>
........................

.. doxygenstruct:: tinytc::creator< atomic_max_inst >

.. _tinytc::creator\< atomic_min_inst \>:

creator<atomic_min_inst>
........................

.. doxygenstruct:: tinytc::creator< atomic_min_inst >

.. _tinytc::creator\< atomic_store_inst \>:

creator<atomic_store_inst>
..........................

.. doxygenstruct:: tinytc::creator< atomic_store_inst >

.. _tinytc::creator\< axpby_inst \>:

creator<axpby_inst>
...................

.. doxygenstruct:: tinytc::creator< axpby_inst >

.. _tinytc::creator\< barrier_inst \>:

creator<barrier_inst>
.....................

.. doxygenstruct:: tinytc::creator< barrier_inst >

.. _tinytc::creator\< cast_inst \>:

creator<cast_inst>
..................

.. doxygenstruct:: tinytc::creator< cast_inst >

.. _tinytc::creator\< conj_inst \>:

creator<conj_inst>
..................

.. doxygenstruct:: tinytc::creator< conj_inst >

.. _tinytc::creator\< constant_inst \>:

creator<constant_inst>
......................

.. doxygenstruct:: tinytc::creator< constant_inst >

.. _tinytc::creator\< cooperative_matrix_apply_inst \>:

creator<cooperative_matrix_apply_inst>
......................................

.. doxygenstruct:: tinytc::creator< cooperative_matrix_apply_inst >

.. _tinytc::creator\< cooperative_matrix_atomic_add_inst \>:

creator<cooperative_matrix_atomic_add_inst>
...........................................

.. doxygenstruct:: tinytc::creator< cooperative_matrix_atomic_add_inst >

.. _tinytc::creator\< cooperative_matrix_atomic_load_inst \>:

creator<cooperative_matrix_atomic_load_inst>
............................................

.. doxygenstruct:: tinytc::creator< cooperative_matrix_atomic_load_inst >

.. _tinytc::creator\< cooperative_matrix_atomic_max_inst \>:

creator<cooperative_matrix_atomic_max_inst>
...........................................

.. doxygenstruct:: tinytc::creator< cooperative_matrix_atomic_max_inst >

.. _tinytc::creator\< cooperative_matrix_atomic_min_inst \>:

creator<cooperative_matrix_atomic_min_inst>
...........................................

.. doxygenstruct:: tinytc::creator< cooperative_matrix_atomic_min_inst >

.. _tinytc::creator\< cooperative_matrix_atomic_store_inst \>:

creator<cooperative_matrix_atomic_store_inst>
.............................................

.. doxygenstruct:: tinytc::creator< cooperative_matrix_atomic_store_inst >

.. _tinytc::creator\< cooperative_matrix_extract_inst \>:

creator<cooperative_matrix_extract_inst>
........................................

.. doxygenstruct:: tinytc::creator< cooperative_matrix_extract_inst >

.. _tinytc::creator\< cooperative_matrix_insert_inst \>:

creator<cooperative_matrix_insert_inst>
.......................................

.. doxygenstruct:: tinytc::creator< cooperative_matrix_insert_inst >

.. _tinytc::creator\< cooperative_matrix_load_inst \>:

creator<cooperative_matrix_load_inst>
.....................................

.. doxygenstruct:: tinytc::creator< cooperative_matrix_load_inst >

.. _tinytc::creator\< cooperative_matrix_mul_add_inst \>:

creator<cooperative_matrix_mul_add_inst>
........................................

.. doxygenstruct:: tinytc::creator< cooperative_matrix_mul_add_inst >

.. _tinytc::creator\< cooperative_matrix_prefetch_inst \>:

creator<cooperative_matrix_prefetch_inst>
.........................................

.. doxygenstruct:: tinytc::creator< cooperative_matrix_prefetch_inst >

.. _tinytc::creator\< cooperative_matrix_reduce_add_inst \>:

creator<cooperative_matrix_reduce_add_inst>
...........................................

.. doxygenstruct:: tinytc::creator< cooperative_matrix_reduce_add_inst >

.. _tinytc::creator\< cooperative_matrix_reduce_max_inst \>:

creator<cooperative_matrix_reduce_max_inst>
...........................................

.. doxygenstruct:: tinytc::creator< cooperative_matrix_reduce_max_inst >

.. _tinytc::creator\< cooperative_matrix_reduce_min_inst \>:

creator<cooperative_matrix_reduce_min_inst>
...........................................

.. doxygenstruct:: tinytc::creator< cooperative_matrix_reduce_min_inst >

.. _tinytc::creator\< cooperative_matrix_scale_inst \>:

creator<cooperative_matrix_scale_inst>
......................................

.. doxygenstruct:: tinytc::creator< cooperative_matrix_scale_inst >

.. _tinytc::creator\< cooperative_matrix_store_inst \>:

creator<cooperative_matrix_store_inst>
......................................

.. doxygenstruct:: tinytc::creator< cooperative_matrix_store_inst >

.. _tinytc::creator\< cos_inst \>:

creator<cos_inst>
.................

.. doxygenstruct:: tinytc::creator< cos_inst >

.. _tinytc::creator\< cumsum_inst \>:

creator<cumsum_inst>
....................

.. doxygenstruct:: tinytc::creator< cumsum_inst >

.. _tinytc::creator\< div_inst \>:

creator<div_inst>
.................

.. doxygenstruct:: tinytc::creator< div_inst >

.. _tinytc::creator\< equal_inst \>:

creator<equal_inst>
...................

.. doxygenstruct:: tinytc::creator< equal_inst >

.. _tinytc::creator\< exp2_inst \>:

creator<exp2_inst>
..................

.. doxygenstruct:: tinytc::creator< exp2_inst >

.. _tinytc::creator\< exp_inst \>:

creator<exp_inst>
.................

.. doxygenstruct:: tinytc::creator< exp_inst >

.. _tinytc::creator\< expand_inst \>:

creator<expand_inst>
....................

.. doxygenstruct:: tinytc::creator< expand_inst >

.. _tinytc::creator\< for_inst \>:

creator<for_inst>
.................

.. doxygenstruct:: tinytc::creator< for_inst >

.. _tinytc::creator\< foreach_inst \>:

creator<foreach_inst>
.....................

.. doxygenstruct:: tinytc::creator< foreach_inst >

.. _tinytc::creator\< foreach_tile_inst \>:

creator<foreach_tile_inst>
..........................

.. doxygenstruct:: tinytc::creator< foreach_tile_inst >

.. _tinytc::creator\< fuse_inst \>:

creator<fuse_inst>
..................

.. doxygenstruct:: tinytc::creator< fuse_inst >

.. _tinytc::creator\< gemm_inst \>:

creator<gemm_inst>
..................

.. doxygenstruct:: tinytc::creator< gemm_inst >

.. _tinytc::creator\< gemv_inst \>:

creator<gemv_inst>
..................

.. doxygenstruct:: tinytc::creator< gemv_inst >

.. _tinytc::creator\< ger_inst \>:

creator<ger_inst>
.................

.. doxygenstruct:: tinytc::creator< ger_inst >

.. _tinytc::creator\< greater_than_equal_inst \>:

creator<greater_than_equal_inst>
................................

.. doxygenstruct:: tinytc::creator< greater_than_equal_inst >

.. _tinytc::creator\< greater_than_inst \>:

creator<greater_than_inst>
..........................

.. doxygenstruct:: tinytc::creator< greater_than_inst >

.. _tinytc::creator\< group_id_inst \>:

creator<group_id_inst>
......................

.. doxygenstruct:: tinytc::creator< group_id_inst >

.. _tinytc::creator\< hadamard_inst \>:

creator<hadamard_inst>
......................

.. doxygenstruct:: tinytc::creator< hadamard_inst >

.. _tinytc::creator\< if_inst \>:

creator<if_inst>
................

.. doxygenstruct:: tinytc::creator< if_inst >

.. _tinytc::creator\< im_inst \>:

creator<im_inst>
................

.. doxygenstruct:: tinytc::creator< im_inst >

.. _tinytc::creator\< less_than_equal_inst \>:

creator<less_than_equal_inst>
.............................

.. doxygenstruct:: tinytc::creator< less_than_equal_inst >

.. _tinytc::creator\< less_than_inst \>:

creator<less_than_inst>
.......................

.. doxygenstruct:: tinytc::creator< less_than_inst >

.. _tinytc::creator\< lifetime_stop_inst \>:

creator<lifetime_stop_inst>
...........................

.. doxygenstruct:: tinytc::creator< lifetime_stop_inst >

.. _tinytc::creator\< load_inst \>:

creator<load_inst>
..................

.. doxygenstruct:: tinytc::creator< load_inst >

.. _tinytc::creator\< max_inst \>:

creator<max_inst>
.................

.. doxygenstruct:: tinytc::creator< max_inst >

.. _tinytc::creator\< min_inst \>:

creator<min_inst>
.................

.. doxygenstruct:: tinytc::creator< min_inst >

.. _tinytc::creator\< mul_inst \>:

creator<mul_inst>
.................

.. doxygenstruct:: tinytc::creator< mul_inst >

.. _tinytc::creator\< native_cos_inst \>:

creator<native_cos_inst>
........................

.. doxygenstruct:: tinytc::creator< native_cos_inst >

.. _tinytc::creator\< native_exp2_inst \>:

creator<native_exp2_inst>
.........................

.. doxygenstruct:: tinytc::creator< native_exp2_inst >

.. _tinytc::creator\< native_exp_inst \>:

creator<native_exp_inst>
........................

.. doxygenstruct:: tinytc::creator< native_exp_inst >

.. _tinytc::creator\< native_sin_inst \>:

creator<native_sin_inst>
........................

.. doxygenstruct:: tinytc::creator< native_sin_inst >

.. _tinytc::creator\< neg_inst \>:

creator<neg_inst>
.................

.. doxygenstruct:: tinytc::creator< neg_inst >

.. _tinytc::creator\< not_equal_inst \>:

creator<not_equal_inst>
.......................

.. doxygenstruct:: tinytc::creator< not_equal_inst >

.. _tinytc::creator\< not_inst \>:

creator<not_inst>
.................

.. doxygenstruct:: tinytc::creator< not_inst >

.. _tinytc::creator\< num_groups_inst \>:

creator<num_groups_inst>
........................

.. doxygenstruct:: tinytc::creator< num_groups_inst >

.. _tinytc::creator\< num_subgroups_inst \>:

creator<num_subgroups_inst>
...........................

.. doxygenstruct:: tinytc::creator< num_subgroups_inst >

.. _tinytc::creator\< or_inst \>:

creator<or_inst>
................

.. doxygenstruct:: tinytc::creator< or_inst >

.. _tinytc::creator\< parallel_inst \>:

creator<parallel_inst>
......................

.. doxygenstruct:: tinytc::creator< parallel_inst >

.. _tinytc::creator\< re_inst \>:

creator<re_inst>
................

.. doxygenstruct:: tinytc::creator< re_inst >

.. _tinytc::creator\< rem_inst \>:

creator<rem_inst>
.................

.. doxygenstruct:: tinytc::creator< rem_inst >

.. _tinytc::creator\< shl_inst \>:

creator<shl_inst>
.................

.. doxygenstruct:: tinytc::creator< shl_inst >

.. _tinytc::creator\< shr_inst \>:

creator<shr_inst>
.................

.. doxygenstruct:: tinytc::creator< shr_inst >

.. _tinytc::creator\< sin_inst \>:

creator<sin_inst>
.................

.. doxygenstruct:: tinytc::creator< sin_inst >

.. _tinytc::creator\< size_inst \>:

creator<size_inst>
..................

.. doxygenstruct:: tinytc::creator< size_inst >

.. _tinytc::creator\< store_inst \>:

creator<store_inst>
...................

.. doxygenstruct:: tinytc::creator< store_inst >

.. _tinytc::creator\< sub_inst \>:

creator<sub_inst>
.................

.. doxygenstruct:: tinytc::creator< sub_inst >

.. _tinytc::creator\< subgroup_broadcast_inst \>:

creator<subgroup_broadcast_inst>
................................

.. doxygenstruct:: tinytc::creator< subgroup_broadcast_inst >

.. _tinytc::creator\< subgroup_exclusive_scan_add_inst \>:

creator<subgroup_exclusive_scan_add_inst>
.........................................

.. doxygenstruct:: tinytc::creator< subgroup_exclusive_scan_add_inst >

.. _tinytc::creator\< subgroup_exclusive_scan_max_inst \>:

creator<subgroup_exclusive_scan_max_inst>
.........................................

.. doxygenstruct:: tinytc::creator< subgroup_exclusive_scan_max_inst >

.. _tinytc::creator\< subgroup_exclusive_scan_min_inst \>:

creator<subgroup_exclusive_scan_min_inst>
.........................................

.. doxygenstruct:: tinytc::creator< subgroup_exclusive_scan_min_inst >

.. _tinytc::creator\< subgroup_id_inst \>:

creator<subgroup_id_inst>
.........................

.. doxygenstruct:: tinytc::creator< subgroup_id_inst >

.. _tinytc::creator\< subgroup_inclusive_scan_add_inst \>:

creator<subgroup_inclusive_scan_add_inst>
.........................................

.. doxygenstruct:: tinytc::creator< subgroup_inclusive_scan_add_inst >

.. _tinytc::creator\< subgroup_inclusive_scan_max_inst \>:

creator<subgroup_inclusive_scan_max_inst>
.........................................

.. doxygenstruct:: tinytc::creator< subgroup_inclusive_scan_max_inst >

.. _tinytc::creator\< subgroup_inclusive_scan_min_inst \>:

creator<subgroup_inclusive_scan_min_inst>
.........................................

.. doxygenstruct:: tinytc::creator< subgroup_inclusive_scan_min_inst >

.. _tinytc::creator\< subgroup_linear_id_inst \>:

creator<subgroup_linear_id_inst>
................................

.. doxygenstruct:: tinytc::creator< subgroup_linear_id_inst >

.. _tinytc::creator\< subgroup_local_id_inst \>:

creator<subgroup_local_id_inst>
...............................

.. doxygenstruct:: tinytc::creator< subgroup_local_id_inst >

.. _tinytc::creator\< subgroup_reduce_add_inst \>:

creator<subgroup_reduce_add_inst>
.................................

.. doxygenstruct:: tinytc::creator< subgroup_reduce_add_inst >

.. _tinytc::creator\< subgroup_reduce_max_inst \>:

creator<subgroup_reduce_max_inst>
.................................

.. doxygenstruct:: tinytc::creator< subgroup_reduce_max_inst >

.. _tinytc::creator\< subgroup_reduce_min_inst \>:

creator<subgroup_reduce_min_inst>
.................................

.. doxygenstruct:: tinytc::creator< subgroup_reduce_min_inst >

.. _tinytc::creator\< subgroup_size_inst \>:

creator<subgroup_size_inst>
...........................

.. doxygenstruct:: tinytc::creator< subgroup_size_inst >

.. _tinytc::creator\< subview_inst \>:

creator<subview_inst>
.....................

.. doxygenstruct:: tinytc::creator< subview_inst >

.. _tinytc::creator\< sum_inst \>:

creator<sum_inst>
.................

.. doxygenstruct:: tinytc::creator< sum_inst >

.. _tinytc::creator\< xor_inst \>:

creator<xor_inst>
.................

.. doxygenstruct:: tinytc::creator< xor_inst >

.. _tinytc::creator\< yield_inst \>:

creator<yield_inst>
...................

.. doxygenstruct:: tinytc::creator< yield_inst >

Program
=======

* Functions

  * :ref:`tinytc::add_function`

  * :ref:`tinytc::create_prog`

Program Functions
-----------------

.. _tinytc::add_function:

add_function
............

.. doxygenfunction:: tinytc::add_function

.. _tinytc::create_prog:

create_prog
...........

.. doxygenfunction:: tinytc::create_prog

Recipe
======

* Functions

  * :ref:`tinytc::create_small_gemm_batched`

  * :ref:`tinytc::create_tall_and_skinny`

  * :ref:`tinytc::create_tall_and_skinny_specialized`

  * :ref:`tinytc::get_prog`

  * :ref:`tinytc::get_binary`

  * :ref:`tinytc::get_recipe`

  * :ref:`tinytc::set_small_gemm_batched_args`

  * :ref:`tinytc::set_tall_and_skinny_args`

Recipe Functions
----------------

.. _tinytc::create_small_gemm_batched:

create_small_gemm_batched
.........................

.. doxygenfunction:: tinytc::create_small_gemm_batched

.. _tinytc::create_tall_and_skinny:

create_tall_and_skinny
......................

.. doxygenfunction:: tinytc::create_tall_and_skinny

.. _tinytc::create_tall_and_skinny_specialized:

create_tall_and_skinny_specialized
..................................

.. doxygenfunction:: tinytc::create_tall_and_skinny_specialized

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

.. _tinytc::set_small_gemm_batched_args:

set_small_gemm_batched_args
...........................

.. doxygenfunction:: tinytc::set_small_gemm_batched_args

.. _tinytc::set_tall_and_skinny_args:

set_tall_and_skinny_args
........................

.. doxygenfunction:: tinytc::set_tall_and_skinny_args

Region
======

* Classes

  * :ref:`tinytc::region_builder`

* Functions

  * :ref:`tinytc::append`

  * :ref:`tinytc::begin`

  * :ref:`tinytc::end`

  * :ref:`tinytc::get_parameters`

  * :ref:`tinytc::insert`

  * :ref:`tinytc::next`

  * :ref:`tinytc::prev`

Region Classes
--------------

.. _tinytc::region_builder:

region_builder
..............

.. doxygenclass:: tinytc::region_builder

Region Functions
----------------

.. _tinytc::append:

append
......

.. doxygenfunction:: tinytc::append

.. _tinytc::begin:

begin
.....

.. doxygenfunction:: tinytc::begin

.. _tinytc::end:

end
...

.. doxygenfunction:: tinytc::end

.. _tinytc::get_parameters:

get_parameters
..............

.. doxygenfunction:: tinytc::get_parameters

.. _tinytc::insert:

insert
......

.. doxygenfunction:: tinytc::insert

.. _tinytc::next:

next
....

.. doxygenfunction:: tinytc::next

.. _tinytc::prev:

prev
....

.. doxygenfunction:: tinytc::prev

