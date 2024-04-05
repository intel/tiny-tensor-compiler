.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

.. _IR builder:

=======
Builder
=======

The IR builder classes provide an alternative to providing intermediate representation
in textual form.
For example,

.. literalinclude:: builder_example.cpp
   :language: c++
   :linenos:

produces the following IR:

.. literalinclude:: builder_example.ir
   :linenos:

In line 5 and 10 of the builder sample, functors are provided that accept a
`function_builder&` or `region_builder&`, respectively.
All commands issued to the builder within the functor are encapsulated in
a region, thus, the hierarchical structure of the builder code is very similar
to that of the IR it produces.

While the use of builders is less concise than writing IR in textual form,
code generation can become much simpler.
For example, if the shape of "A" in line 6 depends on some run-time computation,
one can simply put a function call returning a shape there, instead of having
to search-and-replace in textual IR.
Another example is that several implementations of the functions body exist.
These can be swapped out by providing a different functor to `fb.body` while
the `function_builder` stays unaltered.

Builder classes
===============

.. doxygenclass:: tinytc::ir::region_builder
   :members:

.. doxygenclass:: tinytc::ir::function_builder
   :members:

.. doxygenclass:: tinytc::ir::program_builder
   :members:

Constants
=========

.. doxygenvariable:: tinytc::ir::dynamic

Functions
=========

.. doxygenfunction:: tinytc::ir::void_type

.. doxygenfunction:: tinytc::ir::memref_type

.. doxygenfunction:: tinytc::ir::group_type

.. doxygenfunction:: tinytc::ir::size

.. doxygenfunction:: tinytc::ir::is_floating_type

.. doxygenfunction:: tinytc::ir::is_dynamic_value

.. doxygenstruct:: tinytc::ir::to_scalar_type

.. doxygenvariable:: tinytc::ir::to_scalar_type_v

.. doxygenfunction:: tinytc::ir::to_string(binary_op)

.. doxygenfunction:: tinytc::ir::to_string(cmp_condition)

.. doxygenfunction:: tinytc::ir::to_string(scalar_type)

.. doxygenfunction:: tinytc::ir::to_string(transpose)

Enum classes
============

.. doxygenenum:: tinytc::ir::scalar_type

.. doxygenenum:: tinytc::ir::binary_op

.. doxygenenum:: tinytc::ir::cmp_condition

.. doxygenenum:: tinytc::ir::inst_kind

.. doxygenenum:: tinytc::ir::transpose

Handles
=======

.. doxygenclass:: tinytc::ir::data_type
   :members:

.. doxygenclass:: tinytc::ir::func
   :members:

.. doxygenclass:: tinytc::ir::inst
   :members:

.. doxygenclass:: tinytc::ir::prog
   :members:

.. doxygenclass:: tinytc::ir::region
   :members:

.. doxygenclass:: tinytc::ir::slice
   :members:

.. doxygenclass:: tinytc::ir::value
   :members:
