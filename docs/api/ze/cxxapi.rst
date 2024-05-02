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

  * :ref:`compile_to_binary`

  * :ref:`get_group_count`

  * :ref:`get_group_size(ze_kernel_handle_t)`

  * :ref:`make_kernel(ze_module_handle_t,char const *)`

  * :ref:`make_kernel_bundle(ze_context_handle_t,ze_device_handle_t,binary const&,source_context)`

  * :ref:`make_kernel_bundle(ze_context_handle_t,ze_device_handle_t,prog,tinytc_core_feature_flags_t,source_context)`

  * :ref:`make_kernel_bundle(ze_context_handle_t,ze_device_handle_t,source const&,source_context)`

Kernel Functions
----------------

compile_to_binary
.................

.. doxygenfunction:: tinytc::compile_to_binary

get_group_count
...............

.. doxygenfunction:: tinytc::get_group_count

get_group_size(ze_kernel_handle_t)
..................................

.. doxygenfunction:: tinytc::get_group_size(ze_kernel_handle_t)

make_kernel(ze_module_handle_t,char const *)
............................................

.. doxygenfunction:: tinytc::make_kernel(ze_module_handle_t,char const *)

make_kernel_bundle(ze_context_handle_t,ze_device_handle_t,binary const&,source_context)
.......................................................................................

.. doxygenfunction:: tinytc::make_kernel_bundle(ze_context_handle_t,ze_device_handle_t,binary const&,source_context)

make_kernel_bundle(ze_context_handle_t,ze_device_handle_t,prog,tinytc_core_feature_flags_t,source_context)
..........................................................................................................

.. doxygenfunction:: tinytc::make_kernel_bundle(ze_context_handle_t,ze_device_handle_t,prog,tinytc_core_feature_flags_t,source_context)

make_kernel_bundle(ze_context_handle_t,ze_device_handle_t,source const&,source_context)
.......................................................................................

.. doxygenfunction:: tinytc::make_kernel_bundle(ze_context_handle_t,ze_device_handle_t,source const&,source_context)

Recipe
======

* Functions

  * :ref:`make_recipe_handler(ze_context_handle_t,ze_device_handle_t,recipe const&,source_context)`

* Classes

  * :ref:`level_zero_recipe_handler`

Recipe Functions
----------------

make_recipe_handler(ze_context_handle_t,ze_device_handle_t,recipe const&,source_context)
........................................................................................

.. doxygenfunction:: tinytc::make_recipe_handler(ze_context_handle_t,ze_device_handle_t,recipe const&,source_context)

Recipe Classes
--------------

level_zero_recipe_handler
.........................

.. doxygenclass:: tinytc::level_zero_recipe_handler

