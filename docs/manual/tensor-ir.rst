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
Each work group consists of a fixed number of subgroups that execute concurrently.
Subgroups can be further divided into work-items, where the number of work-items per subgroup
is given by the subgroup size.

The language distinguishes between *collective*, *SPMD*, and *mixed* instructions.
A collective instruction distributes the work among the work-items in an implementation-defined manner.
Local variables passed to or returned from a collective instruction are always uniform, meaning
that each work-item holds the same value.
An SPMD instruction follows the OpenCL execution model, where local variables may have a different value
for each work-item.
Mixed instructions accept both varying and uniform local variables.

In an SPMD region, we call an argument *dynamically uniform* if all work-items in a subgroup have
the same value.

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

    constant                    = boolean-constant / integer-constant / floating-constant / complex-constant
    boolean-constant            = "true" / "false"
    integer-constant            = [sign] 1*DIGIT
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

Attributes
==========

.. code:: abnf

    attribute                   = array-attribute /
                                  boolean-attribute /
                                  dictionary-attribute /
                                  integer-attribute /
                                  string-attribute
    array-attribute             = "[" [attribute *(", " attribute)] "]"
    boolean-attribute           = boolean-constant
    dictionary-attribute        = "{" [named-attribute *("," named-attribute)] "}"
    named-attribute             = attribute-name "=" attribute
    attribute-name              = "alignment" /
                                  "shape_gcd" /
                                  "stride_gcd" /
                                  "subgroup_size" /
                                  "unroll" /
                                  "work_group_size" /
                                  string-attribute
    integer-attribute           = integer-constant
    string-attribute            = %x22 *(%x20-21 / %x23-7E) %x22

Attributes add information about an operation, for example to assert properties or to direct the compiler.

.. _tensor language functions:

Functions
=========

.. code:: abnf

    function-definition         = "func" global-identifier "(" [argument-list] ")"
                                  ["attributes" dictionary-attribute] region
    argument-list               = argument *("," argument)
    argument                    = local-identifier ":" type [dictionary-attribute]

Defines a function that is callable from the host.

Attributes
----------

Subgroup size and work-group size are determined automatically by the compiler, but can be overriden
using the function's attribute dictionary:

.. list-table::

    * - Name
      - Type
      - Description
    * - subgroup_size
      - integer-attribute
      - Subgroup size; valid values depend on the target device (typically 16 or 32)
    * - work_group_size
      - array-attribute with 2 integer-attribute entries
      - Two dimensional work-group size in number of work-items

The work-group size attribute defines the size of the local work group.
Due to the focus on matrix operations, the work-group size is always two-dimensional,
where the first mode is used to tile the rows and the second mode is used
to tile the columns.
The first mode must be a multiple of the subgroup size.
If the subgroup size is omitted, then the first mode must be a multiple of one of
the subgroup sizes supported by the device.
The product of the work-group size modes must be smaller or equal than the maximum
work-group size of device.

The subgroup size attribute enforces a particular subgroup size that must be supported by
the device.

Parameter attributes
--------------------

Parameters with memref or group type accept the following named attributes:

.. list-table::

    * - Name
      - Type
      - Description
    * - alignment
      - integer-attribute
      - Minimum pointer alignment
    * - shape_gcd
      - array-attribute of integer-attribute
      - Greatest common divisors of shape
    * - stride_gcd
      - array-attribute of integer-attribute
      - Greatest common divisors of stride

Cf. the documentation of the :ref:`memref type <memref attributes>` and the :ref:`group type <group attributes>`.

Restrictions
------------

* Arguments must not have coopmatrix type.

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

    type                        = void-type / boolean-type / scalar-type / memref-type / group-type
    void-type                   = "void"

Boolean type
------------

.. code:: abnf

    boolean-type                = "bool"

Boolean type that only has two states (true or false).

Scalar types
------------

.. code:: abnf

    scalar-type                 = integer-type / floating-type / complex-type
    integer-type                = "i8" / "i16" / "i32" / "i64" / "index"
    floating-type               = "bf16" / "f16" / "f32" / "f64"
    complex-type                = "c32" / "c64"

Scalar types are either signless integer ("i"), floating point ("f"),
or complex floating point ("c").
The number behind the scalar type prefix denotes the number of bits,
e.g. "f64" are double precision floating point numbers.
The "bf16" type encodes bfloat16 floating point numbers.
The "index" type is an integer type whose width is platform-specific.

Type sizes in bytes are given by

=========================== ==== ==== ==== ==== ==== ==== ==== ==== ==== ====
:math:`\alpha`                i8  i16  i32  i64 bf16  f16  f32  f64  c32  c64
=========================== ==== ==== ==== ==== ==== ==== ==== ==== ==== ====
:math:`\text{size}(\alpha)`    1    2    4    8    2    2    4    8    8   16
=========================== ==== ==== ==== ==== ==== ==== ==== ==== ==== ====


Mixed precision operands might be allowed in instructions if the operands' types are *promotable*.
The scalar type :math:`\alpha` may be promoted to the scalar type :math:`\beta` if all values an operand
of type :math:`\alpha` may take can be exactly represented in type :math:`\beta`.
Formally, :math:`\alpha` is promotable to :math:`\beta` if :math:`\alpha \preceq \beta`,
where the partial order :math:`\preceq` is defined by the following relation matrix:

=============== ==== ==== ==== ==== ==== ==== ==== ==== ==== ====
:math:`\preceq`   i8  i16  i32  i64 bf16  f16  f32  f64  c32  c64
=============== ==== ==== ==== ==== ==== ==== ==== ==== ==== ====
i8                 1    1    1    1    1    1    1    1    1    1
i16                     1    1    1              1    1    1    1
i32                          1    1                   1    1    1
i64                               1
bf16                                   1         1    1    1    1
f16                                         1    1    1    1    1
f32                                              1    1    1    1
f64                                                   1         1
c32                                                        1    1
c64                                                             1
=============== ==== ==== ==== ==== ==== ==== ==== ==== ==== ====

Moreover, for scalar types :math:`\alpha,\beta` we define

