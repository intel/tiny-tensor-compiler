.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

.. _Tall and skinny GEMM recipe:

====================
Tall and skinny GEMM
====================

The tall and skinny GEMM recipe implements the following tensor operation:

.. math::

    C = \alpha AB + \beta C

where
:math:`A \in \mathbb{R}^{M\times K}`,
:math:`B \in \mathbb{R}^{K\times N}`,
:math:`C \in \mathbb{R}^{M\times N}`,
and :math:`M \gg K`, :math:`M \gg N`.


The recipe can be used as following, assuming the SYCL runtime is selected
and the memory layout is packed:

.. code-block:: cpp

    #include <tinytc/tinytc-sycl.hpp>
    #include <tinytc/tinytc.hpp>

    using namespace tinytc;

    try {
        auto gemm = recipe::tall_and_skinny<T, sycl_runtime>(N, K,
                                                             get_core_info(q.get_device()),
                                                             q.get_context(), q.get_device());
        gemm(M, 1.0, A, M, B, K, 0.0, C, M, q).wait();
    } catch (std::compilation_error const &e) {
        std::cerr << e.loc() << ": " << e.what() << std::endl;
    } catch (sycl::exception const &e) {
        std::cerr << e.what() << std::endl;
    }

Advanced users may call the :cpp:func:`tinytc::recipe::generate_tall_and_skinny_binary` function
to create  a device binary.
The binary may contain several kernels, e.g. a separate kernel for :math:`\beta=0` and
:math:`\beta\neq 0`.

.. warning::

    JIT compilation is expensive. Each recipe parameterization should only be created once
    and be reused many times.

Classes
=======

.. doxygenclass:: tinytc::recipe::tall_and_skinny

Functions
=========

.. doxygenfunction:: tinytc::recipe::generate_tall_and_skinny_binary

