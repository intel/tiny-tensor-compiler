.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

======================
Core programming guide
======================

Memory
======

Objects are created, retained, and released.
At creation, objects are constructed and the reference count is set to 1.
The retain operation increases the reference count by 1 and the release operation
decreases the reference count by 1.
If the reference count is equal to 0, the object is destroyed and the memory is freed.
Relasing an object is always safe after passing it to a function, because the function either
does not need the object anymore, or it increased the reference count by one.

.. tabs::

    .. tab:: C

       The reference count of an object of *type* is managed with

       * tinytc\_\ *type*\ _create

       * tinytc\_\ *type*\ _retain

       * tinytc\_\ *type*\ _release

    .. tab:: C++

       C handles are wrapped in the :ref:`tinytc::shared_handle` class.
       The wrapper implements copy constructor, move constructor, copy operator, and move operator,
       correctly handling the reference count.
       Objects of *type* are created using the make\_\ *type* functions.

       **Important:** The default constructor of a :ref:`tinytc::shared_handle` or any of its
       derivatives always gives an invalid object, wrapping a nullptr.
       Always use the make\_\ *type* function unless you know what you are doing.

Error
=====

The C-API returns error codes, the C++-API throws exceptions.

.. tabs::

    .. tab:: C

       Cf. :ref:`tinytc_status_t` for a list of error codes.
       Level Zero and OpenCL codes are translated to :ref:`tinytc_status_t`.

    .. tab:: C++

       Functions throw the :ref:`tinytc::status` enum.
       The following minimum error handling code is recommended:

       .. code:: C++

          try {
              ...
          } catch (tinytc::status const& st) {
              std::cerr << static_cast<int>(st) << ": " << tinytc::error_string(st) << std::endl;
          } catch (std::exception const& e) {
              std::cerr << e.what() << std::endl;
          }

       **Hint:** The IR builder API throws the :ref:`tinytc::builder_error`
       (deriving from std::exception) instead of the status enum for better
       source code location tracking.

Parser
======

Programs written in the :ref:`tensor language <tensor language>`
are parsed from a file, stdin, or a string.
A :ref:`tinytc_source_context_t` (:ref:`source_context`) object can be attached any of the parse functions.
The source context stores the file name and source text and enhances error messages with source code context.
For example, if a parse error occurs,
then the error log of the source context contains the following error message:

.. code-block::

   func @kernel(%K0: memref<f32>) {
     %0 = load %K0[] : memref<f64>
               ~~~~~~~~~~~~~~~~~~~
   test/codegen/type_mismatch0.ir:6.13-31: Type of SSA value does not match operand type


.. tabs::

    .. tab:: C

       Example:

       .. code:: C

          tinytc_status_t status;
          tinytc_source_context_t source_ctx = NULL;
          tinytc_prog_t program = NULL;
          status = tinytc_source_context_create(&source_ctx);
          // ... check status ...
          status = tinytc_parse_file(&program, "test/codegen/type_mismatch0.ir"), source_ctx)
          if (status != tinytc_status_success) {
              printf("Error: %d\n", status);
              char const* error_log;
              status = tinytc_source_context_get_error_log(source_ctx, &error_log);
              // ... check status ...
              printf("Error log:\n%s\n", error_log);
          }
          // ...
          tinytc_prog_release(program);
          tinytc_source_context_release(source_ctx);

    .. tab:: C++

       Example:

       .. code:: C++

          try {
              auto source_ctx = tinytc::make_source_context();
              auto program = tinytc::parse_file("test/codegen/type_mismatch0.ir", source_ctx);
          } catch (tinytc::status const& st) {
              std::cerr << "Error: " << tinytc::error_string(st) << std::endl;
              std::cerr << "Error log: " << source_ctx.get_error_log() << std::endl;
          }

Compiler
========

Program objects (:ref:`tinytc_prog_t`, :ref:`tinytc::prog`) are online-compiled
using the :ref:`tinytc_prog_compile_to_opencl` (:ref:`tinytc::compile_to_opencl`) function.
The program object is hereby modified as compiler passes are necessary.
A source object is returned that contains OpenCL-C source text.

Some compiler passes specialize the code based on properties of the GPU device.
Therefore, a :ref:`tinytc_core_info_t` (:ref:`tinytc::core_info`) object is required.
It is recommend to query the core info from the runtime using any of the tinytc\_\ *runtime*\ _core_info_create
functions (make_core_info in C++), but one may also look up the core info from a table,
as done in the example code below.

A source context can be added to capture potential errors in the optimizer.

