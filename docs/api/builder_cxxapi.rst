.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

===============
Builder C++-API
===============

Common
======

* Enumerations

  * :ref:`address_space`

  * :ref:`arithmetic`

  * :ref:`arithmetic_unary`

  * :ref:`cmp_condition`

  * :ref:`matrix_use`

  * :ref:`scalar_type`

  * :ref:`store_flag`

  * :ref:`transpose`

* Functions

  * :ref:`is_dynamic_value`

  * :ref:`to_string(address_space)`

  * :ref:`to_string(arithmetic)`

  * :ref:`to_string(arithmetic_unary)`

  * :ref:`to_string(cmp_condition)`

  * :ref:`to_string(matrix_use)`

  * :ref:`to_string(scalar_type)`

  * :ref:`to_string(store_flag)`

  * :ref:`to_string(transpose)`

  * :ref:`size`

* Classes

  * :ref:`builder_error`

* Typedefs

  * :ref:`position`

  * :ref:`location`

* Variables

  * :ref:`dynamic`

Common Enumerations
-------------------

address_space
.............

.. doxygenenum:: tinytc::address_space

arithmetic
..........

.. doxygenenum:: tinytc::arithmetic

arithmetic_unary
................

.. doxygenenum:: tinytc::arithmetic_unary

cmp_condition
.............

.. doxygenenum:: tinytc::cmp_condition

matrix_use
..........

.. doxygenenum:: tinytc::matrix_use

scalar_type
...........

.. doxygenenum:: tinytc::scalar_type

store_flag
..........

.. doxygenenum:: tinytc::store_flag

transpose
.........

.. doxygenenum:: tinytc::transpose

Common Functions
----------------

is_dynamic_value
................

.. doxygenfunction:: tinytc::is_dynamic_value

to_string(address_space)
........................

.. doxygenfunction:: tinytc::to_string(address_space)

to_string(arithmetic)
.....................

.. doxygenfunction:: tinytc::to_string(arithmetic)

to_string(arithmetic_unary)
...........................

.. doxygenfunction:: tinytc::to_string(arithmetic_unary)

to_string(cmp_condition)
........................

.. doxygenfunction:: tinytc::to_string(cmp_condition)

to_string(matrix_use)
.....................

.. doxygenfunction:: tinytc::to_string(matrix_use)

to_string(scalar_type)
......................

.. doxygenfunction:: tinytc::to_string(scalar_type)

to_string(store_flag)
.....................

.. doxygenfunction:: tinytc::to_string(store_flag)

to_string(transpose)
....................

.. doxygenfunction:: tinytc::to_string(transpose)

size
....

.. doxygenfunction:: tinytc::size

Common Classes
--------------

builder_error
.............

.. doxygenclass:: tinytc::builder_error

Common Typedefs
---------------

position
........

.. doxygentypedef:: tinytc::position

location
........

.. doxygentypedef:: tinytc::location

Common Variables
----------------

dynamic
.......

.. doxygenvariable:: tinytc::dynamic

Data Type
=========

* Functions

  * :ref:`get_coopmatrix`

  * :ref:`get_group`

  * :ref:`get_memref`

  * :ref:`get_scalar`

* Structures

  * :ref:`to_scalar_type`

* Typedefs

  * :ref:`data_type`

* Variables

  * :ref:`to_scalar_type_v`

Data Type Functions
-------------------

get_coopmatrix
..............

.. doxygenfunction:: tinytc::get_coopmatrix

get_group
.........

.. doxygenfunction:: tinytc::get_group

get_memref
..........

.. doxygenfunction:: tinytc::get_memref

get_scalar
..........

.. doxygenfunction:: tinytc::get_scalar

Data Type Structures
--------------------

to_scalar_type
..............

.. doxygenstruct:: tinytc::to_scalar_type

Data Type Typedefs
------------------

data_type
.........

.. doxygentypedef:: tinytc::data_type

Data Type Variables
-------------------

to_scalar_type_v
................

.. doxygenvariable:: tinytc::to_scalar_type_v

Function
========

* Functions

  * :ref:`make_func`

* Classes

  * :ref:`func`

Function Functions
------------------

make_func
.........

.. doxygenfunction:: tinytc::make_func

Function Classes
----------------

func
....

.. doxygenclass:: tinytc::func

Instruction
===========

* Functions

  * :ref:`make_alloca`

  * :ref:`make_axpby`

  * :ref:`make_arith(arithmetic,value,value,location const&)`

  * :ref:`make_arith(arithmetic_unary,value,location const&)`

  * :ref:`make_cast`

  * :ref:`make_cmp`

  * :ref:`make_constant(std::complex\<double\>,data_type,location const&)`

  * :ref:`make_constant(double,data_type,location const&)`

  * :ref:`make_constant(std::int32_t,data_type,location const&)`

  * :ref:`make_constant(std::int64_t,data_type,location const&)`

  * :ref:`make_constant_one`

  * :ref:`make_constant_zero`

  * :ref:`make_cooperative_matrix_load`

  * :ref:`make_cooperative_matrix_mul_add`

  * :ref:`make_cooperative_matrix_scale`

  * :ref:`make_cooperative_matrix_store`

  * :ref:`make_expand`

  * :ref:`make_for`

  * :ref:`make_foreach`

  * :ref:`make_fuse`

  * :ref:`make_gemm`

  * :ref:`make_gemv`

  * :ref:`make_ger`

  * :ref:`make_group_id`

  * :ref:`make_group_size`

  * :ref:`make_hadamard`

  * :ref:`make_if`

  * :ref:`make_load`

  * :ref:`make_num_subgroups`

  * :ref:`make_parallel`

  * :ref:`make_size`

  * :ref:`make_store`

  * :ref:`make_subgroup_id`

  * :ref:`make_subgroup_local_id`

  * :ref:`make_subgroup_size`

  * :ref:`make_subview`

  * :ref:`make_sum`

  * :ref:`make_yield`

