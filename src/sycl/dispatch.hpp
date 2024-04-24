// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DISPATCH_20240424_HPP
#define DISPATCH_20240424_HPP

#include "tinytc/tinytc_cl.hpp"
#include "tinytc/types.hpp"

#include <CL/cl.h>
#include <sycl/sycl.hpp>
#include <utility>

namespace tinytc {

template <sycl::backend B> struct dispatch_traits;
template <> struct dispatch_traits<sycl::backend::ext_oneapi_level_zero> {
    static void release(auto &&) {}
};
template <> struct dispatch_traits<sycl::backend::opencl> {
    static void release(cl_device_id obj) { CL_CHECK_STATUS(clReleaseDevice(obj)); }
    static void release(cl_kernel obj) { CL_CHECK_STATUS(clReleaseKernel(obj)); }
};

template <template <sycl::backend> class Dispatcher, typename... Args>
auto dispatch(sycl::backend be, Args &&...args) {
    switch (be) {
    case sycl::backend::ext_oneapi_level_zero:
        return Dispatcher<sycl::backend::ext_oneapi_level_zero>{}(std::forward<Args>(args)...);
    case sycl::backend::opencl:
        return Dispatcher<sycl::backend::opencl>{}(std::forward<Args>(args)...);
    default:
        break;
    };
    throw status::unsupported_backend;
}

} // namespace tinytc

#endif // DISPATCH_20240424_HPP
