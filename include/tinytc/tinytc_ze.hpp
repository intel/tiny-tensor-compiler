// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TINYTC_ZE_20240416_HPP
#define TINYTC_ZE_20240416_HPP

#include "tinytc/tinytc.hpp"
#include "tinytc/tinytc_ze.h"

#include <array>
#include <level_zero/ze_api.h>
#include <type_traits>

namespace tinytc {

////////////////////////////
/////////// Error //////////
////////////////////////////

//! Throw exception for unsuccessful call to C-API and convert result code to tinytc status
inline void ZE_CHECK_STATUS(ze_result_t result) {
    if (result != ZE_RESULT_SUCCESS) {
        throw status{std::underlying_type_t<status>(::tinytc_ze_convert_status(result))};
    }
}

////////////////////////////
//////// Device info ///////
////////////////////////////

/**
 * @brief Get support level of Level Zero device
 *
 * @param device Device handle
 *
 * @return Support level
 */
inline auto get_support_level(ze_device_handle_t device) -> support_level {
    tinytc_support_level_t level;
    CHECK_STATUS(::tinytc_ze_get_support_level(device, &level));
    return support_level{std::underlying_type_t<support_level>(level)};
}

/**
 * @brief Query core info from level zero runtime
 *
 * @param device device handle
 *
 * @return core info
 */
inline auto make_core_info(ze_device_handle_t device) -> core_info {
    tinytc_core_info_t info;
    CHECK_STATUS(::tinytc_ze_core_info_create(&info, device));
    return core_info{info};
}

////////////////////////////
////////// Kernel //////////
////////////////////////////

/**
 * @brief Compile source to binary
 *
 * @param src Source object
 * @param ip_version IP version (pass tinytc_intel_gpu_architecture_t here)
 * @param format Bundle format (SPIR-V or Native)
 * @param ctx Source context for improved error reporting
 *
 * @return Binary
 */
inline auto compile_to_binary(source const &src, std::uint32_t ip_version, bundle_format format,
                              source_context ctx = {}) -> binary {
    tinytc_binary_t bin;
    CHECK_STATUS(tinytc_ze_source_compile_to_binary(
        &bin, src.get(), ip_version, static_cast<tinytc_bundle_format_t>(format), ctx.get()));
    return binary{bin};
}

namespace internal {
template <> struct unique_handle_traits<ze_kernel_handle_t> {
    static void destroy(ze_kernel_handle_t obj) { zeKernelDestroy(obj); }
};
template <> struct unique_handle_traits<ze_module_handle_t> {
    static void destroy(ze_module_handle_t obj) { zeModuleDestroy(obj); }
};
} // namespace internal

/**
 * @brief Make a Level Zero module from a tinytc source
 *
 * @param context Context
 * @param device Device
 * @param src Source
 * @param source_ctx Source context for improved error reporting
 *
 * @return Level Zero module (unique handle)
 */
inline auto make_kernel_bundle(ze_context_handle_t context, ze_device_handle_t device,
                               source const &src, source_context source_ctx = {})
    -> unique_handle<ze_module_handle_t> {
    ze_module_handle_t obj;
    CHECK_STATUS(tinytc_ze_kernel_bundle_create_with_source(&obj, context, device, src.get(),
                                                            source_ctx.get()));
    return unique_handle<ze_module_handle_t>{obj};
}

/**
 * @brief Make a Level Zero module from a tinytc program
 *
 * @param context Context
 * @param device Device
 * @param prg Program
 * @param core_features requested core features; must be 0 (default) or a combination of
 * tinytc_core_feature_flag_t
 * @param source_ctx Source context for improved error reporting
 *
 * @return Level Zero module (unique handle)
 */
inline auto make_kernel_bundle(ze_context_handle_t context, ze_device_handle_t device, prog prg,
                               tinytc_core_feature_flags_t core_features = 0,
                               source_context source_ctx = {})
    -> unique_handle<ze_module_handle_t> {
    ze_module_handle_t obj;
    CHECK_STATUS(tinytc_ze_kernel_bundle_create_with_program(&obj, context, device, prg.get(),
                                                             core_features, source_ctx.get()));
    return unique_handle<ze_module_handle_t>{obj};
}

/**
 * @brief Make a Level Zero module from a tinytc binary
 *
 * @param context Context
 * @param device Device
 * @param bin Binary
 * @param source_ctx Source context for improved error reporting
 *
 * @return Level Zero module (unique handle)
 */
inline auto make_kernel_bundle(ze_context_handle_t context, ze_device_handle_t device,
                               binary const &bin, source_context source_ctx = {})
    -> unique_handle<ze_module_handle_t> {
    ze_module_handle_t obj;
    CHECK_STATUS(tinytc_ze_kernel_bundle_create_with_binary(&obj, context, device, bin.get(),
                                                            source_ctx.get()));
    return unique_handle<ze_module_handle_t>{obj};
}

/**
 * @brief Make a Level Zero kernel and set its group size
 *
 * @param mod Module
 * @param name Kernel name
 *
 * @return Level Zero kernel (unique handle)
 */
inline auto make_kernel(ze_module_handle_t mod, char const *name)
    -> unique_handle<ze_kernel_handle_t> {
    ze_kernel_handle_t obj;
    CHECK_STATUS(tinytc_ze_kernel_create(&obj, mod, name));
    return unique_handle<ze_kernel_handle_t>{obj};
}

/**
 * @brief Get work group size
 *
 * @param kernel kernel
 *
 * @return Work-group size
 */
inline auto get_group_size(ze_kernel_handle_t kernel) -> std::array<std::uint32_t, 3u> {
    std::array<std::uint32_t, 3u> group_size;
    CHECK_STATUS(tinytc_ze_get_group_size(kernel, &group_size[0], &group_size[1], &group_size[2]));
    return group_size;
}

/**
 * @brief Convert group size to Level Zero group count
 *
 * @param howmany Group size
 *
 * @return Group count
 */
inline auto get_group_count(std::uint32_t howmany) -> ze_group_count_t {
    return tinytc_ze_get_group_count(howmany);
}

////////////////////////////
////////// Recipe //////////
////////////////////////////

/**
 * @brief Recipe handler for the Level Zero runtime
 */
class level_zero_recipe_handler : public recipe_handler {
  public:
    using recipe_handler::recipe_handler;

