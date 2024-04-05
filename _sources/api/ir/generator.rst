.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

==============
GEMM generator
==============

The GEMM generator produces GEMM OpenCL functions that can be regulary called from an OpenCL kernel.
Therefore, the GEMM generator is exposed here.

The GEMM function always have the following signature:

.. code:: c

   void gemm(uint M, uint N, uint K,
             <alpha_type> alpha,
             <A_type> double* A, uint A_stride0, uint A_stride1,
             <B_type> double* B, uint B_stride0, uint B_stride1,
             <beta_type> beta,
             <C_type>* C, uint C_stride0, uint C_stride1);

If strides or the value of alpha and beta are known at compile-time,
they may be hard-coded in the GEMM function.
These values must still be passed as function argument, but the arguments are ignored in the kernel.

Moreover, it is required that the subgroup size and the number of subgroups in the kernel calling the GEMM
match the GEMM configuration.


Functions
=========

.. doxygenfunction:: generate_gemm
.. doxygenfunction:: max_register_block_gemm

Structs
=======

.. doxygenstruct:: tinytc::ir::gemm_scalar_type
   :members:
.. doxygenstruct:: tinytc::ir::gemm_configuration
   :members:
