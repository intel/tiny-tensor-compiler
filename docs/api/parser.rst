.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

======
Parser
======

Programs written in the :ref:`tensor language <tensor ir>`
are either parsed using a `source manager`_ or the `parse function`_.
Using the source manager is the recommended method as it enhanced error
messages with code context.

For example, if the source manager is set up as following

.. code-block:: cpp

    #include <tinytc/tinytc.hpp>

    auto srcman = tinytc::source_manager(&std::cerr);
    auto p = srcman.parse_file("test/codegen/type_mismatch0.ir");

and the source file contains an error, then the error message written to cerr contains context:

.. code-block::

   func @kernel(%K0: memref<f32>) {
     %0 = load %K0[] : memref<f64>
               ~~~~~~~~~~~~~~~~~~~
   test/codegen/type_mismatch0.ir:6.13-31: Type of SSA value does not match operand type


.. _source manager:

Source manager
==============

.. doxygenclass:: tinytc::source_manager
   :members:

.. _parse function:

Parse function
==============

.. doxygenfunction:: tinytc::parse


