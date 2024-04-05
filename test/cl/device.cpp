// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include <tinytc/tinytc-opencl.hpp>
#include <tinytc/tinytc.hpp>

#include <CL/cl.h>
#include <CL/cl_ext.h>
#include <cstdint>
#include <doctest/doctest.h>

using namespace tinytc;

TEST_CASE("device") {
    cl_uint num_platforms = 1;
    cl_platform_id platform;
    CL_CHECK(clGetPlatformIDs(num_platforms, &platform, nullptr));

    cl_uint num_devices = 1;
    cl_device_id device;
    CL_CHECK(clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, num_devices, &device, nullptr));

    cl_version ip_ver;
    CL_CHECK(clGetDeviceInfo(device, CL_DEVICE_IP_VERSION_INTEL, sizeof(ip_ver), &ip_ver, nullptr));

    if (ip_ver >= static_cast<std::uint32_t>(intel_gpu_architecture::pvc)) {
        auto info = get_core_info(device);

        REQUIRE(info->subgroup_sizes().size() == 2);
        CHECK(info->subgroup_sizes()[0] == 16);
        CHECK(info->subgroup_sizes()[1] == 32);

        CHECK(info->num_registers_per_thread() == 128);
        info->set_core_feature(core_feature_flag::large_register_file);
        CHECK(info->num_registers_per_thread() == 256);
    } else {
        WARN_MESSAGE(false, "Device test only works on PVC");
    }
}
