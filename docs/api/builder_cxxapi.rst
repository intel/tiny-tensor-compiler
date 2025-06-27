.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

.. _Builder C++-API:

===============
Builder C++-API
===============

Common
======

* Enumerations

  * :ref:`tinytc::address_space`

  * :ref:`tinytc::checked_flag`

  * :ref:`tinytc::cmp_condition`

  * :ref:`tinytc::comp3`

  * :ref:`tinytc::math_unary`

  * :ref:`tinytc::matrix_use`

  * :ref:`tinytc::reduce_mode`

  * :ref:`tinytc::scalar_type`

  * :ref:`tinytc::store_flag`

  * :ref:`tinytc::transpose`

* Functions

  * :ref:`tinytc::is_dynamic_value`

  * :ref:`tinytc::to_string(address_space)`

  * :ref:`tinytc::to_string(checked_flag)`

  * :ref:`tinytc::to_string(cmp_condition)`

  * :ref:`tinytc::to_string(comp3)`

  * :ref:`tinytc::to_string(math_unary)`

  * :ref:`tinytc::to_string(matrix_use)`

  * :ref:`tinytc::to_string(reduce_mode)`

  * :ref:`tinytc::to_string(scalar_type)`

  * :ref:`tinytc::to_string(store_flag)`

  * :ref:`tinytc::to_string(transpose)`

  * :ref:`tinytc::size`

* Classes

  * :ref:`tinytc::builder_error`

* Typedefs

  * :ref:`tinytc::location`

  * :ref:`tinytc::position`

* Variables

  * :ref:`tinytc::dynamic`

Common Enumerations
-------------------

.. _tinytc::address_space:

address_space
.............

.. doxygenenum:: tinytc::address_space

.. _tinytc::checked_flag:

checked_flag
............

.. doxygenenum:: tinytc::checked_flag

.. _tinytc::cmp_condition:

cmp_condition
.............

.. doxygenenum:: tinytc::cmp_condition

.. _tinytc::comp3:

comp3
.....

.. doxygenenum:: tinytc::comp3

.. _tinytc::math_unary:

math_unary
..........

.. doxygenenum:: tinytc::math_unary

.. _tinytc::matrix_use:

matrix_use
..........

.. doxygenenum:: tinytc::matrix_use

.. _tinytc::reduce_mode:

reduce_mode
...........

.. doxygenenum:: tinytc::reduce_mode

.. _tinytc::scalar_type:

scalar_type
...........

.. doxygenenum:: tinytc::scalar_type

.. _tinytc::store_flag:

store_flag
..........

.. doxygenenum:: tinytc::store_flag

.. _tinytc::transpose:

transpose
.........

.. doxygenenum:: tinytc::transpose

Common Functions
----------------

.. _tinytc::is_dynamic_value:

is_dynamic_value
................

.. doxygenfunction:: tinytc::is_dynamic_value

.. _tinytc::to_string(address_space):

to_string(address_space)
........................

.. doxygenfunction:: tinytc::to_string(address_space)

.. _tinytc::to_string(checked_flag):

to_string(checked_flag)
.......................

.. doxygenfunction:: tinytc::to_string(checked_flag)

.. _tinytc::to_string(cmp_condition):

to_string(cmp_condition)
........................

.. doxygenfunction:: tinytc::to_string(cmp_condition)

.. _tinytc::to_string(comp3):

to_string(comp3)
................

.. doxygenfunction:: tinytc::to_string(comp3)

.. _tinytc::to_string(math_unary):

to_string(math_unary)
.....................

.. doxygenfunction:: tinytc::to_string(math_unary)

.. _tinytc::to_string(matrix_use):

to_string(matrix_use)
.....................

.. doxygenfunction:: tinytc::to_string(matrix_use)

.. _tinytc::to_string(reduce_mode):

to_string(reduce_mode)
......................

.. doxygenfunction:: tinytc::to_string(reduce_mode)

.. _tinytc::to_string(scalar_type):

to_string(scalar_type)
......................

.. doxygenfunction:: tinytc::to_string(scalar_type)

.. _tinytc::to_string(store_flag):

to_string(store_flag)
.....................

.. doxygenfunction:: tinytc::to_string(store_flag)

.. _tinytc::to_string(transpose):

to_string(transpose)
....................

.. doxygenfunction:: tinytc::to_string(transpose)

.. _tinytc::size:

size
....

.. doxygenfunction:: tinytc::size

Common Classes
--------------

.. _tinytc::builder_error:

builder_error
.............

.. doxygenclass:: tinytc::builder_error

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

  * :ref:`get_array_attr`

  * :ref:`get_boolean_attr`

  * :ref:`get_dictionary_attr`

  * :ref:`get_dictionary_attr_with_sorted`

  * :ref:`get_integer_attr`

  * :ref:`get_string_attr`

  * :ref:`sort_items`

* Typedefs

  * :ref:`tinytc::attr`

  * :ref:`tinytc::named_attr`

Attribute Functions
-------------------

.. _get_array_attr:

get_array_attr
..............

.. doxygenfunction:: get_array_attr

.. _get_boolean_attr:

get_boolean_attr
................

.. doxygenfunction:: get_boolean_attr

.. _get_dictionary_attr:

get_dictionary_attr
...................

.. doxygenfunction:: get_dictionary_attr

.. _get_dictionary_attr_with_sorted:

get_dictionary_attr_with_sorted
...............................

.. doxygenfunction:: get_dictionary_attr_with_sorted

.. _get_integer_attr:

get_integer_attr
................

.. doxygenfunction:: get_integer_attr

.. _get_string_attr:

get_string_attr
...............

.. doxygenfunction:: get_string_attr

.. _sort_items:

sort_items
..........

.. doxygenfunction:: sort_items

Attribute Typedefs
------------------

.. _tinytc::attr:

attr
....

.. doxygentypedef:: tinytc::attr

.. _tinytc::named_attr:

named_attr
..........

.. doxygentypedef:: tinytc::named_attr

Data Type
=========

* Functions

  * :ref:`tinytc::get_boolean`

  * :ref:`tinytc::get_coopmatrix`

  * :ref:`tinytc::get_group`

  * :ref:`tinytc::get_memref`

  * :ref:`tinytc::get_scalar`

  * :ref:`tinytc::get_void`

* Structures

  * :ref:`tinytc::to_scalar_type`

* Typedefs

  * :ref:`tinytc::data_type`

* Variables

  * :ref:`tinytc::to_scalar_type_v`

Data Type Functions
-------------------

.. _tinytc::get_boolean:

get_boolean
...........

.. doxygenfunction:: tinytc::get_boolean

.. _tinytc::get_coopmatrix:

get_coopmatrix
..............

.. doxygenfunction:: tinytc::get_coopmatrix

.. _tinytc::get_group:

get_group
.........

.. doxygenfunction:: tinytc::get_group

.. _tinytc::get_memref:

get_memref
..........

.. doxygenfunction:: tinytc::get_memref

.. _tinytc::get_scalar:

get_scalar
..........

.. doxygenfunction:: tinytc::get_scalar

.. _tinytc::get_void:

get_void
........

.. doxygenfunction:: tinytc::get_void

Data Type Structures
--------------------

.. _tinytc::to_scalar_type:

to_scalar_type
..............

.. doxygenstruct:: tinytc::to_scalar_type

Data Type Typedefs
------------------

.. _tinytc::data_type:

data_type
.........

.. doxygentypedef:: tinytc::data_type

Data Type Variables
-------------------

.. _tinytc::to_scalar_type_v:

to_scalar_type_v
................

.. doxygenvariable:: tinytc::to_scalar_type_v

Function
========

* Functions

  * :ref:`tinytc::get_body`

  * :ref:`tinytc::make_func`

  * :ref:`tinytc::set_attr(func&,attr)`

  * :ref:`tinytc::set_parameter_attr`

* Classes

  * :ref:`tinytc::func`

Function Functions
------------------

.. _tinytc::get_body:

get_body
........

.. doxygenfunction:: tinytc::get_body

.. _tinytc::make_func:

make_func
.........

.. doxygenfunction:: tinytc::make_func

.. _tinytc::set_attr(func&,attr):

set_attr(func&,attr)
....................

.. doxygenfunction:: tinytc::set_attr(func&,attr)

.. _tinytc::set_parameter_attr:

set_parameter_attr
..................

.. doxygenfunction:: tinytc::set_parameter_attr

Function Classes
----------------

.. _tinytc::func:

func
....

.. doxygenclass:: tinytc::func

Instruction
===========

* Functions

  * :ref:`tinytc::create`

