// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "../device_info.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc_cl.h"

#include <CL/cl_ext.h>
#include <vector>

extern "C" {
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

