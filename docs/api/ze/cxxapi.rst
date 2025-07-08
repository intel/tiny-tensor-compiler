.. Copyright (C) 2025 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

.. _Level Zero C++-API:

=======
C++-API
=======

Common
======

* Functions

  * :ref:`tinytc::ZE_CHECK_STATUS`

Common Functions
----------------

.. _tinytc::ZE_CHECK_STATUS:

ZE_CHECK_STATUS
...............

.. doxygenfunction:: tinytc::ZE_CHECK_STATUS

Device Info
===========

* Functions

  * :ref:`tinytc::get_support_level(ze_device_handle_t)`

  * :ref:`tinytc::create_core_info(ze_device_handle_t)`

Device Info Functions
---------------------

.. _tinytc::get_support_level(ze_device_handle_t):

get_support_level(ze_device_handle_t)
.....................................

.. doxygenfunction:: tinytc::get_support_level(ze_device_handle_t)

.. _tinytc::create_core_info(ze_device_handle_t):

create_core_info(ze_device_handle_t)
....................................

.. doxygenfunction:: tinytc::create_core_info(ze_device_handle_t)

Kernel
======

* Functions

  * :ref:`tinytc::get_group_size(ze_kernel_handle_t)`

  * :ref:`tinytc::create_kernel(ze_module_handle_t,char const \*)`

  * :ref:`tinytc::create_kernel_bundle(ze_context_handle_t,ze_device_handle_t,const_tinytc_binary_t)`

  * :ref:`tinytc::create_kernel_bundle(ze_context_handle_t,ze_device_handle_t,tinytc_prog_t,tinytc_core_feature_flags_t)`

Kernel Functions
----------------

.. _tinytc::get_group_size(ze_kernel_handle_t):

get_group_size(ze_kernel_handle_t)
..................................

.. doxygenfunction:: tinytc::get_group_size(ze_kernel_handle_t)

.. _tinytc::create_kernel(ze_module_handle_t,char const \*):

create_kernel(ze_module_handle_t,char const \*)
...............................................

.. doxygenfunction:: tinytc::create_kernel(ze_module_handle_t,char const *)

.. _tinytc::create_kernel_bundle(ze_context_handle_t,ze_device_handle_t,const_tinytc_binary_t):

create_kernel_bundle(ze_context_handle_t,ze_device_handle_t,const_tinytc_binary_t)
..................................................................................

.. doxygenfunction:: tinytc::create_kernel_bundle(ze_context_handle_t,ze_device_handle_t,const_tinytc_binary_t)

.. _tinytc::create_kernel_bundle(ze_context_handle_t,ze_device_handle_t,tinytc_prog_t,tinytc_core_feature_flags_t):

create_kernel_bundle(ze_context_handle_t,ze_device_handle_t,tinytc_prog_t,tinytc_core_feature_flags_t)
......................................................................................................

.. doxygenfunction:: tinytc::create_kernel_bundle(ze_context_handle_t,ze_device_handle_t,tinytc_prog_t,tinytc_core_feature_flags_t)

Recipe
======

* Functions

  * :ref:`tinytc::create_recipe_handler(ze_context_handle_t,ze_device_handle_t,tinytc_recipe_t)`

  * :ref:`tinytc::submit(tinytc_recipe_handler_t,ze_command_list_handle_t,ze_event_handle_t,uint32_t,ze_event_handle_t\*)`

Recipe Functions
----------------

.. _tinytc::create_recipe_handler(ze_context_handle_t,ze_device_handle_t,tinytc_recipe_t):

create_recipe_handler(ze_context_handle_t,ze_device_handle_t,tinytc_recipe_t)
.............................................................................

.. doxygenfunction:: tinytc::create_recipe_handler(ze_context_handle_t,ze_device_handle_t,tinytc_recipe_t)

.. _tinytc::submit(tinytc_recipe_handler_t,ze_command_list_handle_t,ze_event_handle_t,uint32_t,ze_event_handle_t\*):

submit(tinytc_recipe_handler_t,ze_command_list_handle_t,ze_event_handle_t,uint32_t,ze_event_handle_t\*)
.......................................................................................................

.. doxygenfunction:: tinytc::submit(tinytc_recipe_handler_t,ze_command_list_handle_t,ze_event_handle_t,uint32_t,ze_event_handle_t*)

