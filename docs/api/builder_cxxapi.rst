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

  * :ref:`tinytc::arithmetic`

  * :ref:`tinytc::arithmetic_unary`

  * :ref:`tinytc::builtin`

  * :ref:`tinytc::cmp_condition`

  * :ref:`tinytc::group_arithmetic`

  * :ref:`tinytc::group_operation`

  * :ref:`tinytc::math_unary`

  * :ref:`tinytc::matrix_use`

  * :ref:`tinytc::scalar_type`

  * :ref:`tinytc::store_flag`

  * :ref:`tinytc::transpose`

* Functions

  * :ref:`tinytc::is_dynamic_value`

  * :ref:`tinytc::to_string(address_space)`

  * :ref:`tinytc::to_string(arithmetic)`

  * :ref:`tinytc::to_string(arithmetic_unary)`

  * :ref:`tinytc::to_string(builtin)`

  * :ref:`tinytc::to_string(checked_flag)`

  * :ref:`tinytc::to_string(cmp_condition)`

  * :ref:`tinytc::to_string(group_arithmetic)`

  * :ref:`tinytc::to_string(group_operation)`

  * :ref:`tinytc::to_string(math_unary)`

  * :ref:`tinytc::to_string(matrix_use)`

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

.. _tinytc::arithmetic:

arithmetic
..........

.. doxygenenum:: tinytc::arithmetic

.. _tinytc::arithmetic_unary:

arithmetic_unary
................

.. doxygenenum:: tinytc::arithmetic_unary

.. _tinytc::builtin:

builtin
.......

.. doxygenenum:: tinytc::builtin

.. _tinytc::cmp_condition:

cmp_condition
.............

.. doxygenenum:: tinytc::cmp_condition

.. _tinytc::group_arithmetic:

group_arithmetic
................

.. doxygenenum:: tinytc::group_arithmetic

.. _tinytc::group_operation:

group_operation
...............

.. doxygenenum:: tinytc::group_operation

.. _tinytc::math_unary:

math_unary
..........

.. doxygenenum:: tinytc::math_unary

.. _tinytc::matrix_use:

matrix_use
..........

.. doxygenenum:: tinytc::matrix_use

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

.. _tinytc::to_string(arithmetic):

to_string(arithmetic)
.....................

.. doxygenfunction:: tinytc::to_string(arithmetic)

.. _tinytc::to_string(arithmetic_unary):

to_string(arithmetic_unary)
...........................

.. doxygenfunction:: tinytc::to_string(arithmetic_unary)

.. _tinytc::to_string(builtin):

to_string(builtin)
..................

.. doxygenfunction:: tinytc::to_string(builtin)

.. _tinytc::to_string(checked_flag):

to_string(checked_flag)
.......................

.. doxygenfunction:: tinytc::to_string(checked_flag)

.. _tinytc::to_string(cmp_condition):

to_string(cmp_condition)
........................

.. doxygenfunction:: tinytc::to_string(cmp_condition)

.. _tinytc::to_string(group_arithmetic):

to_string(group_arithmetic)
...........................

.. doxygenfunction:: tinytc::to_string(group_arithmetic)

.. _tinytc::to_string(group_operation):

to_string(group_operation)
..........................

.. doxygenfunction:: tinytc::to_string(group_operation)

.. _tinytc::to_string(math_unary):

to_string(math_unary)
.....................

.. doxygenfunction:: tinytc::to_string(math_unary)

.. _tinytc::to_string(matrix_use):

to_string(matrix_use)
.....................

.. doxygenfunction:: tinytc::to_string(matrix_use)

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

  * :ref:`tinytc::make_func`

* Classes

  * :ref:`tinytc::func`

Function Functions
------------------

.. _tinytc::make_func:

make_func
.........

.. doxygenfunction:: tinytc::make_func

Function Classes
----------------

.. _tinytc::func:

func
....

.. doxygenclass:: tinytc::func

Instruction
===========

