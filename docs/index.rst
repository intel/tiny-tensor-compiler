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
The library exposes a C-API and a C++-API.

License
-------

`BSD 3-Clause License <https://www.opensource.org/licenses/BSD-3-Clause>`_

Table of contents
-----------------

.. toctree::
   :maxdepth: 2
   :caption: User manual

   manual/build
   manual/core
   manual/builder
   manual/calling_convention
   manual/tensor-ir
   manual/tutorial_matrix_chain

.. toctree::
   :maxdepth: 2
   :caption: API

   api/index
   api/cl/index
   api/sycl/index
   api/ze/index

.. toctree::
   :maxdepth: 2
   :caption: Developer guide

   dev/coopmatrix_layout

Index
-----

:ref:`genindex`
