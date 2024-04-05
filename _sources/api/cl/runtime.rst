.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

=======
Runtime
=======

Here, functions to online compile binaries to native OpenCL types are described
as well as the implementation of the runtime abstraction for OpenCL.
In most cases, the API documented here needs not be used directly but one just needs
to pass the :cpp:class:`tinytc::opencl_runtime` type to the :cpp:class:`tinytc::tensor_kernel_bundle`
class as template argument.

Concepts
========

Standard OpenCL does not support Unified Shared Memory but only via the
`cl_intel_unified_shared_memory <https://registry.khronos.org/OpenCL/extensions/intel/cl_intel_unified_shared_memory.html>`_
extension.
Therefore, one needs to handle pointer kernel arguments differently from `cl_mem` and other kernel arguments.

.. doxygenconcept:: tinytc::pointer_kernel_argument
.. doxygenconcept:: tinytc::regular_kernel_argument

Functions
=========

.. doxygenfunction:: tinytc::make_kernel_bundle(std::uint8_t const*,std::size_t,bundle_format,std::uint32_t,cl_context,cl_device_id)
.. doxygenfunction:: tinytc::make_kernel(cl_program,char const*)
.. doxygenfunction:: tinytc::get_opencl_nd_range

Classes
=======

.. doxygenclass:: tinytc::opencl_runtime
.. doxygenclass:: tinytc::opencl_object_wrapper
.. doxygenstruct:: tinytc::opencl_nd_range
.. doxygenclass:: tinytc::opencl_argument_handler
