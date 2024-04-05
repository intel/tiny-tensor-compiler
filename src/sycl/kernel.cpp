// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/sycl/kernel.hpp"
#include "tinytc/cl/error.hpp"

#include <stdexcept>
#include <utility>

using namespace sycl;

namespace tinytc {

auto make_kernel_bundle(std::uint8_t const *binary, std::size_t binary_size, bundle_format format,
                        std::uint32_t core_features, sycl::context ctx, sycl::device dev)
    -> kernel_bundle<bundle_state::executable> {
    switch (dev.get_backend()) {
    case backend::ext_oneapi_level_zero: {
        auto native_context = get_native<backend::ext_oneapi_level_zero, context>(ctx);
        auto native_device = get_native<backend::ext_oneapi_level_zero, device>(dev);
        auto mod = make_kernel_bundle(binary, binary_size, format, core_features, native_context,
                                      native_device);
        return make_kernel_bundle<backend::ext_oneapi_level_zero, bundle_state::executable>(
            {mod, ext::oneapi::level_zero::ownership::transfer}, ctx);
    }
    case backend::opencl: {
        auto native_context = get_native<backend::opencl, context>(ctx);
        auto native_device = get_native<backend::opencl, device>(dev);
        auto mod = make_kernel_bundle(binary, binary_size, format, core_features, native_context,
                                      native_device);
        auto bundle = make_kernel_bundle<backend::opencl, bundle_state::executable>(mod, ctx);
        CL_CHECK(clReleaseProgram(mod));
        CL_CHECK(clReleaseDevice(native_device));
        CL_CHECK(clReleaseContext(native_context));
        return bundle;
    }
    default:
        break;
    }
    throw std::runtime_error("Unsupported backend");
}

auto make_kernel(kernel_bundle<bundle_state::executable> mod, char const *name) -> kernel {
    switch (mod.get_backend()) {
    case backend::ext_oneapi_level_zero: {
        auto native_mod = get_native<backend::ext_oneapi_level_zero, bundle_state::executable>(mod);
        auto native_kernel = make_kernel(native_mod.front(), name);
        return make_kernel<backend::ext_oneapi_level_zero>(
            {mod, native_kernel, ext::oneapi::level_zero::ownership::transfer}, mod.get_context());
    }
    case backend::opencl: {
        auto native_mod = get_native<backend::opencl, bundle_state::executable>(mod);
        auto native_kernel = make_kernel(native_mod.front(), name);
        auto kernel = make_kernel<backend::opencl>(native_kernel, mod.get_context());
        for (auto &m : native_mod) {
            CL_CHECK(clReleaseProgram(m));
        }
        CL_CHECK(clReleaseKernel(native_kernel));
        return kernel;
    }
    default:
        break;
    }
    throw std::runtime_error("Unsupported backend");
}
auto get_sycl_nd_range(std::array<std::uint32_t, 2> work_group_size, std::size_t howmany)
    -> nd_range<3u> {
    return {{howmany, work_group_size[1], work_group_size[0]},
            {1u, work_group_size[1], work_group_size[0]}};
}

sycl_argument_handler::sycl_argument_handler(platform plat) {
    switch (plat.get_backend()) {
    case backend::opencl: {
        auto native_plat = get_native<backend::opencl, platform>(std::move(plat));
        cl_arg_ = opencl_argument_handler(native_plat);
        break;
    }
    default:
        break;
    }
}

void sycl_argument_handler::set_arg(kernel krnl, std::uint32_t arg_index, std::size_t arg_size,
                                    void const *arg_value) {
    switch (krnl.get_backend()) {
    case backend::ext_oneapi_level_zero: {
        auto native_krnl = get_native<backend::ext_oneapi_level_zero, kernel>(std::move(krnl));
        ze_arg_.set_arg(native_krnl, arg_index, arg_size, arg_value);
        return;
    }
    case backend::opencl: {
        auto native_krnl = get_native<backend::opencl, kernel>(std::move(krnl));
        cl_arg_.set_arg(native_krnl, arg_index, arg_size, arg_value);
        CL_CHECK(clReleaseKernel(native_krnl));
        return;
    }
    default:
        break;
    }
    throw std::runtime_error("Unsupported backend");
}

} // namespace tinytc
