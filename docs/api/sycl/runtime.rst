.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

=======
Runtime
=======

Here, functions to online compile binaries to native SYCL types are described
as well as the implementation of the runtime abstraction for SYCL.
In most cases, the API documented here needs not be used directly but one just needs
to pass the :cpp:class:`tinytc::sycl_runtime` type to the :cpp:class:`tinytc::tensor_kernel_bundle`
class as template argument.

Functions
=========

.. doxygenfunction:: tinytc::make_kernel_bundle(std::uint8_t const*,std::size_t,bundle_format,std::uint32_t,sycl::context,sycl::device)
.. doxygenfunction:: tinytc::make_kernel(sycl::kernel_bundle<sycl::bundle_state::executable>,char const*)
.. doxygenfunction:: tinytc::get_sycl_nd_range

Classes
=======

.. doxygenclass:: tinytc::sycl_runtime
.. doxygenclass:: tinytc::sycl_argument_handler
