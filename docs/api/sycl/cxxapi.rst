.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

=======
C++-API
=======

Device Info
===========

* Functions

  * :ref:`make_core_info(sycl::device const&)`

Device Info Functions
---------------------

make_core_info(sycl::device const&)
...................................

.. doxygenfunction:: tinytc::make_core_info(sycl::device const&)

Kernel
======

* Functions

  * :ref:`get_execution_range`

  * :ref:`get_global_size(std::uint32_t,sycl::range<3u> const &)`

  * :ref:`get_group_size(sycl::kernel const &)`

  * :ref:`make_kernel(sycl::kernel_bundle<sycl::bundle_state::executable> const &,char const *)`

  * :ref:`make_kernel_bundle(sycl::context const &,sycl::device const &,binary const &)`

Kernel Functions
----------------

get_execution_range
...................

.. doxygenfunction:: tinytc::get_execution_range

get_global_size(std::uint32_t,sycl::range<3u> const &)
......................................................

.. doxygenfunction:: tinytc::get_global_size(std::uint32_t,sycl::range<3u> const &)

get_group_size(sycl::kernel const &)
....................................

.. doxygenfunction:: tinytc::get_group_size(sycl::kernel const &)

make_kernel(sycl::kernel_bundle<sycl::bundle_state::executable> const &,char const *)
.....................................................................................

.. doxygenfunction:: tinytc::make_kernel(sycl::kernel_bundle<sycl::bundle_state::executable> const &,char const *)

make_kernel_bundle(sycl::context const &,sycl::device const &,binary const &)
.............................................................................

.. doxygenfunction:: tinytc::make_kernel_bundle(sycl::context const &,sycl::device const &,binary const &)

Recipe
======

* Functions

  * :ref:`make_recipe_handler(sycl::context const &,sycl::device const &,recipe const &)`

  * :ref:`make_recipe_handler(sycl::queue const&,recipe const&)`

* Classes

  * :ref:`sycl_recipe_handler`

Recipe Functions
----------------

make_recipe_handler(sycl::context const &,sycl::device const &,recipe const &)
..............................................................................

.. doxygenfunction:: tinytc::make_recipe_handler(sycl::context const &,sycl::device const &,recipe const &)

make_recipe_handler(sycl::queue const&,recipe const&)
.....................................................

.. doxygenfunction:: tinytc::make_recipe_handler(sycl::queue const&,recipe const&)

Recipe Classes
--------------

sycl_recipe_handler
...................

.. doxygenclass:: tinytc::sycl_recipe_handler