* Classes

  * :ref:`tinytc::inst`

* Structures

  * :ref:`tinytc::creator\< alloca_inst \>`

  * :ref:`tinytc::creator\< barrier_inst \>`

  * :ref:`tinytc::creator\< cast_inst \>`

  * :ref:`tinytc::creator\< compare_inst \>`

  * :ref:`tinytc::creator\< constant_inst \>`

  * :ref:`tinytc::creator\< cooperative_matrix_apply_inst \>`

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

  * :ref:`tinytc::creator\< expand_inst \>`

  * :ref:`tinytc::creator\< fuse_inst \>`

  * :ref:`tinytc::creator\< if_inst \>`

  * :ref:`tinytc::creator\< lifetime_stop_inst \>`

  * :ref:`tinytc::creator\< load_inst \>`

  * :ref:`tinytc::creator\< math_unary_inst \>`

  * :ref:`tinytc::creator\< parallel_inst \>`

  * :ref:`tinytc::creator\< size_inst \>`

  * :ref:`tinytc::creator\< subgroup_broadcast_inst \>`

  * :ref:`tinytc::creator\< subview_inst \>`

  * :ref:`tinytc::creator\< store_inst \>`

  * :ref:`tinytc::creator\< yield_inst \>`

  * :ref:`tinytc::creator\< add_inst \>`

  * :ref:`tinytc::creator\< sub_inst \>`

  * :ref:`tinytc::creator\< mul_inst \>`

  * :ref:`tinytc::creator\< div_inst \>`

  * :ref:`tinytc::creator\< rem_inst \>`

  * :ref:`tinytc::creator\< shl_inst \>`

  * :ref:`tinytc::creator\< shr_inst \>`

  * :ref:`tinytc::creator\< and_inst \>`

  * :ref:`tinytc::creator\< or_inst \>`

  * :ref:`tinytc::creator\< xor_inst \>`

  * :ref:`tinytc::creator\< min_inst \>`

  * :ref:`tinytc::creator\< max_inst \>`

  * :ref:`tinytc::creator\< abs_inst \>`

  * :ref:`tinytc::creator\< neg_inst \>`

  * :ref:`tinytc::creator\< not_inst \>`

  * :ref:`tinytc::creator\< conj_inst \>`

  * :ref:`tinytc::creator\< im_inst \>`

  * :ref:`tinytc::creator\< re_inst \>`

  * :ref:`tinytc::creator\< axpby_inst \>`

  * :ref:`tinytc::creator\< cumsum_inst \>`

  * :ref:`tinytc::creator\< sum_inst \>`

  * :ref:`tinytc::creator\< gemm_inst \>`

  * :ref:`tinytc::creator\< gemv_inst \>`

  * :ref:`tinytc::creator\< ger_inst \>`

  * :ref:`tinytc::creator\< hadamard_inst \>`

  * :ref:`tinytc::creator\< group_id_inst \>`

  * :ref:`tinytc::creator\< num_groups_inst \>`

  * :ref:`tinytc::creator\< num_subgroups_inst \>`

  * :ref:`tinytc::creator\< subgroup_size_inst \>`

  * :ref:`tinytc::creator\< subgroup_id_inst \>`

  * :ref:`tinytc::creator\< subgroup_linear_id_inst \>`

  * :ref:`tinytc::creator\< subgroup_local_id_inst \>`

  * :ref:`tinytc::creator\< for_inst \>`

  * :ref:`tinytc::creator\< foreach_inst \>`

  * :ref:`tinytc::creator\< subgroup_exclusive_scan_add_inst \>`

  * :ref:`tinytc::creator\< subgroup_exclusive_scan_max_inst \>`

  * :ref:`tinytc::creator\< subgroup_exclusive_scan_min_inst \>`

  * :ref:`tinytc::creator\< subgroup_inclusive_scan_add_inst \>`

  * :ref:`tinytc::creator\< subgroup_inclusive_scan_max_inst \>`

  * :ref:`tinytc::creator\< subgroup_inclusive_scan_min_inst \>`

  * :ref:`tinytc::creator\< subgroup_reduce_add_inst \>`

  * :ref:`tinytc::creator\< subgroup_reduce_max_inst \>`

  * :ref:`tinytc::creator\< subgroup_reduce_min_inst \>`

Instruction Functions
---------------------

.. _tinytc::create:

create
......

.. doxygenfunction:: tinytc::create

