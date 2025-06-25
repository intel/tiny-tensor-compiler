// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/tinytc.hpp"
#include "tinytc/tinytc_ze.hpp"
#include "tinytc/types.h"

#include <level_zero/ze_api.h>

#include <cstdint>
#include <exception>
#include <iomanip>
#include <iostream>
#include <type_traits>
#include <vector>

namespace tinytc {
enum class spirv_feature;
enum class status;
} // namespace tinytc

using namespace tinytc;

constexpr int width = 30;

int main() {
    try {
        ZE_CHECK_STATUS(zeInit(0));

        std::uint32_t driver_count = 0;
        ZE_CHECK_STATUS(zeDriverGet(&driver_count, nullptr));
        auto drivers = std::vector<ze_driver_handle_t>(driver_count);
        ZE_CHECK_STATUS(zeDriverGet(&driver_count, drivers.data()));

        std::int32_t driver_no = 0;
        for (auto &driver : drivers) {
            ze_api_version_t version;
            ZE_CHECK_STATUS(zeDriverGetApiVersion(driver, &version));
            std::cout << "Driver " << driver_no++ << ": v" << ZE_MAJOR_VERSION(version) << "."
                      << ZE_MINOR_VERSION(version) << std::endl;

            std::uint32_t device_count = 0;
            ZE_CHECK_STATUS(zeDeviceGet(driver, &device_count, nullptr));
            auto devices = std::vector<ze_device_handle_t>(device_count);
            ZE_CHECK_STATUS(zeDeviceGet(driver, &device_count, devices.data()));

            std::int32_t device_no = 0;
            for (auto &device : devices) {
                ze_device_properties_t props;
                ZE_CHECK_STATUS(zeDeviceGetProperties(device, &props));
                std::cout << "\tDevice " << device_no++ << ": " << props.name << std::endl;
                auto info = make_core_info(device);
                std::cout << "\t\t" << "Subgroup sizes  :";
                for (auto sgs : info.get_subgroup_sizes()) {
                    std::cout << " " << sgs;
                }
                std::cout << std::endl;
                std::cout << "\t\tRegister space  : " << info.get_register_space() << std::endl;
                std::cout << "\t\tSPIR-V features : " << std::endl;
                for (int i = 0; i < TINYTC_ENUM_NUM_SPIRV_FEATURE; ++i) {
                    const auto feature = spirv_feature{std::underlying_type_t<spirv_feature>(i)};
                    std::cout << "\t\t\t" << std::setw(width) << std::left << to_string(feature)
                              << ": ";
                    std::cout << (info.have_spirv_feature(feature) ? "yes" : "no") << std::endl;
                }
            }
        }

    } catch (status const &st) {
        std::cerr << "Error (" << static_cast<int>(st) << "): " << error_string(st) << std::endl;
        return 1;
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
