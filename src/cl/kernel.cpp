// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/cl/kernel.hpp"
#include "tinytc/cl/error.hpp"
#include "tinytc/device_info.hpp"
#include "tinytc/internal/compiler_options.hpp"

#include <CL/cl_ext.h>
#include <cstdint>
#include <stdexcept>
#include <utility>

namespace tinytc {

auto make_kernel_bundle(std::uint8_t const *binary, std::size_t binary_size, bundle_format format,
                        std::uint32_t core_features, cl_context context, cl_device_id device)
    -> cl_program {
    cl_program mod;
    cl_int err;
    switch (format) {
    case bundle_format::spirv:
        mod = clCreateProgramWithIL(context, binary, binary_size, &err);
        break;

    case bundle_format::native:
        mod = clCreateProgramWithBinary(context, 1, &device, &binary_size, &binary, nullptr, &err);
        break;
    default:
        throw std::logic_error("Unknown module format");
    }
    CL_CHECK(err);

    char const *options = "";
    if (core_features & static_cast<std::uint32_t>(core_feature_flag::large_register_file)) {
        options = internal::large_register_file_compiler_option_cl;
    }
    err = clBuildProgram(mod, 1, &device, options, nullptr, nullptr);
    if (err != CL_SUCCESS) {
        std::string log;
        std::size_t log_size;
        CL_CHECK(clGetProgramBuildInfo(mod, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_size));
        log.resize(log_size);
        CL_CHECK(clGetProgramBuildInfo(mod, device, CL_PROGRAM_BUILD_LOG, log_size, log.data(),
                                       nullptr));
        char what[256];
        snprintf(what, sizeof(what), "clBuildProgram returned %s (%d).\n", cl_status_to_string(err),
                 err);
        throw opencl_error(std::string(what) + log, err);
    }

    return mod;
}

auto make_kernel(cl_program mod, char const *name) -> cl_kernel {
    cl_int err;
    cl_kernel kernel = clCreateKernel(mod, name, &err);
    CL_CHECK(err);
    return kernel;
}

auto get_opencl_nd_range(std::array<std::uint32_t, 2> work_group_size, std::size_t howmany)
    -> opencl_nd_range {
    auto result = opencl_nd_range{};
    result.local_work_size = {work_group_size[0], work_group_size[1], 1u};
    result.global_work_size = {work_group_size[0], work_group_size[1], howmany};
    return result;
}

opencl_argument_handler::opencl_argument_handler() : clSetKernelArgMemPointerINTEL_(nullptr) {}
opencl_argument_handler::opencl_argument_handler(cl_platform_id plat)
    : clSetKernelArgMemPointerINTEL_(
          (clSetKernelArgMemPointerINTEL_t)clGetExtensionFunctionAddressForPlatform(
              plat, "clSetKernelArgMemPointerINTEL")) {}

void opencl_argument_handler::set_arg_mem_pointer(cl_kernel kernel, std::uint32_t arg_index,
                                                  void const *arg_value) {
    if (clSetKernelArgMemPointerINTEL_ == nullptr) {
        throw opencl_error("Unified shared memory extension is not available", CL_INVALID_PLATFORM);
    }
    CL_CHECK(clSetKernelArgMemPointerINTEL_(kernel, arg_index, arg_value));
}
void opencl_argument_handler::set_arg(cl_kernel kernel, std::uint32_t arg_index,
                                      std::size_t arg_size, void const *arg_value) {
    CL_CHECK(clSetKernelArg(kernel, arg_index, arg_size, arg_value));
}

} // namespace tinytc
