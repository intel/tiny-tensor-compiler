// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "../device_info.hpp"
#include "device_info_helper.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc_cl.h"
#include "tinytc/types.h"

#include <CL/cl.h>
#include <CL/cl_ext.h>
#include <CL/cl_platform.h>
#include <cstddef>
#include <cstring>
#include <string>
#include <vector>

extern "C" {
tinytc_status_t tinytc_cl_get_support_level(cl_device_id device, tinytc_support_level_t *level) {
    if (level == nullptr) {
        return tinytc_status_invalid_arguments;
    }

    std::size_t extensions_size;
    TINYTC_CL_CHECK_STATUS(
        clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, 0, nullptr, &extensions_size));
    std::string extensions;
    extensions.resize(extensions_size);
    TINYTC_CL_CHECK_STATUS(
        clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, extensions_size, extensions.data(), nullptr));

    bool has_subgroups = tinytc::has_subgroups_extension(extensions.size(), extensions.c_str());
    if (!has_subgroups) {
        std::size_t features_size;
        TINYTC_CL_CHECK_STATUS(
            clGetDeviceInfo(device, CL_DEVICE_OPENCL_C_FEATURES, 0, nullptr, &features_size));
        auto features = std::vector<cl_name_version>(features_size / sizeof(cl_name_version));
        TINYTC_CL_CHECK_STATUS(clGetDeviceInfo(device, CL_DEVICE_OPENCL_C_FEATURES, features_size,
                                               features.data(), nullptr));
        for (auto &feature : features) {
            if (strncmp("__opencl_c_subgroups", feature.name, CL_NAME_VERSION_MAX_NAME_SIZE) == 0) {
                has_subgroups = true;
                break;
            }
        }
    }

    if (!has_subgroups) {
        *level = tinytc_support_level_none;
        return tinytc_status_success;
    }
    *level = tinytc_support_level_basic;

    cl_version ip_ver = 0;
    cl_int err =
        clGetDeviceInfo(device, CL_DEVICE_IP_VERSION_INTEL, sizeof(ip_ver), &ip_ver, nullptr);
    if (err == CL_SUCCESS && ip_ver == tinytc_intel_gpu_architecture_pvc) {
        *level = tinytc_support_level_tuned;
    }

    return tinytc_status_success;
}

tinytc_status_t tinytc_cl_core_info_create(tinytc_core_info_t *info, cl_device_id device) {
    if (info == nullptr) {
        return tinytc_status_invalid_arguments;
    }

    cl_version ip_ver;
    cl_uint num_eus_per_subslice, num_threads_per_eu;
    cl_ulong local_mem_size;

    TINYTC_CL_CHECK_STATUS(
        clGetDeviceInfo(device, CL_DEVICE_IP_VERSION_INTEL, sizeof(ip_ver), &ip_ver, nullptr));
    TINYTC_CL_CHECK_STATUS(clGetDeviceInfo(device, CL_DEVICE_NUM_EUS_PER_SUB_SLICE_INTEL,
                                           sizeof(num_eus_per_subslice), &num_eus_per_subslice,
                                           nullptr));
    TINYTC_CL_CHECK_STATUS(clGetDeviceInfo(device, CL_DEVICE_NUM_THREADS_PER_EU_INTEL,
                                           sizeof(num_threads_per_eu), &num_threads_per_eu,
                                           nullptr));
    TINYTC_CL_CHECK_STATUS(clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(local_mem_size),
                                           &local_mem_size, nullptr));

    std::size_t subgroup_sizes_size = 0;
    TINYTC_CL_CHECK_STATUS(
        clGetDeviceInfo(device, CL_DEVICE_SUB_GROUP_SIZES_INTEL, 0, nullptr, &subgroup_sizes_size));
    auto subgroup_sizes_long = std::vector<std::size_t>(subgroup_sizes_size / sizeof(std::size_t));
    TINYTC_CL_CHECK_STATUS(clGetDeviceInfo(device, CL_DEVICE_SUB_GROUP_SIZES_INTEL,
                                           subgroup_sizes_size, subgroup_sizes_long.data(),
                                           nullptr));
    auto subgroup_sizes =
        std::vector<std::uint32_t>(subgroup_sizes_long.begin(), subgroup_sizes_long.end());

    TINYTC_CHECK_STATUS(tinytc_core_info_intel_create(
        info, ip_ver, num_eus_per_subslice, num_threads_per_eu, local_mem_size,
        subgroup_sizes.size(), subgroup_sizes.data()));

    return tinytc_status_success;
}
}

