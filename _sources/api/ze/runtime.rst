.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

=======
Runtime
=======

Here, functions to online compile binaries to native Level Zero types are described
as well as the implementation of the runtime abstraction for Level Zero.
In most cases, the API documented here needs not be used directly but one just needs
to pass the :cpp:class:`tinytc::level_zero_runtime` type to the :cpp:class:`tinytc::tensor_kernel_bundle`
class as template argument.

Functions
=========

.. doxygenfunction:: tinytc::make_kernel_bundle(std::uint8_t const*,std::size_t,bundle_format,std::uint32_t,ze_context_handle_t,ze_device_handle_t)
.. doxygenfunction:: tinytc::make_kernel(ze_module_handle_t,char const*)
.. doxygenfunction:: tinytc::get_group_count

Classes
=======

.. doxygenclass:: tinytc::level_zero_runtime
.. doxygenclass:: tinytc::shared_handle
.. doxygenclass:: tinytc::level_zero_argument_handler
