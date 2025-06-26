// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "test_runtime.hpp"
#include "argument_handler.hpp"
#include "tinytc/tinytc_cl.hpp"
#include "tinytc/types.h"

#include <CL/cl_platform.h>
#include <array>
#include <stdexcept>
#include <utility>
#include <vector>

using tinytc::CL_CHECK_STATUS;

opencl_test_runtime::opencl_test_runtime() {
    auto platforms = std::vector<cl_platform_id>{};
    cl_uint platform_count = 0;
    CL_CHECK_STATUS(clGetPlatformIDs(platform_count, NULL, &platform_count));
    platforms.resize(platform_count);
    CL_CHECK_STATUS(clGetPlatformIDs(platform_count, platforms.data(), &platform_count));

    cl_uint device_count = 0;
    for (cl_uint p = 0; p < platform_count; ++p) {
        cl_int err =
            clGetDeviceIDs(platforms[p], CL_DEVICE_TYPE_GPU, device_count, NULL, &device_count);
        if (err == CL_SUCCESS && device_count > 0) {
            device_count = 1;
            CL_CHECK_STATUS(
                clGetDeviceIDs(platforms[p], CL_DEVICE_TYPE_GPU, device_count, &dev_, NULL));
            arg_handler_.set_platform(platforms[p]);
            break;
        }
    }
    if (device_count == 0) {
        throw std::runtime_error("No GPU device available");
    }

    cl_int err;
    ctx_ = clCreateContext(nullptr, device_count, &dev_, nullptr, nullptr, &err);
    CL_CHECK_STATUS(err);

    q_ = clCreateCommandQueueWithProperties(ctx_, dev_, 0, &err);
    CL_CHECK_STATUS(err);
}
opencl_test_runtime::~opencl_test_runtime() {
    clReleaseCommandQueue(q_);
    clReleaseContext(ctx_);
    clReleaseDevice(dev_);
}

auto opencl_test_runtime::create_buffer(std::size_t bytes) const -> mem_t {
    cl_int err;
    cl_mem buf = clCreateBuffer(ctx_, CL_MEM_READ_WRITE, bytes, nullptr, &err);
    CL_CHECK_STATUS(err);
    return buf;
}
void opencl_test_runtime::free_buffer(mem_t buf) const { CL_CHECK_STATUS(clReleaseMemObject(buf)); }
void opencl_test_runtime::fill_buffer(mem_t buf, int value, std::size_t bytes) {
    CL_CHECK_STATUS(
        clEnqueueFillBuffer(q_, buf, &value, sizeof(int), 0, bytes, 0, nullptr, nullptr));
    synchronize();
}
void opencl_test_runtime::memcpy_h2d(mem_t dst, void const *src, std::size_t bytes) {
    CL_CHECK_STATUS(clEnqueueWriteBuffer(q_, dst, CL_FALSE, 0, bytes, src, 0, nullptr, nullptr));
    synchronize();
}
void opencl_test_runtime::memcpy_d2h(void *dst, const_mem_t src, std::size_t bytes) {
    CL_CHECK_STATUS(clEnqueueReadBuffer(q_, src, CL_FALSE, 0, bytes, dst, 0, nullptr, nullptr));
    synchronize();
}

auto opencl_test_runtime::get_core_info() const -> tinytc::core_info {
    return ::tinytc::make_core_info(dev_);
}
auto opencl_test_runtime::get_device() -> device_t { return dev_; }
auto opencl_test_runtime::get_context() -> context_t { return ctx_; }
auto opencl_test_runtime::get_command_list() -> command_list_t { return q_; }
auto opencl_test_runtime::get_recipe_handler(tinytc::recipe const &rec) -> recipe_handler_t {
    return tinytc::make_recipe_handler(ctx_, dev_, rec);
}
auto opencl_test_runtime::get_kernel_bundle(tinytc::prog p,
                                            tinytc_core_feature_flags_t core_features)
    -> kernel_bundle_t {
    return ::tinytc::make_kernel_bundle(ctx_, dev_, std::move(p), core_features);
}
auto opencl_test_runtime::get_kernel(kernel_bundle_t const &bundle, char const *name) -> kernel_t {
    return ::tinytc::make_kernel(bundle.get(), name);
}
void opencl_test_runtime::set_arg(kernel_t &kernel, std::uint32_t arg_index, std::size_t arg_size,
                                  const void *arg_value) {
    arg_handler_.set_arg(kernel.get(), arg_index, arg_size, arg_value);
}
void opencl_test_runtime::set_mem_arg(kernel_t &kernel, std::uint32_t arg_index,
                                      const void *arg_value, tinytc::mem_type type) {
    arg_handler_.set_mem_arg(kernel.get(), arg_index, arg_value,
                             static_cast<tinytc_mem_type_t>(type));
}
void opencl_test_runtime::submit(kernel_t &kernel, std::int64_t howmany) {
    auto ng = std::array<std::size_t, 3u>{static_cast<std::size_t>(howmany), 1, 1};
    auto ls = ::tinytc::get_group_size(kernel.get());
    auto gs = ::tinytc::get_global_size(ng, ls);
    CL_CHECK_STATUS(
        clEnqueueNDRangeKernel(q_, kernel.get(), 3u, NULL, gs.data(), ls.data(), 0, NULL, NULL));
}
void opencl_test_runtime::synchronize() { CL_CHECK_STATUS(clFinish(q_)); }

bool opencl_test_runtime::supports_fp64() {
    cl_device_fp_config fp_cfg;
    CL_CHECK_STATUS(
        clGetDeviceInfo(dev_, CL_DEVICE_DOUBLE_FP_CONFIG, sizeof(fp_cfg), &fp_cfg, NULL));
    return bool(fp_cfg & CL_FP_FMA);
}
