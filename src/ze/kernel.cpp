// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "../compiler_options.hpp"
#include "../support/util.hpp"
#include "error.hpp"
#include "opencl_cc.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc.hpp"
#include "tinytc/tinytc_ze.h"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <level_zero/ze_api.h>
#include <string>
#include <vector>

using namespace tinytc;

extern "C" {

tinytc_status_t tinytc_ze_source_compile_to_binary(tinytc_binary_t *bin, const_tinytc_source_t src,
                                                   uint32_t ip_version,
                                                   tinytc_bundle_format_t format,
                                                   tinytc_source_context_t source_ctx) {

    if (bin == nullptr || src == nullptr) {
        return tinytc_status_invalid_arguments;
    }

    size_t code_size = 0;
    char const *code = nullptr;
    tinytc_core_feature_flags_t core_features = 0;
    std::uint32_t extensions_size = 0;
    char const *const *extensions = nullptr;
    TINYTC_CHECK_STATUS(tinytc_source_get_code(src, &code_size, &code));
    TINYTC_CHECK_STATUS(tinytc_source_get_core_features(src, &core_features));
    TINYTC_CHECK_STATUS(tinytc_source_get_extensions(src, &extensions_size, &extensions));

    return exception_to_status_code_ze(
        [&] {
            auto compiler_options =
                std::vector(default_compiler_options.begin(), default_compiler_options.end());
            if (core_features & tinytc_core_feature_flag_large_register_file) {
                compiler_options.push_back(large_register_file_compiler_option_ze);
            }
            auto fmt = enum_cast<bundle_format>(format);
            auto bin_data =
                compile_opencl_c(code_size, code, fmt, ip_version, compiler_options.size(),
                                 compiler_options.data(), extensions_size, extensions);
            CHECK_STATUS(
                tinytc_binary_create(bin, format, bin_data.size(), bin_data.data(), core_features));
        },
        source_ctx);
}

tinytc_status_t tinytc_ze_kernel_bundle_create_with_source(ze_module_handle_t *bundle,
                                                           ze_context_handle_t context,
                                                           ze_device_handle_t device,
                                                           const_tinytc_source_t src,
                                                           tinytc_source_context_t source_ctx) {
    if (bundle == nullptr || src == nullptr) {
        return tinytc_status_invalid_arguments;
    }

    // Get IP version
    auto dev_ip_ver = ze_device_ip_version_ext_t{};
    dev_ip_ver.stype = ZE_STRUCTURE_TYPE_DEVICE_IP_VERSION_EXT;
    dev_ip_ver.pNext = nullptr;
    auto dev_props = ze_device_properties_t{};
    dev_props.stype = ZE_STRUCTURE_TYPE_DEVICE_PROPERTIES;
    dev_props.pNext = &dev_ip_ver;
    TINYTC_ZE_CHECK_STATUS(zeDeviceGetProperties(device, &dev_props));

    // Get binary
    tinytc_binary_t bin = nullptr;
    TINYTC_CHECK_STATUS(tinytc_ze_source_compile_to_binary(
        &bin, src, dev_ip_ver.ipVersion, tinytc_bundle_format_native, source_ctx));

    tinytc_status_t status =
        tinytc_ze_kernel_bundle_create_with_binary(bundle, context, device, bin, source_ctx);
    tinytc_binary_release(bin);
    return status;
}

tinytc_status_t
tinytc_ze_kernel_bundle_create_with_program(ze_module_handle_t *bundle, ze_context_handle_t context,
                                            ze_device_handle_t device, tinytc_prog_t prg,
                                            tinytc_core_feature_flags_t core_features,
                                            tinytc_source_context_t source_ctx) {
    if (bundle == nullptr || prg == nullptr) {
        return tinytc_status_invalid_arguments;
    }

    tinytc_core_info_t info = nullptr;
    tinytc_source_t src = nullptr;
    tinytc_status_t status = tinytc_status_success;

    if (status = tinytc_ze_core_info_create(&info, device); status != tinytc_status_success) {
        goto err;
    }
    if (status = tinytc_core_info_set_core_features(info, core_features);
        status != tinytc_status_success) {
        goto err;
    }
    if (status = tinytc_prog_compile_to_opencl(&src, prg, info, source_ctx);
        status != tinytc_status_success) {
        goto err;
    }
    if (status =
            tinytc_ze_kernel_bundle_create_with_source(bundle, context, device, src, source_ctx);
        status != tinytc_status_success) {
        goto err;
    }
err:
    tinytc_source_release(src);
    tinytc_core_info_release(info);

    return status;
}

tinytc_status_t tinytc_ze_kernel_bundle_create_with_binary(ze_module_handle_t *bundle,
                                                           ze_context_handle_t context,
                                                           ze_device_handle_t device,
                                                           const_tinytc_binary_t bin,
                                                           tinytc_source_context_t source_ctx) {
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
        module_desc.pBuildFlags = large_register_file_compiler_option_ze;
    }
    ze_module_build_log_handle_t build_log;
    ze_result_t status = zeModuleCreate(context, device, &module_desc, bundle, &build_log);
    if (status != ZE_RESULT_SUCCESS) {
        if (source_ctx) {
            std::string log;
            std::size_t log_size;
            zeModuleBuildLogGetString(build_log, &log_size, nullptr);
            log.resize(log_size);
            zeModuleBuildLogGetString(build_log, &log_size, log.data());

            tinytc_location_t loc = {};
            tinytc_source_context_report_error(source_ctx, &loc, log.c_str(), true);
        }
        zeModuleBuildLogDestroy(build_log);
        TINYTC_ZE_CHECK_STATUS(status);
    } else {
        TINYTC_ZE_CHECK_STATUS(zeModuleBuildLogDestroy(build_log));
    }
    return tinytc_status_success;
}

tinytc_status_t tinytc_ze_kernel_create(ze_kernel_handle_t *krnl, ze_module_handle_t mod,
                                        char const *name) {
    if (krnl == nullptr || name == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    uint32_t x, y, z;
    ze_kernel_desc_t kernel_desc = {ZE_STRUCTURE_TYPE_KERNEL_DESC, nullptr, 0, name};
    TINYTC_ZE_CHECK_STATUS(zeKernelCreate(mod, &kernel_desc, krnl));
    TINYTC_CHECK_STATUS(tinytc_ze_get_group_size(*krnl, &x, &y, &z));
    TINYTC_ZE_CHECK_STATUS(zeKernelSetGroupSize(*krnl, x, y, z));
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

ze_group_count_t tinytc_ze_get_group_count(int64_t howmany) {
    return ze_group_count_t{1u, 1u, static_cast<uint32_t>(howmany)};
}
}
