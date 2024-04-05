.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

============
JIT compiler
============

Tensor programs may be compiled at run-time using the following snippet:

.. code-block:: cpp

   #include <tinytc/tinytc.hpp>

   // Create source manager and get core info
   auto srcman = tinytc::source_manager(&std::cerr);
   auto info = tinytc::get_core_info_intel_gpu(tinytc::intel_gpu_architecture::pvc);
   // Parse file
   auto prog = srcman.parse_file(argv[1]);
   if (!prog) { // nullptr returned on error
       return -1;
   }
   // JIT compile program
   auto bin = tinytc::optimize_and_make_binary(std::move(prog), tinytc::bundle_format::spirv,
                                               std::move(info), srcman.error_reporter());
   if (!bin) { // nullptr returned on error
       return -1;
   }

The call to *optimize_and_make_binary* first runs compiler passes on *prog*, that is, *prog* is modified.
It then returns a *shared_ptr* to a *binary* object.
The *binary* object contains either SPIR-V or a native device binary.

Some compiler passes depend on the properties of the GPU device.
Therefore, the *core_info* object is required.
A *core_info* is obtained from an architecture look-up table by calling *get_core_info_intel_gpu*
in the above example.
As one typically combines a GPU run-time with JIT compilation, it is recommended to call 
**get_core_info** to obtain the *core_info* (available for Level Zero, OpenCL, and SYCL).

.. note::

   Code generation targets OpenCL-C. Currently, the library requires the
   `cl_intel_required_subgroup_size <https://registry.khronos.org/OpenCL/extensions/intel/cl_intel_required_subgroup_size.html>`_ extension,
   the `cl_intel_subgroups <https://registry.khronos.org/OpenCL/extensions/intel/cl_intel_subgroups.html>`_ extension,
   the `cl_intel_subgroups_long <https://registry.khronos.org/OpenCL/extensions/intel/cl_intel_subgroups_long.html>`_ extension,
   and the `cl_intel_subgroups_short <https://registry.khronos.org/OpenCL/extensions/intel/cl_intel_subgroups_short.html>`_ extension.

Enumerations
============

.. doxygenenum:: tinytc::bundle_format
.. doxygenenum:: tinytc::core_feature_flag
.. doxygenenum:: tinytc::intel_gpu_architecture

Functions
=========

.. doxygenfunction:: tinytc::get_core_info_intel_gpu
.. doxygenfunction:: tinytc::optimize_and_make_binary
.. doxygenfunction:: tinytc::compile_opencl_c

Classes
=======

.. doxygenclass:: tinytc::core_config
.. doxygenclass:: tinytc::core_info
.. doxygenclass:: tinytc::core_info_intel
.. doxygenclass:: tinytc::binary
.. doxygenstruct:: tinytc::kernel_metadata
