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

          tinytc_scalar_type_t sty = ...;
          int64_t M = ...;
          int64_t N = ...;

          char const *copy_fun_name = "copy";
          uint32_t num_results;
          uint32_t num_params;
          tinytc_compiler_context_t ctx;
          tinytc_prog_t program;
          tinytc_data_type_t element_ty, ty;
          tinytc_func_t copy_fun;
          tinytc_region_t copy_body;
          tinytc_inst_t tmp;
          tinytc_value_t params[2];
          tinytc_value_t alpha, beta;

          tinytc_compiler_context_create(&ctx);

          // Create program
          tinytc_prog_create(&program, ctx, NULL);

          // Get types
          tinytc_scalar_type_get(&element_ty, ctx, sty);
          int64_t shape[2] = {M, N};
          tinytc_memref_type_get(&ty, element_ty, 2, shape, 0, NULL, tinytc_address_space_global, NULL);

          // Create function
          tinytc_data_type_t param_types[2] = {ty, ty};
          tinytc_func_create(&copy_fun, sizeof(copy_fun_name) - 1, copy_fun_name, 2, param_types, NULL);
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

          tinytc_axpby_inst_create(&tmp, tinytc_transpose_N, 0, alpha, params[0], beta, params[1], NULL);
          tinytc_region_append(copy_body, tmp);

          // Dump program
          tinytc_prog_dump(program);

          // Clean-up
          tinytc_prog_release(program);
          tinytc_compiler_context_release(ctx);

    .. tab:: C++

       .. code:: C++

          scalar_type sty = ...;
          int64_t M = ...;
          int64_t N = ...;

          auto ctx = make_compiler_context();
          auto element_ty = get_scalar(ctx, sty);
          auto ty = get_memref(element_ty, {M, N});

          auto f = make_func("copy", {ty, ty});

          auto body = f.get_body();
          std::array<value, 2u> params;
          body.get_parameters(params);

          auto bb = region_builder{body};
          auto alpha = bb.add(make_constant_one(element_ty));
          auto beta = bb.add(make_constant_zero(element_ty));
          bb.add(make_axpby(transpose::N, false, alpha, params[0], beta, params[1]));

          auto p = make_prog(ctx);
          p.add_function(std::move(f));

          p.dump();
