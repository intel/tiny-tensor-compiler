// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/device_info.hpp"
#include "tinytc/cl/device_info.hpp"
#include "tinytc/cl/error.hpp"

#include <CL/cl_ext.h>
#include <utility>
#include <vector>

namespace tinytc {

auto get_core_info(cl_device_id device) -> std::shared_ptr<core_info> {
    cl_version ip_ver;
    cl_uint num_eus_per_subslice, num_threads_per_eu;
    cl_ulong local_mem_size;

    CL_CHECK(clGetDeviceInfo(device, CL_DEVICE_IP_VERSION_INTEL, sizeof(ip_ver), &ip_ver, nullptr));
    CL_CHECK(clGetDeviceInfo(device, CL_DEVICE_NUM_EUS_PER_SUB_SLICE_INTEL,
                             sizeof(num_eus_per_subslice), &num_eus_per_subslice, nullptr));
    CL_CHECK(clGetDeviceInfo(device, CL_DEVICE_NUM_THREADS_PER_EU_INTEL, sizeof(num_threads_per_eu),
                             &num_threads_per_eu, nullptr));
    CL_CHECK(clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(local_mem_size),
                             &local_mem_size, nullptr));

    std::size_t subgroup_sizes_size = 0;
    CL_CHECK(
        clGetDeviceInfo(device, CL_DEVICE_SUB_GROUP_SIZES_INTEL, 0, nullptr, &subgroup_sizes_size));
    auto subgroup_sizes_long = std::vector<std::size_t>(subgroup_sizes_size / sizeof(std::size_t));
    CL_CHECK(clGetDeviceInfo(device, CL_DEVICE_SUB_GROUP_SIZES_INTEL, subgroup_sizes_size,
                             subgroup_sizes_long.data(), nullptr));
    auto subgroup_sizes =
        std::vector<std::uint32_t>(subgroup_sizes_long.begin(), subgroup_sizes_long.end());

    return std::make_shared<core_info_intel>(ip_ver, num_eus_per_subslice, num_threads_per_eu,
                                             local_mem_size, std::move(subgroup_sizes));
}

auto get_number_of_cores(cl_device_id device) -> std::uint32_t {
    cl_uint num_slices, num_subslices;
    CL_CHECK(clGetDeviceInfo(device, CL_DEVICE_NUM_SLICES_INTEL, sizeof(num_slices), &num_slices,
                             nullptr));
    CL_CHECK(clGetDeviceInfo(device, CL_DEVICE_NUM_SUB_SLICES_PER_SLICE_INTEL,
                             sizeof(num_subslices), &num_subslices, nullptr));
    return num_slices * num_subslices;
}

} // namespace tinytc

