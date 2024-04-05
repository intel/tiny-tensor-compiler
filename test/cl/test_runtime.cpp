// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "test_runtime.hpp"

#include <cstdint>

opencl_test_runtime::opencl_test_runtime() {
    cl_uint platform_count = 1;
    cl_platform_id platform;
    CL_CHECK(clGetPlatformIDs(platform_count, &platform, nullptr));

    cl_uint device_count = 1;
    CL_CHECK(clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, device_count, &dev_, nullptr));

    cl_int err;
    ctx_ = clCreateContext(nullptr, device_count, &dev_, nullptr, nullptr, &err);
    CL_CHECK(err);

    q_ = clCreateCommandQueueWithProperties(ctx_, dev_, 0, &err);
    CL_CHECK(err);
}
opencl_test_runtime::~opencl_test_runtime() {
    clReleaseCommandQueue(q_);
    clReleaseContext(ctx_);
    clReleaseDevice(dev_);
}

auto opencl_test_runtime::create_buffer(std::size_t bytes) const -> mem_t {
    cl_int err;
    cl_mem buf = clCreateBuffer(ctx_, CL_MEM_READ_WRITE, bytes, nullptr, &err);
    CL_CHECK(err);
    return buf;
}
void opencl_test_runtime::free_buffer(mem_t buf) const { CL_CHECK(clReleaseMemObject(buf)); }
void opencl_test_runtime::fill_buffer(mem_t buf, int value, std::size_t bytes) {
    CL_CHECK(clEnqueueFillBuffer(q_, buf, &value, sizeof(int), 0, bytes, 0, nullptr, nullptr));
    synchronize();
}
void opencl_test_runtime::memcpy_h2d(mem_t dst, void const *src, std::size_t bytes) {
    CL_CHECK(clEnqueueWriteBuffer(q_, dst, CL_FALSE, 0, bytes, src, 0, nullptr, nullptr));
    synchronize();
}
void opencl_test_runtime::memcpy_d2h(void *dst, const_mem_t src, std::size_t bytes) {
    CL_CHECK(clEnqueueReadBuffer(q_, src, CL_FALSE, 0, bytes, dst, 0, nullptr, nullptr));
    synchronize();
}

auto opencl_test_runtime::get_core_info() const -> std::shared_ptr<tinytc::core_info> {
    return ::tinytc::get_core_info(dev_);
}
auto opencl_test_runtime::get_device() -> device_t { return dev_; }
auto opencl_test_runtime::get_context() -> context_t { return ctx_; }
auto opencl_test_runtime::get_command_list() -> command_list_t { return q_; }
void opencl_test_runtime::synchronize() { CL_CHECK(clFinish(q_)); }
