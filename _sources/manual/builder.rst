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
        axpby.n 1.0, %A, 0.0, %B : ${type}, memref<${type}x${M}x${N}>, ${type}, memref<${type}x${M}x${N}>
    }

In the following example we build the above code programmatically and replace the place-holders (${.})
by actual values:

.. tabs::

    .. tab:: C

       .. code:: C

          tinytc_scalar_type_t type = ...;
          int64_t M = ...;
          int64_t N = ...;

          tinytc_data_type_t dt;
          int64_t shape[2] = {M, N};
          tinytc_memref_type_create(&dt, type, 2, shape, 0, NULL, NULL);

          tinytc_value_t A, B, alpha, beta;
          tinytc_value_create(&A, dt, NULL);
          tinytc_value_create(&B, dt, NULL);
          tinytc_float_imm_create(&alpha, 1.0, type, NULL);
          tinytc_float_imm_create(&beta, 0.0, type, NULL);
          tinytc_data_type_release(dt);

          tinytc_inst_t copy_inst;
          tinytc_axpby_inst_create(&copy_inst, tinytc_transpose_N, 0, alpha, A, beta, B, NULL);
          tinytc_value_release(alpha);
          tinytc_value_release(beta);

          tinytc_func_t copy_proto;
          tinytc_value_t args[2] = {A, B};
          tinytc_function_prototype_create(&copy_proto, "copy", 2, args, NULL);
          tinytc_value_release(A);
          tinytc_value_release(B);

          tinytc_region_t copy_body;
          tinytc_region_create(&copy_body, 1, &copy_inst, NULL);
          tinytc_inst_release(copy_inst);

          tinytc_func_t copy_fun;
          tinytc_function_create(&copy_fun, copy_proto, copy_body, NULL);
          tinytc_func_release(copy_proto);
          tinytc_region_release(copy_body);

          tinytc_prog_t program;
          tinytc_program_create(&program, 1, &copy_fun, NULL);
          tinytc_func_release(copy_fun);

    .. tab:: C++

       .. code:: C++

          scalar_type type = ...;
          int64_t M = ...;
          int64_t N = ...;

          auto pb = program_builder{};
          pb.create("copy", [&](function_builder &fb) {
              auto dt = make_memref(type, {M, N});
              auto A = fb.argument(dt);
              auto B = fb.argument(dt);
              fb.body([&](region_builder &bb) {
                  auto alpha = make_imm(1.0, type);
                  auto beta = make_imm(0.0, type);
                  bb.add(make_axpby(transpose::N, false, alpha, A, beta, B));
              });
          });
          auto program = pb.get_product();

