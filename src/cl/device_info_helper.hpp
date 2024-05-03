// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CL_DEVICE_INFO_HELPER_20240503_HPP
#define CL_DEVICE_INFO_HELPER_20240503_HPP

#include <cstddef>

namespace tinytc {

bool has_subgroups_extension(std::size_t str_length, const char *str);

} // namespace tinytc

#endif // CL_DEVICE_INFO_HELPER_20240503_HPP
