// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/sycl/device_info.hpp"
#include "tinytc/cl/device_info.hpp"
#include "tinytc/cl/error.hpp"
#include "tinytc/ze/device_info.hpp"

using namespace sycl;

namespace tinytc {

auto get_core_info(device dev) -> std::shared_ptr<core_info> {
    switch (dev.get_backend()) {
    case backend::ext_oneapi_level_zero: {
        auto native_device = get_native<backend::ext_oneapi_level_zero, device>(dev);
        return get_core_info(native_device);
    }
    case backend::opencl: {
        auto native_device = get_native<backend::opencl, device>(dev);
        auto info = get_core_info(native_device);
        CL_CHECK(clReleaseDevice(native_device));
        return info;
    }
    default:
        break;
    }
    return nullptr;
}

auto get_number_of_cores(device dev) -> std::uint32_t {
    switch (dev.get_backend()) {
    case backend::ext_oneapi_level_zero: {
        auto native_device = get_native<backend::ext_oneapi_level_zero, device>(dev);
        return get_number_of_cores(native_device);
    }
    case backend::opencl: {
        auto native_device = get_native<backend::opencl, device>(dev);
        auto num_cores = get_number_of_cores(native_device);
        CL_CHECK(clReleaseDevice(native_device));
        return num_cores;
    }
    default:
        break;
    }
    return 0u;
}

} // namespace tinytc
