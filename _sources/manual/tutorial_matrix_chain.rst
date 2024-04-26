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
                       %A: group<memref<f32x9x9>>,
                       %Q: memref<f32x56x9x?>) {
        %gid = group_id                                ; Get our index e
    
        %p = subview %P[:,:,%gid] : memref<f32x56x9x?> ; %p has type memref<f32x56x9>
        %a = load %A[%gid] : group<memref<f32x9x9>>    ; %a has type memref<f32x9x9>
        %q = subview %Q[:,:,%gid] : memref<f32x56x9x?> ; %q has type memref<f32x56x9>
    
        %tmp = alloca -> memref<f32x56x9>              ; Reserve temporary memory
    
        gemm.n.n 1.0, %K, %p, 0.0, %tmp                ; Compute tmp <- K P(:,:,e)
            : f32, memref<f32x56x56>, memref<f32x56x9>, f32, memref<f32x56x9>
        gemm.n.n 1.0, %tmp, %a, 1.0, %q                ; Update Q(:,:,e) <- Q(:,:,e) + tmp A_e
            : f32, memref<f32x56x9>, memref<f32x9x9>, f32, memref<f32x56x9>
    }

Compilation with the Tiny Tensor Compiler generates the following OpenCL-C code

.. code-block:: c

    kernel
    __attribute__((reqd_work_group_size(64,1,1)))
    __attribute__((intel_reqd_sub_group_size(32)))
    fused_kernel(global float *K, global float *P, uint P_shape2, global float *global *A,
                 global float *Q, uint Q_shape2) {
        local uchar stack[2016] __attribute__((aligned(64)));
        uint gid = get_global_id(2);
        global float *p = P + 0ll * 1 + 0ll * 56 + gid * 504;
        global float *a = *(A + gid);
        global float *q = Q + 0ll * 1 + 0ll * 56 + gid * 504;
        local float *tmp = (local float *)(stack + 0);
        gemm_f32f32f32f32f32_An_Bn_M56_N9_K56_Astride1_56_Bstride1_56_Cstride1_56_alpha3ff0000000000000_beta0(
            56, 9, 56, 0x1p+0f, K, 1, 56, p, 1, 56, 0x0p+0f, tmp, 1, 56);
        barrier(CLK_LOCAL_MEM_FENCE);
        gemm_f32f32f32f32f32_An_Bn_M56_N9_K9_Astride1_56_Bstride1_9_Cstride1_56_alpha3ff0000000000000_beta3ff0000000000000(
            56, 9, 9, 0x1p+0f, tmp, 1, 56, a, 1, 9, 0x1p+0f, q, 1, 56);
    }

where the definition of the generated GEMM functions have been omitted for brevity.
We observe that

* a GEMM is processed in parallel by a work-group with 64 threads,
* temporary memory is mapped to shared local memory (local uchar stack),
* load and subview calls translate to simple pointer manipulation,
* and that a barrier has been introduced between the GEMM calls to avoid data races.

When using SYCL, we can run the kernel using the following pseudo-code:

.. code-block:: cpp

    #include <tinytc/tinytc.hpp>
    #include <tinytc/tinytc_sycl.hpp>
    #include <sycl/sycl.hpp>

    auto source_ctx = tinytc::make_source_context();
    try {
        // Parse tensor program
        auto prog = tinytc::parse_file("fused_kernel.ir", source_ctx);

        // JIT compile program
        auto q = sycl::queue{};
        auto info = tinytc::make_core_info(q.get_device());
        auto bin = tinytc::compile_to_binary(std::move(prog), info, tinytc::bundle_format::native,
                                             source_ctx);

        // Initialize tensors
        float* K = ...;
        float* P = ...;
        float** A = ...;
        float* Q = ...;

        auto bundle = tinytc::make_kernel_bundle(q.get_context(), q.get_device(), bin);
        auto kernel = tinytc::make_kernel(bundle, "fused_kernel");
        auto exe_range = tinytc::get_execution_range(kernel, howmany);
        for (int timestep = 0; timestep < num_timesteps; ++timestep) {
            q.submit([&](sycl::handler &h) {
                h.set_args(K, P, howmany, A, Q, howmany);
                h.parallel_for(exec_range, kernel);
            }).wait();
        }
    } catch (tinytc::status const& st) {
        std::cerr << "Error (" << static_cast<int>(st) << "): "
                  << tinytc::error_string(st) << std::endl;
        std::cerr << "Error log:" << std::endl
                  << source_ctx.get_error_log() << std::endl;
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
    }

Note that a fictional time-loop was introduced around `q.submit`.
As a general rule, JIT compilation is expensive in comparison to kernel execution,
hence, a compiled program should be reused many times.
