// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SYCL_DEVICE_INFO_20240305_HPP
#define SYCL_DEVICE_INFO_20240305_HPP

#include "tinytc/device_info.hpp"
#include "tinytc/export.h"

#include <cstdint>
#include <memory>
#include <sycl/sycl.hpp>

namespace tinytc {

//! Returns device info for a device
TINYTC_EXPORT auto get_core_info(sycl::device device) -> std::shared_ptr<core_info>;
//! Returns number of cores for a device
TINYTC_EXPORT auto get_number_of_cores(sycl::device device) -> std::uint32_t;

} // namespace tinytc

#endif // SYCL_DEVICE_INFO_20240305_HPP
