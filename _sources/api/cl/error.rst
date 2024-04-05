.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

==============
Error handling
==============

All calls to the OpenCL API are wrapped in the :c:macro:`CL_CHECK` macro.
The macro checks the return code and throws a :cpp:class:`tinytc::opencl_error`
should the return code not equal `CL_SUCCESS`.

.. doxygendefine:: CL_CHECK
.. doxygenclass:: tinytc::opencl_error
.. doxygenfunction:: tinytc::cl_status_to_string