.. math::

   \text{promote}(\alpha, \beta) = \left\{\begin{array}{rcl}
       \beta & \text{ if } & \alpha \preceq \beta, \\
       \alpha & \text{ if } & \beta \preceq \alpha, \\
       \text{fail} & \text{ else.}
   \end{array}\right.

Here, "fail" means that the promotion is not allowed and the compiler should throw an error.



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

Definitions
...........

Let V be a value of memref type.
The :math:`\text{order}(V)` operation returns the memref's order.
The :math:`\text{shape}(V)` returns the tensor shape as tuple.
:math:`\text{rows}(V)` and :math:`\text{columns}(V)` return the size of the first
and second mode, respectively.
The :math:`\text{element_type}(V)` operation gives the underlying scalar type.

For example, let B be a value of memref<f32x8x16x4> type, then

* :math:`\text{order}(B) = 3`
* :math:`\text{shape}(B) = (8,16,4)`
* :math:`\text{rows}(B) = 8`
* :math:`\text{columns}(B) = 16`
* :math:`\text{element_type}(B) = \text{f32}`


Memory layout
.............

.. code:: abnf

    memory-layout               = strided-layout

.. _strided layout:

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

.. _memref attributes:

Alignment attribute
...................

The *alignment=X* attribute gives the alignment X of the memref's base pointer in bytes.
That is, for the pointer P pointing to the first element of the memref we must have :math:`P = 0 \pmod{X}`.

**Restriction:** The alignment must be a multiple of the size of the memref's element type.


Greatest common divisor (GCD) attributes
........................................

The *shape_gcd=[d_1,...,d_k]* attribute asserts that :math:`s_i = 0 \pmod{d_i}, i=1,\dots,k`, where k is
smaller or equal than the order of the tensor n and :math:`s_i` is the i-th entry of the shape vector.
The divisors are understood to be the greatest common divisors for the set of shapes that the kernel is used for.
For example, if we know that :math:`s_1` is always a multiple of 4 then we can set *shape_gcd=[4]*.

The *stride_gcd=[D_1,...,D_m]* attribute asserts that :math:`S_i = 0 \pmod{D_i}, i=1,\dots,m`, where m is
smaller or equal than the order of the tensor n and :math:`S_i` is the i-th entry of the stride vector.
The divisors are understood to be the greatest common divisors for the set of strides that the kernel is used for.
For example, if we know that :math:`S_2` is always a multiple of 4 then we can set *stride_gcd=[1,4]*.

Group type
----------

.. code:: abnf

    group-type                  = "group<" memref-type "x" constant-or-dynamic ["," "offset" ":" constant-or-dynamic] ">"

The group type collects unstructured pointers to memref's with potentially different dynamic mode sizes.
The C-analogy of a group is a pointer-to-a-pointer.
For example, the C-analogue of a ``group<memref<f32x16x16>x?>`` is a ``float**``.

The group shape is always one-dimensional and may be queried using the
:ref:`size instruction <size instruction>`.

The optional offset parameter is used to offset each pointer by the given number of elements.
Given the C-analogue ``float** group``, loading element ``i`` with offset ``off`` gives the
pointer ``float* tmp = group[i] + off``.
The default offset is 0.

Dynamic values ('?') may appear in the memref-type, in the group shape, and in the offset.
These values are stored in the dope vector;
the calling convention for groups is implementation-defined.

.. _group attributes:

Attributes
..........

Attributes applied on a group type are passed through to the memrefs.
That is, when a memref is loaded from the group then the :ref:`memref attributes <memref attributes>`
are equal to the attributes of the group.

Cooperative matrix type
-----------------------

.. code:: abnf

    coopmatrix-type             = "coopmatrix<" scalar-type 2*2("x" integer-constant) "," matrix-use ">"
    matrix-use                  = "matrix_a" / "matrix_b" / "matrix_acc"

The coopmatrix represents a matrix distributed across a subgroup, where each work-item in a subgroup
stores a part of the matrix.
The scalar-type specifies the matrix element type, the first integer-constant the number of rows,
and the second integer-constant the number of columns.
The matrix-use may affect the distribution of the matrix in the subgroup, and the name refers to the
position of the matrix in a matrix multiplication.

Not all matrix shapes need to be supported in the implementation.
The supported matrix shapes may depend on data type, matrix use, and target hardware.

An argument to any instruction that has coopmatrix type **must** be dynamically uniform.

Definitions
...........

Let V be a value of coopmatrix type.
The :math:`\text{rows}(V)` and :math:`\text{columns}(V)` functions return the size of the first
and second mode, respectively, and :math:`\text{shape}(V)` returns rows and cols as tuple.
The :math:`\text{component_type}(V)` operation gives the underlying scalar type
and :math:`\text{use}(V)` returns the use.

For example, let B be a value of coopmatrix<f32x8x16,matrix_acc> type, then

* :math:`\text{shape}(B) = (8,16)`
* :math:`\text{rows}(B) = 8`
* :math:`\text{columns}(B) = 16`
* :math:`\text{component_type}(B) = \text{f32}`
* :math:`\text{use}(B) = \text{matrix_acc}`

Instructions
============

Instructions may return zero, one, or multiple values, and follow the following format:

.. code:: abnf

    value-instruction-assignment        = local-identifier "=" value-instruction
    multi-value-instruction-assignment  = [local-identifier-list "="] multi-value-instruction
    local-identifier-list               = local-identifier *("," local-identifier)
    instruction                         = value-instruction-assignment
                                          / multi-value-instruction-assignment

That is, on the left-hand side we have list of values that are produced by the instruction followed by an equals sign,
or an empty string, if the instruction does not produce values.
On the right-hand side, after the equals sign or empty string, the name of the instruction is written, e.g. "ger", optionally followed by instruction modifiers, e.g. "ger.atomic".
Then, a list of operands follows that is usually comma-seperated but might also be printed in a custom format
(e.g. for "load", "store", "subview", etc.).
If the instruction produces values, then the types of the returned values must be annotated after a colon.



Collective instructions
-----------------------

Alloca
......

.. code:: abnf

    value-instruction   = "alloca" [dictionary-attribute] ":" memref-type

Overview
~~~~~~~~

The alloca instruction allocates temporary memory that is freed automatically at the end of the block that contains the alloca.

Attributes
~~~~~~~~~~

Alloca accepts the following named attributes:

.. list-table::

    * - Name
      - Type
      - Description
    * - alignment
      - integer-attribute
      - Base pointer alignment; must not be larger than the :ref:`default alignment <tinytc_core_info_get_default_alignment>`.

Restrictions
~~~~~~~~~~~~

* The memref's size must known at compile-time, i.e. the tensor shape must not contain any dynamic modes.
* The address space must be "local".

Axpby
.....

.. code:: abnf

    transpose       =  ".t" / ".n"
    instruction     =/ "axpby" transpose [".atomic"] local-identifier "," local-identifier ","
                               local-identifier "," local-identifier

Overview
~~~~~~~~

Axpby implements

.. math::

    B := \alpha \text{op}(A) + \beta B

for vectors and matrices, where :math:`\text{op}(X)` is defined as

.. math::

    \text{op}(X) := \left\{
                    \begin{array}{rcl}
                        X^T & \text{ if } & \text{transpose} = \text{".t"} \wedge \text{order}(X) = 2,\\
                        X   & \text{ else. }
                    \end{array}
                    \right.

If the atomic flag is set, B is updated atomically.

Operands
~~~~~~~~

======= =========== ============== 
Op.-No. Type        Description
======= =========== ==============
1       scalar-type :math:`\alpha` 
2       memref-type A
3       scalar-type :math:`\beta`  
4       memref-type B
======= =========== ==============

Restrictions
~~~~~~~~~~~~

* :math:`\text{shape}(B) = \text{shape}(\text{op}(A))`
* :math:`\text{order}(B) = 0 \lor \text{order}(B) = 1 \lor \text{order}(B) = 2`
* :math:`\text{type}(\alpha) \preceq \text{element_type}(A) \preceq \text{element_type}(B)`
* :math:`\text{type}(\beta) \preceq \text{element_type}(B)`
* If the atomic flag is set, :math:`\beta` must be constant and :math:`\beta \in \{0,1\}`.

Cumulative sum
..............

.. code:: abnf

    instruction     =/ "cumsum" [".atomic"] local-identifier "," local-identifier "," integer-constant ","
                                local-identifier "," local-identifier

Overview
~~~~~~~~

Computes the n-mode cumulative sum

.. math::

    B := \alpha A \times_{n} L_{s_n} + \beta B,

where :math:`L_{s_n}` is the lower triangular matrix of ones of size :math:`s_n\times s_n` and
:math:`s_n` is the n-th entry of the shape vector of A.
In index notation, we have equivalently

.. math::

    B_{i_1\dots i_{n-1}ji_{n+1}\dots i_M}
        := \alpha \sum_{i_n=1}^{j}A_{i_1\dots i_{n-1}i_ni_{n+1}\dots i_M}
           + \beta B_{i_1\dots i_{n-1}ji_{n+1}\dots i_M},

If the atomic flag is set, B is updated atomically.


Operands
~~~~~~~~

======= ================ ==================
Op.-No. Type             Description
======= ================ ==================
1       scalar-type      :math:`\alpha` 
2       memref-type      A
3       integer-constant n (summation mode)
4       scalar-type      :math:`\beta`
5       memref-type      B
======= ================ ==================

Restrictions
~~~~~~~~~~~~

* :math:`\text{order}(A) \geq 1`
* :math:`\text{shape}(A) = \text{shape}(B)`
* :math:`\text{type}(\alpha) \preceq \text{element_type}(A) \preceq \text{element_type}(B)`
* :math:`\text{type}(\beta) \preceq \text{element_type}(B)`
* If the atomic flag is set, :math:`\beta` must be constant and :math:`\beta \in \{0,1\}`.

Foreach
.......

.. code:: abnf

    instruction     =/ "foreach" "(" local-identifier-list ")" [":" integer-type] "="
                       "(" local-identifier-list ")" "," "(" local-identifier-list ")" region

Overview
~~~~~~~~

A foreach loop that executes the loop's range without any sequence guarantee.
The region of a foreach is a *spmd region*.

The three local identifier lists define the loop range and the local identifiers that
make the trip count available within the loop body.
All three lists must have the same length and have the following format:

.. math::

    (\text{var}_1, \dots, \text{var}_N) = (\text{from}_1, \dots, \text{from}_N),
                                          (\text{to}_1, \dots, \text{to}_N),

where :math:`N` is the common length of each of the three lists.
The loop range is defined as the cartesian product of the half-open intervals
:math:`[\text{from}_i; \text{to}_i)` such that the trip count take the values

.. math::

    (\text{var}_1, \dots, \text{var}_N) \in [\text{from}_1; \text{to}_1) \times \dots \times
    [\text{from}_N; \text{to}_N)

The integer type of the loop variable and the loop bounds can be optionally set after the colon.
The default integer type is ``index``.

The mapping of trip count to work-item is implementation-defined.

GEMM
....

.. code:: abnf

    instruction     =/ "gemm" transpose transpose [".atomic"] local-identifier "," local-identifier ","
                              local-identifier "," local-identifier "," local-identifier

Overview
~~~~~~~~

GEMM implements the well-known GEMM BLAS-3 operation.

.. math::

    C := \alpha \text{op}_1(A) \text{op}_2(B) + \beta C

The functions :math:`\text{op}_1` and :math:`\text{op}_2` are defined as

.. math::

    \text{op}_i(X) := \left\{
                      \begin{array}{rcl}
                        X^T & \text{ if } & \text{transpose}_i = \text{".t"},\\
                        X   & \text{ if } & \text{transpose}_i = \text{".n"}.
                      \end{array}
                      \right.

where transpose\ :sub:`1` and transpose\ :sub:`2` refer to the first and second transpose modifier, respectively.

If the atomic flag is set, C is updated atomically.

Operands
~~~~~~~~

======= =========== ============== 
Op.-No. Type        Description
======= =========== ==============
1       scalar-type :math:`\alpha` 
2       memref-type A
3       memref-type B
4       scalar-type :math:`\beta`
5       memref-type C
======= =========== ==============

Restrictions
~~~~~~~~~~~~

* :math:`\text{order}(A) = \text{order}(B) = \text{order}(C) = 2`
* :math:`\text{colums}(\text{op}_1(A)) = \text{rows}(\text{op}_2(B))`
* :math:`\text{rows}(C) = \text{rows}(\text{op}_1(A))`
* :math:`\text{columns}(C) = \text{columns}(\text{op}_2(B))`
* :math:`\text{type}(\alpha) \preceq \text{promote}(\text{element_type}(A), \text{element_type}(B)) \preceq \text{element_type}(C)`
* :math:`\text{type}(\beta) \preceq \text{element_type}(C)`
* If the atomic flag is set, :math:`\beta` must be constant and :math:`\beta \in \{0,1\}`.

GEMV
....

.. code:: abnf

    instruction     =/ "gemv" transpose [".atomic"] local-identifier "," local-identifier ","
                              local-identifier "," local-identifier "," local-identifier

Overview
~~~~~~~~

GEMV implements the well-known GEMM BLAS-2 operation.

.. math::

    c := \alpha \text{op}_1(A) b + \beta c

where :math:`\text{op}_1` is defined as in GEMM.

If the atomic flag is set, c is updated atomically.

Operands
~~~~~~~~

======= =========== ============== 
Op.-No. Type        Description
======= =========== ==============
1       scalar-type :math:`\alpha` 
2       memref-type A
3       memref-type b
4       scalar-type :math:`\beta`
5       memref-type c
======= =========== ==============

Restrictions
~~~~~~~~~~~~

* :math:`\text{order}(A) = 2`
* :math:`\text{order}(b) = \text{order}(c) = 1`
* :math:`\text{colums}(\text{op}_1(A)) = \text{rows}(b)`
* :math:`\text{rows}(c) = \text{rows}(\text{op}_1(A))`
* :math:`\text{type}(\alpha) \preceq \text{promote}(\text{element_type}(A), \text{element_type}(b)) \preceq \text{element_type}(C)`
* :math:`\text{type}(\beta) \preceq \text{element_type}(C)`
* If the atomic flag is set, :math:`\beta` must be constant and :math:`\beta \in \{0,1\}`.

GER
...

.. code:: abnf

    instruction     =/ "ger" [".atomic"] local-identifier "," local-identifier ","
                             local-identifier "," local-identifier "," local-identifier

Overview
~~~~~~~~

Computes the general rank-1 update:

.. math::

    C := \alpha a b^T + \beta C

If the atomic flag is set, C is updated atomically.

Operands
~~~~~~~~

======= =========== ============== 
Op.-No. Type        Description
======= =========== ==============
1       scalar-type :math:`\alpha` 
2       memref-type a
3       memref-type b
4       scalar-type :math:`\beta`
5       memref-type C
======= =========== ==============

Restrictions
~~~~~~~~~~~~

* :math:`\text{order}(a) = \text{order}(b) = 1`
* :math:`\text{order}(C) = 2`
* :math:`\text{rows}(C) = \text{rows}(a)`
* :math:`\text{columns}(C) = \text{rows}(b)`
* :math:`\text{type}(\alpha) \preceq \text{promote}(\text{element_type}(A), \text{element_type}(b)) \preceq \text{element_type}(C)`
* :math:`\text{type}(\beta) \preceq \text{element_type}(C)`
* If the atomic flag is set, :math:`\beta` must be constant and :math:`\beta \in \{0,1\}`.


Hadamard product
................

.. code:: abnf

    instruction     =/ "hadamard_product" [".atomic"] local-identifier "," local-identifier ","
                                          local-identifier "," local-identifier "," local-identifier

Overview
~~~~~~~~

Computes the Hadamard product of two vectors or two matrices.
That is, in index notation we have

.. math::

    c_{i} := \alpha a_{i} b_{i} + \beta c_{i}

for vectors and

.. math::

    C_{ij} := \alpha A_{ij} B_{ij} + \beta C_{ij}

for matrices. If the atomic flag is set, c/C is updated atomically.

Operands
~~~~~~~~

======= =========== ============== 
Op.-No. Type        Description
======= =========== ==============
1       scalar-type :math:`\alpha` 
2       memref-type a/A
3       memref-type b/B
4       scalar-type :math:`\beta`
5       memref-type c/C
======= =========== ==============

Restrictions
~~~~~~~~~~~~

* :math:`\text{order}(a) = \text{order}(b) = \text{order}(c) = o` with :math:`o\in\{1,2\}`
* :math:`\text{shape}(a) = \text{shape}(b) = \text{shape}(c)`
* :math:`\text{type}(\alpha) \preceq \text{promote}(\text{element_type}(A), \text{element_type}(b)) \preceq \text{element_type}(C)`
* :math:`\text{type}(\beta) \preceq \text{element_type}(C)`
* If the atomic flag is set, :math:`\beta` must be constant and :math:`\beta \in \{0,1\}`.

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

    instruction     =/ "sum" transpose [".atomic"] local-identifier "," local-identifier ","
                             local-identifier "," local-identifier

Overview
~~~~~~~~

Computes the matrix-vector product or the dot product of A with a vector of ones.
That is, if the result is a vector we have

.. math::

    b := \alpha \text{op}(A) \vec{1} + \beta b,

where :math:`\text{op}(A)` is defined as in the axpby instruction,
and if the result is a scalar we have

.. math::

    b := \alpha \left<A,\vec{1}\right> + \beta b

If the atomic flag is set, b is updated atomically.


Operands
~~~~~~~~

======= =========== ============== 
Op.-No. Type        Description
======= =========== ==============
1       scalar-type :math:`\alpha` 
2       memref-type A
3       scalar-type :math:`\beta`
4       memref-type b
======= =========== ==============

Restrictions
~~~~~~~~~~~~

* :math:`\text{order}(b) = 1 \lor \text{order}(b) = 0`
* :math:`\text{order}(A) = \text{order}(b)+1`
* :math:`\text{rows}(b) = \text{rows}(\text{op}(A)) \text{ if } \text{order}(b) = 1`
* :math:`\text{type}(\alpha) \preceq \text{element_type}(A) \preceq \text{element_type}(B)`
* :math:`\text{type}(\beta) \preceq \text{element_type}(B)`
* If the atomic flag is set, :math:`\beta` must be constant and :math:`\beta \in \{0,1\}`.


Mixed instructions
------------------

Arithmetic (binary)
...................

.. code:: abnf

    arith-binary-type       =  "arith.add" /
                               "arith.sub" /
                               "arith.mul" /
                               "arith.div" /
                               "arith.rem" /
                               "arith.min" /
                               "arith.max" /
                               "arith.shl" /
                               "arith.shr" /
                               "arith.and" /
                               "arith.or"  /
                               "arith.xor"
    value-instruction       =/ arith-binary-type local-identifier "," local-identifier
                               ":" (boolean-type / scalar-type / coopmatrix-type)

Overview
~~~~~~~~

Binary arithmetic operation on scalars and cooperative matrices.
Both operands, as well as the returned type, have the same scalar or component type.
Arithmetic on cooperative matrices is done component-wise.

The following table shows the operations' description and the types that are allowed for the operation.
The backslash "\\" is used to exclude types from the list of allowed types.

=== ============================= ======================================================
Op  Allowed type                  Description
=== ============================= ======================================================
add scalar-type / coopmatrix-type Sum of operands
sub scalar-type / coopmatrix-type Difference of operands
mul scalar-type / coopmatrix-type Product of operands
div scalar-type / coopmatrix-type Quotient of operands
rem scalar-type \\ complex-type   Remainder from the division of operands
shl integer-type                  Left shift first operand by second operand
shr integer-type                  Arithmetic right shift first operand by second operand
and boolean-type / integer-type   Bitwise and
or  boolean-type / integer-type   Bitwise or
xor boolean-type / integer-type   Bitwise xor
min scalar-type \\ complex-type   Minimum of operands
max scalar-type \\ complex-type   Maximum of operands
=== ============================= ======================================================

Arithmetic (unary)
..................

.. code:: abnf

    arith-unary-type        =  "arith.abs" /
                               "arith.neg" /
                               "arith.not" /
                               "arith.conj" /
                               "arith.im" /
                               "arith.re"
    value-instruction       =/ arith-unary-type local-identifier
                               ":" (scalar-type / coopmatrix-type)

Overview
~~~~~~~~

Unary arithmetic operation on scalars and cooperative matrices.
For integer and floating point input, the operand must have the same type as the returned value.
For complex input, the returned value has the component floating point type
for ".abs", ".im", and ".re", and the returned value has the same type as the operand
for ".neg" and ".conj".

The following table shows the operations' description and the types that are allowed for the operation.

==== ============================= =============================
Op   Allowed type                  Description
==== ============================= =============================
abs  scalar-type                   Compute absolute value
neg  scalar-type / coopmatrix-type Negation
not  boolean-type / integer-type   Bitwise not
conj complex-type                  Complex conjugate
im   complex-type                  Extract imaginary part
re   complex-type                  Extract real part
==== ============================= =============================

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

Builtin (mixed)
...............

.. code:: abnf

    mixed-builtin-type      =  "builtin.group_id"  /
                               "builtin.group_size"  /
                               "builtin.num_subgroups"  /
                               "builtin.subgroup_size"
    value-instruction       =/ mixed-builtin-type ":" integer-type

Overview
~~~~~~~~

Returns a builtin value.
The following table shows the builtins' description and the types that are returned.

============= ===== ====================================================================
Builtin       Type  Description
============= ===== ====================================================================
group_id      index Returns the group id, an integer inbetween 0 and the group size - 1
group_size    index Returns the group size
num_subgroups i32   Returns the number of subgroups the work-group is divided in 
subgroup_size i32   Returns the subgroup size
============= ===== ====================================================================

Cast
....

.. code:: abnf

    value-instruction       =/ "cast" local-identifier ":" scalar-type
    value-instruction       =/ "cast" local-identifier ":" coopmatrix-type

Overview
~~~~~~~~

Cast scalar values or cooperative matrices to type indicated after the colon.
The shape and the use the coopmatrix types must match.

Casts from complex types to non-complex types are forbidden.
The following table summarizes the casts and the mapping to SPIR-V
(the casts are done component-wise for coopmatrix types):

============= ============= ==================================================
Operand type  Result type   SPIR-V Op
============= ============= ==================================================
integer-type  integer-type  OpSConvert
floating-type floating-type OpFConvert
complex-type  complex-type  OpFConvert (on vector2)
integer-type  floating-type OpConvertSToF
floating-type integer-type  OpConvertFToS
floating-type complex-type  OpFConvert on real part, imaginary part is zero
integer-type  complex-type  OpConvertSToF on real part, imaginary part is zero
complex-type  integer-type  Forbidden
complex-type  floating-type Forbidden
============= ============= ==================================================

Comparison
..........

.. code:: abnf

    cmp-type                =  "cmp.eq" /
                               "cmp.ne" /
                               "cmp.gt" /
                               "cmp.ge" /
                               "cmp.lt" /
                               "cmp.le"
    value-instruction       =/ cmp-type local-identifier "," local-identifier ":" "bool"

Overview
~~~~~~~~

Scalar comparison.
Both operands must have the same scalar type and the returned value has boolean type.

The following table shows the comparisons' description and the types that are allowed for the comparison.
The backslash "\\" is used to exclude types from the list of allowed types.

==== =========================== =====================
Cond Allowed type Description
==== =========================== =====================
eq   scalar-type                 Equal
ne   scalar-type                 Not equal
gt   scalar-type \\ complex-type Greater than
ge   scalar-type \\ complex-type Greater than or equal
lt   scalar-type \\ complex-type Less than
le   scalar-type \\ complex-type Less than or equal
==== =========================== =====================

Constant
........

.. code:: abnf

    value-instruction       =/ "constant" constant ":" (boolean-type / scalar-type / coopmatrix-type)

Overview
~~~~~~~~

Sets the result value to a constant value.
The type of the constant must match the scalar or component type
(e.g. an integer type requires an integer-constant and a floating type requires a floating-constant).

When the result is a cooperative matrix, all entries are set to the same constant value.

Expand
......

.. code:: abnf

    value-instruction       =/ "expand" local-identifier "[" integer-constant "->" expand-shape "]" ":" memref-type
    expand-shape            =  integer-constant-or-identifier 1*("x" integer-constant-or-identifier)
    integer-constant-or-identifier = integer-constant / local-identifier

Overview
~~~~~~~~

The expand instruction returns a view on a tensor with a mode viewed as higher-order mode.

Operands
~~~~~~~~

The first argument must point to a value of memref type.
The first integer constant before "->" gives the mode that shall be expanded.
The expand shape coming after "->" gives the new shape of the mode.
Dynamic values in the expand shape must have `index` type.

Restrictions
~~~~~~~~~~~~

The memref type of the result must conform with the following rules:

#. Element type and address space must match the operand's memref type.
#. **Shape:** The mode size is replaced with the expand shape.
   The product of the expand shape must equal the size of the expanded mode.

   .. code::

       expand %0[1 -> 2x8]      : memref<f32x32x2x8x8>     ; %0: memref<f32x32x16x8>
       expand %0[1 -> 2x2x2x2]  : memref<f32x32x2x2x2x2x8> ; %0: memref<f32x32x16x8>

#. **Identifiers:** Local identifiers in the expand shape are dynamic in the resulting memref type.
   The product of the dynamic expand shape must equal the size of the expanded mode.

   .. code::

       expand %0[1 -> %1 x 2]      : memref<f32x32x?x2>   ; %0: memref<f32x32x?>
       expand %0[1 -> 2 x %1]      : memref<f32x32x2x?>   ; %0: memref<f32x32x?>
       expand %0[1 -> %1 x 2]      : memref<f32x32x?x2>   ; %0: memref<f32x32x16>
       expand %0[1 -> %1 x 2]      : memref<f32x32x?x2>   ; %0: memref<f32x32x?>
       expand %0[1 -> %1 x %2 x 2] : memref<f32x32x?x?x2> ; %0: memref<f32x32x16>
       expand %0[1 -> %2 x 2 x %1] : memref<f32x32x?x2x?> ; %0: memref<f32x32x16>
       expand %0[1 -> %1 x %2]     : memref<f32x32x?x?>   ; %0: memref<f32x32x?>
       expand %0[1 -> %1 x %2]     : memref<f32x32x?x?>   ; %0: memref<f32x32x16>

   *Note:* In the third example above, %1 must be equal to 8.
   The output mode corresponding to %1 is still dynamic.

#. **Stride:** A new stride entry is entered that follows the canonical stride computation.
   It is also permissible to put '?' for a stride instead of the constant value.

   .. code::

       expand %0[0->4 x 8]  : memref<f32x4x8x7,strided<2,8,64>> ; %0: memref<f32x32x7,strided<2,64>>
       expand %0[0->4 x 8]  : memref<f32x4x8x7,strided<2,?,?>>  ; %0: memref<f32x32x7,strided<2,64>>
       expand %0[0->%1 x 4] : memref<f32x?x4x7,strided<2,?,?>>  ; %0: memref<f32x?x7,strided<2,?>>
       expand %0[0->4 x %1] : memref<f32x4x?x7,strided<2,8,?>>  ; %0: memref<f32x?x7,strided<2,?>>
       expand %0[0->4 x %1] : memref<f32x4x?x7,strided<2,?,?>>  ; %0: memref<f32x?x7,strided<2,?>>

Further restrictions:

* The product of the expand shape must be the same as the mode size.
* If the product of the expand shape is only known at runtime, then it is undefined behaviour
  if the dynamic product does not match the mode size.

For
...

.. code:: abnf

    multi-value-instruction = "for" local-identifier [":" integer-type] "="
                                    local-identifier "," local-identifier ["," local-identifier]
                              ["init" "(" init-value-list ")" "->" "(" return-type-list ")" ] region
                              [dictionary-attribute]
    init-value-list         = init-value *("," init-value)
    init-value              = local-identifier "=" local-identifier
    return-type-list        = return-type *("," return-type)
    return-type             = boolean-type / scalar-type / coopmatrix-type


Overview
~~~~~~~~

A for loop.
Instructions in the for loop execute sequentially and its region is a *mixed region*.

Arguments
~~~~~~~~~

The trip count is stored in the first local identifier and is accessible within the loop body.
The loop's range [from; to) is given by the first and the second local identifier after the equals sign,
and a step size may be given with the third local identifier after the equals sign.
The step size defaults to 1 if omitted.
The integer type of the loop variable and the loop bounds is given after the colon and
the default integer type is ``index``.

Values that are given in the init-value-list may be carried from one iteration to the next.
The local identifier gives the name of the loop-carried value as it is accessible in the loop body.
The local identifier given on the right-hand side of the init-value expression determines
the initial value of the loop-carried value, and its type must coincide with the scalar-type-list.
When loop-carried values are present, the loop's last instruction must be a yield instruction that
updates the loop-carried values for the next iteration.
The number and types of the yielded values must correspond the scalar-type-list.

Returns
~~~~~~~

The final value of the loop-carried values are returned by the for instruction.


Example:

   .. code::

       %from = constant 2 -> i32
       %to = constant 6 -> i32
       %f0 = constant 0 -> i64
       %f1 = constant 1 -> i64
       %fn_1, %fn = for %n:i32=%from,%to init(%fn_2=%f0,%fn_1=%f1) -> (i64,i64) {
           %fn = arith.add %fn_2, %fn_1 : i64
           yield (%fn_1, %fn)
       }
       ; %fn_1 contains the fourth Fibonacci number and %fn the fifth Fibonacci number 

Attributes
~~~~~~~~~~

The following named attributes may be passed in the attribute dictionary:

.. list-table::

    * - Name
      - Type
      - Description
    * - unroll
      - boolean-attribute
      - true: request to unroll loop, false: request to not unroll loop

Fuse
....

.. code:: abnf

    value-instruction       =/ "fuse" local-identifier "[" integer-constant "," integer-constant "]"
                                      ":" memref-type

Overview
~~~~~~~~

The fuse instruction returns a view on a tensor with two or more adjacent modes viewed as a single mode.

Fused modes are specified as the interval [from, to], where counting starts from 0.
From and to must refer to existing modes, that is, we require :math:`0 \leq \text{from} < \text{to} < \text{order}(\text{tensor})`.
Moreover, the stride vector S and the shape vector s must satisify the following compatibility condition:

:math:`\forall k \in [\text{from},\text{to}): S_{k}s_{k} = S_{k+1}`

If S(i:j) and s(i:j) are known at compile time, the fuse instruction is illegal if the compatibility
condition is not satisfied.
If a single entry in S(i:j) or s(i:j) is dynamic, then fusing modes that violate the compatbility condition
is undefined beheaviour, e.g.

.. code::

       ; Illegal, modes cannot be fused
       fuse %0[0,1] : memref<f32x128>              ; %0: memref<f32x8x16,strided<1,10>>
       ; Undefined behaviour if dynamic stride != 8 
       fuse %0[0,1] : memref<f32x128,strided<1,?>> ; %0: memref<f32x8x16,strided<1,?>>

Operands
~~~~~~~~

======= ================ ===========
Op.-No. Type             Description
======= ================ ===========
1       memref-type      tensor
2       integer-constant from
3       integer-constant to
======= ================ ===========

Restrictions
~~~~~~~~~~~~

The memref type of the result must conform with the following rules:

#. Element type and address space must match the operand's memref type.
#. **Shape:** The mode size of the fused modes is the product of the mode sizes. If one mode is dynamic the fused mode size is dynamic.

   .. code::

       fuse %0[1,3] : memref<f32x32x512x42>               ; %0: memref<f32x32x16x8x4x42>
       fuse %0[1,3] : memref<f32x32x?x42,strided<1,32,?>> ; %0: memref<f32x32x16x?x4x42,strided<1,16,?,?,?>>
                                                         
#. **Stride:** Strides remain unchanged or are replaced by '?'.

   .. code::

       fuse %0[1,2] : memref<f32x32x32x2,strided<1,48,1536>> ; %0: memref<f32x32x16x2x2,strided<1,48,768,1536>>
       fuse %0[1,2] : memref<f32x32x32x2,strided<1,?,?>>     ; %0: memref<f32x32x16x2x2,strided<1,48,768,1536>>
       fuse %0[0,1] : memref<f32x?x32,strided<1,?>>          ; %0: memref<f32x8x?x32,strided<1,?,?>>

If
..

.. code:: abnf

    multi-value-instruction =/ "if" local-identifier ["->" "(" return-type-list ")"]
                               region ["else" region]

Overview
~~~~~~~~

An if statement.
Both regions are *mixed regions*.

The condition (first operand) must have boolean type.

Returns
~~~~~~~

The if instruction may return multiple values, where the number of values and the value types
are given by the return-type-list.
If values are returned, the last instruction in both the "then"-region and the "else"-region must
be a yield instruction (the "else"-region cannot be omitted).

Example:

   .. code::

       %1 = cmp.lt %0, 16 : i32
       %x = if %1 -> (i32) {
           yield (%0)
       } else {
           %c16 = constant 16 : i32
           yield (%c16)
       }


Load
....

.. code:: abnf

    value-instruction           =/ "load" local-identifier "[" [local-identifier-list] "]"
                                          ":" scalar-or-memref-type
    scalar-or-memref-type       =  scalar-type / memref-type

Overview
~~~~~~~~

Load the element given by the index list from a memref or group.
The number of indices must match the order of the memref
and a single index must be given for a group.

Operands
~~~~~~~~~

======= ======================== ===========
Op.-No. Type                     Description
======= ======================== ===========
1       memref-type / group-type tensor
2...    index                    index list
======= ======================== ===========

Returns
~~~~~~~

A value of the memref's element type or the group's memref type.
Examples:

#. ``load %0[] : f32 ; %0: memref<f32>``
#. ``load %0[5, %1] : f32 ; %0: memref<f32x10x?>``
#. ``load %0[%1] : memref<f32x42> ; %0: group<memref<f32x42>x?>``
#. ``load %0[%1] : memref<f32x42> ; %0: group<memref<f32x42>x?, offset: ?>``

Math (unary)
............

.. code:: abnf

    math-unary-type         =  "math.exp" /
                               "math.native_exp"
    value-instruction       =/ math-unary-type local-identifier ":" scalar-type

Overview
~~~~~~~~

Unary math operation on scalars.
The operand must have the same type as the returned value.

The following table shows the operations' description and the types that are allowed for the operation.

=========== ============================= ==============================================================
Op          Allowed type                  Description
=========== ============================= ==============================================================
exp         floating-type / complex-type  Compute exponential function
native_exp  floating-type / complex-type  Compute exponential function with implementation-defined error
=========== ============================= ==============================================================

.. _size instruction:

Size
....

.. code:: abnf

    value-instruction       =/ "size" local-identifier "[" integer-constant "]" ":" "index"

Overview
~~~~~~~~

The size instruction returns the i-th entry of the tensor's shape, where "i" is given by the integer
constant in square brackets.
"i" must be in bounds, i.e. :math:`0 \leq i < \text{order}(tensor)`.

For group types, the group size is returned and "i" must be 0.

Operands
~~~~~~~~~

======= ======================== ===========
Op.-No. Type                     Description
======= ======================== ===========
1       memref-type / group-type tensor
2       integer-constant         mode index
======= ======================== ===========

Subview
.......

.. code:: abnf

    value-instruction       =/ "subview" local-identifier "[" [index-or-slice-list] "]"
                                         ":" memref-type
    index-or-slice-list     =  index-or-slice *("," index-or-slice)
    index-or-slice          =  integer-constant-or-identifier [":" integer-constant-or-identifier]

Overview
~~~~~~~~

The subview instruction returns a view on a tensor.

The first argument must point to a value of memref type.
The number of indices in square brackets must match the order of the memref type.
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

Restrictions
~~~~~~~~~~~~

The memref type of the result must conform with the following rules:

#. Element type and address space must match the operand's memref type.
#. **Invariant-stride:** The stride is not changed or replaced with '?'.

   .. code::

       subview %0[4:8,8:4]  : memref<f32x8x4,strided<1,32>> ; %0: memref<f32x32x16>
       subview %0[4:8,8:4]  : memref<f32x8x4,strided<1,?>>  ; %0: memref<f32x32x16>


#. **Rank-reduction:** A mode accessed by offset only or a mode with size statically known to be 0 is removed from the output tensor.

   .. code::

       subview %0[2:4, %1]   : memref<f32x4>                 ; %0: memref<f32x16x8>
       subview %0[2:4, %1:0] : memref<f32x4>                 ; %0: memref<f32x16x8>
       subview %0[2:4, %1:1] : memref<f64x4x1,strided<1,16>> ; %0: memref<f64x16x8>

#. **Output-mode size:** The size of the output mode is determined by the size field of a slice
   and may be dynamic.

   .. code::

       subview %0[%1:4]            : memref<f32x4>                      ; %0: memref<f32x16>
       subview %0[%2:%2]           : memref<f32x?>                      ; %0: memref<f32x16>
       subview %0[2:4, %2:%2, 6:7] : memref<f32x4x?x7,strided<1,16,672> ; %0: memref<f32x16x42x13>
       subview %0[2:4, %2:%2, 6:7] : memref<f32x4x?x7,strided<1,?,?>    ; %0: memref<f32x16x42x13,strided<1,?,?>>

Store
.....

.. code:: abnf

    instruction     =/ "store" [store-flag] local-identifier ","
                               local-identifier "[" [local-identifier-list] "]"
    store-flag      = ".atomic" / ".atomic_add"

Overview
~~~~~~~~

Store a scalar value (first operand) in a memref (second operand) at the position given by the index list.
The number of indices must match the order of the memref.

The store is atomic when the atomic flag is set with relaxed memory ordering.
When the atomic_add flag is set, the following steps are done atomically:
The value at the memory location is fetched, the scalar value is added to the fetched value,
and the resulting value is stored at the memory location.

When storing a complex value the update may be pseudo-atomic, meaning that an atomic store is used
for the the real and imaginary separately.

*Note:* Store should only be used in SPMD regions as otherwise the same memory location is written
from all work-items.

Operands
~~~~~~~~

======= ================ ===========
Op.-No. Type             Description
======= ================ ===========
1       scalar-type      value
2       memref-type      tensor
3...    index            index list
======= ================ ===========

Restrictions
~~~~~~~~~~~~

* :math:`\text{type}(value) = \text{element_type}(tensor)`

Yield
.....

.. code:: abnf

    instruction                 =/ "yield" "(" [local-identifier-list] ")"

Overview
~~~~~~~~

Yield returns values from an if or for instruction.

Operands
~~~~~~~~

======= ============================================ ===========
Op.-No. Type                                         Description
======= ============================================ ===========
1...    boolean-type / scalar-type / coopmatrix-type value
======= ============================================ ===========

Additional instructions
.......................

.. code:: abnf

    instruction             =/ "lifetime_stop" local-identifier

SPMD instructions
-----------------

Builtin (SPMD)
..............

.. code:: abnf

    spmd-builtin-type       =  "builtin.subgroup_id"  /
                               "builtin.subgroup_local_id"
    value-instruction       =/ spmd-builtin-type ":" integer-type

Overview
~~~~~~~~

Returns a builtin value.
The following table shows the builtins' description and the types that are returned.

================== ===== =================================================================================
Builtin            Type  Description
================== ===== =================================================================================
subgroup_id        i32   Returns the subgroup id; integer from 0 to num_subgroups - 1.
subgroup_local_id  i32   Returns the work-item id within the subgroup; integer from 0 to subgroup_size - 1
================== ===== =================================================================================

Cooperative matrix load
.......................

.. code:: abnf

    value-instruction           =/ "cooperative_matrix_load" transpose [checked-flag]
                                   local-identifier "[" local-identifier "," local-identifier "]"
                                   ":" coopmatrix-type
    checked-flag                = ".rows_checked" / ".cols_checked" / ".both_checked"

Overview
~~~~~~~~

Load a cooperative matrix from a 2d-memref at the position given by the indices in square brackets.
The position gives the starting row and column index, that is,
when a coopmatrix of size :math:`X\times Y` is loaded from memref :math:`M` at
position :math:`x, y`, then the components :math:`A_{ij}` of the coopmatrix are given by

.. math::

    \forall i \in [0,X), j \in [0,Y): A_{ij} := M[(x + i) S_1 + (y + j) S_2],

where :math:`S_1` and :math:`S_2` are the entries of the memref's stride array.
When the transpose modifier ".t" is given, we have

.. math::

    \forall i \in [0,X), j \in [0,Y): A_{ij} := M[(x + j) S_1 + (y + i) S_2] 

When the checked flag is set, the following out-of-bound checks are added
(with memref shape :math:`s_1\times s_2`):

=============== ===================================================================
Flag            Description
=============== ===================================================================
.n.rows_checked :math:`A_{ij} := M[...] \text{ if } 0 \leq x+i < s_1 \text{ else } 0`
.t.rows_checked :math:`A_{ij} := M[...] \text{ if } 0 \leq y+i < s_2 \text{ else } 0`
.n.cols_checked :math:`A_{ij} := M[...] \text{ if } 0 \leq y+j < s_2 \text{ else } 0`
.t.cols_checked :math:`A_{ij} := M[...] \text{ if } 0 \leq x+j < s_1 \text{ else } 0`
.n.both_checked .n.rows_checked.n and .n.cols_checked
.t.both_checked .t.rows_checked.t and .t.cols_checked
=============== ===================================================================

Operands
~~~~~~~~

======= =============== ===========
Op.-No. Type            Description
======= =============== ===========
1       memref-type     M
2       index           x
3       index           y
======= =============== ===========

Restrictions
~~~~~~~~~~~~

* :math:`\text{order}(M) = 2`
* :math:`\text{component_type}(A) = \text{element_type}(M)`
* All arguments **must** be dynamically uniform.

Cooperative matrix mul add
..........................

.. code:: abnf

    value-instruction           =/ "cooperative_matrix_mul_add" local-identifier ","
                                   local-identifier "," local-identifier ":" coopmatrix-type

Overview
~~~~~~~~

Matrix mul add returns the value of 

.. math::

    D := AB + C,

where A, B, and C are matrices given by the three operands.

Operands
~~~~~~~~

======= =============== ========== ===========
Op.-No. Type            Use        Description
======= =============== ========== ===========
1       coopmatrix-type matrix_a   A
2       coopmatrix-type matrix_b   B
3       coopmatrix-type matrix_acc C
======= =============== ========== ===========

Restrictions
~~~~~~~~~~~~

* :math:`\text{columns}(A) = \text{rows}(B)`
* :math:`\text{rows}(C) = \text{rows}(A) \land \text{columns}(C) = \text{columns}(B)`
* :math:`\text{shape}(D) = \text{shape}(C)`
* :math:`\text{use}(D) = \text{matrix_acc}`
* :math:`\text{promote}(\text{component_type}(A), \text{component_type}(B)) \preceq \text{component_type}(C)`
* Cast of :math:`\text{component_type}(C)` to :math:`\text{component_type}(D)` must be allowed

Cooperative matrix scale
........................

.. code:: abnf

    value-instruction           =/ "cooperative_matrix_scale" local-identifier "," local-identifier
                                   ":" coopmatrix-type

Overview
~~~~~~~~

Scale a coopmatrix by a scalar. 
The scalar type of the scalar and the component type of the coopmatrix must match,
and the returned must have the same coopmatrix type as the matrix operand.

Operands
~~~~~~~~

======= =============== ===========
Op.-No. Type            Description
======= =============== ===========
1       scalar-type     scalar
2       coopmatrix-type matrix
======= =============== ===========

Restrictions
~~~~~~~~~~~~

* :math:`\text{type}(scalar) = \text{component_type}(matrix)`
* :math:`\text{type}(result) = \text{type}(matrix)`

Cooperative matrix store
........................

.. code:: abnf

    instruction     =/ "cooperative_matrix_store" [checked-flag] [store-flag] local-identifier ","
                       local-identifier "[" local-identifier "," local-identifier "]"

Overview
~~~~~~~~

Store a cooperative matrix value in a 2d-memref at the position given by the indices in square brackets.
The position gives the starting row and column index, that is,
when a coopmatrix of size :math:`X\times Y` is written to memref :math:`M` at
position :math:`x, y`, then the components :math:`A_{ij}` of the coopmatrix are written to

.. math::

    \forall i \in [0,X), j \in [0,Y): M[(x + i) S_1 + (y + j) S_2] := A_{ij},

where :math:`S_1` and :math:`S_2` are the entries of the memref's stride array.
When the checked flag is set, the following out-of-bound checks are added
(with memref shape :math:`s_1\times s_2`):

============= =======================================================================================================
Flag            Description
============= =======================================================================================================
.rows_checked Only execute store if :math:`0 \leq x+i < s_1`
.cols_checked Only execute store if :math:`0 \leq y+j < s_2`
.both_checked .rows_checked + .cols_checked
============= =======================================================================================================

The store is atomic when the atomic flag is set with relaxed memory ordering.
When the atomic_add flag is set, the coopmatrix is added to the memref atomically.

When storing a complex value the update may be pseudo-atomic, meaning that an atomic store is used
for the the real and imaginary separately.

Operands
~~~~~~~~

======= =============== ===========
Op.-No. Type            Description
======= =============== ===========
1       coopmatrix-type A
2       memref-type     M
3       index           x
4       index           y
======= =============== ===========

Restrictions
~~~~~~~~~~~~

* :math:`\text{component_type}(A) = \text{element_type}(B)`
* All arguments **must** be dynamically uniform.

Subgroup broadcast
..................

.. code:: abnf

    value-instruction       =/ "subgroup_broadcast" local-identifier "," local-identifier ":" scalar-type

Overview
~~~~~~~~

Broadcast a scalar to all work-items in the subgroup.
The scalar type of the first operand and the type of the result must match.
The second identifier must have i32 type.

Operands
~~~~~~~~

======= =============== ==================================================================================================
Op.-No. Type            Description
======= =============== ==================================================================================================
1       scalar-type     Value that is to be distributed to all work-items of the sub-group
2       i32             Subgroup local index that identifies the work-item whose value is returned to all other work-items
======= =============== ==================================================================================================

Restrictions
~~~~~~~~~~~~

* The second operand **must** be dynamically uniform.

Subgroup add
............

.. code:: abnf

    subgroup-add-type       = "subgroup_add.exclusive_scan" /
                              "subgroup_add.inclusive_scan" /
                              "subgroup_add.reduce" /
    value-instruction       =/ subgroup-add-type local-identifier ":" scalar-type

Overview
~~~~~~~~

Computes the :ref:`subgroup operation` with :math:`\diamond:=+` and :math:`I:=0`.

Subgroup max
............

.. code:: abnf

    subgroup-max-type       = "subgroup_max.exclusive_scan" /
                              "subgroup_max.inclusive_scan" /
                              "subgroup_max.reduce" /
    value-instruction       =/ subgroup-max-type local-identifier ":" scalar-type

Overview
~~~~~~~~

Computes the :ref:`subgroup operation` with :math:`\diamond:=\max` and identity as given in the following table:

============= ==============================================
Identity      Value
============= ==============================================
integer-type  Smallest integer representable by integer type
floating-type :math:`-\infty`
complex type  Forbidden
============= ==============================================

Subgroup min
............

.. code:: abnf

    subgroup-min-type       = "subgroup_min.exclusive_scan" /
                              "subgroup_min.inclusive_scan" /
                              "subgroup_min.reduce" /
    value-instruction       =/ subgroup-min-type local-identifier ":" scalar-type

Overview
~~~~~~~~

Computes the :ref:`subgroup operation` with :math:`\diamond:=\min` and identity as given in the following table:

============= =============================================
Identity      Value
============= =============================================
integer-type  Largest integer representable by integer type
floating-type :math:`+\infty`
complex type  Forbidden
============= =============================================

.. _subgroup operation:

Subgroup operation
..................

Let :math:`[x_0,x_1,\dots,x_{n-1}]` be the input vector contributed by a subgroup of size *n*.
(The work-item with subgroup local id *i* contributes :math:`x_i`.)
Let :math:`\diamond` be the binary operator and *I* the identity.
We define the output vector of size *n* for the group operations in the following table:

============== =============================================================================================
Operation type Result
============== =============================================================================================
exclusive_scan :math:`[I, x_0, (x_0 \diamond x_1), \dots, x_0 \diamond x_1 \diamond \dots \diamond x_{n-2}]`
inclusive_scan :math:`[x_0, (x_0 \diamond x_1), \dots, x_0 \diamond x_1 \diamond \dots \diamond x_{n-1}]`
reduce         :math:`[s,s,\dots,s] \text{ with } s := x_0 \diamond \dots \diamond x_{n-1}`
============== =============================================================================================


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
                         %A: group<memref<f32x16x8>x?>,
                         %B: memref<f32x8x8>,
                         %C: memref<f32x8x16>,
                         %D: memref<f32x16x16x?>) {
      %0 = group_id : index
      %1 = load %A[%0]        : memref<f32x16x8>
      %2 = subview %D[:,:,%0] : memref<f32x16x16>
      %tmp0 = alloca : memref<f32x16x8>
      %zero = constant 0.0 : f32
      %one = constant 1.0 : f32
      gemm.n.t %one, %1, %B, %zero, %tmp0
      gemm.n.n %alpha, %tmp0, %C, %one, %2
    }