* Classes

  * :ref:`inst`

Instruction Functions
---------------------

make_alloca
...........

.. doxygenfunction:: tinytc::make_alloca

make_axpby
..........

.. doxygenfunction:: tinytc::make_axpby

make_arith(arithmetic,value,value,location const&)
..................................................

.. doxygenfunction:: tinytc::make_arith(arithmetic,value,value,location const&)

make_arith(arithmetic_unary,value,location const&)
..................................................

.. doxygenfunction:: tinytc::make_arith(arithmetic_unary,value,location const&)

make_cast
.........

.. doxygenfunction:: tinytc::make_cast

make_cmp
........

.. doxygenfunction:: tinytc::make_cmp

make_constant(std::complex<double>,data_type,location const&)
.............................................................

.. doxygenfunction:: tinytc::make_constant(std::complex<double>,data_type,location const&)

make_constant(double,data_type,location const&)
...............................................

.. doxygenfunction:: tinytc::make_constant(double,data_type,location const&)

make_constant(std::int32_t,data_type,location const&)
.....................................................

.. doxygenfunction:: tinytc::make_constant(std::int32_t,data_type,location const&)

make_constant(std::int64_t,data_type,location const&)
.....................................................

.. doxygenfunction:: tinytc::make_constant(std::int64_t,data_type,location const&)

make_constant_one
.................

.. doxygenfunction:: tinytc::make_constant_one

make_constant_zero
..................

.. doxygenfunction:: tinytc::make_constant_zero

make_cooperative_matrix_load
............................

.. doxygenfunction:: tinytc::make_cooperative_matrix_load

make_cooperative_matrix_mul_add
...............................

.. doxygenfunction:: tinytc::make_cooperative_matrix_mul_add

make_cooperative_matrix_scale
.............................

.. doxygenfunction:: tinytc::make_cooperative_matrix_scale

make_cooperative_matrix_store
.............................

.. doxygenfunction:: tinytc::make_cooperative_matrix_store

make_expand
...........

.. doxygenfunction:: tinytc::make_expand

make_for
........

.. doxygenfunction:: tinytc::make_for

make_foreach
............

.. doxygenfunction:: tinytc::make_foreach

make_fuse
.........

.. doxygenfunction:: tinytc::make_fuse

make_gemm
.........

.. doxygenfunction:: tinytc::make_gemm

make_gemv
.........

.. doxygenfunction:: tinytc::make_gemv

make_ger
........

.. doxygenfunction:: tinytc::make_ger

make_group_id
.............

.. doxygenfunction:: tinytc::make_group_id

make_group_size
...............

.. doxygenfunction:: tinytc::make_group_size

make_hadamard
.............

.. doxygenfunction:: tinytc::make_hadamard

make_if
.......

.. doxygenfunction:: tinytc::make_if

make_load
.........

.. doxygenfunction:: tinytc::make_load

make_num_subgroups
..................

.. doxygenfunction:: tinytc::make_num_subgroups

make_parallel
.............

.. doxygenfunction:: tinytc::make_parallel

make_size
.........

.. doxygenfunction:: tinytc::make_size

make_store
..........

.. doxygenfunction:: tinytc::make_store

make_subgroup_id
................

.. doxygenfunction:: tinytc::make_subgroup_id

make_subgroup_local_id
......................

.. doxygenfunction:: tinytc::make_subgroup_local_id

make_subgroup_size
..................

.. doxygenfunction:: tinytc::make_subgroup_size

make_subview
............

.. doxygenfunction:: tinytc::make_subview

make_sum
........

.. doxygenfunction:: tinytc::make_sum

make_yield
..........

.. doxygenfunction:: tinytc::make_yield

Instruction Classes
-------------------

inst
....

.. doxygenclass:: tinytc::inst

Program
=======

* Functions

  * :ref:`make_prog`

* Classes

  * :ref:`prog`

Program Functions
-----------------

make_prog
.........

.. doxygenfunction:: tinytc::make_prog

Program Classes
---------------

prog
....

.. doxygenclass:: tinytc::prog

Region
======

* Classes

  * :ref:`region`

  * :ref:`region_builder`

Region Classes
--------------

region
......

.. doxygenclass:: tinytc::region

region_builder
..............

.. doxygenclass:: tinytc::region_builder

Value
=====

* Classes

  * :ref:`value`

Value Classes
-------------

value
.....

.. doxygenclass:: tinytc::value