    /**
     * @brief Append recipe to command list
     *
     * Cf. @ref tinytc_ze_recipe_handler_submit
     *
     * @param list Command list
     * @param signal_event Event to be signalled on completetion
     * @param num_wait_events Number of wait events to wait on
     * @param wait_events Array of num_wait_events events to wait on
     */
    inline void submit(ze_command_list_handle_t list, ze_event_handle_t signal_event = nullptr,
                       uint32_t num_wait_events = 0, ze_event_handle_t *wait_events = nullptr) {
        CHECK_STATUS(tinytc_ze_recipe_handler_submit(obj_, list, signal_event, num_wait_events,
                                                     wait_events));
    }
};

/**
 * @brief Make recipe handler
 *
 * @param context Context
 * @param device Device
 * @param rec Recipe
 * @param source_ctx Source context for improved error reporting
 *
 * @return Level Zero recipe handler
 */
inline auto make_recipe_handler(ze_context_handle_t context, ze_device_handle_t device,
                                recipe const &rec, source_context source_ctx = {})
    -> level_zero_recipe_handler {
    tinytc_recipe_handler_t handler;
    CHECK_STATUS(
        tinytc_ze_recipe_handler_create(&handler, context, device, rec.get(), source_ctx.get()));
    return level_zero_recipe_handler{handler};
}

} // namespace tinytc

#endif // TINYTC_ZE_20240416_HPP
