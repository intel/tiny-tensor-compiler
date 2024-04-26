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
 * @brief Query core info from level zero runtime
 *
 * @param device [in] device handle
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

namespace internal {
template <> struct unique_handle_traits<ze_kernel_handle_t> {
    static void destroy(ze_kernel_handle_t obj) { zeKernelDestroy(obj); }
};
template <> struct unique_handle_traits<ze_module_handle_t> {
    static void destroy(ze_module_handle_t obj) { zeModuleDestroy(obj); }
};
} // namespace internal

inline auto make_kernel_bundle(ze_context_handle_t context, ze_device_handle_t device,
                               binary const &bin, ze_module_build_log_handle_t *build_log = nullptr)
    -> unique_handle<ze_module_handle_t> {
    ze_module_handle_t obj;
    CHECK_STATUS(tinytc_ze_module_create(&obj, context, device, bin.get(), build_log));
    return {obj};
}

inline auto make_kernel(ze_module_handle_t mod, char const *name)
    -> unique_handle<ze_kernel_handle_t> {
    ze_kernel_handle_t obj;
    CHECK_STATUS(tinytc_ze_kernel_create(&obj, mod, name));
    return {obj};
}

inline auto get_group_size(ze_kernel_handle_t kernel) -> std::array<std::uint32_t, 3u> {
    std::array<std::uint32_t, 3u> group_size;
    CHECK_STATUS(tinytc_ze_get_group_size(kernel, &group_size[0], &group_size[1], &group_size[2]));
    return group_size;
}

inline auto get_group_count(std::uint32_t howmany) -> ze_group_count_t {
    return tinytc_ze_get_group_count(howmany);
}

////////////////////////////
////////// Recipe //////////
////////////////////////////

class level_zero_recipe_handler : public recipe_handler {
  public:
    using recipe_handler::recipe_handler;

    inline void submit(ze_command_list_handle_t list, ze_event_handle_t signal_event = nullptr,
                       uint32_t num_wait_events = 0, ze_event_handle_t *wait_events = nullptr) {
        CHECK_STATUS(tinytc_ze_recipe_handler_submit(obj_, list, signal_event, num_wait_events,
                                                     wait_events));
    }
};

inline auto make_recipe_handler(ze_context_handle_t context, ze_device_handle_t device,
                                recipe const &rec) -> level_zero_recipe_handler {
    tinytc_recipe_handler_t handler;
    CHECK_STATUS(tinytc_ze_recipe_handler_create(&handler, context, device, rec.get()));
    return {handler};
}

} // namespace tinytc

#endif // TINYTC_ZE_20240416_HPP
