.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

===============
Builder C++-API
===============

Common
======

* Functions

  * :ref:`is_dynamic_value`

  * :ref:`to_string(binary_op)`

  * :ref:`to_string(cmp_condition)`

  * :ref:`to_string(scalar_type)`

  * :ref:`to_string(transpose)`

  * :ref:`size`

* Classes

  * :ref:`builder_error`

Common Functions
----------------

is_dynamic_value
................

.. doxygenfunction:: tinytc::is_dynamic_value

to_string(binary_op)
....................

.. doxygenfunction:: tinytc::to_string(binary_op)

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

Common Classs
-------------

builder_error
.............

.. doxygenclass:: tinytc::builder_error

Data Type
=========

* Functions

  * :ref:`create_memref`

  * :ref:`create_group`

* Classes

  * :ref:`data_type`

* Structures

  * :ref:`to_scalar_type`

* Variables

  * :ref:`to_scalar_type_v`

Data Type Functions
-------------------

create_memref
.............

.. doxygenfunction:: tinytc::create_memref

create_group
............

.. doxygenfunction:: tinytc::create_group

Data Type Classs
----------------

data_type
.........

.. doxygenclass:: tinytc::data_type

Data Type Structs
-----------------

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

  * :ref:`create_function`

  * :ref:`create_function_prototype`

  * :ref:`set_work_group_size`

  * :ref:`set_subgroup_size`

* Classes

  * :ref:`func`

  * :ref:`function_builder`

Function Functions
------------------

create_function
...............

.. doxygenfunction:: tinytc::create_function

create_function_prototype
.........................

.. doxygenfunction:: tinytc::create_function_prototype

set_work_group_size
...................

.. doxygenfunction:: tinytc::set_work_group_size

set_subgroup_size
.................

.. doxygenfunction:: tinytc::set_subgroup_size

Function Classs
---------------

func
....

.. doxygenclass:: tinytc::func

function_builder
................

.. doxygenclass:: tinytc::function_builder

Instruction
===========

* Functions

  * :ref:`create_alloca`

  * :ref:`create_axpby`

  * :ref:`create_binary_op`

  * :ref:`create_cast`

  * :ref:`create_cmp`

  * :ref:`create_expand`

  * :ref:`create_for`

  * :ref:`create_foreach`

  * :ref:`create_fuse`

  * :ref:`create_gemm`

  * :ref:`create_gemv`

  * :ref:`create_ger`

  * :ref:`create_group_id`

  * :ref:`create_group_size`

  * :ref:`create_hadamard`

  * :ref:`create_if`

  * :ref:`create_load`

  * :ref:`create_neg`

  * :ref:`create_size`

  * :ref:`create_store`

  * :ref:`create_subview`

  * :ref:`create_sum`

  * :ref:`create_yield`

* Classes

  * :ref:`inst`

Instruction Functions
---------------------

create_alloca
.............

.. doxygenfunction:: tinytc::create_alloca

create_axpby
............

.. doxygenfunction:: tinytc::create_axpby

create_binary_op
................

.. doxygenfunction:: tinytc::create_binary_op

create_cast
...........

.. doxygenfunction:: tinytc::create_cast

create_cmp
..........

.. doxygenfunction:: tinytc::create_cmp

create_expand
.............

.. doxygenfunction:: tinytc::create_expand

create_for
..........

.. doxygenfunction:: tinytc::create_for

create_foreach
..............

.. doxygenfunction:: tinytc::create_foreach

create_fuse
...........

.. doxygenfunction:: tinytc::create_fuse

create_gemm
...........

.. doxygenfunction:: tinytc::create_gemm

create_gemv
...........

.. doxygenfunction:: tinytc::create_gemv

create_ger
..........

.. doxygenfunction:: tinytc::create_ger

create_group_id
...............

.. doxygenfunction:: tinytc::create_group_id

create_group_size
.................

.. doxygenfunction:: tinytc::create_group_size

create_hadamard
...............

.. doxygenfunction:: tinytc::create_hadamard

create_if
.........

.. doxygenfunction:: tinytc::create_if

create_load
...........

.. doxygenfunction:: tinytc::create_load

create_neg
..........

.. doxygenfunction:: tinytc::create_neg

create_size
...........

.. doxygenfunction:: tinytc::create_size

create_store
............

.. doxygenfunction:: tinytc::create_store

create_subview
..............

.. doxygenfunction:: tinytc::create_subview

create_sum
..........

.. doxygenfunction:: tinytc::create_sum

create_yield
............

.. doxygenfunction:: tinytc::create_yield

Instruction Classs
------------------

inst
....

.. doxygenclass:: tinytc::inst

Program
=======

* Classes

  * :ref:`prog`

  * :ref:`program_builder`

Program Classs
--------------

prog
....

.. doxygenclass:: tinytc::prog

program_builder
...............

.. doxygenclass:: tinytc::program_builder

Region
======

* Classes

  * :ref:`region`

  * :ref:`region_builder`

Region Classs
-------------

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

Value Classs
------------

value
.....

.. doxygenclass:: tinytc::value

