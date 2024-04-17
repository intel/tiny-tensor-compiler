// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "../compiler_options.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc_cl.h"

#include <CL/cl_ext.h>
#include <cstdint>
#include <stdexcept>
#include <utility>

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
    TINYTC_CL_CHECK(err);

    uint32_t core_features;
    TINYTC_CHECK(tinytc_binary_get_core_features(bin, &core_features));

    char const *options = "";
    if (core_features & static_cast<std::uint32_t>(tinytc_core_feature_flag_large_register_file)) {
        options = tinytc::large_register_file_compiler_option_cl;
    }
    TINYTC_CL_CHECK(clBuildProgram(*mod, 1, &device, options, nullptr, nullptr));
    return tinytc_status_success;
}

tinytc_status_t tinytc_cl_get_group_size(cl_kernel kernel, cl_device_id dev, std::size_t *x,
                                         std::size_t *y, std::size_t *z) {
    if (x == nullptr || y == nullptr || z == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    std::size_t wgs[3];
    auto status = clGetKernelWorkGroupInfo(kernel, dev, CL_KERNEL_COMPILE_WORK_GROUP_SIZE,
                                           sizeof(wgs), wgs, nullptr);
    *x = wgs[0];
    *y = wgs[1];
    *z = wgs[2];
    return tinytc_cl_convert_status(status);
}

void tinytc_cl_get_global_size(std::size_t howmany, std::size_t local_size_x,
                               std::size_t local_size_y, std::size_t local_size_z,
                               std::size_t *global_size_x, std::size_t *global_size_y,
                               std::size_t *global_size_z) {
    *global_size_x = local_size_x;
    *global_size_y = local_size_y;
    *global_size_z = local_size_z * howmany;
}
}
