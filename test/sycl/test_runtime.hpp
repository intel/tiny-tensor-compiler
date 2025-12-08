// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SYCL_TEST_RUNTIME_20240314_HPP
#define SYCL_TEST_RUNTIME_20240314_HPP

#include "sycl/argument_handler.hpp"

#include <tinytc/tinytc.hpp>
#include <tinytc/tinytc_sycl.hpp>

#include <memory>
#include <sycl/sycl.hpp>

class sycl_test_runtime {
  public:
    using device_t = sycl::device;
    using context_t = sycl::context;
    using command_list_t = sycl::queue;
    using kernel_bundle_t = sycl::kernel_bundle<sycl::bundle_state::executable>;
    using kernel_t = sycl::kernel;
    using mem_t = void *;
    using const_mem_t = const void *;

    sycl_test_runtime();

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
    void memcpy(void *dst, void const *src, std::size_t bytes);

    command_list_t q_;
    std::unique_ptr<tinytc::sycl_argument_handler> arg_handler_;
};

#endif // SYCL_TEST_RUNTIME_20240314_HPP
