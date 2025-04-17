.. Copyright (C) 2025 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

=================
Coopmatrix layout
=================

A cooperative matrix is distributed to work-items such that each work-item holds an equal
share of the matrix, potentially with zero-padding if the matrix size is not divisible by
the subgroup size.

Definitions
===========

Let a coopmatrix :math:`A` of size :math:`M\times N` be given, and let the subgroup size
be given by :math:`S`, where :math:`S\geq 1` must be a power of two.

Matrix Accumulator
==================

For cooperative matrices with use *matrix_acc* we restrict :math:`M` to being a multiple of :math:`S`.
Internally, we represent the matrix as the :math:`I \times J \times K` tensor :math:`C`,
where the mapping of :math:`A` to :math:`C` is

.. math::

   C_{ijk} = \left\{\begin{array}{rcl}
                A_{(i+kI)j} & \text{ if } & i+kI\leq M \wedge j \leq N, \\
                0 & \text{ else.}
             \end{array}\right.

The shape of :math:`C` is given by

.. math::

   \begin{aligned}
   I &:= S,\\
   J &:= N,\\
   K &:= M / I.
   \end{aligned}

Work-item mapping
-----------------

We linearize the index of the C tensor canonically:

.. math::

   L(i,j,k) := i + j I + k IJ

Every work-item stores a vector :math:`v` with :math:`V:=JK` components.
We define the per work-item vector as

.. math::

   W^p := (v \in [V] : B[L^{-1}(p+vS)]),

where :math:`p=0,\dots,S-1`.

Mapping properties
------------------

The inverse of :math:`L` is

.. math::

   \begin{aligned}
   i &= L \bmod I, \\
   j &= \lfloor L / I\rfloor \bmod J, \\
   k &= \lfloor L / (IJ)\rfloor.
   \end{aligned}

Applying :math:`L^{-1}` on :math:`p+vS` and using that :math:`I=S` we find

.. math::

   \begin{aligned}
   i &= (p+vS) \bmod I = p \bmod S + vS \bmod S = p, \\
   j &= \lfloor(p+vS) / I\rfloor \bmod J = v \bmod J, \\
   k &= \lfloor(p+vS) / (IJ)\rfloor = \lfloor(\lfloor p/S\rfloor + v) / J\rfloor = \lfloor v/J\rfloor.
   \end{aligned}

We let :math:`v=u+wJ`, with :math:`u=0,\dots,J-1` and :math:`w=0,\dots,K-1` and get

.. math::

   \begin{aligned}
   i &= p, \\
   j &= (u + wJ) \bmod J = u, \\
   k &= \lfloor(u + wJ) / J\rfloor = w.
   \end{aligned}

Load pseudo-code (SIMT)
-----------------------

.. code-block:: cpp

   template <typename RealT, int M, int N, bool Transpose, bool RowsChecked, bool ColsChecked>
   vector<V> load_coopmatrix_acc(RealT* C, int pos0, int pos1, int shape0, int shape1,
                                 int stride0, int stride1) {
       int p = get_sub_group_local_id();
       constexpr int S = get_sub_group_size();
       constexpr int o = max(1, 4 / sizeof(RealT));
       constexpr int I = S;
       constexpr int J = ceil(N/o)*o;
       constexpr int K = M/I;

       if (Transpose) {
           std::swap(shape0, shape1);
           std::swap(stride0, stride1);
       }

       constexpr int V = J*K;
       vector<V> R;
       int i = p;
       for (int w = 0; w < K; ++w) {
           int k = w;
           int row = pos0 + i + k*I;
           bool row_ok = !RowsChecked || (row >= 0 && row < shape0);
           if (row_ok) {
               for (int u = 0; u < J; ++u) {
                   int j = u;
                   int col = pos1 + j;
                   bool col_ok = !ColsChecked || (col >= 0 && col < shape1);
                   R[u + w*J] = col_ok ? C[row * stride0 + col * stride1] : 0;
               }
           } else {
               for (int u = 0; u < J; ++u) {
                   R[u + w*J] = 0;
               }
           }
       }
       return R;
    }

Matrix A
========

For cooperative matrices with use *matrix_a* we restrict :math:`M` to being a multiple of
:math:`S`, too.
The internal representation of a matrix A is obtained by VNNI transforming the C tensor,
where the C tensor is defined in the same way as for matrices with use *matrix_acc*.
The VNNI transform is defined as following:

.. math::

   C'_{i,j,k} = C_{\lfloor i/o\rfloor + (j\bmod o)(S/o),i\bmod o+\lfloor j/o\rfloor o,k}

The inverse mapping is

.. math::

   C_{i,j,k} = C'_{io\bmod S + j\bmod o,\lfloor i/(S/o)\rfloor + \lfloor j/o\rfloor o,k}

Using the mapping properties of *matrix_acc* we have

.. math::

   C'_{p,u,w} = C_{\lfloor p/o\rfloor + (u\bmod o)(S/o),p\bmod o+\lfloor u/o\rfloor o,w}

Matrix B
========

For cooperative matrices with use *matrix_b* we restrict :math:`M` to be a power of two.
Internally, we represent the matrix the :math:`I \times K \times J` tensor :math:`B`,
where the mapping of :math:`A` to :math:`B` is

.. math::

   B_{ikj} = \left\{\begin{array}{rcl}
                A_{(i+kI)j} & \text{ if } & i+kI\leq M \wedge j \leq N, \\
                0 & \text{ else.}
             \end{array}\right.

The shape of :math:`B` is given by

.. math::

   \begin{aligned}
   I &:= \min(M, S),\\
   K &:= M/I,\\
   J &:= \min(\{n\in\mathbb N : n \geq N \wedge (In) \bmod{S} = 0\})\\
   \end{aligned}

As both :math:`S` and :math:`I` are powers of two, an explicit formula for :math:`J` is given by
:math:`J = (\lceil IN/S\rceil S) / I`.

Work-item mapping
-----------------

We linearize the index of the B matrix canonically:

.. math::

   L(i,k,j) := i + k I + j IK

Every work-item stores a vector :math:`v` with :math:`V:=IKJ/S` components.
We define the per work-item vector as

.. math::

   W^p := (v \in [V] : B[L^{-1}(p+vS)]),

where :math:`p=0,\dots,S-1`.

An example is helpful at this point.
Say we have a :math:`4\times 15` matrix and subgroup size :math:`S=16`, then the following table
shows how the work-item id maps to the 2d matrix index (the per work-item vectors are given by the
columns):

== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ====
p     0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15
== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ====
.x 0,0  1,0  2,0  3,0  0,1  1,1  2,1  3,1  0,2  1,2  2,2  3,2  0,3  1,3  2,3  3,3
.y 0,4  1,4  2,4  3,4  0,5  1,5  2,5  3,5  0,6  1,6  2,6  3,6  0,7  1,7  2,7  3,7
.z 0,8  1,8  2,8  3,8  0,9  1,9  2,9  3,9  0,10 1,10 2,10 3,10 0,11 1,11 2,11 3,11
.w 0,12 1,12 2,12 3,12 0,13 1,13 2,13 3,13 0,14 1,14 2,14 3,14 -, - -, - -, - -, -
== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ====

For a :math:`1\times 17` coopmatrix in f32 we have

== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ====
p     0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15
== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ====
.x 0,0  0,1  0,2  0,3  0,4  0,5  0,6  0,7  0,8  0,9  0,10 0,11 0,12 0,13 0,14 0,15
.y 0,16 -,-  -,-  -,-  -,-  -,-  -,-  -,-  -,-  -,-  -,-  -,-  -,-  -,-  -,-  -,-
== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ====

Mapping properties
------------------

The inverse of :math:`L` is

.. math::

   \begin{aligned}
   i &= L \bmod I, \\
   k &= \lfloor L / I\rfloor \bmod K, \\
   j &= \lfloor L / (IK)\rfloor, \\
   \end{aligned}

Let :math:`v=w+uK`, with :math:`w=0,\dots,K-1` and :math:`u=0,\dots,V/K-1`.

We first assume that :math:`I=S`. Then
   
.. math::

   \begin{aligned}
   i &= (p + wS + uKS) \bmod S = p, \\
   k &= \lfloor (p + wS + uKS) / S \rfloor \bmod K = w, \\
   j &= \lfloor (p + wS + uKS) / (SK)\rfloor = u, \\
   \end{aligned}

Now we assume :math:`I<S` (which implies :math:`K=1, w=0,` and :math:`S/I\in\mathbb{N}`).
We have

.. math::

   \begin{aligned}
   i &= (p + uS) \bmod I = p \bmod I, \\
   k &= \lfloor (p + uS) / I \rfloor \bmod 1 = 0, \\
   j &= \lfloor (p + uS) / I\rfloor = \lfloor p/I \rfloor + u (S/I) , \\
   \end{aligned}

Combining both cases into a single formula we get

.. math::

   \begin{aligned}
   i &= p \bmod I, \\
   k &= w, \\
   j &= \lfloor p/I \rfloor + u (S/I) , \\
   \end{aligned}


Load pseudo-code (SIMT)
-----------------------

.. code-block:: cpp

   template <typename RealT, int M, int N, bool Transpose, bool RowsChecked, bool ColsChecked>
   vector<V> load_coopmatrix_b(RealT* B, int pos0, int pos1, int shape0, int shape1,
                             int stride0, int stride1) {
       int p = get_sub_group_local_id();
       constexpr int S = get_sub_group_size();
       constexpr int o = max(1, 4 / sizeof(RealT));
       constexpr int e = o*S;
       constexpr int I = min(M,S);
       constexpr int K = M/I;
       constexpr int J = ceil(I*N/S)*S/I;

       if (Transpose) {
           std::swap(shape0, shape1);
           std::swap(stride0, stride1);
       }

       constexpr int V = I*K*J/S;
       vector<V> R;
       int i0 = p % I;
       int j0 = p / I;
       for (int w = 0; w < K; ++w) {
           int k = w;
           int row = pos0 + i0 + k*I;
           bool row_ok = !RowsChecked || (row >= 0 && row < shape0);
           if (row_ok) {
               for (int u = 0; u < V/K; ++u) {
                   int j = j0 + u*(S/I);
                   int col = pos1 + j;
                   bool col_ok = !ColsChecked || (col >= 0 && col < shape1);
                   bool mask_ok = (u+1)*(S/I) <= N || j < N;
                   R[w + u*K] = mask_ok && col_ok ? A[row * stride0 + col * stride1] : 0;
               }
           } else {
               for (int u = 0; u < V/K; ++u) {
                   R[w + u*K] = 0;
               }
           }
       }
       return R;
    }

