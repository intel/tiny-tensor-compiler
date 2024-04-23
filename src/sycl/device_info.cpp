// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "error.hpp"
#include "tinytc/tinytc_cl.hpp"
#include "tinytc/tinytc_ze.hpp"

#include <sycl/sycl.hpp>

using namespace sycl;
using namespace tinytc;

extern "C" {

tinytc_status_t tinytc_sycl_core_info_create(tinytc_core_info_t *info, const void *dev) {
    if (info == nullptr || dev == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code_sycl([&] {
        auto const &sycl_dev = *static_cast<device const *>(dev);
        switch (sycl_dev.get_backend()) {
        case backend::ext_oneapi_level_zero: {
            auto native_device = get_native<backend::ext_oneapi_level_zero, device>(sycl_dev);
            *info = create_core_info(native_device).release();
            return;
        }
        case backend::opencl: {
            auto native_device = get_native<backend::opencl, device>(sycl_dev);
            auto info_ = create_core_info(native_device);
            CL_CHECK_STATUS(clReleaseDevice(native_device));
            *info = info_.release();
            return;
        }
        default:
            break;
        }
        *info = nullptr;
        throw status::unsupported_backend;
    });
}
}

