.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

==============
Error handling
==============

Any input error or internal error that is detected during creation of the abstract syntax tree
during IR parsing or IR building throws a compilation error.
The compilation error contains the source location as well as an explanatory string.

Exceptions
==========

.. doxygenclass:: tinytc::ir::compilation_error
   :members:

Typedefs
========

.. doxygentypedef:: tinytc::ir::error_reporter_function

Functions
=========

.. doxygenfunction:: tinytc::ir::report_error_with_context
.. doxygenfunction:: tinytc::ir::operator+=(position&,int)
.. doxygenfunction:: tinytc::ir::operator+(position,int)
.. doxygenfunction:: tinytc::ir::operator-=(position&,int)
.. doxygenfunction:: tinytc::ir::operator-(position,int)
.. doxygenfunction:: std::operator<<(ostream&,::tinytc::ir::position const&)
.. doxygenfunction:: std::operator<<(ostream&,::tinytc::ir::location const&)

Location tracking
=================

.. doxygenclass:: tinytc::ir::position
   :members:

.. doxygenclass:: tinytc::ir::location
   :members:
