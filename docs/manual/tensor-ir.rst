.. Copyright (C) 2023 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

.. _tensor language:

=========================
Tensor language reference
=========================

The grammar is given in `ABNF syntax <https://www.ietf.org/rfc/rfc5234.txt>`_.

Execution model
===============

The unit of execution described by a function written in the tensor language
is called a **kernel**. 
Kernels are launched in batches, where each instance of the kernel is called a work-group.
The kernel has access to its group id that is used to select the work done in the work group.
Each work group consists of a fixed number of work-items that execute concurrently. 

The language distinguishes between *collective*, *SPMD*, and *mixed* instructions.
A collective instruction distributes the work among the work-items in an implementation-defined manner.
Local variables passed to or returned from a collective instruction are always uniform, meaning
that each work-item holds the same value.
An SPMD instruction follows the OpenCL execution model, where local variables may have a different value
for each work-item.
Mixed instructions accept both varying and uniform local variables.

Regions come in two different kinds: collective and SPMD.
A collective instructions must only appear in a collective region, and an SPMD instruction
must only appear in a SPMD region. Mixed instructions might appear in both kinds of regions.
SPMD regions may be nested in collective regions but collective regions must not be nested in SPMD regions.

Core rules
==========

White space is used to separate tokens, where a token is either an identifier,
a literal, a keyword, or characters such as punctuation or delimiters.
Otherwise, white space has no meaning.

Comments start with ``;`` and stop at the end of the line (``\n``). 


Identifier
==========

Identifiers are either named or unnamed.
Named identifiers are letter followed by letters, underscores, or digits.
Unnamed identifiers are simply numbers.
As in LLVM, local identifiers are prefixed with ``%``, whereas global identifiers
are prefixed with ``@``.

.. code:: abnf

    identifier                  = unnamed-identifier / named-identifier
    unnamed-identifier          = 1*DIGIT
    named-identifier            = ALPHA *(ALPHA / DIGIT / "_")
    local-identifier            = "%" identifier
    global-identifier           = "@" identifier

Constants
=========

.. code:: abnf

    constant                    = complex-constant / floating-constant / integer-constant
    integer-constant            = "true" / "false" / [sign] 1*DIGIT
    sign                        = "-" / "+"
    floating-constant           = [sign] *DIGIT "." 1*DIGIT ["e" [sign] 1*DIGIT]
    mantissa-dec                = *DIGIT "." 1*DIGIT / 1*DIGIT "."
    mantissa-hex                = *HEXDIG "." 1*HEXDIG / 1*HEXDIG "."
    exponent                    = [sign] 1*DIGIT
    floating-constant-dec       = [sign] (mantissa-dec ["e" exponent] / 1*DIGIT "e" exponent)
    floating-constant-hex       = [sign] "0x" (mantissa-hex ["p" exponent] / 1*HEXDIG "p" exponent)
    floating-constant           = floating-constant-dec / floating-constant-hex
    complex-constant            = "[" floating-constant "," floating-constant "]"

Integer constants must lie in the range :math:`-2^{63}+1,\dots,2^{63}-1`.

Floating point constants are given in C syntax and expected to be in the range of double precision numbers.
The hexadecimal floating point syntax is supported, too.
`strtod <https://en.cppreference.com/w/c/string/byte/strtof>`_ can be used for parsing floating
point numbers.

.. _tensor language functions:

Functions
=========

.. code:: abnf

    function-definition         = "func" global-identifier "(" [argument-list] ")" *attribute region
    argument-list               = argument *("," argument)
    argument                    = local-identifier ":" type
    attribute                   = work-group-size-attribute / subgroup-size-attribute
    work-group-size-attribute   = "work_group_size" "(" 1*DIGIT "," 1*DIGIT ")"
    subgroup-size-attribute     = "subgroup_size" "(" 1*DIGIT ")"

Defines a function that is callable from the host.

Attributes are optional and autoatically determined if omitted.

The work-group size attribute defines the size of the local work group.
Due to the focus on matrix operations, the work-group size is always two-dimensional,
where the first mode is used to tile the rows and the second mode is used
to tile the columns.
The first mode must be a multiple of the subgroup size.
If the subgroup size is omitted, then the first mode must be a multiple of one of
the subgroup sizes supported by the device.
The product of the work-group size modes must be smaller or equal than the maximum
work-group size of device.

The work-group is divided into full subgroups, therefore the work-group size
is always a multiple of the subgroup size.
The subgroup size attribute enforces a particular subgroup device supported by
the device.


Regions
=======

.. code:: abnf

    region                      = "{" *instruction "}"

A region is an ordered list of instructions.
An instruction might contain a region.
Regions have access to values from its enclosing region, but the enclosing region does not have access to 
values assigned in the region.

Types
=====

.. code:: abnf

    type                        = void-type / scalar-type / memref-type / group-type
    void-type                   = "void"

Scalar types
------------

.. code:: abnf

    scalar-type                 = integer-type / floating-type / complex-type
    integer-type                = "i" ("1" / "8" / "16" / "32" / "64") / "index"
    floating-type               = "f" ("32" / "64")
    complex-type                = "c" ("32" / "64")

Scalar types are either signless integer ("i"), floating point ("f"),
or complex floating point ("c").
The number behind the scalar type prefix denotes the number of bits,
e.g. "f64" are double precision floating point numbers.
The "index" type is an integer type whose width is platform-specific.

Memref type
-----------

.. code:: abnf

    memref-type                 = "memref<" scalar-type tensor-shape ["," memory-layout] ["," address-space] ">"
    constant-or-dynamic         = integer-constant / "?"
    tensor-shape                = *("x" constant-or-dynamic)
    address-space               = "global" / "local"

A memref is a reference to a region of memory.
In analogy to the C/C++-language, the memref can be thought of as a pointer,
but with additional information on the size and memory layout of the memory region.
The size information can be either fixed or dynamic.
For example, the ``memref<f32x4x8>`` is analogue to ``float*`` with the additional information
that the memory region contains 32 floats structured in 4 rows and 8 columns.
The ``memref<f32x4x?>`` type is analogue to ``float*``, too, but here the number of floats
and the number of columns is only known at run-time.

Run-time size information is stored in a dope vector; the calling convention for memrefs is
implementation-defined.

The memref can have order 0. E.g. ``memref<f32>`` can be thought of as a pointer to a single precision float.
A vector is a tensor of order 1, e.g. ``memref<f64x4>``.
A matrix is a tensor of order 2, e.g. ``memref<f64x4x4>``.
A tensor of order n is given by ``memref<f32xs_1x...xs_n>``.

Dynamic mode sizes are written using a question mark in place of an integer constant.


The default memory layout is the packed dense layout.
E.g. the memory layout of ``memref<f32x5x6x7>`` is ``strided<1,5,30>``.
We note that ``memref<f32x5x6x7>`` and ``memref<f32x5x6x7,strided<1,5,30>>``
are the same type.

Memrefs have an optional address space attribute.
The global address space referse to memory objects allocated from the global memory pool
that is shared by all work groups.
The local memory space is shared by all work-items of the work-group but inaccessible to another work-group.
The default address space is "global", memrefs with "local" address space are returned by
the alloca instruction.


Memory layout
.............

.. code:: abnf

    memory-layout               = strided-layout

Strided layout
~~~~~~~~~~~~~~

.. code:: abnf

    strided-layout              = "strided<" [constant-or-dynamic-list] ">"
    constant-or-dynamic-list    = constant-or-dynamic *("," constant-or-dynamic)

The strided layout is a sequence of integers :math:`S_1,S_2,...,S_n`, where *n* must be equal
to the order of the tensor.
The strided layout is defined as the map

.. math::

    (i_1,i_2,...,i_n) \mapsto i_1 S_1 + i_2 S_2 + ... + i_n S_n

We further impose the following restriction for a tensor with shape :math:`s_1\times s_2 \times ... \times s_n`:

* :math:`1 \leq S_1`
* :math:`\forall i \in [2,n]: S_{i-1}s_{i-1} \leq S_i`

Therefore, we have the "column-major" layout.
The default packed dense layout is given by

* :math:`1 = S_1`
* :math:`\forall i \in [2,n]: S_{i-1}s_{i-1} = S_i`

Stride modes might be dynamic as well, indicated by a question mark.

Group type
----------

.. code:: abnf

    group-type                  = "group<" memref-type ["," "offset" ":" constant-or-dynamic] ">"

The group type collects unstructured pointers to memref's with potentially different dynamic mode sizes.
The C-analogy of a group is a pointer-to-a-pointer.
For example, the C-analogue of a ``group<memref<f32x16x16>>`` is a ``float**``.

The optional offset parameter is used to offset each pointer by the given number of elements.
Given the C-analogue ``float** group``, loading element ``i`` with offset ``off`` gives the
pointer ``float* tmp = group[i] + off``.
The default offset is 0.

Dynamic values ('?') may appear in the memref-type and in the offset.
These values are stored in the dope vector;
the calling convention for groups is implementation-defined.

Instructions
============

.. code:: abnf

    value-instruction-assignment        = local-identifier "=" value-instruction
    multi-value-instruction-assignment  = [local-identifier-list "="] multi-value-instruction
    local-identifier-list               = local-identifier *("," local-identifier)
    instruction                         = value-instruction-assignment
                                          / multi-value-instruction-assignment


Collective instructions
-----------------------

Alloca
......

.. code:: abnf

    value-instruction   = "alloca" "->" memref-type

Overview
~~~~~~~~

The alloca instruction allocates temporary memory that is freed automatically at the end of the block that contains the alloca.

Returns
~~~~~~~

A memref of the memref-type.

Restrictions
~~~~~~~~~~~~

- The memref's size must known at compile-time, i.e. the tensor shape must not contain any dynamic modes.
- The address space must be "local".

Axpby
.....

.. code:: abnf

    transpose       =  ".t" / ".n"
    instruction     =/ "axpby" transpose [".atomic"]
                               local-identifier "," local-identifier "," local-identifier "," local-identifier
                               ":" scalar-type "," memref-type "," scalar-type "," memref-type

Overview
~~~~~~~~

Axpby implements

.. math::

    B := \alpha \text{op}(A) + \beta B

for vectors and matrices.
If the atomic flag is set, B is updated atomically.

Arguments
~~~~~~~~~

The first argument gives :math:`\alpha`, and the third argument gives :math:`\beta`.
The second and the fourth argument must have memref type and give A and B, respectively.

The transpose modifier defines :math:`\text{op}` as following:

.. math::

    \text{op}_i(X) := \left\{
                      \begin{array}{rcl}
                        X^T & \text{ if } & \text{modifier}_i= t \wedge \text{order}(X) = 2,\\
                        X   & \text{ else. }
                      \end{array}
                      \right.

(Note that ".t" has no effect on vectors.)

The shape of :math:`\text{op}(A)` and B must be identical and the order of A and B needs to be 1 (vector)
or 2 (matrix).

Foreach
.......

.. code:: abnf

    instruction     =/ "foreach" local-identifier "=" local-identifier "," local-identifier
                       [":" integer-type] region

Overview
~~~~~~~~

A foreach loop that executes the loop's range [from; to) without any sequence guarantee.
The region of a foreach is a *spmd region*.

The loop's range [from; to) is given by the first integer value and second integer value,
and the trip count is stored in the local identifier.
The integer type of the loop variable and the loop bounds is given after the colon.
The default integer type is ``index``.

GEMM
....

.. code:: abnf

    instruction     =/ "gemm" transpose transpose [".atomic"]
                       "," local-identifier "," local-identifier "," local-identifier "," local-identifier "," local-identifier
                       ":" scalar-type "," memref-type "," memref-type "," scalar-type "," memref-type

Overview
~~~~~~~~

GEMM implements the well-known GEMM BLAS-3 operation.

.. math::

    C := \alpha \text{op}_1(A) \text{op}_2(B) + \beta C

If the atomic flag is set, C is updated atomically.

Arguments
~~~~~~~~~

The first argument gives :math:`\alpha` and the fourth argument gives :math:`\beta`.
The second, the third, and the fifth argument must have memref type and give
A, B, and C, respectively.

The first transpose modifier defines :math:`\text{op}_1` and the second transpose modifier
defines :math:`\text{op}_2` as following:

.. math::

    \text{op}_i(X) := \left\{
                      \begin{array}{rcl}
                        X^T & \text{ if } & \text{modifier}_i = t,\\
                        X   & \text{ if } & \text{modifier}_i = n.
                      \end{array}
                      \right.


If :math:`\text{op}_1(A)` has the shape MxK and
:math:`\text{op}_2(B)` has the shape KxN then C must have the shape MxN.

GEMV
....

.. code:: abnf

    instruction     =/ "gemv" transpose [".atomic"]
                       "," local-identifier "," local-identifier "," local-identifier "," local-identifier "," local-identifier
                       ":" scalar-type "," memref-type "," memref-type "," scalar-type "," memref-type

Overview
~~~~~~~~

GEMV implements the well-known GEMM BLAS-2 operation.

.. math::

    c := \alpha \text{op}_1(A) b + \beta C

If the atomic flag is set, c is updated atomically.

Arguments
~~~~~~~~~

The first argument gives :math:`\alpha` and the fourth argument gives :math:`\beta`.
The second, the third, and the fifth argument must have memref type and give
A, b, and c, respectively.

The transpose modifier for A as in GEMM.

:math:`\text{op}_1(A)` has the shape MxK and :math:`B` has the shape K then c must have the shape M.

GER
...

.. code:: abnf

    instruction     =/ "ger" [".atomic"]
                       local-identifier "," local-identifier "," local-identifier "," local-identifier "," local-identifier
                       ":" scalar-type "," memref-type "," memref-type "," scalar-type "," memref-type

Overview
~~~~~~~~

Computes the general rank-1 update:

.. math::

    C := \alpha a b^T + \beta C

If the atomic flag is set, C is updated atomically.

Arguments
~~~~~~~~~

The first argument gives :math:`\alpha` and the fourth argument gives :math:`\beta`.
The second, the third, and the fifth argument must have memref type and give
a, b, and C, respectively.

a and b must be vectors. If the size of a is M and the size of b is N the shape of C must be :math:`M\times N`.


Hadamard product
................

.. code:: abnf

    instruction     =/ "hadamard_product" [".atomic"]
                       local-identifier "," local-identifier "," local-identifier "," local-identifier "," local-identifier
                       ":" scalar-type "," memref-type "," memref-type "," scalar-type "," memref-type

Overview
~~~~~~~~

Computes the Hadamard product of two tensors.
That is, in index notation we have

.. math::

    c_{i} := \alpha a_{i} b_{i} + \beta c_{i}

If the atomic flag is set, c is updated atomically.

Arguments
~~~~~~~~~

The first argument gives :math:`\alpha` and the fourth argument gives :math:`\beta`.
The second, the third, and the fifth argument must have memref type and give
a, b, and c, respectively.

a, b, and c must be vectors and have equal shape.

Parallel
........

.. code:: abnf

    instruction     =/ "parallel" region

Overview
~~~~~~~~

Opens an *spmd region*.

Sum
...

.. code:: abnf

    instruction     =/ "sum" transpose [".atomic"]
                       "," local-identifier "," local-identifier "," local-identifier "," local-identifier
                       ":" scalar-type "," memref-type "," scalar-type "," memref-type

Overview
~~~~~~~~

Computes the matrix-vector product or the dot product of A with a vector of ones.
That is, for matrices we have

.. math::

    B := \alpha \text{op}(A) \vec{1} + \beta B

and for vectors we have

.. math::

    b := \alpha \left<a,\vec{1}\right> + \beta b

If the atomic flag is set, B is updated atomically.


Arguments
~~~~~~~~~

The first argument gives :math:`\alpha` and the third argument gives :math:`\beta`.
The second and the fourth argument must have memref type and give A and B, respectively.
If A is a matrix then B must be a vector.
The first mode size of :math:`\text{op}(A)` must match the size of B.
If A is a vector, then B must be a scalar memref.

The transpose op is defined as in the axpby instruction.



Mixed instructions
------------------

Arithmetic (binary)
...................

.. code:: abnf

    arith-binary-type       =  ".add"  /
                               ".sub"  /
                               ".mul"  /
                               ".div" /
                               ".rem" /
                               ".shl"  /
                               ".shr" /
                               ".and"  /
                               ".or"   /
                               ".xor"
    value-instruction       =/ "arith" arith-binary-type local-identifier "," local-identifier ":" scalar-type

Overview
~~~~~~~~

Binary arithmetic operation on scalars.
Both operands, as well as the returned type, have the same scalar type.

The following table shows the operations' description and the types that are allowed for the operation.
The backslash "\\" is used to exclude types from the list of allowed types.

==== ============================ ================================================================
Op   Allowed type                 Description
==== ============================ ================================================================
.add scalar-type                  Sum of operands
.sub scalar-type                  Difference of operands
.mul scalar-type                  Product of operands
.div scalar-type                  Quotient of operands
.rem scalar-type  \\ complex-type Remainder from the division of operands
.shl integer-type \\ i1           Left shift first operand by second operand
.shr integer-type \\ i1           Arithmetic right shift first operand by second operand
.and integer-type                 Bitwise and
.or  integer-type                 Bitwise or
.xor integer-type                 Bitwise xor
==== ============================ ================================================================

Arithmetic (unary)
..................

.. code:: abnf

    arith-unary-type        =  ".neg"  / ".not"
    value-instruction       =/ "arith" arith-unary-type local-identifier ":" scalar-type

Overview
~~~~~~~~

Unary arithmetic operation on scalars.
The returned value has the same type as the operand.

The following table shows the operations' description and the types that are allowed for the operation.

==== ============ ==============================================================================
Op   Allowed type Description
==== ============ ==============================================================================
.neg scalar-type  Negation
.not integer-type Bitwise not
==== ============ ==============================================================================

Barrier
.......

.. code:: abnf

    instruction             =/ "barrier" [".global"] [".local"]

Overview
~~~~~~~~

**Note:** Barriers are inserted automatically in collective regions, but not in SPMD regions.
Manual barrier insertion should only be only necessesary in SPMD regions.


Control barrier.
The barrier must be encountered by all work-items.
A work-item in a work-group is not allowed to continue until all work-items in the work-group
have reached the barrier.

Aditional memory fences are controlled by the following attributes:

========= ======================================================================================
Attribute Description
========= ======================================================================================
.global   Ensure that global memory accesses become visible to the work-group.
.local    Ensure that local memory accesses become visible to the work-group.
========= ======================================================================================

Cast
....

.. code:: abnf

    value-instruction       =/ "cast" local-identifier ":" scalar-type "->" scalar-type

Overview
~~~~~~~~

Cast scalar values.

Comparison
..........

.. code:: abnf

    value-instruction       =/ "cmp" (".eq" / ".ne" / ".gt" / ".ge" / ".lt" / ".le")
                               local-identifier "," local-identifier ":" scalar-type

Overview
~~~~~~~~

Scalar comparison.
Both operands must have the same scalar type and the returned value is boolean.

The following table shows the comparisons' description and the types that are allowed for the comparison.
The backslash "\\" is used to exclude types from the list of allowed types.

==== =========================== =====================
Cond Allowed type Description
==== =========================== =====================
.eq  scalar-type                 Equal
.ne  scalar-type                 Not equal
.gt  scalar-type \\ complex-type Greater than
.ge  scalar-type \\ complex-type Greater than or equal
.lt  scalar-type \\ complex-type Less than
.le  scalar-type \\ complex-type Less than or equal
==== =========================== =====================

Constant
........

.. code:: abnf

    value-instruction       =/ "constant" constant "->" scalar-type

Overview
~~~~~~~~

Sets the result value to a constant value.
The type of the constant must match the scalar type
(e.g. an integer type requires an integer-constant and a floating type requires a floating-constant).

Expand
......

.. code:: abnf

    value-instruction       =/ "expand" local-identifier "[" integer-constant "->" expand-shape "]" ":" memref-type
    expand-shape            =  integer-constant-or-identifier 1*("x" integer-constant-or-identifier)
    integer-constant-or-identifier = integer-constant / local-identifier

Overview
~~~~~~~~

The expand instruction returns a view on a tensor with a mode viewed as higher-order mode.

Arguments
~~~~~~~~~

The first argument must point to a value of memref type.
The first integer constant before "->" gives the mode that shall be expanded.
The expand shape coming after "->" gives the new shape of the mode.
Dynamic values in the expand shape must have index type.

The output type is a memref type according to the following rules:

#. **Shape:** The mode size is replaced with the expand shape.
   The product of the expand shape must equal the size of the expanded mode.

   .. code::

       expand %0[1 -> 2x8]      : memref<f32x32x16x8> ; -> memref<f32x32x2x8x8>
       expand %0[1 -> 2x2x2x2]  : memref<f32x32x16x8> ; -> memref<f32x32x2x2x2x2x8>

#. **Identifiers:** Local identifiers in the expand shape are dynamic in the resulting memref type.
   The product of the dynamic expand shape must equal the size of the expanded mode.

   .. code::

       expand %0[1 -> %1 x 2]      : memref<f32x32x?>  ; -> memref<f32x32x?x2>
       expand %0[1 -> 2 x %1]      : memref<f32x32x?>  ; -> memref<f32x32x2x?>
       expand %0[1 -> %1 x 2]      : memref<f32x32x16> ; -> memref<f32x32x?x2>
       expand %0[1 -> %1 x 2]      : memref<f32x32x?>  ; -> memref<f32x32x?x2>
       expand %0[1 -> %1 x %2 x 2] : memref<f32x32x16> ; -> memref<f32x32x?x?x2>
       expand %0[1 -> %2 x 2 x %1] : memref<f32x32x16> ; -> memref<f32x32x?x2x?>
       expand %0[1 -> %1 x %2]     : memref<f32x32x?>  ; -> memref<f32x32x?x?>
       expand %0[1 -> %1 x %2]     : memref<f32x32x16> ; -> memref<f32x32x?x?>

   *Note:* In the third example above, %1 must be equal to 8.
   The output mode corresponding to %1 is still dynamic.

#. **Stride:** A new stride entry is entered that follows the canonical stride computation.

   .. code::

       expand %0[0->4 x 8] : memref<f32x32x7,strided<2,64>> ; -> memref<f32x4x8x7,strided<2,8,64>>
       expand %0[0->%1 x 4] : memref<f32x?x7,strided<2,?>>   ; -> memref<f32x?x4x7,strided<2,?,?>>
       expand %0[0->4 x %1] : memref<f32x?x7,strided<2,?>>   ; -> memref<f32x4x?x7,strided<2,8,?>>

Restrictions
~~~~~~~~~~~~

The product of the expand shape must be the same as the mode size.
If the product of the expand shape is only known at runtime, then it is undefined behaviour
if the dynamic product does not match the mode size.

Fuse
....

.. code:: abnf

    value-instruction       =& "fuse" local-identifier "[" integer-constant "," integer-constant "]" ":" memref-type

Overview
~~~~~~~~

The fuse instruction returns a view on a tensor with two or more adjacent modes viewed as a single mode.

Arguments
~~~~~~~~~

The first argument must point to a value of memref type.
The fused modes are specified as the interval [from, to], where from is given
by the first integer and to is given by the second integer.
Counting starts from 0 so we have

.. math::
    
    0 \leq from < to < order(memref)

The local identifier must have the memref type specified last.
The output type is a memref type according to the following rules:

#. **Shape:** The mode size of the fused modes is the product of the mode sizes. If one mode is dynamic the fused mode size is dynamic.

   .. code::

       fuse %0[1,3] : memref<f32x32x16x8x4x42>                     ; -> memref<f32x32x512x42>
       fuse %0[1,3] : memref<f32x32x16x?x4x42,strided<1,16,?,?,?>> ; -> memref<f32x32x?x42,strided<1,32,?>>

#. **Stride:** Strides remain unchanged.

   .. code::

       fuse %0[1,2] : memref<f32x32x16x2x2,strided<1,48,768,1536>> ; -> memref<f32x32x32x2,strided<1,48,1536>>
       fuse %0[0,1] : memref<f32x8x?x32,strided<1,?,?>>            ; -> memref<f32x?x32,strided<1,?>>

Restrictions
~~~~~~~~~~~~

Let i be the first mode and j the last mode.
The stride vector S and the shape vector s must satisify the following compatibility condition:

:math:`\forall k \in [i,j): S_{k}s_{k} = S_{k+1}`

If S(i:j) and s(i:j) are known at compile time, the fuse instruction is illegal if the compatibility
condition is not satisfied.
If a single entry in S(i:j) or s(i:j) is dynamic, then fusing modes that violate the compatbility condition
is undefined beheaviour.

.. code::

       fuse %0[0,1] : memref<f32x8x16,strided<1,10>> ; Illegal, modes cannot be fused
       fuse %0[0,1] : memref<f32x8x16,strided<1,?>>  ; Undefined behaviour if dynamic stride != 8


Group id
........

.. code:: abnf

    value-instruction       =/ "group_id"

Overview
~~~~~~~~

Returns the group id, an integer of type "index" inbetween 0 and the group size - 1.

Group size
..........

.. code:: abnf

    value-instruction       =/ "group_size"

Overview
~~~~~~~~

Returns the group size, an integer of type "index".

If
..

.. code:: abnf

    multi-value-instruction = "if" local-identifier ["->" "(" scalar-type-list ")"]
                              region ["else" region]
    type-list               = scalar-type *("," scalar-type)

Overview
~~~~~~~~

An if statement.
Both regions are *mixed regions*.

The condition must be of bool type.

Arguments
~~~~~~~~~

The if instruction may return multiple values, where the number of values and the value types
are given by the scalar-type-list.
If values are returned, the last instruction in both the "then"-region and the "else"-region must
be a yield instruction (the "else"-region cannot be omitted).

Example:

   .. code::

       %1 = cmp.lt %0, 16 : i32
       %x = if %1 -> (i32) {
           yield %0 : i32
       } else {
           yield 16 : i32
       }


Load
....

.. code:: abnf

    value-instruction           =/ "load" local-identifier "[" [local-identifier-list] "]" ":" memref-or-group-type
    memref-or-group-type        =  memref-type / group-type

Overview
~~~~~~~~

Load the element given by the index list from a memref or group.
The number of indices must match the order of the memref
and a single index must be given for a group.

Arguments
~~~~~~~~~

The first operand must have memref or group type.
The indices must be of ``index`` type.

Returns
~~~~~~~

A value of the memref's element type or the group's memref type.
Examples:

#. ``load %0[] : memref<f32>`` returns a ``f32`` value.
#. ``load %0[5, %1] : memref<f32x10x?>`` returns a ``f32`` value.
#. ``load %0[%1] : group<memref<f32x42>>`` returns a ``memref<f32x42>`` value.
#. ``load %0[%1] : group<memref<f32x42>, offset: ?>`` returns a ``memref<f32x42>`` value.

Number of subgroups
...................

.. code:: abnf

    value-instruction       =/ "num_subgroups"

Overview
~~~~~~~~

Returns the number of subgroups the work-group is divided in; i32 integer.

For
...

.. code:: abnf

    instruction     =/ "for" local-identifier "=" local-identifier "," local-identifier
                       ["," local-identifier] [":" integer-type] region

Overview
~~~~~~~~

A for loop.
Instructions in the for loop execute sequentially and its region is a *mixed region*.

The loop's range [from; to) is given by the first integer constant and second integer constant,
and the trip count is stored in the local identifier.
A step size can be given with the third integer constant.
The step size defaults to 1 if omitted.
The integer type of the loop variable and the loop bounds is given after the colon.
The default integer type is ``index``.

Size
....

.. code:: abnf

    value-instruction       =/ "size" local-identifier "[" integer-constant "]" ":" memref-type

Overview
~~~~~~~~

The size instruction returns the i-th entry of the tensor's shape, where "i" is given by the integer
constant in square brackets.

Arguments
~~~~~~~~~

The first argument must point to a value of memref type.
The integer constant i gives the mode for which the size shall be returned.
It is required that

.. math::
    
    0 \leq i < order(memref)

The local identifier must have the memref type specified last.
The instruction returns an integer of index type.

Subgroup size
.............

.. code:: abnf

    value-instruction       =/ "subgroup_size"

Overview
~~~~~~~~

Returns the subgroup size; i32 integer.


Subview
.......

.. code:: abnf

    value-instruction       =/ "subview" local-identifier "[" [index-or-slice-list] "]" ":" memref-type
    index-or-slice-list     =  index-or-slice *("," index-or-slice)
    index-or-slice          =  integer-constant-or-identifier [":" integer-constant-or-identifier]

Overview
~~~~~~~~

The subview instruction returns a view on a tensor.

Arguments
~~~~~~~~~

The first argument must point to a value of memref type.
The number of indices in square brackets must match the order of the memref.
The indices are either given as single index or as a slice, where
slices are given in offset plus size notation ("%offset : %size").
E.g. the slice "%0 : %1" extracts a block of %1 elements beginning from %0, which is equivalent
to the index interval [%0, %0 + %1).

.. admonition:: Note

    A slice is often defined as "%0 : %1" being the index interval [%0, %1).
    However, then the compiler needs to figure out whether %1 - %0 is constant or not in order
    to determine whether the mode size is known at compile-time or not.
    Therefore, we prefer the offset plus size notation.

Zero sizes are used to encode that a rank-reduction is required, that is,
the rank of size 0 is removed from the output memref type.
A single index is syntactic sugar for offset plus size 0, e.g. %0 is syntactic sugar for %0:0.
(Note that a zero-size rank, e.g. in memref<f32x8x0>, is non-sense, because any multi-index passed
to the memref would be out-of-bounds. However, a one-sized rank, e.g. memref<f32x8x1>, might be desirable.)
A dynamic size of zero is undefined behaviour.



There is no run-time check whether the indices are within bounds.
Offset and size must be of index type.
Offset must be non-negative and size must be positive.

The local identifier must have the memref type specified last.
The output type is a memref type according to the following rules:

#. **Invariant-stride:** The stride is not changed.

   .. code::

       subview %0[4:8,8:4]  : memref<f32x32x16> ; Returns memref<f32x8x4,strided<1,32>>


#. **Rank-reduction:** A mode accessed by offset only or a mode with size statically known to be 0 is removed from the output tensor.

   .. code::

       subview %0[2:4, %1]   : memref<f32x16x8> ; Returns memref<f32x4>
       subview %0[2:4, %1:0] : memref<f32x16x8> ; Returns memref<f32x4>
       subview %0[2:4, %1:1] : memref<f64x16x8> ; Returns memref<f64x4x1,strided<1,16>>

#. **Output-mode size:** The size of the output mode is determined by the size field of a slice
   and may be dynamic.

   .. code::

       subview %0[%1:4]            : memref<f32x16> ; Returns memref<f32x4>
       subview %0[%2:%2]           : memref<f32x16> ; Returns memref<f32x?>
       subview %0[2:4, %2:%2, 6:7] : memref<f32x16x42x13> ; Returns memref<f32x4x?x7,strided<1,16,672>
       subview %0[2:4, %2:%2, 6:7] : memref<f32x16x42x13,strided<1,?,?>> ; Returns memref<f32x4x?x7,strided<1,?,?>

Store
.....

.. code:: abnf

    instruction     =/ "store" local-identifier "," local-identifier "[" [local-identifier-list] "]" ":" memref-type

Overview
~~~~~~~~

Store a scalar value in a memref at the position given by the index list.
The number of indices must match the order of the memref.

*Note:* Store should only be used in SPMD regions as otherwise the same memory location is written
from all work-items.

Arguments
~~~~~~~~~

The first operand must have the same scalar type as the memref type.
The indices must be of ``index`` type.

Yield
.....

.. code:: abnf

    instruction                 =/ "yield" [local-identifier-list]  ":" [scalar-type-list]

Overview
~~~~~~~~

Yield returns values from an if or for instruction.

Arguments
~~~~~~~~~

The length of the local identifier list must equal the length of the scalar type list.

Additional instructions
.......................

.. code:: abnf

    instruction             =/ "lifetime_stop" local-identifier

SPMD instructions
-----------------

Subgroup id
...........

.. code:: abnf

    value-instruction       =/ "subgroup_id"

Overview
~~~~~~~~

Returns the subgroup id; i32 integer from 0 to num_subgroups - 1.

Subgroup local id
.................

.. code:: abnf

    value-instruction       =/ "subgroup_local_id"

Overview
~~~~~~~~

Returns the work-item id within the sub-group; i32 integer from 0 to subgroup_size - 1.

Sample code
===========

The following sample implements the kernel

.. math::

    D := \alpha A B^T C + D \text{ with }
        A \in \mathbb{R}^{16\times 8},
        B \in \mathbb{R}^{8\times 8},
        C \in \mathbb{R}^{8\times 16},
        D \in \mathbb{R}^{16\times 16}

where B and C are constant matrices and A and D are matrix batches.

.. code::

    func @fused_kernel(%alpha: f32,
                         %A: group<memref<f32x16x8>>,
                         %B: memref<f32x8x8>,
                         %C: memref<f32x8x16>,
                         %D: memref<f32x16x16x?>) {
      %0 = group_id
      %1 = load %A[%0]        : group<memref<f32x16x8>> ; Returns memref<f32x16x8>
      %2 = subview %D[:,:,%0] : memref<f32x16x16x?>     ; Returns memref<f32x16x16>
      %tmp0 = alloca -> memref<f32x16x8>
      %zero = constant 0.0 : f32
      %one = constant 1.0 : f32
      gemm.n.t %one, %1, %B, %zero, %tmp0
         : f32, memref<f32x16x8>, memref<f32x8x8>, f32, memref<f32x16x8>
      gemm.n.n %alpha, %tmp0, %C, %one, %2
         : f32, memref<f32x16x8>, memref<f32x8x16>, f32, memref<f32x16x16>
    }
