// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CL_DEVICE_INFO_HELPER_20240503_HPP
#define CL_DEVICE_INFO_HELPER_20240503_HPP

#include "tinytc/tinytc_cl.hpp"

#include <CL/cl.h>

#include <cstddef>
#include <cstdint>
#include <string>

namespace tinytc {

struct opencl_version {
    int major;
    int minor;
};

enum opencl_ext_t {
    opencl_ext_cl_khr_fp16 = 0x1,
    opencl_ext_cl_khr_fp64 = 0x2,
    opencl_ext_cl_khr_subgroups = 0x4,
    opencl_ext_cl_intel_subgroups = 0x8,
    opencl_ext_cl_intel_required_subgroup_size = 0x10,
    opencl_ext_cl_intel_subgroups_long = 0x20,
    opencl_ext_cl_intel_subgroups_short = 0x40,
    opencl_ext_cl_intel_spirv_subgroups = 0x80,
    opencl_ext_cl_khr_int64_base_atomics = 0x100,
    opencl_ext_cl_khr_int64_extended_atomics = 0x200,
    opencl_ext_cl_ext_float_atomics = 0x400,
};
//! Type for combination of core feature flags
using opencl_exts_t = std::uint32_t;

auto get_opencl_extensions(std::size_t str_length, const char *str) -> opencl_exts_t;
auto get_opencl_extensions(cl_device_id device) -> opencl_exts_t;
auto get_opencl_version(std::size_t str_length, const char *str) -> opencl_version;
auto get_opencl_version(cl_device_id device) -> opencl_version;

template <typename T> auto device_info(cl_device_id device, cl_device_info param_name) -> T {
    T val = {};
    CL_CHECK_STATUS(clGetDeviceInfo(device, param_name, sizeof(T), &val, nullptr));
    return val;
}

template <>
inline auto device_info<std::string>(cl_device_id device,
                                     cl_device_info param_name) -> std::string {
    std::string str;
    std::size_t str_len;
    CL_CHECK_STATUS(clGetDeviceInfo(device, param_name, 0, nullptr, &str_len));
    str.resize(str_len);
    CL_CHECK_STATUS(clGetDeviceInfo(device, param_name, str_len, str.data(), nullptr));
    return str;
}

} // namespace tinytc

#endif // CL_DEVICE_INFO_HELPER_20240503_HPP
