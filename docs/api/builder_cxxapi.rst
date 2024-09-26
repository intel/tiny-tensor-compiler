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

  * :ref:`scalar_type`

  * :ref:`transpose`

* Functions

  * :ref:`is_dynamic_value`

  * :ref:`to_string(address_space)`

  * :ref:`to_string(arithmetic)`

  * :ref:`to_string(arithmetic_unary)`

  * :ref:`to_string(cmp_condition)`

  * :ref:`to_string(scalar_type)`

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

scalar_type
...........

.. doxygenenum:: tinytc::scalar_type

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

to_string(scalar_type)
......................

.. doxygenfunction:: tinytc::to_string(scalar_type)

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

  * :ref:`get_memref`

  * :ref:`get_group`

  * :ref:`get_scalar`

* Structures

  * :ref:`to_scalar_type`

* Variables

  * :ref:`to_scalar_type_v`

Data Type Functions
-------------------

get_memref
..........

.. doxygenfunction:: tinytc::get_memref

get_group
.........

.. doxygenfunction:: tinytc::get_group

get_scalar
..........

.. doxygenfunction:: tinytc::get_scalar

Data Type Structures
--------------------

to_scalar_type
..............

.. doxygenstruct:: tinytc::to_scalar_type

Data Type Variables
-------------------

to_scalar_type_v
................

.. doxygenvariable:: tinytc::to_scalar_type_v

Function
========

* Functions

  * :ref:`make_function`

  * :ref:`set_work_group_size`

  * :ref:`set_subgroup_size`

* Classes

  * :ref:`func`

  * :ref:`function_builder`

Function Functions
------------------

make_function
.............

.. doxygenfunction:: tinytc::make_function

set_work_group_size
...................

.. doxygenfunction:: tinytc::set_work_group_size

set_subgroup_size
.................

.. doxygenfunction:: tinytc::set_subgroup_size

Function Classes
----------------

func
....

.. doxygenclass:: tinytc::func

function_builder
................

.. doxygenclass:: tinytc::function_builder

Instruction
===========

* Functions

  * :ref:`make_alloca`

  * :ref:`make_axpby`

  * :ref:`make_arith(arithmetic,value const&,value const&,location const&)`

  * :ref:`make_arith(arithmetic_unary,value const&,location const&)`

  * :ref:`make_cast`

  * :ref:`make_cmp`

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

make_arith(arithmetic,value const&,value const&,location const&)
................................................................

.. doxygenfunction:: tinytc::make_arith(arithmetic,value const&,value const&,location const&)

make_arith(arithmetic_unary,value const&,location const&)
.........................................................

.. doxygenfunction:: tinytc::make_arith(arithmetic_unary,value const&,location const&)

make_cast
.........

.. doxygenfunction:: tinytc::make_cast

make_cmp
........

.. doxygenfunction:: tinytc::make_cmp

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

  * :ref:`make_program`

* Classes

  * :ref:`prog`

  * :ref:`program_builder`

Program Functions
-----------------

make_program
............

.. doxygenfunction:: tinytc::make_program

Program Classes
---------------

prog
....

.. doxygenclass:: tinytc::prog

program_builder
...............

.. doxygenclass:: tinytc::program_builder

Region
======

* Functions

  * :ref:`make_region`

* Classes

  * :ref:`region`

  * :ref:`region_builder`

Region Functions
----------------

make_region
...........

.. doxygenfunction:: tinytc::make_region

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

* Functions

  * :ref:`make_fimm`

  * :ref:`make_imm`

  * :ref:`make_value`

* Classes

  * :ref:`value`

Value Functions
---------------

make_fimm
.........

.. doxygenfunction:: tinytc::make_fimm

make_imm
........

.. doxygenfunction:: tinytc::make_imm

make_value
..........

.. doxygenfunction:: tinytc::make_value

Value Classes
-------------

value
.....

.. doxygenclass:: tinytc::value

