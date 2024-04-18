// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "../compiler_options.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc_ze.h"

#include <cstdint>
#include <cstdio>
#include <level_zero/ze_api.h>
#include <stdexcept>
#include <string>

extern "C" {

tinytc_status_t tinytc_ze_module_create(ze_module_handle_t *mod, ze_context_handle_t context,
                                        ze_device_handle_t device, tinytc_binary_t bin,
                                        ze_module_build_log_handle_t *build_log) {
    if (bin == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    auto const zformat = [](tinytc_bundle_format_t format) -> ze_module_format_t {
        if (format == tinytc_bundle_format_native) {
            return ZE_MODULE_FORMAT_NATIVE;
        }
        return ZE_MODULE_FORMAT_IL_SPIRV;
    };

    tinytc_bundle_format_t format;
    uint64_t data_size;
    uint8_t const *data;
    TINYTC_CHECK_STATUS(tinytc_binary_get_raw(bin, &format, &data_size, &data));
    ze_module_desc_t module_desc = {
        ZE_STRUCTURE_TYPE_MODULE_DESC, nullptr, zformat(format), data_size, data, nullptr, nullptr};

    uint32_t core_features;
    TINYTC_CHECK_STATUS(tinytc_binary_get_core_features(bin, &core_features));

    if (core_features & static_cast<std::uint32_t>(tinytc_core_feature_flag_large_register_file)) {
        module_desc.pBuildFlags = tinytc::large_register_file_compiler_option_ze;
    }
    TINYTC_ZE_CHECK_STATUS(zeModuleCreate(context, device, &module_desc, mod, build_log));
    return tinytc_status_success;
}

tinytc_status_t tinytc_ze_get_group_size(ze_kernel_handle_t kernel, uint32_t *x, uint32_t *y,
                                         uint32_t *z) {
    if (x == nullptr || y == nullptr || z == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    auto props = ze_kernel_properties_t{};
    props.stype = ZE_STRUCTURE_TYPE_KERNEL_PROPERTIES;
    props.pNext = nullptr;
    auto status = zeKernelGetProperties(kernel, &props);
    *x = props.requiredGroupSizeX;
    *y = props.requiredGroupSizeY;
    *z = props.requiredGroupSizeZ;
    return tinytc_ze_convert_status(status);
}

ze_group_count_t tinytc_ze_get_group_count(uint32_t howmany) {
    return ze_group_count_t{1u, 1u, howmany};
}

} // namespace tinytc