Instruction Classes
-------------------

.. _tinytc::inst:

inst
....

.. doxygenclass:: tinytc::inst

Instruction Structures
----------------------

.. _tinytc::creator\< alloca_inst \>:

creator<alloca_inst>
....................

.. doxygenstruct:: tinytc::creator< alloca_inst >

.. _tinytc::creator\< barrier_inst \>:

creator<barrier_inst>
.....................

.. doxygenstruct:: tinytc::creator< barrier_inst >

.. _tinytc::creator\< cast_inst \>:

creator<cast_inst>
..................

.. doxygenstruct:: tinytc::creator< cast_inst >

.. _tinytc::creator\< compare_inst \>:

creator<compare_inst>
.....................

.. doxygenstruct:: tinytc::creator< compare_inst >

.. _tinytc::creator\< constant_inst \>:

creator<constant_inst>
......................

.. doxygenstruct:: tinytc::creator< constant_inst >

.. _tinytc::creator\< cooperative_matrix_apply_inst \>:

creator<cooperative_matrix_apply_inst>
......................................

.. doxygenstruct:: tinytc::creator< cooperative_matrix_apply_inst >

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

.. _tinytc::creator\< expand_inst \>:

creator<expand_inst>
....................

.. doxygenstruct:: tinytc::creator< expand_inst >

.. _tinytc::creator\< fuse_inst \>:

creator<fuse_inst>
..................

.. doxygenstruct:: tinytc::creator< fuse_inst >

.. _tinytc::creator\< if_inst \>:

creator<if_inst>
................

.. doxygenstruct:: tinytc::creator< if_inst >

.. _tinytc::creator\< lifetime_stop_inst \>:

creator<lifetime_stop_inst>
...........................

.. doxygenstruct:: tinytc::creator< lifetime_stop_inst >

.. _tinytc::creator\< load_inst \>:

creator<load_inst>
..................

.. doxygenstruct:: tinytc::creator< load_inst >

.. _tinytc::creator\< math_unary_inst \>:

creator<math_unary_inst>
........................

.. doxygenstruct:: tinytc::creator< math_unary_inst >

.. _tinytc::creator\< parallel_inst \>:

creator<parallel_inst>
......................

.. doxygenstruct:: tinytc::creator< parallel_inst >

.. _tinytc::creator\< size_inst \>:

creator<size_inst>
..................

.. doxygenstruct:: tinytc::creator< size_inst >

.. _tinytc::creator\< subgroup_broadcast_inst \>:

creator<subgroup_broadcast_inst>
................................

.. doxygenstruct:: tinytc::creator< subgroup_broadcast_inst >

.. _tinytc::creator\< subview_inst \>:

creator<subview_inst>
.....................

.. doxygenstruct:: tinytc::creator< subview_inst >

.. _tinytc::creator\< store_inst \>:

creator<store_inst>
...................

.. doxygenstruct:: tinytc::creator< store_inst >

.. _tinytc::creator\< yield_inst \>:

creator<yield_inst>
...................

.. doxygenstruct:: tinytc::creator< yield_inst >

.. _tinytc::creator\< add_inst \>:

creator<add_inst>
.................

.. doxygenstruct:: tinytc::creator< add_inst >

.. _tinytc::creator\< sub_inst \>:

creator<sub_inst>
.................

.. doxygenstruct:: tinytc::creator< sub_inst >

.. _tinytc::creator\< mul_inst \>:

creator<mul_inst>
.................

.. doxygenstruct:: tinytc::creator< mul_inst >

.. _tinytc::creator\< div_inst \>:

creator<div_inst>
.................

.. doxygenstruct:: tinytc::creator< div_inst >

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

.. _tinytc::creator\< and_inst \>:

creator<and_inst>
.................

.. doxygenstruct:: tinytc::creator< and_inst >

.. _tinytc::creator\< or_inst \>:

creator<or_inst>
................

.. doxygenstruct:: tinytc::creator< or_inst >

.. _tinytc::creator\< xor_inst \>:

creator<xor_inst>
.................

.. doxygenstruct:: tinytc::creator< xor_inst >

.. _tinytc::creator\< min_inst \>:

creator<min_inst>
.................

.. doxygenstruct:: tinytc::creator< min_inst >

.. _tinytc::creator\< max_inst \>:

creator<max_inst>
.................

