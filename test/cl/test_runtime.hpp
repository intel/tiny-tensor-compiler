// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CL_TEST_RUNTIME_20240314_HPP
#define CL_TEST_RUNTIME_20240314_HPP

#include "cl/argument_handler.hpp"
#include "tinytc/tinytc_cl.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <CL/cl.h>
#include <cstddef>
#include <cstdint>

class opencl_test_runtime {
  public:
    using device_t = cl_device_id;
    using context_t = cl_context;
    using command_list_t = cl_command_queue;
    using kernel_bundle_t = tinytc::shared_handle<cl_program>;
    using kernel_t = tinytc::shared_handle<cl_kernel>;
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

    auto get_core_info() const -> tinytc::shared_handle<tinytc_core_info_t>;
    auto get_device() -> device_t;
    auto get_context() -> context_t;
    auto get_command_list() -> command_list_t;
    auto get_recipe_handler(tinytc_recipe_t rec) -> tinytc::shared_handle<tinytc_recipe_handler_t>;
    auto get_kernel_bundle(tinytc_prog_t p, tinytc_core_feature_flags_t core_features = 0)
        -> kernel_bundle_t;
    auto get_kernel(kernel_bundle_t const &bundle, char const *name) -> kernel_t;
    void set_arg(kernel_t &kernel, std::uint32_t arg_index, std::size_t arg_size,
                 const void *arg_value);
    void set_mem_arg(kernel_t &kernel, std::uint32_t arg_index, const void *arg_value,
                     tinytc::mem_type type);
    void submit(kernel_t &kernel, std::int64_t howmany = 1);
    void synchronize();

    bool supports_fp64();

  private:
    device_t dev_;
    context_t ctx_;
    command_list_t q_;
    tinytc::opencl_argument_handler arg_handler_;
};

#endif // CL_TEST_RUNTIME_20240314_HPP
