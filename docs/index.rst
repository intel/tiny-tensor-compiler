.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

Tiny Tensor Compiler
====================

The Tiny Tensor Compiler compiles programs written in the :ref:`tensor language <tensor language>`
to OpenCL-C and provides methods to run these programs on GPU devices and other devices,
supporting
the `OpenCL <https://www.khronos.org/opencl/>`_,
`Level Zero <https://spec.oneapi.io/level-zero/latest/>`_,
and `SYCL <https://www.khronos.org/sycl/>`_ runtime.

License
-------

`BSD 3-Clause License <https://www.opensource.org/licenses/BSD-3-Clause>`_

Table of contents
-----------------

.. toctree::
   :maxdepth: 2
   :caption: User manual

   manual/build
   manual/usage
   manual/calling_convention
   manual/tensor-ir

.. toctree::
   :maxdepth: 2
   :caption: API

   api/index
   api/ze/index
   api/cl/index
   api/sycl/index

Index
-----

:ref:`genindex`