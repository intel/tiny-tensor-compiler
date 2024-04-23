// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/tinytc_sycl.h"
#include "tinytc/types.h"

#include <sycl/sycl.hpp>

extern "C" {
tinytc_status_t tinytc_sycl_convert_status(int value) {
    switch (static_cast<sycl::errc>(value)) {
    case sycl::errc::success:
        return tinytc_status_success;
    case sycl::errc::runtime:
        return tinytc_status_sycl_runtime;
    case sycl::errc::kernel:
        return tinytc_status_sycl_kernel;
    case sycl::errc::accessor:
        return tinytc_status_sycl_accessor;
    case sycl::errc::nd_range:
        return tinytc_status_sycl_nd_range;
    case sycl::errc::event:
        return tinytc_status_sycl_event;
    case sycl::errc::kernel_argument:
        return tinytc_status_sycl_kernel_argument;
    case sycl::errc::build:
        return tinytc_status_sycl_build;
    case sycl::errc::invalid:
        return tinytc_status_sycl_invalid;
    case sycl::errc::memory_allocation:
        return tinytc_status_sycl_memory_allocation;
    case sycl::errc::platform:
        return tinytc_status_sycl_platform;
    case sycl::errc::profiling:
        return tinytc_status_sycl_profiling;
    case sycl::errc::feature_not_supported:
        return tinytc_status_sycl_feature_not_supported;
    case sycl::errc::kernel_not_supported:
        return tinytc_status_sycl_kernel_not_supported;
    case sycl::errc::backend_mismatch:
        return tinytc_status_sycl_backend_mismatch;
    }
    return tinytc_status_unknown;
}
}
