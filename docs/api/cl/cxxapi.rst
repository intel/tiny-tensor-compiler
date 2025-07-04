.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

.. _OpenCL C++-API:

=======
C++-API
=======

Common
======

* Functions

  * :ref:`tinytc::CL_CHECK_STATUS`

Common Functions
----------------

.. _tinytc::CL_CHECK_STATUS:

CL_CHECK_STATUS
...............

.. doxygenfunction:: tinytc::CL_CHECK_STATUS

Device Info
===========

* Functions

  * :ref:`tinytc::get_support_level(cl_device_id)`

  * :ref:`tinytc::make_core_info(cl_device_id)`

Device Info Functions
---------------------

.. _tinytc::get_support_level(cl_device_id):

get_support_level(cl_device_id)
...............................

.. doxygenfunction:: tinytc::get_support_level(cl_device_id)

.. _tinytc::make_core_info(cl_device_id):

make_core_info(cl_device_id)
............................

.. doxygenfunction:: tinytc::make_core_info(cl_device_id)

Kernel
======

* Functions

  * :ref:`tinytc::get_global_size(std::array\<std::size_t,3u\> const &,std::array\<std::size_t,3u\> const &)`

  * :ref:`tinytc::get_group_size(cl_kernel)`

  * :ref:`tinytc::make_kernel(cl_program,char const\*)`

  * :ref:`tinytc::make_kernel_bundle(cl_context,cl_device_id,const_tinytc_binary_t)`

  * :ref:`tinytc::make_kernel_bundle(cl_context,cl_device_id,tinytc_prog_t,tinytc_core_feature_flags_t)`

Kernel Functions
----------------

.. _tinytc::get_global_size(std::array\<std::size_t,3u\> const &,std::array\<std::size_t,3u\> const &):

get_global_size(std::array<std::size_t,3u> const &,std::array<std::size_t,3u> const &)
......................................................................................

.. doxygenfunction:: tinytc::get_global_size(std::array<std::size_t,3u> const &,std::array<std::size_t,3u> const &)

.. _tinytc::get_group_size(cl_kernel):

get_group_size(cl_kernel)
.........................

.. doxygenfunction:: tinytc::get_group_size(cl_kernel)

.. _tinytc::make_kernel(cl_program,char const\*):

make_kernel(cl_program,char const\*)
....................................

.. doxygenfunction:: tinytc::make_kernel(cl_program,char const*)

.. _tinytc::make_kernel_bundle(cl_context,cl_device_id,const_tinytc_binary_t):

make_kernel_bundle(cl_context,cl_device_id,const_tinytc_binary_t)
.................................................................

.. doxygenfunction:: tinytc::make_kernel_bundle(cl_context,cl_device_id,const_tinytc_binary_t)

.. _tinytc::make_kernel_bundle(cl_context,cl_device_id,tinytc_prog_t,tinytc_core_feature_flags_t):

make_kernel_bundle(cl_context,cl_device_id,tinytc_prog_t,tinytc_core_feature_flags_t)
.....................................................................................

.. doxygenfunction:: tinytc::make_kernel_bundle(cl_context,cl_device_id,tinytc_prog_t,tinytc_core_feature_flags_t)

Recipe
======

* Functions

  * :ref:`tinytc::make_recipe_handler(cl_context,cl_device_id,tinytc_recipe_t)`

  * :ref:`tinytc::submit(tinytc_recipe_handler_t,cl_command_queue,uint32_t,cl_event\*)`

  * :ref:`tinytc::submit_no_event`

* Structures

  * :ref:`tinytc::auto_mem_type\< cl_mem \>`

Recipe Functions
----------------

.. _tinytc::make_recipe_handler(cl_context,cl_device_id,tinytc_recipe_t):

make_recipe_handler(cl_context,cl_device_id,tinytc_recipe_t)
............................................................

.. doxygenfunction:: tinytc::make_recipe_handler(cl_context,cl_device_id,tinytc_recipe_t)

.. _tinytc::submit(tinytc_recipe_handler_t,cl_command_queue,uint32_t,cl_event\*):

submit(tinytc_recipe_handler_t,cl_command_queue,uint32_t,cl_event\*)
....................................................................

.. doxygenfunction:: tinytc::submit(tinytc_recipe_handler_t,cl_command_queue,uint32_t,cl_event*)

.. _tinytc::submit_no_event:

submit_no_event
...............

.. doxygenfunction:: tinytc::submit_no_event

Recipe Structures
-----------------

.. _tinytc::auto_mem_type\< cl_mem \>:

auto_mem_type<cl_mem>
.....................

.. doxygenstruct:: tinytc::auto_mem_type< cl_mem >