.. tabs::

    .. tab:: C

       Example:

       .. code:: C

          tinytc_status_t status;
          tinytc_core_info_t info = NULL;
          tinytc_source_t source = NULL;
          status = tinytc_core_info_intel_create_from_arch(&info, tinytc_intel_gpu_architecture_pvc);
          // ... check status ...
          status = tinytc_prog_compile_to_opencl(&source, program, info, source_ctx);
          // ...
          tinytc_source_release(source);
          tinytc_core_info_release(info);

    .. tab:: C++

       Example:

       .. code:: C++

          try {
              auto info = tinytc::make_core_info_intel_from_arch(tinytc::intel_gpu_architecture::pvc);
              auto source = tinytc::compile_to_opencl(program, info, source_ctx);
          } catch (tinytc::status const& st) {
              ...
          }

.. note::

   Code generation targets OpenCL-C. Currently, the library requires the
   `cl_intel_required_subgroup_size <https://registry.khronos.org/OpenCL/extensions/intel/cl_intel_required_subgroup_size.html>`_ extension,
   the `cl_intel_subgroups <https://registry.khronos.org/OpenCL/extensions/intel/cl_intel_subgroups.html>`_ extension,
   the `cl_intel_subgroups_long <https://registry.khronos.org/OpenCL/extensions/intel/cl_intel_subgroups_long.html>`_ extension,
   and the `cl_intel_subgroups_short <https://registry.khronos.org/OpenCL/extensions/intel/cl_intel_subgroups_short.html>`_ extension.

Device info
===========

Kernels are specialized for properties of the target device, such as the subgroup size, the
maximum work group size, and the register space available to a subgroup.
Moreover, the device's :ref:`support level <tinytc_support_level_t>` can be queried from the
run-time.

.. tabs::

    .. tab:: Level Zero (C)

       .. code:: C

          tinytc_support_level_t level;
          tinytc_ze_get_support_level(device, &level);
          if (level >= tinytc_support_level_basic) {
              tinytc_core_info_t info;
              tinytc_ze_core_info_create(&info, device);
              // ...
              tinytc_core_info_release(info);
          }

    .. tab:: OpenCL (C)

       .. code:: C

          tinytc_support_level_t level;
          tinytc_cl_get_support_level(device, &level);
          if (level >= tinytc_support_level_basic) {
              tinytc_core_info_t info;
              tinytc_cl_core_info_create(&info, device);
              // ...
              tinytc_core_info_release(info);
          }

    .. tab:: SYCL (C++)

       .. code:: C++

          if (tinytc::get_support_level(device) >= tinytc::support_level::basic) {
              auto info = tinytc::make_core_info(device);
              // ...
          }

Runtime
=======

The JIT compiler compiles tensor programs into OpenCL-C code.
The libray provides functions to create the runtime's kernel bundle object
(cl_program, sycl::kernel_bundle, ze_module_handle_t) from a source object.
The runtime's kernel objects are obtained using the native API or the Tiny Tensor Compiler API (if applicable).
Setting the kernel arguments should following the :ref:`calling convention <calling convention>`.
The Tiny Tensor Compiler should be used to translate the 2D work-group size of the tensor language
to a 3D work-group size, and to translate the group size to the global size that is passed to the runtime.

Example for "func @foo(%a: i32, ...) { ... }" (without error handling code):

.. tabs::

    .. tab:: Level Zero (C)

       .. code:: C

          ze_module_handle_t module = NULL;
          ze_kernel_handle_t kernel = NULL;
          int a = 42;
          tinytc_ze_kernel_bundle_create_with_source(&module, context, device, source, source_ctx);
          tinytc_ze_kernel_create(&kernel, module, "foo"); // Sets the work-group size
          zeKernelSetArgumentValue(kernel, 0, sizeof(a), &a);
          // ...
          ze_group_count_t group_count = tinytc_ze_get_group_count(howmany);
          zeCommandListAppendLaunchKernel(command_list, kernel, &group_count, NULL, 0, NULL);
          // ...
          zeKernelDestroy(kernel);
          zeModuleDestroy(module);

    .. tab:: OpenCL (C)

       .. code:: C

          cl_program program = NULL;
          cl_kernel kernel;
          cl_int err;
          int a = 42;
          tinytc_cl_kernel_bundle_create_with_source(&program, context, device, binary, source_ctx);
          kernel = clCreateKernel(program, "foo", &err);
          clSetKernelArg(kernel, 0, sizeof(a), &a);
          // ...
          size_t ls[3], gs[3];
          tinytc_cl_get_group_size(kernel, ls);
          tinytc_cl_get_global_size(howmany, ls, gs);
          clEnqueueNDRangeKernel(command_list, kernel, 3u, NULL, gs, ls, 0, NULL, NULL);
          // ...
          clReleaseKernel(kernel);
          clReleaseProgram(program);

    .. tab:: SYCL (C++)

       .. code:: C++

          auto bundle = tinytc::make_kernel_bundle(context, device, source, source_ctx);
          auto kernel = tinytc::make_kernel(bundle, "foo");
          auto exe_range = tinytc::get_execution_range(kernel, howmany);
          queue.submit([&](sycl::handler &h) {
              h.set_args(42, ...);
              h.parallel_for(exe_range, kernel);
          });

