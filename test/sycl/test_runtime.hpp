// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SYCL_TEST_RUNTIME_20240314_HPP
#define SYCL_TEST_RUNTIME_20240314_HPP

#include <tinytc/tinytc.hpp>
#include <tinytc/tinytc_sycl.hpp>

#include <memory>
#include <sycl/sycl.hpp>

class sycl_test_runtime {
  public:
    using device_t = sycl::device;
    using context_t = sycl::context;
    using command_list_t = sycl::queue;
    using recipe_handler_t = tinytc::sycl_recipe_handler;
    using mem_t = void *;
    using const_mem_t = const void *;

    auto create_buffer(std::size_t bytes) const -> mem_t;
    void free_buffer(mem_t buf) const;
    void fill_buffer(mem_t buf, int value, std::size_t bytes);
    void memcpy_h2d(mem_t dst, void const *src, std::size_t bytes);
    void memcpy_d2h(void *dst, const_mem_t src, std::size_t bytes);

    auto get_core_info() const -> tinytc::core_info;
    auto get_device() -> device_t;
    auto get_context() -> context_t;
    auto get_command_list() -> command_list_t;
    auto get_recipe_handler(tinytc::recipe const &rec) -> recipe_handler_t;
    void synchronize();

  private:
    void memcpy(void *dst, void const *src, std::size_t bytes);

    command_list_t q_;
};

#endif // SYCL_TEST_RUNTIME_20240314_HPP