* Functions

  * :ref:`tinytc::make_alloca`

  * :ref:`tinytc::make_axpby`

  * :ref:`tinytc::make_arith(arithmetic,value,value,data_type,location const&)`

  * :ref:`tinytc::make_arith(arithmetic_unary,value,data_type,location const&)`

  * :ref:`tinytc::make_barrier`

  * :ref:`tinytc::make_builtin`

  * :ref:`tinytc::make_cast`

  * :ref:`tinytc::make_cmp`

  * :ref:`tinytc::make_constant(bool,data_type,location const&)`

  * :ref:`tinytc::make_constant(std::complex\<double\>,data_type,location const&)`

  * :ref:`tinytc::make_constant(double,data_type,location const&)`

  * :ref:`tinytc::make_constant(std::int32_t,data_type,location const&)`

  * :ref:`tinytc::make_constant(std::int64_t,data_type,location const&)`

  * :ref:`tinytc::make_constant_one`

  * :ref:`tinytc::make_constant_zero`

  * :ref:`tinytc::make_cooperative_matrix_apply`

  * :ref:`tinytc::make_cooperative_matrix_extract`

  * :ref:`tinytc::make_cooperative_matrix_insert`

  * :ref:`tinytc::make_cooperative_matrix_load`

  * :ref:`tinytc::make_cooperative_matrix_mul_add`

  * :ref:`tinytc::make_cooperative_matrix_prefetch`

  * :ref:`tinytc::make_cooperative_matrix_scale`

  * :ref:`tinytc::make_cooperative_matrix_store`

  * :ref:`tinytc::make_cumsum`

  * :ref:`tinytc::make_expand`

  * :ref:`tinytc::make_for`

  * :ref:`tinytc::make_foreach`

  * :ref:`tinytc::make_fuse`

  * :ref:`tinytc::make_gemm`

  * :ref:`tinytc::make_gemv`

  * :ref:`tinytc::make_ger`

  * :ref:`tinytc::make_hadamard`

  * :ref:`tinytc::make_if`

  * :ref:`tinytc::make_load`

  * :ref:`tinytc::make_math(math_unary,value,data_type,location const&)`

  * :ref:`tinytc::make_parallel`

  * :ref:`tinytc::make_size`

  * :ref:`tinytc::make_store`

  * :ref:`tinytc::make_subgroup_broadcast`

  * :ref:`tinytc::make_subgroup_operation`

  * :ref:`tinytc::make_subview`

  * :ref:`tinytc::make_sum`

  * :ref:`tinytc::make_yield`

* Classes

  * :ref:`tinytc::inst`

Instruction Functions
---------------------

.. _tinytc::make_alloca:

make_alloca
...........

.. doxygenfunction:: tinytc::make_alloca

.. _tinytc::make_axpby:

make_axpby
..........

.. doxygenfunction:: tinytc::make_axpby

.. _tinytc::make_arith(arithmetic,value,value,data_type,location const&):

make_arith(arithmetic,value,value,data_type,location const&)
............................................................

.. doxygenfunction:: tinytc::make_arith(arithmetic,value,value,data_type,location const&)

.. _tinytc::make_arith(arithmetic_unary,value,data_type,location const&):

make_arith(arithmetic_unary,value,data_type,location const&)
............................................................

.. doxygenfunction:: tinytc::make_arith(arithmetic_unary,value,data_type,location const&)

.. _tinytc::make_barrier:

make_barrier
............

.. doxygenfunction:: tinytc::make_barrier

.. _tinytc::make_builtin:

make_builtin
............

.. doxygenfunction:: tinytc::make_builtin

.. _tinytc::make_cast:

make_cast
.........

.. doxygenfunction:: tinytc::make_cast

.. _tinytc::make_cmp:

make_cmp
........

.. doxygenfunction:: tinytc::make_cmp

.. _tinytc::make_constant(bool,data_type,location const&):

make_constant(bool,data_type,location const&)
.............................................

.. doxygenfunction:: tinytc::make_constant(bool,data_type,location const&)

.. _tinytc::make_constant(std::complex\<double\>,data_type,location const&):

make_constant(std::complex<double>,data_type,location const&)
.............................................................

.. doxygenfunction:: tinytc::make_constant(std::complex<double>,data_type,location const&)

.. _tinytc::make_constant(double,data_type,location const&):

make_constant(double,data_type,location const&)
...............................................

.. doxygenfunction:: tinytc::make_constant(double,data_type,location const&)

.. _tinytc::make_constant(std::int32_t,data_type,location const&):

make_constant(std::int32_t,data_type,location const&)
.....................................................

.. doxygenfunction:: tinytc::make_constant(std::int32_t,data_type,location const&)

.. _tinytc::make_constant(std::int64_t,data_type,location const&):

make_constant(std::int64_t,data_type,location const&)
.....................................................

.. doxygenfunction:: tinytc::make_constant(std::int64_t,data_type,location const&)

.. _tinytc::make_constant_one:

make_constant_one
.................

.. doxygenfunction:: tinytc::make_constant_one

.. _tinytc::make_constant_zero:

make_constant_zero
..................

.. doxygenfunction:: tinytc::make_constant_zero

.. _tinytc::make_cooperative_matrix_apply:

make_cooperative_matrix_apply
.............................

.. doxygenfunction:: tinytc::make_cooperative_matrix_apply

.. _tinytc::make_cooperative_matrix_extract:

