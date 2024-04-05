// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/ze/device_info.hpp"
#include "tinytc/device_info.hpp"
#include "tinytc/ze/error.hpp"

#include <utility>
#include <vector>

namespace tinytc {

auto get_core_info(ze_device_handle_t device) -> std::shared_ptr<core_info> {
    auto dev_ip_ver = ze_device_ip_version_ext_t{};
    dev_ip_ver.stype = ZE_STRUCTURE_TYPE_DEVICE_IP_VERSION_EXT;
    dev_ip_ver.pNext = nullptr;
    auto dev_props = ze_device_properties_t{};
    dev_props.stype = ZE_STRUCTURE_TYPE_DEVICE_PROPERTIES;
    dev_props.pNext = &dev_ip_ver;
    ZE_CHECK(zeDeviceGetProperties(device, &dev_props));

    auto compute_props = ze_device_compute_properties_t{};
    compute_props.stype = ZE_STRUCTURE_TYPE_DEVICE_COMPUTE_PROPERTIES;
    compute_props.pNext = nullptr;
    ZE_CHECK(zeDeviceGetComputeProperties(device, &compute_props));

    auto subgroup_sizes = std::vector<std::uint32_t>(
        compute_props.subGroupSizes, compute_props.subGroupSizes + compute_props.numSubGroupSizes);

    return std::make_shared<core_info_intel>(
        dev_ip_ver.ipVersion, dev_props.numEUsPerSubslice, dev_props.numThreadsPerEU,
        compute_props.maxSharedLocalMemory, std::move(subgroup_sizes));
}

auto get_number_of_cores(ze_device_handle_t device) -> std::uint32_t {
    auto dev_props = ze_device_properties_t{};
    dev_props.stype = ZE_STRUCTURE_TYPE_DEVICE_PROPERTIES;
    dev_props.pNext = nullptr;
    ZE_CHECK(zeDeviceGetProperties(device, &dev_props));

    return dev_props.numSubslicesPerSlice * dev_props.numSlices;
}

} // namespace tinytc

