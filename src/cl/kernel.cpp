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

extern "C" {

tinytc_status_t tinytc_cl_program_create(cl_program *mod, cl_context context, cl_device_id device,
                                         tinytc_binary_t bin) {
    if (bin == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    cl_int err;
    tinytc_bundle_format_t format;
    uint64_t data_size;
    uint8_t const *data;
    tinytc_binary_get_raw(bin, &format, &data_size, &data);
    if (format == tinytc_bundle_format_native) {
        *mod = clCreateProgramWithBinary(context, 1, &device, &data_size, &data, nullptr, &err);
    } else {
        *mod = clCreateProgramWithIL(context, data, data_size, &err);
    }
    TINYTC_CL_CHECK_STATUS(err);

    uint32_t core_features;
    TINYTC_CHECK_STATUS(tinytc_binary_get_core_features(bin, &core_features));

    char const *options = "";
    if (core_features & static_cast<std::uint32_t>(tinytc_core_feature_flag_large_register_file)) {
        options = tinytc::large_register_file_compiler_option_cl;
    }
    TINYTC_CL_CHECK_STATUS(clBuildProgram(*mod, 1, &device, options, nullptr, nullptr));
    return tinytc_status_success;
}

tinytc_status_t tinytc_cl_get_group_size(cl_kernel kernel, size_t *local_size) {
    if (local_size == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return tinytc_cl_convert_status(
        clGetKernelWorkGroupInfo(kernel, NULL, CL_KERNEL_COMPILE_WORK_GROUP_SIZE,
                                 3 * sizeof(std::size_t), local_size, nullptr));
}

void tinytc_cl_get_global_size(size_t howmany, const size_t *local_size, size_t *global_size) {
    for (size_t i = 0; i < 3; ++i) {
        global_size[i] = local_size[i];
    }
    global_size[2] *= howmany;
}
}
