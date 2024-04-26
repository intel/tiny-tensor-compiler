// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "test_runtime.hpp"

void sycl_test_runtime::memcpy(void *dst, const void *src, std::size_t bytes) {
    q_.memcpy(dst, src, bytes).wait();
}

auto sycl_test_runtime::create_buffer(std::size_t bytes) const -> mem_t {
    return ::sycl::malloc_device(bytes, q_);
}
void sycl_test_runtime::free_buffer(void *ptr) const { ::sycl::free(ptr, q_); }
void sycl_test_runtime::fill_buffer(mem_t buf, int value, std::size_t bytes) {
    q_.memset(buf, value, bytes).wait();
}
void sycl_test_runtime::memcpy_h2d(mem_t dst, void const *src, std::size_t bytes) {
    this->memcpy(dst, src, bytes);
}
void sycl_test_runtime::memcpy_d2h(void *dst, const_mem_t src, std::size_t bytes) {
    this->memcpy(dst, src, bytes);
}

auto sycl_test_runtime::get_core_info() const -> tinytc::core_info {
    return ::tinytc::make_core_info(q_.get_device());
}
auto sycl_test_runtime::get_device() -> device_t { return q_.get_device(); }
auto sycl_test_runtime::get_context() -> context_t { return q_.get_context(); }
auto sycl_test_runtime::get_command_list() -> command_list_t { return q_; }
auto sycl_test_runtime::get_recipe_handler(tinytc::recipe const &rec) -> recipe_handler_t {
    return tinytc::make_recipe_handler(q_, rec);
}
void sycl_test_runtime::synchronize() { q_.wait(); }
