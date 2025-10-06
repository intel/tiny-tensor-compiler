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
        throw status::compute_runtime_error;
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
inline auto create_core_info(ze_device_handle_t device) -> shared_handle<tinytc_core_info_t> {
    tinytc_core_info_t info;
    CHECK_STATUS(::tinytc_ze_core_info_create(&info, device));
    return shared_handle{info};
}

////////////////////////////
////////// Kernel //////////
////////////////////////////

namespace internal {
template <> struct unique_handle_traits<ze_kernel_handle_t> {
    static void destroy(ze_kernel_handle_t obj) { zeKernelDestroy(obj); }
};
template <> struct unique_handle_traits<ze_module_handle_t> {
    static void destroy(ze_module_handle_t obj) { zeModuleDestroy(obj); }
};
} // namespace internal

/**
 * @brief Make a Level Zero module from a tinytc program
 *
 * @param context Context
 * @param device Device
 * @param prg Program
 * @param core_features requested core features; must be 0 (default) or a combination of
 * tinytc_core_feature_flag_t
 *
 * @return Level Zero module (unique handle)
 */
inline auto create_kernel_bundle(ze_context_handle_t context, ze_device_handle_t device,
                                 tinytc_prog_t prg, tinytc_core_feature_flags_t core_features = 0)
    -> unique_handle<ze_module_handle_t> {
    ze_module_handle_t obj;
    CHECK_STATUS(
        tinytc_ze_kernel_bundle_create_with_program(&obj, context, device, prg, core_features));
    return unique_handle<ze_module_handle_t>{obj};
}

/**
 * @brief Make a Level Zero module from a tinytc binary
 *
 * @param context Context
 * @param device Device
 * @param bin Binary
 *
 * @return Level Zero module (unique handle)
 */
inline auto create_kernel_bundle(ze_context_handle_t context, ze_device_handle_t device,
                                 const_tinytc_binary_t bin) -> unique_handle<ze_module_handle_t> {
    ze_module_handle_t obj;
    CHECK_STATUS(tinytc_ze_kernel_bundle_create_with_binary(&obj, context, device, bin));
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
inline auto create_kernel(ze_module_handle_t mod, char const *name)
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

////////////////////////////
////////// Recipe //////////
////////////////////////////

/**
 * @brief Append recipe to command list
 *
 * Cf. @ref tinytc_ze_recipe_handler_submit
 *
 * @param handler Recipe handler
 * @param list Command list
 * @param signal_event Event to be signalled on completetion
 * @param num_wait_events Number of wait events to wait on
 * @param wait_events Array of num_wait_events events to wait on
 */
inline void submit(tinytc_recipe_handler_t handler, ze_command_list_handle_t list,
                   ze_event_handle_t signal_event = nullptr, uint32_t num_wait_events = 0,
                   ze_event_handle_t *wait_events = nullptr) {
    CHECK_STATUS(
        tinytc_ze_recipe_handler_submit(handler, list, signal_event, num_wait_events, wait_events));
}

/**
 * @brief Make recipe handler
 *
 * @param context Context
 * @param device Device
 * @param rec Recipe
 *
 * @return Level Zero recipe handler
 */
inline auto create_recipe_handler(ze_context_handle_t context, ze_device_handle_t device,
                                  tinytc_recipe_t rec) -> shared_handle<tinytc_recipe_handler_t> {
    tinytc_recipe_handler_t handler;
    CHECK_STATUS(tinytc_ze_recipe_handler_create(&handler, context, device, rec));
    return shared_handle{handler};
}

} // namespace tinytc

#endif // TINYTC_ZE_20240416_HPP
