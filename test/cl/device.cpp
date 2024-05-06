// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "device_info_helper.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/tinytc_cl.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <CL/cl.h>
#include <CL/cl_ext.h>
#include <CL/cl_platform.h>
#include <cstddef>
#include <cstdint>
#include <doctest/doctest.h>
#include <vector>

using namespace tinytc;

TEST_CASE("device (OpenCL)") {
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
        if (err == CL_SUCCESS && device_count > 0) {
            device_count = 1;
            CL_CHECK_STATUS(
                clGetDeviceIDs(platforms[p], CL_DEVICE_TYPE_GPU, device_count, &device, NULL));
            break;
        }
    }
    if (device_count == 0) {
        WARN_MESSAGE(false, "Device test needs GPU device");
        return;
    }

    cl_version ip_ver;
    cl_int err =
        clGetDeviceInfo(device, CL_DEVICE_IP_VERSION_INTEL, sizeof(ip_ver), &ip_ver, nullptr);
    if (err == CL_INVALID_VALUE) {
        WARN_MESSAGE(false, "Device test needs Intel GPU");
        return;
    }

    auto info = make_core_info(device);
    std::uint32_t sgs_size;
    std::uint32_t const *sgs;
    info.get_subgroup_sizes(&sgs_size, &sgs);

    if (ip_ver >= static_cast<std::uint32_t>(intel_gpu_architecture::pvc)) {
        REQUIRE(sgs_size == 2);
        CHECK(sgs[0] == 16);
        CHECK(sgs[1] == 32);

        CHECK(info.get_register_space() == 64 * 128);
        info.set_core_features(tinytc_core_feature_flag_large_register_file);
        CHECK(info.get_register_space() == 64 * 256);
    } else if (ip_ver >= static_cast<std::uint32_t>(intel_gpu_architecture::tgl)) {
        REQUIRE(sgs_size == 3);
        CHECK(sgs[0] == 8);
        CHECK(sgs[1] == 16);
        CHECK(sgs[2] == 32);

        CHECK(info.get_register_space() == 32 * 128);
        info.set_core_features(tinytc_core_feature_flag_large_register_file);
        CHECK(info.get_register_space() == 32 * 128);
    } else {
        WARN_MESSAGE(false, "Device test only works on Gen12 / PVC");
    }
}

TEST_CASE("device info helper") {
    static char const subgroups1[] = "cl_intel_subgroups";
    static char const subgroups2[] = "foo cl_khr_subgroups ";
    static char const subgroups3[] = "    foo     cl_khr_subgroups    bar    ";
    static char const subgroups4[] = "cl_khr_ubgroups";
    CHECK(has_subgroup_extension(sizeof(subgroups1), subgroups1));
    CHECK(has_subgroup_extension(sizeof(subgroups2), subgroups2));
    CHECK(has_subgroup_extension(sizeof(subgroups3), subgroups3));
    CHECK(!has_subgroup_extension(sizeof(subgroups4), subgroups4));

    static char const version1[] = "OpenCL 2.0";
    static char const version2[] = "OpenCL 2.0 foobar";
    static char const version3[] = "OCL 1.0";
    static char const version4[] = " OpenCL 3.0";
    static char const version5[] = "OpenCL 42.123";
    opencl_version v;
    v = get_opencl_version(sizeof(version1), version1);
    CHECK(v.major == 2);
    CHECK(v.minor == 0);
    v = get_opencl_version(sizeof(version2), version2);
    CHECK(v.major == 2);
    CHECK(v.minor == 0);
    v = get_opencl_version(sizeof(version3), version3);
    CHECK(v.major == 0);
    CHECK(v.minor == 0);
    v = get_opencl_version(sizeof(version4), version4);
    CHECK(v.major == 0);
    CHECK(v.minor == 0);
    v = get_opencl_version(sizeof(version5), version5);
    CHECK(v.major == 42);
    CHECK(v.minor == 123);
}
