.. Copyright (C) 2025 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

.. _coopmatrix layout:

=================
Coopmatrix layout
=================

A cooperative matrix is distributed to work-items such that each work-item holds an equal
share of the matrix, potentially with zero-padding if the matrix size is not divisible by
the subgroup size.

General layout
==============

Let a coopmatrix :math:`A` of size :math:`M\times N` be given and let the subgroup size
be given by :math:`S`.
We require :math:`M,S` to be powers of two and to be greater or equal than 1.
Internally, we represent the :math:`A` matrix as the :math:`I \times K_1\times J \times K_2` tensor :math:`A^*`.
The mapping of :math:`A` to :math:`A^*` is

.. math::

   A^*_{i,k_1,j,k_2} = \left\{\begin{array}{rcl}
                           A_{i+k_1I+k_2IK_1,j} & \text{ if } & i+k_1I+k_2IK_1 < M \wedge j < N, \\
                           0 & \text{ else.}
                       \end{array}\right.

The shape of :math:`A^*` is given by :math:`(I,K_1,J,K_2)`, where

.. math::

   \begin{aligned}
   I &:= \min(M, S),\\
   J &:= \min(\{n\in\mathbb N : n \geq N \wedge (In) \bmod S = 0\})\\
   K &:= K_1K_2 = M/I.\\
   \end{aligned}

As both :math:`S` and :math:`I` are powers of two, an explicit formula for :math:`J` is given by
:math:`J = (\lceil IN/S\rceil S) / I`.

Work-item mapping
-----------------

We linearize the index of the :math:`A^*` tensor canonically:

.. math::

   L(i,j,k) := i + k_1I + j IK_1 + k_2 IK_1J

Every work-item stores a vector :math:`v` with :math:`V:=IKJ/S` components.
We define the per work-item vector as

.. math::

   W^p := (v \in [V] : A^*[L^{-1}(p+vS)]),

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

For a :math:`1\times 17` coopmatrix we have

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
   k_1 &= \lfloor L / I \rfloor \bmod K_1, \\
   j &= \lfloor L / (IK_1)\rfloor \bmod J, \\
   k_2 &= \lfloor L / (IK_1J)\rfloor. \\
   \end{aligned}

Let :math:`v=w_1 + uK_1 + w_2K_1(V/K)`, with :math:`u=0,\dots,V/K-1`,
:math:`w_1=0,\dots,K_1-1`, and :math:`w=0,\dots,K_2-1`.
(Note that :math:`V/K=IJ/S`.)

We first assume that :math:`I=S`. Then
   
.. math::

   \begin{aligned}
   i &= (p + (w_1 + uK_1 + w_2K_1J)S) \bmod S = p, \\
   k_1 &= \lfloor (p + (w_1 + uK_1 + wK_1J)S) / S \rfloor \bmod K_1 = w_1, \\
   j &= \lfloor (p + (w_1 + uK_1 + wK_1J)S) / (SK_1) \rfloor \bmod J = u, \\
   k_2 &= \lfloor (p + (w_1 + uK_1 + wK_1J)S) / (SK_1J)\rfloor = w_2, \\
   \end{aligned}

Now we assume :math:`I<S` (which implies :math:`K=K_1=K_2=1, w_1=w_2=0,` and :math:`S/I\in\mathbb{N}`).
We have

.. math::

   \begin{aligned}
   i &= (p + uS) \bmod I = p \bmod I, \\
   k_1 &= \lfloor (p + uS) / I \rfloor \bmod 1 = 0, \\
   j &= \lfloor (p + uS) / I \rfloor \bmod J = \lfloor p/I \rfloor + u (S/I), \\
   k_2 &= \lfloor (p + uS) / (IJ) \rfloor = 0, \\
   \end{aligned}

Combining both cases into a single formula we get

.. math::

   \begin{aligned}
   i &= p \bmod I, \\
   k_1 &= w_1, \\
   j &= \lfloor p/I \rfloor + u (S/I) , \\
   k_2 &= w_2, \\
   \end{aligned}

Load pseudo-code (SIMT)
-----------------------

.. code-block:: cpp

   template <typename RealT, int M, int N, int K1, bool Transpose, bool RowsChecked, bool ColsChecked>
   vector<V> load_coopmatrix(RealT* B, int pos0, int pos1, int shape0, int shape1,
                             int stride0, int stride1) {
       constexpr int S = get_sub_group_size();
       constexpr int I = min(M,S);
       constexpr int J = ceil(I*N/S)*S/I;
       constexpr int K = M/I;
       static_assert(K%K1 == 0);
       constexpr int K2 = K/K1;
       constexpr bool needs_mask = J*S/I > N;

       if (Transpose) {
           std::swap(shape0, shape1);
           std::swap(stride0, stride1);
       }

       constexpr int V = I*K*J/S;
       array<RealT, V> R;
       int p =  get_sub_group_local_id();
       int i0 = p % I;
       int j0 = p / I;
       for (int w1 = 0; w1 < K1; ++w1) {
           for (int w2 = 0; w2 < K2; ++w2) {
               int k1 = w1, k2 = w2;
               int row = pos0 + i0 + (k1 + k2*K1)*I;
               bool row_ok = !RowsChecked || (row >= 0 && row < shape0);
               if (row_ok) {
                   for (int u = 0; u < V/K; ++u) {
                       int j = j0 + u*(S/I);
                       int col = pos1 + j;
                       bool col_ok = !ColsChecked || (col >= 0 && col < shape1);
                       bool mask_ok = !needs_mask || j < N;
                       R[w1 + u*K1 + w2*K1*(V/K)] = mask_ok && col_ok ? A[row * stride0 + col * stride1] : 0;
                   }
               } else {
                   for (int u = 0; u < V/K; ++u) {
                       R[w1 + u*K1 + w2*K1*(V/K)] = 0;
                   }
               }
           }
       }
       return R;
    }

Matrix Accumulator
==================

For cooperative matrices with use *matrix_acc* we always have :math:`K_1=1`.

Matrix A
========

For cooperative matrices with use *matrix_a* we always have :math:`K_1=1`.
Moreover, low precision matrices are VNNI transformed if :math:`N` is a multiple of :math:`\omega`,
where :math:`\omega=\max(1, \max(1,4/\text{size}(\text{ty}))` is the number of operands per channel.
We store :math:`A^*` tensors internally
as the :math:`\omega\times I \times K_1\times \lceil J/\omega\rceil \times K_2` tensor :math:`A^{**}`,
The mapping from :math:`A^*` to :math:`A^{**}` is given by

.. math::

   A^{**}_{c,i,k_1,j,k_2} := A^{*}_{i,k_1,c+j\omega,k_2}

and the inverse mapping is given by

.. math::

   A^{*}_{i,k_1,j,k_2} = A^{**}_{j\bmod\omega,i,k_1,\lfloor j/\omega\rfloor,k_2}.

Moreover, all channels of an entry are packed. E.g. for half precision floats we have two channels
and we pack :math:`A^{**}_{0,i,k_1,j,k_2}` in the lower 16 bits of an i32 and :math:`A^{**}_{1,i,k_1,j,k_2}` 
in the higher 16 bits of the i32.
We store an i32 per work-item, so from the SIMT point of view one work-item owns all channels of an entry.

Matrix B
========

Let :math:`\omega_b = \max(1,2/\text{size}(\text{ty}))`.
We choose :math:`K_1 = \omega_b \text{ if } M/S > 1 \text{ else } 1`.
