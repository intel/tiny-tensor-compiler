// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CL_DEVICE_INFO_HELPER_20240503_HPP
#define CL_DEVICE_INFO_HELPER_20240503_HPP

#include <cstddef>

namespace tinytc {

struct opencl_version {
    int major;
    int minor;
};

bool has_subgroup_extension(std::size_t str_length, const char *str);
bool has_additional_subgroup_extensions(std::size_t str_length, const char *str);
auto get_opencl_version(std::size_t str_length, const char *str) -> opencl_version;

} // namespace tinytc

#endif // CL_DEVICE_INFO_HELPER_20240503_HPP
