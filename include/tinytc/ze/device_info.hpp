// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ZE_DEVICE_INFO_20240307_HPP
#define ZE_DEVICE_INFO_20240307_HPP

#include "tinytc/export.hpp"

#include <cstdint>
#include <level_zero/ze_api.h>
#include <memory>

namespace tinytc {

class core_info;

//! Returns core info for a device
TINYTC_EXPORT auto get_core_info(ze_device_handle_t device) -> std::shared_ptr<core_info>;
//! Returns number of cores for a device
TINYTC_EXPORT auto get_number_of_cores(ze_device_handle_t device) -> std::uint32_t;

} // namespace tinytc

#endif // ZE_DEVICE_INFO_20240307_HPP
