.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

=========================
Builder programming guide
=========================

Often, some kind of templating is required.
For example, one might want to create a single and a double-precision variant of the
the same kernel, meaning that one needs to replace every occurence of "f32" with "f64".
To faciliate templating in a programmatic way,
the :ref:`builder API <Builder C-API>` (:ref:`C++ <Builder C++-API>`) offers an alternative
to writing tensor language in textual form.
In the builder API, data type objects, value objects, instruction objects, region objects, and function
objects are stitched together to form an Abstract Syntax Tree.
As such, complex code generation patterns can be realized, e.g. offering different code paths
for specific values of a parameterization.

Consider the following simple copy kernel

.. code-block::

    func @copy(%A: memref<${type}x${M}x${N}>, %B: memref<${type}x${M}x${M}>) {
        %c0 = constant 0.0 : ${type}
        %c1 = constant 1.0 : ${type}
        axpby.n %c1, %A, %c0, %B
    }

In the following example we build the above code programmatically and replace the place-holders (${.})
by actual values:

.. tabs::

    .. tab:: C

       .. code:: C

          int64_t M = ...;
          int64_t N = ...;

          char const *copy_fun_name = "copy";
          size_t num_results;
          size_t num_params;
          tinytc_compiler_context_t ctx;
          tinytc_prog_t program;
          tinytc_type_t void_ty, element_ty, ty;
          tinytc_func_t copy_fun;
          tinytc_region_t copy_body;
          tinytc_inst_t tmp;
          tinytc_value_t params[2];
          tinytc_value_t alpha, beta;

          tinytc_compiler_context_create(&ctx);

          // Create program
          tinytc_prog_create(&program, ctx, NULL);

          // Get types
          tinytc_f32_type_get(&element_ty, ctx);
          int64_t shape[2] = {M, N};
          tinytc_memref_type_get(&ty, element_ty, 2, shape, 0, NULL, tinytc_address_space_global);

          // Get void type
          tinytc_void_type_get(&void_ty, ctx);

          // Create function
          tinytc_type_t param_types[2] = {ty, ty};
          tinytc_func_create(&copy_fun, sizeof(copy_fun_name) - 1, copy_fun_name, 2, param_types, void_ty,
                             NULL);
          tinytc_prog_add_function(program, copy_fun);

          // Get body
          tinytc_func_get_body(copy_fun, &copy_body);
          num_params = 2;
          tinytc_region_get_parameters(copy_body, &num_params, params);

          // Create instructions
          tinytc_constant_inst_create_one(&tmp, element_ty, NULL);
          num_results = 1;
          tinytc_inst_get_values(tmp, &num_results, &alpha);
          tinytc_region_append(copy_body, tmp);

          tinytc_constant_inst_create_zero(&tmp, element_ty, NULL);
          num_results = 1;
          tinytc_inst_get_values(tmp, &num_results, &beta);
          tinytc_region_append(copy_body, tmp);

          tinytc_axpby_inst_create(&tmp, 0, tinytc_transpose_N, alpha, params[0], beta, params[1], NULL);
          tinytc_region_append(copy_body, tmp);

          // Dump program
          tinytc_prog_dump(program);

          // Clean-up
          tinytc_prog_release(program);
          tinytc_compiler_context_release(ctx);

    .. tab:: C++

       .. code:: C++

          int64_t M = ...;
          int64_t N = ...;

          auto ctx = make_compiler_context();
          auto element_ty = get<f32_type>(ctx.get());
          auto ty = get<memref_type>(element_ty, array_view{M, N}, array_view<std::int64_t>{},
                                     address_space::global);

          auto void_ty = get<void_type>(ctx.get());
          auto f = make_func("copy", {ty, ty}, void_ty);

          auto body = get_body(f);
          std::array<tinytc_value_t, 2u> params;
          get_parameters(body, params);

          auto bb = region_builder{body};
          auto alpha = bb.constant_one(element_ty);
          auto beta = bb.constant_zero(element_ty);
          bb.create<axpby_inst>(false, transpose::N, alpha, params[0], beta, params[1]);

          auto p = make_prog(ctx);
          add_function(p, std::move(f));

          dump(p);
