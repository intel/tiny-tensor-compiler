.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

==================
Calling convention
==================

A :ref:`function argument <tensor language functions>` corresponds to one or multiple
arguments in the generated OpenCL-C code.

Scalar types
============

A scalar argument always corresponds to a single scalar argument in the OpenCL-C code.

============= ===========
Argument type OpenCL type
============= ===========
bool          bool
index         long
i8            char
i16           short
i32           int
i64           long
u8            uchar
u16           ushort
u32           uint
u64           ulong
f32           float
f64           double
============= ===========

For example,

.. code::

   func @scalar_example(%a: i16) {}

leads to

.. code:: c

   kernel void scalar_example(short a) {}

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
