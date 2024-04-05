.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

.. _Small GEMM batched recipe:

==================
Batched small GEMM
==================

The batched small GEMM recipe implements the following tensor operation:

.. math::

    C_i = \alpha \text{op}_A(A_i) \text{op}_B(B_i) + \beta C_i

where
:math:`\text{op}_A(A_i) \in \mathbb{R}^{M\times K}`,
:math:`\text{op}_B(B_i) \in \mathbb{R}^{K\times N}`,
:math:`C_i \in \mathbb{R}^{M\times N}`,
:math:`i` is the group id,
and

.. math::

   \text{op}_{X}(Y) = \left\{\begin{array}{rcl}
                        Y^T & \text{if} & t_X = T, \\
                        Y & \text{if} & t_X = N.
                      \end{array}\right.

The matrices in a matrix batch are separated by a fixed stride, that is,
the address is computed as following for a matrix batch X:

.. code-block:: cpp

    X[m + n * ldX + i * strideX] // accesses X_i(m,n)

The recipe can be used as following, assuming the SYCL runtime is selected, no transpose is required,
and the memory layout is packed:

.. code-block:: cpp

    #include <tinytc/tinytc-sycl.hpp>
    #include <tinytc/tinytc.hpp>

    using namespace tinytc;

    try {
        auto gemm = recipe::small_gemm_batched<T, sycl_runtime>(transpose::N, transpose::N,
                                                                M, N, K,
                                                                M, M*K, // ldA, strideA
                                                                K, K*N, // ldB, strideB
                                                                M, M*N, // ldC, strideC
                                                                get_core_info(q.get_device()),
                                                                q.get_context(), q.get_device());
        gemm(howmany, 1.0, A, B, 0.0, C, q).wait();
    } catch (std::compilation_error const &e) {
        std::cerr << e.loc() << ": " << e.what() << std::endl;
    } catch (sycl::exception const &e) {
        std::cerr << e.what() << std::endl;
    }

Advanced users may call the :cpp:func:`tinytc::recipe::generate_small_gemm_batched_binary` function
to create  a device binary.
The binary may contain several kernels, e.g. a separate kernel for :math:`\beta=0` and
:math:`\beta\neq 0`.

.. warning::

    JIT compilation is expensive. Each recipe parameterization should only be created once
    and be reused many times.

Classes
=======

.. doxygenclass:: tinytc::recipe::small_gemm_batched

Functions
=========

.. doxygenfunction:: tinytc::recipe::generate_small_gemm_batched_binary

