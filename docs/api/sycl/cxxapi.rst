.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

.. _SYCL C++-API:

=======
C++-API
=======

Device Info
===========

* Functions

  * :ref:`tinytc::get_support_level(sycl::device const&)`

  * :ref:`tinytc::make_core_info(sycl::device const&)`

Device Info Functions
---------------------

.. _tinytc::get_support_level(sycl::device const&):

get_support_level(sycl::device const&)
......................................

.. doxygenfunction:: tinytc::get_support_level(sycl::device const&)

.. _tinytc::make_core_info(sycl::device const&):

make_core_info(sycl::device const&)
...................................

.. doxygenfunction:: tinytc::make_core_info(sycl::device const&)

Kernel
======

* Functions

  * :ref:`tinytc::get_execution_range`

  * :ref:`tinytc::get_global_size(std::int64_t,sycl::range\<3u\> const &)`

  * :ref:`tinytc::get_group_size(sycl::kernel const &)`

  * :ref:`tinytc::make_kernel(sycl::kernel_bundle\<sycl::bundle_state::executable\> const &,char const \*)`

  * :ref:`tinytc::make_kernel_bundle(sycl::context const &,sycl::device const &,binary const &)`

  * :ref:`tinytc::make_kernel_bundle(sycl::context const &,sycl::device const &,prog,tinytc_core_feature_flags_t)`

Kernel Functions
----------------

.. _tinytc::get_execution_range:

get_execution_range
...................

.. doxygenfunction:: tinytc::get_execution_range

.. _tinytc::get_global_size(std::int64_t,sycl::range\<3u\> const &):

get_global_size(std::int64_t,sycl::range<3u> const &)
.....................................................

.. doxygenfunction:: tinytc::get_global_size(std::int64_t,sycl::range<3u> const &)

.. _tinytc::get_group_size(sycl::kernel const &):

get_group_size(sycl::kernel const &)
....................................

.. doxygenfunction:: tinytc::get_group_size(sycl::kernel const &)

.. _tinytc::make_kernel(sycl::kernel_bundle\<sycl::bundle_state::executable\> const &,char const \*):

make_kernel(sycl::kernel_bundle<sycl::bundle_state::executable> const &,char const \*)
......................................................................................

.. doxygenfunction:: tinytc::make_kernel(sycl::kernel_bundle<sycl::bundle_state::executable> const &,char const *)

.. _tinytc::make_kernel_bundle(sycl::context const &,sycl::device const &,binary const &):

make_kernel_bundle(sycl::context const &,sycl::device const &,binary const &)
.............................................................................

.. doxygenfunction:: tinytc::make_kernel_bundle(sycl::context const &,sycl::device const &,binary const &)

.. _tinytc::make_kernel_bundle(sycl::context const &,sycl::device const &,prog,tinytc_core_feature_flags_t):

make_kernel_bundle(sycl::context const &,sycl::device const &,prog,tinytc_core_feature_flags_t)
...............................................................................................

.. doxygenfunction:: tinytc::make_kernel_bundle(sycl::context const &,sycl::device const &,prog,tinytc_core_feature_flags_t)

Recipe
======

* Functions

  * :ref:`tinytc::make_recipe_handler(sycl::context const &,sycl::device const &,recipe const &)`

  * :ref:`tinytc::make_recipe_handler(sycl::queue const&,recipe const&)`

* Classes

  * :ref:`tinytc::sycl_recipe_handler`

Recipe Functions
----------------

.. _tinytc::make_recipe_handler(sycl::context const &,sycl::device const &,recipe const &):

make_recipe_handler(sycl::context const &,sycl::device const &,recipe const &)
..............................................................................

.. doxygenfunction:: tinytc::make_recipe_handler(sycl::context const &,sycl::device const &,recipe const &)

.. _tinytc::make_recipe_handler(sycl::queue const&,recipe const&):

make_recipe_handler(sycl::queue const&,recipe const&)
.....................................................

.. doxygenfunction:: tinytc::make_recipe_handler(sycl::queue const&,recipe const&)

Recipe Classes
--------------

.. _tinytc::sycl_recipe_handler:

sycl_recipe_handler
...................

.. doxygenclass:: tinytc::sycl_recipe_handler

