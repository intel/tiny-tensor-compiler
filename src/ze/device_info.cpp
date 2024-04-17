// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "../device_info.hpp"
#include "tinytc/tinytc_ze.h"
#include "tinytc/types.h"

#include <level_zero/ze_api.h>
#include <memory>
#include <new>
#include <utility>
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

    try {
        *info = std::make_unique<tinytc::core_info_intel>(
                    dev_ip_ver.ipVersion, dev_props.numEUsPerSubslice, dev_props.numThreadsPerEU,
                    compute_props.maxSharedLocalMemory, std::move(subgroup_sizes))
                    .release();
    } catch (std::bad_alloc const &e) {
        return tinytc_status_bad_alloc;
    } catch (...) {
        return tinytc_status_unknown;
    }
    return tinytc_status_success;
}
}