make_cooperative_matrix_extract
...............................

.. doxygenfunction:: tinytc::make_cooperative_matrix_extract

.. _tinytc::make_cooperative_matrix_insert:

make_cooperative_matrix_insert
..............................

.. doxygenfunction:: tinytc::make_cooperative_matrix_insert

.. _tinytc::make_cooperative_matrix_load:

make_cooperative_matrix_load
............................

.. doxygenfunction:: tinytc::make_cooperative_matrix_load

.. _tinytc::make_cooperative_matrix_mul_add:

make_cooperative_matrix_mul_add
...............................

.. doxygenfunction:: tinytc::make_cooperative_matrix_mul_add

.. _tinytc::make_cooperative_matrix_prefetch:

make_cooperative_matrix_prefetch
................................

.. doxygenfunction:: tinytc::make_cooperative_matrix_prefetch

.. _tinytc::make_cooperative_matrix_scale:

make_cooperative_matrix_scale
.............................

.. doxygenfunction:: tinytc::make_cooperative_matrix_scale

.. _tinytc::make_cooperative_matrix_store:

make_cooperative_matrix_store
.............................

.. doxygenfunction:: tinytc::make_cooperative_matrix_store

.. _tinytc::make_cumsum:

make_cumsum
...........

.. doxygenfunction:: tinytc::make_cumsum

.. _tinytc::make_expand:

make_expand
...........

.. doxygenfunction:: tinytc::make_expand

.. _tinytc::make_for:

make_for
........

.. doxygenfunction:: tinytc::make_for

.. _tinytc::make_foreach:

make_foreach
............

.. doxygenfunction:: tinytc::make_foreach

.. _tinytc::make_fuse:

make_fuse
.........

.. doxygenfunction:: tinytc::make_fuse

.. _tinytc::make_gemm:

make_gemm
.........

.. doxygenfunction:: tinytc::make_gemm

.. _tinytc::make_gemv:

make_gemv
.........

.. doxygenfunction:: tinytc::make_gemv

.. _tinytc::make_ger:

make_ger
........

.. doxygenfunction:: tinytc::make_ger

.. _tinytc::make_hadamard:

make_hadamard
.............

.. doxygenfunction:: tinytc::make_hadamard

.. _tinytc::make_if:

make_if
.......

.. doxygenfunction:: tinytc::make_if

.. _tinytc::make_load:

make_load
.........

.. doxygenfunction:: tinytc::make_load

.. _tinytc::make_math(math_unary,value,data_type,location const&):

make_math(math_unary,value,data_type,location const&)
.....................................................

.. doxygenfunction:: tinytc::make_math(math_unary,value,data_type,location const&)

.. _tinytc::make_parallel:

make_parallel
.............

.. doxygenfunction:: tinytc::make_parallel

.. _tinytc::make_size:

make_size
.........

.. doxygenfunction:: tinytc::make_size

.. _tinytc::make_store:

make_store
..........

.. doxygenfunction:: tinytc::make_store

.. _tinytc::make_subgroup_broadcast:

make_subgroup_broadcast
.......................

.. doxygenfunction:: tinytc::make_subgroup_broadcast

.. _tinytc::make_subgroup_operation:

make_subgroup_operation
.......................

.. doxygenfunction:: tinytc::make_subgroup_operation

.. _tinytc::make_subview:

make_subview
............

.. doxygenfunction:: tinytc::make_subview

.. _tinytc::make_sum:

make_sum
........

.. doxygenfunction:: tinytc::make_sum

.. _tinytc::make_yield:

make_yield
..........

.. doxygenfunction:: tinytc::make_yield

Instruction Classes
-------------------

.. _tinytc::inst:

inst
....

.. doxygenclass:: tinytc::inst

Program
=======

* Functions

  * :ref:`tinytc::make_prog`

* Classes

  * :ref:`tinytc::prog`

Program Functions
-----------------

.. _tinytc::make_prog:

make_prog
.........

.. doxygenfunction:: tinytc::make_prog

Program Classes
---------------

.. _tinytc::prog:

prog
....

.. doxygenclass:: tinytc::prog

Region
======

* Functions

  * :ref:`tinytc::next`

  * :ref:`tinytc::prev`

* Classes

  * :ref:`tinytc::region`

  * :ref:`tinytc::region_builder`

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

.. _tinytc::region:

region
......

.. doxygenclass:: tinytc::region

.. _tinytc::region_builder:

region_builder
..............

.. doxygenclass:: tinytc::region_builder

Value
=====

* Classes

  * :ref:`tinytc::value`

Value Classes
-------------

.. _tinytc::value:

value
.....

.. doxygenclass:: tinytc::value

