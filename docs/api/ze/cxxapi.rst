.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

=======
C++-API
=======

Common
======

* Functions

  * :ref:`ZE_CHECK_STATUS`

Common Functions
----------------

ZE_CHECK_STATUS
...............

.. doxygenfunction:: tinytc::ZE_CHECK_STATUS

Device Info
===========

* Functions

  * :ref:`make_core_info(ze_device_handle_t)`

Device Info Functions
---------------------

make_core_info(ze_device_handle_t)
..................................

.. doxygenfunction:: tinytc::make_core_info(ze_device_handle_t)

Kernel
======

* Functions

  * :ref:`get_group_count`

  * :ref:`get_group_size(ze_kernel_handle_t)`

  * :ref:`make_kernel(ze_module_handle_t,char const *)`

  * :ref:`make_kernel_bundle(ze_context_handle_t,ze_device_handle_t,binary const&,ze_module_build_log_handle_t*)`

Kernel Functions
----------------

get_group_count
...............

.. doxygenfunction:: tinytc::get_group_count

get_group_size(ze_kernel_handle_t)
..................................

.. doxygenfunction:: tinytc::get_group_size(ze_kernel_handle_t)

make_kernel(ze_module_handle_t,char const *)
............................................

.. doxygenfunction:: tinytc::make_kernel(ze_module_handle_t,char const *)

make_kernel_bundle(ze_context_handle_t,ze_device_handle_t,binary const&,ze_module_build_log_handle_t*)
......................................................................................................

.. doxygenfunction:: tinytc::make_kernel_bundle(ze_context_handle_t,ze_device_handle_t,binary const&,ze_module_build_log_handle_t*)

Recipe
======

* Functions

  * :ref:`make_recipe_handler(ze_context_handle_t,ze_device_handle_t,recipe const&)`

* Classes

  * :ref:`level_zero_recipe_handler`

Recipe Functions
----------------

make_recipe_handler(ze_context_handle_t,ze_device_handle_t,recipe const&)
.........................................................................

.. doxygenfunction:: tinytc::make_recipe_handler(ze_context_handle_t,ze_device_handle_t,recipe const&)

Recipe Classes
--------------

level_zero_recipe_handler
.........................

.. doxygenclass:: tinytc::level_zero_recipe_handler
