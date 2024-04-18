// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include <doctest/doctest.h>
#include <level_zero/ze_api.h>
#include <tinytc/tinytc.hpp>
#include <tinytc/tinytc_ze.hpp>

#include <cstdint>
#include <memory>
#include <vector>

using namespace tinytc;

TEST_CASE("device") {
    ZE_CHECK_STATUS(zeInit(0));

    std::uint32_t num_drivers = 1;
    ze_driver_handle_t driver;
    ZE_CHECK_STATUS(zeDriverGet(&num_drivers, &driver));

    std::uint32_t num_devices = 1;
    ze_device_handle_t device;

    ZE_CHECK_STATUS(zeDeviceGet(driver, &num_devices, &device));

    auto dev_ip_ver = ze_device_ip_version_ext_t{};
    dev_ip_ver.stype = ZE_STRUCTURE_TYPE_DEVICE_IP_VERSION_EXT;
    dev_ip_ver.pNext = nullptr;
    auto dev_props = ze_device_properties_t{};
    dev_props.stype = ZE_STRUCTURE_TYPE_DEVICE_PROPERTIES;
    dev_props.pNext = &dev_ip_ver;
    ZE_CHECK_STATUS(zeDeviceGetProperties(device, &dev_props));

    if (dev_ip_ver.ipVersion >= static_cast<std::uint32_t>(intel_gpu_architecture::pvc)) {
        auto info = create_core_info(device);

        std::uint32_t sgs_size;
        std::uint32_t const *sgs;
        info.get_subgroup_sizes(&sgs_size, &sgs);
        REQUIRE(sgs_size == 2);
        CHECK(sgs[0] == 16);
        CHECK(sgs[1] == 32);

        CHECK(info.get_num_registers_per_thread() == 128);
        info.set_core_feature(core_feature_flag::large_register_file);
        CHECK(info.get_num_registers_per_thread() == 256);
    } else {
        WARN_MESSAGE(false, "Device test only works on PVC");
    }
}
