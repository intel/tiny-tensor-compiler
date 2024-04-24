// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ZE_TEST_RUNTIME_20240314_HPP
#define ZE_TEST_RUNTIME_20240314_HPP

#include <tinytc/tinytc.hpp>
#include <tinytc/tinytc_ze.hpp>

#include <cstddef>
#include <level_zero/ze_api.h>
#include <memory>

class level_zero_test_runtime {
  public:
    using device_t = ze_device_handle_t;
    using context_t = ze_context_handle_t;
    using command_list_t = ze_command_list_handle_t;
    using recipe_handler_t = tinytc::level_zero_recipe_handler;
    using mem_t = void *;
    using const_mem_t = const void *;

    level_zero_test_runtime();
    ~level_zero_test_runtime();

    level_zero_test_runtime(level_zero_test_runtime const &other) = delete;
    level_zero_test_runtime(level_zero_test_runtime &&other) = delete;
    level_zero_test_runtime &operator=(level_zero_test_runtime const &other) = delete;
    level_zero_test_runtime &operator=(level_zero_test_runtime &&other) = delete;

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
    void memcpy(void *dst, const void *src, std::size_t bytes);

    device_t dev_;
    context_t ctx_;
    command_list_t list_;
};

#endif // ZE_TEST_RUNTIME_20240314_HPP
