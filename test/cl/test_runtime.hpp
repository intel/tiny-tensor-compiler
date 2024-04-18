// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CL_TEST_RUNTIME_20240314_HPP
#define CL_TEST_RUNTIME_20240314_HPP

#include <tinytc/tinytc.hpp>
#include <tinytc/tinytc_cl.hpp>

#include <CL/cl.h>
#include <memory>

class opencl_test_runtime {
  public:
    using device_t = cl_device_id;
    using context_t = cl_context;
    using command_list_t = cl_command_queue;
    using runtime_t = tinytc::opencl_runtime;
    using mem_t = cl_mem;
    using const_mem_t = const cl_mem;

    opencl_test_runtime();
    ~opencl_test_runtime();

    opencl_test_runtime(opencl_test_runtime const &other) = delete;
    opencl_test_runtime(opencl_test_runtime &&other) = delete;
    opencl_test_runtime &operator=(opencl_test_runtime const &other) = delete;
    opencl_test_runtime &operator=(opencl_test_runtime &&other) = delete;

    auto create_buffer(std::size_t bytes) const -> mem_t;
    void free_buffer(mem_t buf) const;
    void fill_buffer(mem_t buf, int value, std::size_t bytes);
    void memcpy_h2d(mem_t dst, void const *src, std::size_t bytes);
    void memcpy_d2h(void *dst, const_mem_t src, std::size_t bytes);

    auto get_core_info() const -> tinytc::core_info;
    auto get_device() -> device_t;
    auto get_context() -> context_t;
    auto get_command_list() -> command_list_t;
    void synchronize();

  private:
    device_t dev_;
    context_t ctx_;
    command_list_t q_;
};

#endif // CL_TEST_RUNTIME_20240314_HPP