.. doxygenstruct:: tinytc::creator< max_inst >

.. _tinytc::creator\< abs_inst \>:

creator<abs_inst>
.................

.. doxygenstruct:: tinytc::creator< abs_inst >

.. _tinytc::creator\< neg_inst \>:

creator<neg_inst>
.................

.. doxygenstruct:: tinytc::creator< neg_inst >

.. _tinytc::creator\< not_inst \>:

creator<not_inst>
.................

.. doxygenstruct:: tinytc::creator< not_inst >

.. _tinytc::creator\< conj_inst \>:

creator<conj_inst>
..................

.. doxygenstruct:: tinytc::creator< conj_inst >

.. _tinytc::creator\< im_inst \>:

creator<im_inst>
................

.. doxygenstruct:: tinytc::creator< im_inst >

.. _tinytc::creator\< re_inst \>:

creator<re_inst>
................

.. doxygenstruct:: tinytc::creator< re_inst >

.. _tinytc::creator\< axpby_inst \>:

creator<axpby_inst>
...................

.. doxygenstruct:: tinytc::creator< axpby_inst >

.. _tinytc::creator\< cumsum_inst \>:

creator<cumsum_inst>
....................

.. doxygenstruct:: tinytc::creator< cumsum_inst >

.. _tinytc::creator\< sum_inst \>:

creator<sum_inst>
.................

.. doxygenstruct:: tinytc::creator< sum_inst >

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

.. _tinytc::creator\< hadamard_inst \>:

creator<hadamard_inst>
......................

.. doxygenstruct:: tinytc::creator< hadamard_inst >

.. _tinytc::creator\< group_id_inst \>:

creator<group_id_inst>
......................

.. doxygenstruct:: tinytc::creator< group_id_inst >

.. _tinytc::creator\< num_groups_inst \>:

creator<num_groups_inst>
........................

.. doxygenstruct:: tinytc::creator< num_groups_inst >

.. _tinytc::creator\< num_subgroups_inst \>:

creator<num_subgroups_inst>
...........................

.. doxygenstruct:: tinytc::creator< num_subgroups_inst >

.. _tinytc::creator\< subgroup_size_inst \>:

creator<subgroup_size_inst>
...........................

.. doxygenstruct:: tinytc::creator< subgroup_size_inst >

.. _tinytc::creator\< subgroup_id_inst \>:

creator<subgroup_id_inst>
.........................

.. doxygenstruct:: tinytc::creator< subgroup_id_inst >

.. _tinytc::creator\< subgroup_linear_id_inst \>:

creator<subgroup_linear_id_inst>
................................

.. doxygenstruct:: tinytc::creator< subgroup_linear_id_inst >

.. _tinytc::creator\< subgroup_local_id_inst \>:

creator<subgroup_local_id_inst>
...............................

.. doxygenstruct:: tinytc::creator< subgroup_local_id_inst >

.. _tinytc::creator\< for_inst \>:

creator<for_inst>
.................

.. doxygenstruct:: tinytc::creator< for_inst >

.. _tinytc::creator\< foreach_inst \>:

creator<foreach_inst>
.....................

.. doxygenstruct:: tinytc::creator< foreach_inst >

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

Program
=======

* Functions

  * :ref:`tinytc::add_function`

  * :ref:`tinytc::make_prog`

Program Functions
-----------------

.. _tinytc::add_function:

add_function
............

.. doxygenfunction:: tinytc::add_function

.. _tinytc::make_prog:

make_prog
.........

.. doxygenfunction:: tinytc::make_prog

Region
======

* Functions

  * :ref:`tinytc::next`

  * :ref:`tinytc::prev`

* Classes

  * :ref:`tinytc::region_builder`

* Typedefs

  * :ref:`tinytc::region`

Region Functions
----------------

.. _tinytc::next:

next
....

.. doxygenfunction:: tinytc::next

.. _tinytc::prev:

prev
....

.. doxygenfunction:: tinytc::prev

Region Classes
--------------

.. _tinytc::region_builder:

region_builder
..............

.. doxygenclass:: tinytc::region_builder

Region Typedefs
---------------

.. _tinytc::region:

region
......

.. doxygentypedef:: tinytc::region

Value
=====

* Typedefs

  * :ref:`tinytc::value`

Value Typedefs
--------------

.. _tinytc::value:

value
.....

.. doxygentypedef:: tinytc::value

