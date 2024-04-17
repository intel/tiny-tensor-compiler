// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "../device_info.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc_ze.h"

#include <level_zero/ze_api.h>
#include <vector>

extern "C" {

tinytc_status_t tinytc_ze_core_info_create(tinytc_core_info_t *info, ze_device_handle_t device) {
    if (info == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    auto dev_ip_ver = ze_device_ip_version_ext_t{};
    dev_ip_ver.stype = ZE_STRUCTURE_TYPE_DEVICE_IP_VERSION_EXT;
    dev_ip_ver.pNext = nullptr;
    auto dev_props = ze_device_properties_t{};
    dev_props.stype = ZE_STRUCTURE_TYPE_DEVICE_PROPERTIES;
    dev_props.pNext = &dev_ip_ver;
    TINYTC_ZE_CHECK(zeDeviceGetProperties(device, &dev_props));
    auto compute_props = ze_device_compute_properties_t{};
    compute_props.stype = ZE_STRUCTURE_TYPE_DEVICE_COMPUTE_PROPERTIES;
    compute_props.pNext = nullptr;
    TINYTC_ZE_CHECK(zeDeviceGetComputeProperties(device, &compute_props));

    auto subgroup_sizes = std::vector<std::uint32_t>(
        compute_props.subGroupSizes, compute_props.subGroupSizes + compute_props.numSubGroupSizes);

    TINYTC_CHECK(tinytc_core_info_intel_create(
        info, dev_ip_ver.ipVersion, dev_props.numEUsPerSubslice, dev_props.numThreadsPerEU,
        compute_props.maxSharedLocalMemory, subgroup_sizes.size(), subgroup_sizes.data()));
    return tinytc_status_success;
}
}

