.. Copyright (C) 2024 Intel Corporation
   SPDX-License-Identifier: BSD-3-Clause

=======
Runtime
=======

The JIT compiler compiles tensor programs into a binary kernel bundle (SPIR-V or native device binary).
The runtime abstraction is a convenient method to launch a kernel included in a binary
on a device, because it maps the tensor program's work-group execution model to the runtime's
execution model.

The classes :cpp:class:`tinytc::tensor_kernel_bundle` and :cpp:class:`tinytc::tensor_kernel`
are abstractions for the native kernel bundle type and kernel type, respectively.
(We adopt the SYCL terminology "kernel bundle" here. A kernel bundle is called a "program"
in OpenCL, and a "module" in Level Zero.)
Both classes accept a runtime type as template argument that satisfies
the :cpp:concept:`tinytc::runtime` concept.
A runtime type is available for SYCL (:cpp:class:`tinytc::sycl_runtime`),
Level Zero (:cpp:class:`tinytc::level_zero_runtime`),
and OpenCL (:cpp:class:`tinytc::opencl_runtime`).

Events are used for synchronization.
Runtimes have different event memory management policies.
SYCL events and OpenCL events are reference-counted, whereas Level Zero events are user-managed.
Therefore, the signature of the *submit* function depends on runtime R, more specifially,
on the `R::is_event_managed` constant.
Usage for SYCL and OpenCL is as following:

.. code-block:: cpp

    #include <tinytc/tinytc.hpp>

    try {
        auto bundle = tinytc::tensor_kernel_bundle(binary, context, device);
        auto kernel = bundle.get("gemm");
        kernel.set_args(A, B, C);
        auto event = kernel.submit(howmany, q, {dep_event1, ..., dep_eventN})
    } catch (std::exception const& e) {
        ...
    }

The `event` variable is of type `sycl::event` when using the SYCL runtime.
For OpenCL, the native `cl_event` is wrapped in an
:cpp:class:`tinytc::opencl_object_wrapper`.
The object wrapper manages the retain and release calls, such that the call to
`clReleaseEvent` is done automatically once the object wrapper goes out of scope.

For Level Zero, usage is as following:

.. code-block:: cpp

    #include <tinytc/tinytc.hpp>

    ze_event_handle_t event;
    zeEventCreate(..., &event);
    try {
        auto bundle = tinytc::tensor_kernel_bundle(binary, context, device);
        auto kernel = bundle.get("gemm");
        kernel.set_args(A, B, C);
        kernel.submit(howmany, list, event, N, &dep_events);
    catch (std::exception const& e) {
        ...
    }

If events shall not be used, one can pass a `nullptr` for the `signal_event` argument
and call `zeCommandQueueSynchronize` on the command queue (regular command lists) or
`zeCommandListHostSynchronize` on the command list (immediate command lists).

.. note::

    Runtime-specific bits are separated from libtinytc, therefore libtinytc-sycl,
    libtinytc-level-zero, and libtinytc-opencl need to be added to the link line
    for SYCL, Level Zero, and OpenCL runtime classes, respectively.

.. warning::

    JIT compilation is expensive. Each bundle should only be created once and be reused
    many times.

Classes
=======

.. doxygenclass:: tinytc::tensor_kernel_bundle
.. doxygenclass:: tinytc::tensor_kernel

Concepts
========

.. doxygenconcept:: tinytc::runtime
