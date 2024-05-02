// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/tinytc.hpp"
#include "tinytc/tinytc_cl.hpp"
#include "tinytc/types.hpp"

#include <CL/cl.h>
#include <CL/cl_ext.h>
#include <CL/cl_platform.h>
#include <cstdint>
#include <doctest/doctest.h>
#include <vector>

using namespace tinytc;

TEST_CASE("device") {
    auto platforms = std::vector<cl_platform_id>{};
    cl_uint platform_count = 0;
    CL_CHECK_STATUS(clGetPlatformIDs(platform_count, NULL, &platform_count));
    platforms.resize(platform_count);
    CL_CHECK_STATUS(clGetPlatformIDs(platform_count, platforms.data(), &platform_count));

    cl_uint device_count = 0;
    cl_device_id device;
    for (cl_uint p = 0; p < platform_count; ++p) {
        cl_int err =
            clGetDeviceIDs(platforms[p], CL_DEVICE_TYPE_GPU, device_count, NULL, &device_count);
        if (err == CL_SUCCESS) {
            CL_CHECK_STATUS(clGetDeviceIDs(platforms[p], CL_DEVICE_TYPE_GPU, device_count, &device,
                                           &device_count));
            break;
        }
    }
    if (device_count == 0) {
        WARN_MESSAGE(false, "Device test needs GPU device");
        return;
    }

    cl_version ip_ver;
    CL_CHECK_STATUS(
        clGetDeviceInfo(device, CL_DEVICE_IP_VERSION_INTEL, sizeof(ip_ver), &ip_ver, nullptr));

    auto info = make_core_info(device);
    std::uint32_t sgs_size;
    std::uint32_t const *sgs;
    info.get_subgroup_sizes(&sgs_size, &sgs);

    if (ip_ver >= static_cast<std::uint32_t>(intel_gpu_architecture::pvc)) {
        REQUIRE(sgs_size == 2);
        CHECK(sgs[0] == 16);
        CHECK(sgs[1] == 32);

        CHECK(info.get_register_size() == 64);
        CHECK(info.get_num_registers_per_thread() == 128);
        info.set_core_features(tinytc_core_feature_flag_large_register_file);
        CHECK(info.get_num_registers_per_thread() == 256);
    } else if (ip_ver >= static_cast<std::uint32_t>(intel_gpu_architecture::tgl)) {
        REQUIRE(sgs_size == 3);
        CHECK(sgs[0] == 8);
        CHECK(sgs[1] == 16);
        CHECK(sgs[2] == 32);

        CHECK(info.get_register_size() == 32);
        CHECK(info.get_num_registers_per_thread() == 128);
        info.set_core_features(tinytc_core_feature_flag_large_register_file);
        CHECK(info.get_num_registers_per_thread() == 128);
    } else {
        WARN_MESSAGE(false, "Device test only works on Gen12 / PVC");
    }
}
