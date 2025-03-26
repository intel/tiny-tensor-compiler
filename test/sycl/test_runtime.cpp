// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "test_runtime.hpp"

sycl_test_runtime::sycl_test_runtime() {
    switch (q_.get_backend()) {
    case sycl::backend::ext_oneapi_level_zero:
        arg_handler_ = std::make_unique<tinytc::sycl_argument_handler_level_zero_backend>();
        break;
    case sycl::backend::opencl:
        arg_handler_ = std::make_unique<tinytc::sycl_argument_handler_opencl_backend>(
            q_.get_device().get_platform());
        break;
    default:
        throw ::tinytc::status::unsupported_backend;
        break;
    };
}

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
auto sycl_test_runtime::get_kernel_bundle(tinytc::prog p, tinytc_core_feature_flags_t core_features)
    -> kernel_bundle_t {
    return ::tinytc::make_kernel_bundle(q_.get_context(), q_.get_device(), std::move(p),
                                        core_features);
}
auto sycl_test_runtime::get_kernel(kernel_bundle_t const &bundle, char const *name) -> kernel_t {
    return ::tinytc::make_kernel(bundle, name);
}
void sycl_test_runtime::set_arg(kernel_t &kernel, std::uint32_t arg_index, std::size_t arg_size,
                                const void *arg_value) {
    arg_handler_->set_arg(kernel, arg_index, arg_size, arg_value);
}
void sycl_test_runtime::set_mem_arg(kernel_t &kernel, std::uint32_t arg_index,
                                    const void *arg_value, tinytc::mem_type type) {
    arg_handler_->set_mem_arg(kernel, arg_index, arg_value, static_cast<tinytc_mem_type_t>(type));
}
void sycl_test_runtime::submit(kernel_t &kernel, std::int64_t howmany) {
    auto exe_range = ::tinytc::get_execution_range(kernel, howmany);
    q_.submit([&](sycl::handler &h) { h.parallel_for(exe_range, kernel); });
}
void sycl_test_runtime::synchronize() { q_.wait(); }

bool sycl_test_runtime::supports_fp64() { return q_.get_device().has(sycl::aspect::fp64); }
