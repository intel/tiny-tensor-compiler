.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

======================
Core programming guide
======================

Memory management
=================

Objects are either shared, or unique, or managed.
We detail the memory policies in the following.

Shared objects
--------------

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

Unique objects
--------------

Unique objects always have a single owner. In the C-API, when the object is passed to a function, the ownership
may be passed to another object when the function's documentation says so.
For example, when adding an instruction to a region, the region takes ownership of the instruction and the user must not 
destroy the instruction as that would lead to a double free.
In C++, the copy constructor is deleted and unique objects must be moved when a function transfers ownership.

.. tabs::

    .. tab:: C

       An object is created and deleted with

       * tinytc\_\ *type*\ _create

       * tinytc\_\ *type*\ _destroy

    .. tab:: C++

       C handles are wrapped in the :ref:`tinytc::unique_handle` class.
       The wrapper deletes the copy constructor and copy operator, and implements the move constructor and move operator.
       Objects of *type* are created using the make\_\ *type* functions.

Managed ojects
--------------

Some objects are never created or deleted but looked up in a parent object.
In that case the user never needs to destroy the object but only the parent object.
Care must be taken that the parent object is not deleted while the managed object is still in use.

.. tabs::

    .. tab:: C

       The object is obtained with

       * tinytc\_\ *type*\ _get

    .. tab:: C++

       The object is obtained with

       * tinytc::get\_\ *type*

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
The :ref:`tinytc_compiler_context_t` (:ref:`tinytc::compiler_context`) object controls optimization level, optimization flags,
and error logging. (The default compiler context does not print or log errors.)
When an error reporter is installed via :ref:`tinytc_compiler_context_set_error_reporter`,
then errors are printed along with source code locations and source context.
For example:

.. code-block::

   test/lit/opt/check-ir/type_mismatch0.ir:6.8-23: Type of operand must match return type

   func @kernel(%K0: memref<f32>) {

     %0 = load %K0[] : f64
          ~~~~~~~~~~~~~~~~
   test/lit/opt/check-ir/type_mismatch0.ir:5.14-16: value defined here

   func @kernel(%K0: memref<f32>) {
                ~~~


.. tabs::

    .. tab:: C

       Example:

       .. code:: C

          tinytc_status_t status;
          tinytc_compiler_context_t ctx = NULL;
          tinytc_prog_t program = NULL;
          status = tinytc_compiler_context_create(&ctx);
          // ... check status ...
          status = tinytc_compiler_context_set_error_reporter(ctx, error_callback, NULL);
          // ... check status ...
          status = tinytc_parse_file(&program, "test/lit/opt/check-ir/type_mismatch0.ir", ctx)
          if (status != tinytc_status_success) {
              printf("Error: %d\n", status);
          }
          // ...
          err:
          tinytc_prog_release(program);
          tinytc_compiler_context_release(ctx);

    .. tab:: C++

       Example:

       .. code:: C++

          try {
              auto ctx = tinytc::make_compiler_context();
              ctx.set_error_reporter([](char const *what, const tinytc_location_t *,
                                        void *) { std::cerr << what << std::endl; },
                                     nullptr);
              auto program = tinytc::parse_file("test/lit/opt/check-ir/type_mismatch0.ir", ctx);
          } catch (tinytc::status const& st) {
              std::cerr << "Error: " << tinytc::error_string(st) << std::endl;
          } catch (std::exception const &e) {
              std::cerr << e.what() << std::endl;
          }

Compiler
========

Program objects (:ref:`tinytc_prog_t`, :ref:`tinytc::prog`) are online-compiled
using the :ref:`tinytc_prog_compile_to_spirv_and_assemble` (:ref:`tinytc::compile_to_spirv_and_assemble`) function.
The program object is hereby modified as compiler passes are necessary.
A binary object is returned that contains the SPIR-V binary.

Some compiler passes specialize the code based on properties of the GPU device.
Therefore, a :ref:`tinytc_core_info_t` (:ref:`tinytc::core_info`) object is required.
It is recommend to query the core info from the runtime using any of the tinytc\_\ *runtime*\ _core_info_create
functions (make_core_info in C++), but one may also look up the core info from a table,
as done in the example code below.

.. tabs::

    .. tab:: C

       Example:

       .. code:: C

          tinytc_status_t status;
          tinytc_core_info_t info = NULL;
          tinytc_binary_t bin = NULL;
          status = tinytc_core_info_intel_create_from_arch(&info, tinytc_intel_gpu_architecture_pvc);
          // ... check status ...
          status = tinytc_prog_compile_to_spirv_and_assemble(&bin, program, info);
          // ...
          tinytc_binary_release(source);
          tinytc_core_info_release(info);

    .. tab:: C++

       Example:

       .. code:: C++

          try {
              auto info = tinytc::make_core_info_intel_from_arch(tinytc::intel_gpu_architecture::pvc);
              auto source = tinytc::compile_to_spirv_and_assemble(program, info);
          } catch (tinytc::status const& st) {
              ...
          }

.. note::

   Code generation targets SPIR-V.
   As a minimum, the Addresses, SubgroupDispatch, and Int64 capability must be supported by the runtime.


   Further capabilites are required for specific functionality:

   * Int(8|16) for i8, i16 ints
   * Float(16|64) for f16, f64 floats
   * Int64Atomics for atomics on i64
   * Groups for work group operations (e.g. broadcast)
   * AtomicFloat(16|32|64)AddExt for atomics on f16, f32, f64 (SPV_EXT_shader_atomic_float[16]_add extensions)
   * BFloat16ConversionINTEL for bf16 support (SPV_INTEL_bfloat16_conversion extension)
   * SubgroupBufferBlockIOINTEL for efficient block loads and stores (SPV_INTEL_subgroups extension)


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

The JIT compiler compiles tensor programs into SPIR-V binaries.
The libray provides functions to create the runtime's kernel bundle object
(cl_program, sycl::kernel_bundle, ze_module_handle_t) from a binary object.
The runtime's kernel objects are obtained using the native API or the Tiny Tensor Compiler API (if applicable).
Setting the kernel arguments should follow the :ref:`calling convention <calling convention>`.
The Tiny Tensor Compiler should be used to translate the 2D work-group size of the tensor language
to a 3D work-group size, and to translate the group size to the global size that is passed to the runtime.

Example for "func @foo(%a: i32, ...) { ... }" (without error handling code):

.. tabs::

    .. tab:: Level Zero (C)

       .. code:: C

          ze_module_handle_t bundle = NULL;
          ze_kernel_handle_t kernel = NULL;
          int a = 42;
          tinytc_ze_kernel_bundle_create_with_binary(&bundle, context, device, bin);
          tinytc_ze_kernel_create(&kernel, bundle, "foo"); // Sets the work-group size
          zeKernelSetArgumentValue(kernel, 0, sizeof(a), &a);
          // ...
          ze_group_count_t group_count = tinytc_ze_get_group_count(howmany);
          zeCommandListAppendLaunchKernel(command_list, kernel, &group_count, NULL, 0, NULL);
          // ...
          zeKernelDestroy(kernel);
          zeModuleDestroy(bundle);

    .. tab:: OpenCL (C)

       .. code:: C

          cl_program bundle = NULL;
          cl_kernel kernel;
          cl_int err;
          int a = 42;
          tinytc_cl_kernel_bundle_create_with_binary(&bundle, context, device, bin);
          kernel = clCreateKernel(bundle, "foo", &err);
          clSetKernelArg(kernel, 0, sizeof(a), &a);
          // ...
          size_t ls[3], gs[3];
          tinytc_cl_get_group_size(kernel, ls);
          tinytc_cl_get_global_size(howmany, ls, gs);
          clEnqueueNDRangeKernel(command_list, kernel, 3u, NULL, gs, ls, 0, NULL, NULL);
          // ...
          clReleaseKernel(kernel);
          clReleaseProgram(bundle);

    .. tab:: SYCL (C++)

       .. code:: C++

          auto bundle = tinytc::make_kernel_bundle(context, device, bin);
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
          tinytc_recipe_<recipe_name>_create(&recipe, info, <recipe_parameters>, ctx);
          tinytc_ze_recipe_handler_create(&handler, context, device, recipe, ctx);
          tinytc_recipe_<recipe_name>_set_args(handler, <recipe_args>);
          tinytc_ze_recipe_handler_submit(handler, command_list, NULL, 0, NULL);
          // ...
          tinytc_recipe_handler_release(handler);
          tinytc_recipe_release(recipe);

    .. tab:: OpenCL (C)

       .. code:: C

          tinytc_recipe_t recipe = NULL;
          tinytc_recipe_handler_t handler = NULL;
          tinytc_recipe_<recipe_name>_create(&recipe, info, <recipe_parameters>, ctx);
          tinytc_cl_recipe_handler_create(&handler, context, device, recipe, ctx);
          tinytc_recipe_<recipe_name>_set_args(handler, <recipe_args>);
          tinytc_cl_recipe_handler_submit(handler, queue, 0, NULL, NULL);
          // ...
          tinytc_recipe_handler_release(handler);
          tinytc_recipe_release(recipe);

    .. tab:: SYCL (C++)

       .. code:: C++

          auto handler = tinytc::make_recipe_handler(queue,
              tinytc::make_<recipe_name>(info, <recipe_parameters>, ctx), ctx);
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
