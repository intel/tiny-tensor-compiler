.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

===============
Work-group size
===============

Instructions like gemm, axpby, ger, etc., are processed by a work-group collectively.
Choosing the work-group size appropriately is critical for performance.
If the work_group_size and subgroup_size attribute are omitted, heuristics are used.

These work-group size heuristics can also be called manually.
For example, if a kernel takes a memref of dynamic size, then the memref shapes are unknown at compile-time,
and the automatic application of the heuristics is only giving a general-purpose work-group size.
Therefore using the heuristics API, outlined below, one can supply memref shape constraints, like a maximum or
typical shape.

Functions
=========

.. doxygenfunction:: tinytc::ir::suggest_subgroup_size
.. doxygenfunction:: tinytc::ir::suggest_local_tiling(blas_shape const&,core_config const&)
.. doxygenfunction:: tinytc::ir::suggest_local_tiling(std::vector<blas_shape> const&,core_config const&)
.. doxygenfunction:: tinytc::ir::suggest_subgroup_size_and_tiling

Classes
=======

.. doxygenclass:: tinytc::ir::local_tiling
   :members:
.. doxygenstruct:: tinytc::ir::blas_shape
   :members:

