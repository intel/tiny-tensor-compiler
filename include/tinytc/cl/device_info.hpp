// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CL_DEVICE_INFO_20240307_HPP
#define CL_DEVICE_INFO_20240307_HPP

#include "tinytc/export.hpp"

#include <CL/cl.h>
#include <cstdint>
#include <memory>

namespace tinytc {

class core_info;

//! Returns device info for a device
TINYTC_EXPORT auto get_core_info(cl_device_id device) -> std::shared_ptr<core_info>;
//! Returns number of cores for a device
TINYTC_EXPORT auto get_number_of_cores(cl_device_id device) -> std::uint32_t;

} // namespace tinytc

#endif // CL_DEVICE_INFO_20240307_HPP
