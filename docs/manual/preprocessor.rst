.. Copyright (C) 2025 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

======================
Preprocessor reference
======================

The preprocessor allows simple compile-time specialization of the :ref:`tensor language <tensor language>`.
Identifiers starting with a '$' are compile-time variables.
A variable can be either a constant, a type, or an attribute, and can be used in the language wherever
one of the former is expected.
Definitions of variables can either appear in global scope or in a region.
If a variable is given in a region, its scope is limited to the region and its subregions.

**Example:** Assume we want a fixed-size memref and we want to specialize the size of the memref at compile-time
as well as the element type, then we could write

.. code::

    $element_ty = f32
    $chunk_size = 64
    %x = subview %X[0:$chunk_size] : memref<$element_ty x $chunk_size>
    ; ... or ...
    $chunk_ty = memref<$element_ty x $chunk_size>
    %x = subview %X[0:$chunk_size] : $chunk_ty

Moreover, a simple calculator is available.
The operands must be integers and the expression is given in reverse Polish notation.
For example to calculate :math:`c = \text{ceil}(a/b) = 1 + \lfloor(a-1) / b\rfloor` at compile-time we write

.. code::

    $c = !calc($a 1 - b / 1 +)

.. note::
   
   The preprocessor is only implemented in the parser and is considered a syntax extension to the core language.
   Compile time definitions are resolved at parse time and are not represented in the AST.
   Therefore, compile-time variables are not present in the :ref:`Builder C-API` or :ref:`Builder C++-API`,
   and they are lost when dumping the IR, too.

Identifier
==========

The following `ABNF grammer <https://www.ietf.org/rfc/rfc5234.txt>`_ extends the :ref:`tensor language`.

Compile-time variables are prefixed with the '$'.

.. code:: abnf

    def-identifier           = "$" identifier

Compile-time variables may be used in place of any boolean constant, integer constant, floating constant,
type, or attribute.

Definition
==========

A compile-time variable must be assigned only once within a scope and the scope's sub-scopes.
Assignments are written as following:

.. code:: abnf

    def-assign               = def-identifier "=" def-rhs

The *def-assign* non-terminal may appear in global scope as well as region scope, following or preceding
a function or instruction.
Assignment must precede the use of a compile-time variable.

Definition types
================

Compile-time variables can be constant:

.. code:: abnf

    def-rhs     = boolean-constant / floating-constant / integer-constant

The array, or dictionary, or string attributes can be defined,
but boolean and integer attribute cannot be defined to not confuse those with constants.

.. code:: abnf

   def-rhs      =/ array-attribute
   def-rhs      =/ dictionary-attribute
   def-rhs      =/ string-attribute

Lastly, definitions can be types or alias another definition.

.. code:: abnf

   def-rhs      =/ data-type
   def-rhs      =/ def-identifier

Calculator
==========

The right-hand side of a definition can stem from a calculation given reverse Polish notation

.. code:: abnf

    def-rhs     =/ "!calc" "(" rpn-expr ")"
    rpn-expr    = int-or-def /
                  rpn-expr rpn-expr "^" /
                  rpn-expr rpn-expr "-" /
                  rpn-expr rpn-expr "+" /
                  rpn-expr rpn-expr "*" /
                  rpn-expr rpn-expr "/" /
                  rpn-expr rpn-expr "%" /
                  rpn-expr rpn-expr "min" /
                  rpn-expr rpn-expr "max"
    int-or-def = integer-constant / def-identifier

The operators are summarized in the following table:

======== ==================
Operator Description
======== ==================
``^``    Power
``-``    Subtraction
``+``    Addition
``*``    Multiplication
``/``    Integer division
``%``    Remainder
``min``  Minimum
``max``  Maximum
======== ==================