.. note::

   Kernel bundles can also be created from program objects directly, e.g. with 
   :ref:`tinytc_cl_kernel_bundle_create_with_program` or :ref:`tinytc_ze_kernel_bundle_create_with_program`.


Recipe
======

Recipes provide a code generator for common applications.
Their usage is quite simple in comparison, as writing the code, parsing, and compiling
are all encapsulated in the recipe.

Recipes are submitted to the runtime using a recipe handler.
The general usage of a recipe is as following:

.. tabs::

    .. tab:: Level Zero (C)

       .. code:: C

          tinytc_recipe_t recipe = NULL;
          tinytc_recipe_handler_t handler = NULL;
          tinytc_recipe_<recipe_name>_create(&recipe, info, <recipe_parameters>, source_ctx);
          tinytc_ze_recipe_handler_create(&handler, context, device, recipe, source_ctx);
          tinytc_recipe_<recipe_name>_set_args(handler, <recipe_args>);
          tinytc_ze_recipe_handler_submit(handler, command_list, NULL, 0, NULL);
          // ...
          tinytc_recipe_handler_release(handler);
          tinytc_recipe_release(recipe);

    .. tab:: OpenCL (C)

       .. code:: C

          tinytc_recipe_t recipe = NULL;
          tinytc_recipe_handler_t handler = NULL;
          tinytc_recipe_<recipe_name>_create(&recipe, info, <recipe_parameters>, source_ctx);
          tinytc_cl_recipe_handler_create(&handler, context, device, recipe, source_ctx);
          tinytc_recipe_<recipe_name>_set_args(handler, <recipe_args>);
          tinytc_cl_recipe_handler_submit(handler, queue, 0, NULL, NULL);
          // ...
          tinytc_recipe_handler_release(handler);
          tinytc_recipe_release(recipe);

    .. tab:: SYCL (C++)

       .. code:: C++

          auto handler = tinytc::make_recipe_handler(queue,
              tinytc::make_<recipe_name>(info, <recipe_parameters>, source_ctx), source_ctx);
          <recipe_name>::set_args(handler, <recipe_args>);
          handler.submit(queue);

Memory objects are either buffers (e.g. cl_mem in OpenCL) or Unified Shared Memory pointers
or Shared Virtual Memory pointers.
The unified interface requires the memory object to be given as void-pointer, annotated with
:ref:`tinytc_mem_type_t`.
For example:

.. code:: C

   // OpenCL
   cl_mem A = ...;
   tinytc_recipe_<recipe_name>_set_args(..., A, tinytc_mem_type_buffer, ...);
   
   // Level Zero
   void* A = ...;
   tinytc_recipe_<recipe_name>_set_args(..., A, tinytc_mem_type_usm_pointer, ...);

In C++, one only needs to pass the memory object.
The memory object is implicitly converted to the :ref:`tinytc::mem` type that
automatically determines whether a pointer or a cl_mem object is given.
A pointer maps to tinytc_mem_type_usm_pointer and a cl_mem object maps
to tinytc_mem_type_buffer.
For SVM pointers, one needs to explicitly call `mem(pointer, tinytc_mem_type_svm_pointer)`.



Batched small GEMM
------------------

The batched small GEMM recipe implements the following tensor operation:

.. math::

    C_i = \alpha \text{op}_A(A_i) \text{op}_B(B_i) + \beta C_i

where
:math:`\text{op}_A(A_i) \in \mathbb{R}^{M\times K}`,
:math:`\text{op}_B(B_i) \in \mathbb{R}^{K\times N}`,
:math:`C_i \in \mathbb{R}^{M\times N}`,
:math:`i` is the group id,
and

.. math::

   \text{op}_{X}(Y) = \left\{\begin{array}{rcl}
                        Y^T & \text{if} & t_X = T, \\
                        Y & \text{if} & t_X = N.
                      \end{array}\right.

The matrices in a matrix batch are separated by a fixed stride, that is,
the address is computed as following for a matrix batch X:

.. code-block:: cpp

    X[m + n * ldX + i * strideX] // accesses X_i(m,n)


Tall and skinny GEMM
--------------------

The tall and skinny GEMM recipe implements the following tensor operation:

.. math::

    C = \alpha AB + \beta C

where
:math:`A \in \mathbb{R}^{M\times K}`,
:math:`B \in \mathbb{R}^{K\times N}`,
:math:`C \in \mathbb{R}^{M\times N}`,
and :math:`M \gg K`, :math:`M \gg N`.
