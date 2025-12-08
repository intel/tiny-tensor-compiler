.. Copyright (C) 2025 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

.. _SYCL C++-API:

=======
C++-API
=======

Device Info
===========

* Functions

  * :ref:`tinytc::get_support_level(sycl::device const&)`

  * :ref:`tinytc::create_core_info(sycl::device const&)`

Device Info Functions
---------------------

.. _tinytc::get_support_level(sycl::device const&):

get_support_level(sycl::device const&)
......................................

.. doxygenfunction:: tinytc::get_support_level(sycl::device const&)

.. _tinytc::create_core_info(sycl::device const&):

create_core_info(sycl::device const&)
.....................................

.. doxygenfunction:: tinytc::create_core_info(sycl::device const&)

Kernel
======

* Functions

  * :ref:`tinytc::get_execution_range`

  * :ref:`tinytc::get_global_size(sycl::range\<3u\> const &,sycl::range\<3u\> const &)`

  * :ref:`tinytc::get_group_size(sycl::kernel const &)`

  * :ref:`tinytc::create_kernel(sycl::kernel_bundle\<sycl::bundle_state::executable\> const &,char const \*)`

  * :ref:`tinytc::create_kernel_bundle(sycl::context const &,sycl::device const &,const_tinytc_binary_t)`

  * :ref:`tinytc::create_kernel_bundle(sycl::context const &,sycl::device const &,tinytc_prog_t,tinytc_core_feature_flags_t)`

Kernel Functions
----------------

.. _tinytc::get_execution_range:

get_execution_range
...................

.. doxygenfunction:: tinytc::get_execution_range

.. _tinytc::get_global_size(sycl::range\<3u\> const &,sycl::range\<3u\> const &):

get_global_size(sycl::range<3u> const &,sycl::range<3u> const &)
................................................................

.. doxygenfunction:: tinytc::get_global_size(sycl::range<3u> const &,sycl::range<3u> const &)

.. _tinytc::get_group_size(sycl::kernel const &):

get_group_size(sycl::kernel const &)
....................................

.. doxygenfunction:: tinytc::get_group_size(sycl::kernel const &)

.. _tinytc::create_kernel(sycl::kernel_bundle\<sycl::bundle_state::executable\> const &,char const \*):

create_kernel(sycl::kernel_bundle<sycl::bundle_state::executable> const &,char const \*)
........................................................................................

.. doxygenfunction:: tinytc::create_kernel(sycl::kernel_bundle<sycl::bundle_state::executable> const &,char const *)

.. _tinytc::create_kernel_bundle(sycl::context const &,sycl::device const &,const_tinytc_binary_t):

create_kernel_bundle(sycl::context const &,sycl::device const &,const_tinytc_binary_t)
......................................................................................

.. doxygenfunction:: tinytc::create_kernel_bundle(sycl::context const &,sycl::device const &,const_tinytc_binary_t)

.. _tinytc::create_kernel_bundle(sycl::context const &,sycl::device const &,tinytc_prog_t,tinytc_core_feature_flags_t):

create_kernel_bundle(sycl::context const &,sycl::device const &,tinytc_prog_t,tinytc_core_feature_flags_t)
..........................................................................................................

.. doxygenfunction:: tinytc::create_kernel_bundle(sycl::context const &,sycl::device const &,tinytc_prog_t,tinytc_core_feature_flags_t)

Recipe
======

* Functions

  * :ref:`tinytc::create_recipe_handler(sycl::context const &,sycl::device const &,tinytc_recipe_t)`

  * :ref:`tinytc::create_recipe_handler(sycl::queue const&,tinytc_recipe_t)`

  * :ref:`tinytc::parallel_for`

  * :ref:`tinytc::submit(tinytc_recipe_handler_t,sycl::queue)`

  * :ref:`tinytc::submit(tinytc_recipe_handler_t,sycl::queue,std::vector\<sycl::event\> const &)`

  * :ref:`tinytc::submit(tinytc_recipe_handler_t,sycl::queue,sycl::event const &)`

Recipe Functions
----------------

.. _tinytc::create_recipe_handler(sycl::context const &,sycl::device const &,tinytc_recipe_t):

create_recipe_handler(sycl::context const &,sycl::device const &,tinytc_recipe_t)
.................................................................................

.. doxygenfunction:: tinytc::create_recipe_handler(sycl::context const &,sycl::device const &,tinytc_recipe_t)

.. _tinytc::create_recipe_handler(sycl::queue const&,tinytc_recipe_t):

create_recipe_handler(sycl::queue const&,tinytc_recipe_t)
.........................................................

.. doxygenfunction:: tinytc::create_recipe_handler(sycl::queue const&,tinytc_recipe_t)

.. _tinytc::parallel_for:

parallel_for
............

.. doxygenfunction:: tinytc::parallel_for

.. _tinytc::submit(tinytc_recipe_handler_t,sycl::queue):

submit(tinytc_recipe_handler_t,sycl::queue)
...........................................

.. doxygenfunction:: tinytc::submit(tinytc_recipe_handler_t,sycl::queue)

.. _tinytc::submit(tinytc_recipe_handler_t,sycl::queue,std::vector\<sycl::event\> const &):

submit(tinytc_recipe_handler_t,sycl::queue,std::vector<sycl::event> const &)
............................................................................

.. doxygenfunction:: tinytc::submit(tinytc_recipe_handler_t,sycl::queue,std::vector<sycl::event> const &)

.. _tinytc::submit(tinytc_recipe_handler_t,sycl::queue,sycl::event const &):

submit(tinytc_recipe_handler_t,sycl::queue,sycl::event const &)
...............................................................

.. doxygenfunction:: tinytc::submit(tinytc_recipe_handler_t,sycl::queue,sycl::event const &)

