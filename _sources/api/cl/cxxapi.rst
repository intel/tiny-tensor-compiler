.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

=======
C++-API
=======

Common
======

* Functions

  * :ref:`CL_CHECK_STATUS`

Common Functions
----------------

CL_CHECK_STATUS
...............

.. doxygenfunction:: tinytc::CL_CHECK_STATUS

Device Info
===========

* Functions

  * :ref:`make_core_info(cl_device_id)`

Device Info Functions
---------------------

make_core_info(cl_device_id)
............................

.. doxygenfunction:: tinytc::make_core_info(cl_device_id)

Kernel
======

* Functions

  * :ref:`get_global_size(std::uint32_t,std::array<std::size_t, 3u> const &)`

  * :ref:`get_group_size(cl_kernel)`

  * :ref:`make_kernel(cl_program,char const*)`

  * :ref:`make_kernel_bundle(cl_context,cl_device_id,binary const&)`

Kernel Functions
----------------

get_global_size(std::uint32_t,std::array<std::size_t, 3u> const &)
..................................................................

.. doxygenfunction:: tinytc::get_global_size(std::uint32_t,std::array<std::size_t, 3u> const &)

get_group_size(cl_kernel)
.........................

.. doxygenfunction:: tinytc::get_group_size(cl_kernel)

make_kernel(cl_program,char const*)
...................................

.. doxygenfunction:: tinytc::make_kernel(cl_program,char const*)

make_kernel_bundle(cl_context,cl_device_id,binary const&)
.........................................................

.. doxygenfunction:: tinytc::make_kernel_bundle(cl_context,cl_device_id,binary const&)

Recipe
======

* Functions

  * :ref:`make_recipe_handler(cl_context,cl_device_id,recipe const&)`

* Classes

  * :ref:`opencl_recipe_handler`

Recipe Functions
----------------

make_recipe_handler(cl_context,cl_device_id,recipe const&)
..........................................................

.. doxygenfunction:: tinytc::make_recipe_handler(cl_context,cl_device_id,recipe const&)

Recipe Classes
--------------

opencl_recipe_handler
.....................

.. doxygenclass:: tinytc::opencl_recipe_handler

