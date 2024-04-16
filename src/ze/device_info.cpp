// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "../device_info.hpp"
#include "error.hpp"
#include "tinytc/tinytc_ze.h"

#include <memory>
#include <utility>
#include <vector>

extern "C" {

ze_result_t tinytc_ze_core_info_create(tinytc_core_info_t *info, ze_device_handle_t device) {
    if (info == nullptr) {
        return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
    }
    return tinytc::exception_to_ze_result([&] {
        auto dev_ip_ver = ze_device_ip_version_ext_t{};
        dev_ip_ver.stype = ZE_STRUCTURE_TYPE_DEVICE_IP_VERSION_EXT;
        dev_ip_ver.pNext = nullptr;
        auto dev_props = ze_device_properties_t{};
        dev_props.stype = ZE_STRUCTURE_TYPE_DEVICE_PROPERTIES;
        dev_props.pNext = &dev_ip_ver;
        if (auto status = zeDeviceGetProperties(device, &dev_props); status != ZE_RESULT_SUCCESS) {
            return status;
        }

        auto compute_props = ze_device_compute_properties_t{};
        compute_props.stype = ZE_STRUCTURE_TYPE_DEVICE_COMPUTE_PROPERTIES;
        compute_props.pNext = nullptr;
        if (auto status = zeDeviceGetComputeProperties(device, &compute_props);
            status != ZE_RESULT_SUCCESS) {
            return status;
        }

        auto subgroup_sizes = std::vector<std::uint32_t>(compute_props.subGroupSizes,
                                                         compute_props.subGroupSizes +
                                                             compute_props.numSubGroupSizes);

        *info = std::make_unique<tinytc::core_info_intel>(
                    dev_ip_ver.ipVersion, dev_props.numEUsPerSubslice, dev_props.numThreadsPerEU,
                    compute_props.maxSharedLocalMemory, std::move(subgroup_sizes))
                    .release();
        return ZE_RESULT_SUCCESS;
    });
}
}

