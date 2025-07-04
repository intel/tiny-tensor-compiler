// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "device_info_helper.hpp"
#include "tinytc/core.hpp"
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
    const auto sgs = get_subgroup_sizes(info.get());

    if (ip_ver >= static_cast<std::uint32_t>(intel_gpu_architecture::pvc)) {
        REQUIRE(sgs.size() == 2u);
        CHECK(sgs[0] == 16);
        CHECK(sgs[1] == 32);

        CHECK(get_register_space(info.get()) == 64 * 128);
        set_core_features(info.get(), tinytc_core_feature_flag_large_register_file);
        CHECK(get_register_space(info.get()) == 64 * 256);
    } else if (ip_ver >= static_cast<std::uint32_t>(intel_gpu_architecture::tgl)) {
        REQUIRE(sgs.size() == 3u);
        CHECK(sgs[0] == 8);
        CHECK(sgs[1] == 16);
        CHECK(sgs[2] == 32);

        CHECK(get_register_space(info.get()) == 32 * 128);
        set_core_features(info.get(), tinytc_core_feature_flag_large_register_file);
        CHECK(get_register_space(info.get()) == 32 * 128);
    } else {
        WARN_MESSAGE(false, "Device test only works on Gen12 / PVC");
    }
}

TEST_CASE("device info helper") {
    static char const subgroups1[] = "cl_intel_subgroups";
    static char const subgroups2[] = "foo cl_khr_subgroups ";
    static char const subgroups3[] = "    foo     cl_khr_subgroups    bar    ";
    static char const subgroups4[] = "cl_khr_ubgroups";
    static char const subgroups5[] = "cl_intel_subgroups cl_khr_fp16";

    CHECK(get_opencl_extensions(sizeof(subgroups1) - 1, subgroups1) ==
          opencl_ext_cl_intel_subgroups);
    CHECK(get_opencl_extensions(sizeof(subgroups2) - 1, subgroups2) == opencl_ext_cl_khr_subgroups);
    CHECK(get_opencl_extensions(sizeof(subgroups3) - 1, subgroups3) == opencl_ext_cl_khr_subgroups);
    CHECK(get_opencl_extensions(sizeof(subgroups4) - 1, subgroups4) == 0);
    const auto ocl_exts5 = get_opencl_extensions(sizeof(subgroups5) - 1, subgroups5);
    CHECK(ocl_exts5 == (opencl_ext_cl_intel_subgroups | opencl_ext_cl_khr_fp16));

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
