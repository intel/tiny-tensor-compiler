.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

.. _calling convention:

==================
Calling convention
==================

A :ref:`function argument <tensor language functions>` corresponds to one or multiple
arguments in the generated OpenCL-C code.

Scalar types
============

A scalar argument always corresponds to a single scalar argument in the OpenCL-C code.

============= ============= ====================
Argument type OpenCL C type Kernel argument type
============= ============= ====================
i1            bool          n/a [#f1]_
i8            char          cl_char
i16           short         cl_short
i32           int           cl_int
i64           long          cl_long
index         long          cl_long
f32           float         cl_float
f64           double        cl_double
============= ============= ====================

For example,

.. code::

   func @scalar_example(%a: i16) {}

leads to

.. code:: c

   kernel void scalar_example(short a) {}

.. rubric:: Footnotes

.. [#f1] Arguments to kernel functions cannot be declared with the type bool in OpenCL

Memref types
============

A memref argument might require multiple arguments in the OpenCL-C code.
The rule is that the first argument in the OpenCL kernel is a global pointer to the underlying scalar type
and then an argument follows for every '?' in the memref's shape or stride, ordered from left-to-right.

For example,

.. code::

   func @memref_example1(%a: memref<f32x5x10>) {}
   func @memref_example2(%a: memref<f64x5x?,strided<1,5>>) {}
   func @memref_example3(%a: memref<i64x5x?x6,strided<1,7,?>>) {}
   func @memref_example4(%a: memref<i64x5x?x6>) {}

leads to

.. code:: c

   kernel void memref_example1(global float* a) {}
   kernel void memref_example2(global double* a, long a_shape1) {}
   kernel void memref_example3(global long* a, long a_shape1, long a_stride2) {}
   kernel void memref_example4(global long* a, long a_shape1, long a_stride2) {}

Note that `memref_example3` and `memref_example4` have the same signature,
because `memref<i64x5x?x6>` has the canonical stride `strided<1,5,?>`.

.. _memref alignment requirements:

**Memory alignment:** The base pointer must be sufficiently aligned.
The required alignment depends on the core info and may be queried with
:ref:`tinytc_core_info_get_default_alignment`.
Using :ref:`tinytc_core_info_set_default_alignment` the alignment requirements may be overriden.
The alignment requirement may also be overriden per memref using the
:ref:`"align" attribute <tensor language functions>`.
When the core info object is created using :ref:`tinytc_cl_core_info_create` the default alignment
is queried from `CL_DEVICE_MEM_BASE_ADDR_ALIGN <https://registry.khronos.org/OpenCL/specs/3.0-unified/html/OpenCL_API.html#platform-querying-devices>`_.

Group types
===========

A group argument might require multiple arguments in the OpenCL-C code.
The rule is that the first argument in the OpenCL kernel is a global pointer to a global pointer to the
underlying scalar type of the memref.
Then a global pointer argument follows for every '?' in the memref's shape or stride, ordered from left-to-right.
If an dynamic offset is given, the offset is the last argument.


.. code::

   func @group_example1(%a: group<memref<i16x5x6>) {}
   func @group_example2(%a: group<memref<i32x5x?x6>>) {}
   func @group_example3(%a: group<memref<f32x?>, offset: ?>) {}

leads to

.. code:: c

   kernel void group_example1(global short*global* a) {}
   kernel void group_example2(global int*global* a, global long* a_shape1, global long* a_stride2) {}
   kernel void group_example3(global float*global* a, global long* a_shape0, long a_offset) {}

Note that `a_shape_0`, `a_shape1`, and `a_stride2` must contain at least as many values as the group size.
That is, if a is accessed with `load %a[%id] : group<memref<i32x5x?x6>>`, then
`*(a_shape0 + id)`, `*(a_shape1 + id)`, and `*(a_stride2 + id)` must not lead to out-of-bounds memory access.

**Memory alignment:** The memrefs the group points to are subject to the same alignment requirements as a
:ref:`regular memref argument (see above) <memref alignment requirements>`. 
