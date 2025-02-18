.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

==============================
Tutorial: Matrix chain product
==============================

We have the tensors
:math:`Q \in \mathbb{R}^{56x9xE}`,
:math:`K \in \mathbb{R}^{56x9}`,
:math:`P \in \mathbb{R}^{56x9xE}`,
and :math:`A_e \in \mathbb{R}^{9x9}, e\in[0,E)`.
For all :math:`e\in[0,E)` we want to compute the matrix chain multiplication

.. math::

   Q(:,:,e) \gets Q(:,:,e) + K P(:,:,e) A_e,

where :math:`Q(:,:,e)` selects a :math:`\mathbb{R}^{56x9}` submatrix from the tensor Q
and likewise for :math:`P(:,:,e)`.

In the :ref:`tensor language <tensor language>` we can implement the kernel as following:

.. _fused kernel example:

.. code-block::

    func @fused_kernel(%K: memref<f32x56x56>,
                       %P: memref<f32x56x9x?>,
                       %A: group<memref<f32x9x9>x?>,
                       %Q: memref<f32x56x9x?>) {
        %gid = builtin.group_id : index                   ; Get our index e

        %p = subview %P[0:56,0:9,%gid] : memref<f32x56x9> ; Get view on submatrix
        %a = load %A[%gid]             : memref<f32x9x9>  ; Load matrix from group
        %q = subview %Q[0:56,0:9,%gid] : memref<f32x56x9> ; Get view on submatrix

        %tmp = alloca : memref<f32x56x9,local>            ; Reserve temporary memory
                                                          ; in the Shared Local Memory
        %c0 = constant 0.0 : f32
        %c1 = constant 1.0 : f32
        gemm.n.n %c1, %K, %p, %c0, %tmp                   ; Compute tmp <- K P(:,:,e)
        gemm.n.n %c1, %tmp, %a, %c1, %q                   ; Update Q(:,:,e) <- Q(:,:,e) + tmp A_e
    }

Using the *tinytc-opt* tool we can run compiler passes on the code to get insight on what is happening under the hood.
For example, running the insert-lifetime-stop, insert-barrier, and work-group-size pass,

.. code-block:: bash

   tinytc-opt -pinsert-lifetime-stop -pinsert-barrier -pwork-group-size test.ir

we get

.. code-block::
   :emphasize-lines: 4, 13, 15

    func @fused_kernel(%K: memref<f32x56x56>,
                       %P: memref<f32x56x9x?>,
                       %A: group<memref<f32x9x9>x?>,
                       %Q: memref<f32x56x9x?>) attributes{subgroup_size=32, work_group_size=[64,1]} {
      %gid = builtin.group_id : index
      %p = subview %P[0:56,0:9,%gid] : memref<f32x56x9>
      %a = load %A[%gid] : memref<f32x9x9>
      %q = subview %Q[0:56,0:9,%gid] : memref<f32x56x9>
      %tmp = alloca : memref<f32x56x9,local>
      %c0 = constant 0x0p+0 : f32
      %c1 = constant 0x1p+0 : f32
      gemm.n.n %c1, %K, %p, %c0, %tmp
      barrier.local
      gemm.n.n %c1, %tmp, %a, %c1, %q
      lifetime_stop %tmp
    }

We observe that

* the kernel is executed concurrently by 64 work-items,
* temporary memory is only needed until after the lifetime_stop instruction after the GEMM
  (if multiple alloca's are present that do not overlap, that is, lifetime_stop for alloca #1 appears before alloca #2,
  then Shared Local Memory is reused, reducing the total amount needed),
* and that a barrier has been introduced between the GEMM calls to avoid data races.

When using SYCL, we can run the kernel using the following pseudo-code:

.. code-block:: cpp

    #include <tinytc/tinytc.hpp>
    #include <tinytc/tinytc_sycl.hpp>
    #include <sycl/sycl.hpp>

    #include <iostream>

    auto ctx = tinytc::make_compiler_context();
    ctx.set_error_reporter([](char const *what, const tinytc_location_t *,
                              void *) { std::cerr << what << std::endl; },
                           nullptr);
    try {
        // Parse tensor program
        auto prog = tinytc::parse_file("fused_kernel.ir", ctx);

        // Initialize tensors
        float* K = ...;
        float* P = ...;
        float** A = ...;
        float* Q = ...;

        // JIT compile program
        auto q = sycl::queue{};
        auto bundle = tinytc::make_kernel_bundle(q.get_context(), q.get_device(), prog);

        auto kernel = tinytc::make_kernel(bundle, "fused_kernel");
        auto exe_range = tinytc::get_execution_range(kernel, howmany);
        for (int timestep = 0; timestep < num_timesteps; ++timestep) {
            q.submit([&](sycl::handler &h) {
                h.set_args(K, P, howmany, A, howmany, Q, howmany);
                h.parallel_for(exec_range, kernel);
            }).wait();
        }
    } catch (tinytc::status const& st) {
        std::cerr << "Error (" << static_cast<int>(st) << "): "
                  << tinytc::error_string(st) << std::endl;
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
    }

Note that a fictional time-loop was introduced around `q.submit`.
As a general rule, JIT compilation is expensive in comparison to kernel execution,
hence, a compiled program should be reused many times.
