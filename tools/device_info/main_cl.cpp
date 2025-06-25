// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/tinytc.hpp"
#include "tinytc/tinytc_cl.hpp"
#include "tinytc/types.h"

#include <CL/cl.h>
#include <CL/cl_platform.h>

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
        cl_uint platform_count = 0;
        CL_CHECK_STATUS(clGetPlatformIDs(platform_count, nullptr, &platform_count));
        auto platforms = std::vector<cl_platform_id>(platform_count);
        CL_CHECK_STATUS(clGetPlatformIDs(platform_count, platforms.data(), nullptr));

        std::int32_t platform_no = 0;
        for (auto &platform : platforms) {
            char version[128];
            CL_CHECK_STATUS(clGetPlatformInfo(platform, CL_PLATFORM_VERSION, sizeof(version),
                                              version, nullptr));
            std::cout << "Driver " << platform_no++ << ": " << version << std::endl;

            cl_uint device_count = 0;
            CL_CHECK_STATUS(
                clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, device_count, nullptr, &device_count));
            auto devices = std::vector<cl_device_id>(device_count);
            CL_CHECK_STATUS(clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, device_count,
                                           devices.data(), nullptr));

            std::int32_t device_no = 0;
            for (auto &device : devices) {
                char name[256];
                cl_device_type dev_type;
                CL_CHECK_STATUS(
                    clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(dev_type), &dev_type, nullptr));
                CL_CHECK_STATUS(
                    clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(name), name, nullptr));
                std::cout << "\tDevice " << device_no++;
                switch (dev_type) {
                case CL_DEVICE_TYPE_CPU:
                    std::cout << " (CPU)";
                    break;
                case CL_DEVICE_TYPE_GPU:
                    std::cout << " (GPU)";
                    break;
                case CL_DEVICE_TYPE_ACCELERATOR:
                    std::cout << " (ACCELERATOR)";
                    break;
                case CL_DEVICE_TYPE_CUSTOM:
                    std::cout << " (CUSTOM)";
                    break;
                }
                std::cout << ": " << name << std::endl;
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
