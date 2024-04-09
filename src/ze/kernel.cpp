// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/ze/kernel.hpp"
#include "tinytc/bundle_format.hpp"
#include "tinytc/device_info.hpp"
#include "tinytc/internal/compiler_options.hpp"
#include "tinytc/ze/error.hpp"

#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <string>

namespace tinytc {

auto make_kernel_bundle(std::uint8_t const *binary, std::size_t binary_size, bundle_format format,
                        std::uint32_t core_features, ze_context_handle_t context,
                        ze_device_handle_t device) -> ze_module_handle_t {
    auto const zformat = [](bundle_format format) -> ze_module_format_t {
        switch (format) {
        case bundle_format::spirv:
            return ZE_MODULE_FORMAT_IL_SPIRV;
        case bundle_format::native:
            return ZE_MODULE_FORMAT_NATIVE;
        default:
            break;
        }
        throw std::logic_error("Unknown module format");
    };

    ze_module_handle_t mod;
    ze_module_desc_t module_desc = {ZE_STRUCTURE_TYPE_MODULE_DESC,
                                    nullptr,
                                    zformat(format),
                                    binary_size,
                                    binary,
                                    nullptr,
                                    nullptr};
    if (core_features & static_cast<std::uint32_t>(core_feature_flag::large_register_file)) {
        module_desc.pBuildFlags = internal::large_register_file_compiler_option_ze;
    }
    ze_result_t err;
    ze_module_build_log_handle_t build_log;
    err = zeModuleCreate(context, device, &module_desc, &mod, &build_log);
    if (err != ZE_RESULT_SUCCESS) {
        std::string log;
        std::size_t log_size;
        ZE_CHECK(zeModuleBuildLogGetString(build_log, &log_size, nullptr));
        log.resize(log_size);
        ZE_CHECK(zeModuleBuildLogGetString(build_log, &log_size, log.data()));
        char what[256];
        snprintf(what, sizeof(what), "zeModuleCreate returned %s (%d).\n", ze_result_to_string(err),
                 err);
        throw level_zero_error(std::string(what) + log, err);
    }
    ZE_CHECK(zeModuleBuildLogDestroy(build_log));

    return mod;
}

auto make_kernel(ze_module_handle_t mod, char const *name) -> ze_kernel_handle_t {
    ze_kernel_desc_t kernel_desc = {ZE_STRUCTURE_TYPE_KERNEL_DESC, nullptr, 0, name};
    ze_kernel_handle_t kernel;
    ZE_CHECK(zeKernelCreate(mod, &kernel_desc, &kernel));
    return kernel;
}

auto get_group_count(std::uint32_t howmany) -> ze_group_count_t { return {1, 1, howmany}; }

void level_zero_argument_handler::set_arg(ze_kernel_handle_t kernel, std::uint32_t arg_index,
                                          std::size_t arg_size, void const *arg_value) {
    ZE_CHECK(zeKernelSetArgumentValue(kernel, arg_index, arg_size, arg_value));
}

} // namespace tinytc
