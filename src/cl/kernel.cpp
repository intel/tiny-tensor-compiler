// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "../compiler_options.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc_cl.h"
#include "tinytc/types.h"

#include <CL/cl.h>
#include <CL/cl_platform.h>
#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>
#include <utility>

extern "C" {

tinytc_status_t tinytc_cl_kernel_bundle_create_with_source(cl_program *bundle, cl_context context,
                                                           cl_device_id device,
                                                           const_tinytc_source_t src,
                                                           tinytc_source_context_t source_ctx) {
    if (bundle == nullptr || src == nullptr) {
        return tinytc_status_invalid_arguments;
    }

    size_t length = 0;
    char const *code = nullptr;
    tinytc_core_feature_flags_t core_features = 0;
    TINYTC_CL_CHECK_STATUS(tinytc_source_get_code(src, &length, &code));
    TINYTC_CL_CHECK_STATUS(tinytc_source_get_core_features(src, &core_features));

    cl_int err;
    cl_program p = clCreateProgramWithSource(context, 1, &code, &length, &err);
    TINYTC_CL_CHECK_STATUS(err);

    auto options = std::ostringstream{};
    for (auto const &opt : tinytc::default_compiler_options) {
        options << opt << " ";
    }
    if (core_features & tinytc_core_feature_flag_large_register_file) {
        options << tinytc::large_register_file_compiler_option_cl;
    }
    auto options_str = std::move(options).str();
    if (err = clBuildProgram(p, 1, &device, options_str.c_str(), nullptr, nullptr);
        err != CL_SUCCESS) {
        if (source_ctx) {
            std::string log;
            std::size_t log_size;
            clGetProgramBuildInfo(p, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_size);
            log.resize(log_size);
            clGetProgramBuildInfo(p, device, CL_PROGRAM_BUILD_LOG, log_size, log.data(), nullptr);

            tinytc_location_t loc = {};
            tinytc_source_get_location(src, &loc);
            tinytc_source_context_report_error(source_ctx, &loc, log.c_str(), true);
        }
        clReleaseProgram(p);
        TINYTC_CL_CHECK_STATUS(err);
    }
    *bundle = p;
    return tinytc_status_success;
}

tinytc_status_t tinytc_cl_kernel_bundle_create_with_program(
    cl_program *bundle, cl_context context, cl_device_id device, tinytc_prog_t prg,
    tinytc_core_feature_flags_t core_features, tinytc_source_context_t source_ctx) {
    if (bundle == nullptr || prg == nullptr) {
        return tinytc_status_invalid_arguments;
    }

    tinytc_core_info_t info = nullptr;
    tinytc_source_t src = nullptr;
    tinytc_status_t status = tinytc_status_success;

    if (status = tinytc_cl_core_info_create(&info, device); status != tinytc_status_success) {
        goto err;
    }
    if (status = tinytc_core_info_set_core_features(info, core_features);
        status != tinytc_status_success) {
        goto err;
    }
    if (status = tinytc_prog_compile_to_opencl(&src, prg, info, source_ctx);
        status != tinytc_status_success) {
        goto err;
    }
    if (status =
            tinytc_cl_kernel_bundle_create_with_source(bundle, context, device, src, source_ctx);
        status != tinytc_status_success) {
        goto err;
    }
err:
    tinytc_source_release(src);
    tinytc_core_info_release(info);

    return status;
}

tinytc_status_t tinytc_cl_kernel_bundle_create_with_binary(cl_program *bundle, cl_context context,
                                                           cl_device_id device,
                                                           const_tinytc_binary_t bin,
                                                           tinytc_source_context_t source_ctx) {
    if (bin == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    cl_int err;
    tinytc_bundle_format_t format;
    uint64_t data_size;
    uint8_t const *data;
    tinytc_binary_get_raw(bin, &format, &data_size, &data);

    cl_program p;
    if (format == tinytc_bundle_format_native) {
        p = clCreateProgramWithBinary(context, 1, &device, &data_size, &data, nullptr, &err);
    } else {
        p = clCreateProgramWithIL(context, data, data_size, &err);
    }
    TINYTC_CL_CHECK_STATUS(err);

    tinytc_core_feature_flags_t core_features;
    TINYTC_CHECK_STATUS(tinytc_binary_get_core_features(bin, &core_features));

    char const *options = "";
    if (core_features & tinytc_core_feature_flag_large_register_file) {
        options = tinytc::large_register_file_compiler_option_cl;
    }
    if (err = clBuildProgram(p, 1, &device, options, nullptr, nullptr); err != CL_SUCCESS) {
        if (source_ctx) {
            std::string log;
            std::size_t log_size;
            clGetProgramBuildInfo(p, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_size);
            log.resize(log_size);
            clGetProgramBuildInfo(p, device, CL_PROGRAM_BUILD_LOG, log_size, log.data(), nullptr);

            tinytc_location_t loc = {};
            tinytc_source_context_report_error(source_ctx, &loc, log.c_str(), true);
        }
        clReleaseProgram(p);
        TINYTC_CL_CHECK_STATUS(err);
    }
    *bundle = p;
    return tinytc_status_success;
}

tinytc_status_t tinytc_cl_get_group_size(cl_kernel kernel, size_t *local_size) {
    if (local_size == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    cl_program p;
    cl_device_id d;
    TINYTC_CL_CHECK_STATUS(clGetKernelInfo(kernel, CL_KERNEL_PROGRAM, sizeof(p), &p, nullptr));
    TINYTC_CL_CHECK_STATUS(clGetProgramInfo(p, CL_PROGRAM_DEVICES, sizeof(d), &d, nullptr));
    return tinytc_cl_convert_status(
        clGetKernelWorkGroupInfo(kernel, d, CL_KERNEL_COMPILE_WORK_GROUP_SIZE,
                                 3 * sizeof(std::size_t), local_size, nullptr));
}

void tinytc_cl_get_global_size(int64_t howmany, const size_t *local_size, size_t *global_size) {
    for (size_t i = 0; i < 3; ++i) {
        global_size[i] = local_size[i];
    }
    global_size[2] *= howmany;
}
}
