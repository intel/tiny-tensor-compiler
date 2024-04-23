// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TINYTC_ZE_20240416_H
#define TINYTC_ZE_20240416_H

#include "tinytc/export.h"
#include "tinytc/types.h"

#include <level_zero/ze_api.h>

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////
/////////// Error //////////
////////////////////////////

TINYTC_EXPORT tinytc_status_t tinytc_ze_convert_status(ze_result_t result);

#define TINYTC_ZE_CHECK_STATUS(X)                                                                  \
    do {                                                                                           \
        ze_result_t result = X;                                                                    \
        if (result != ZE_RESULT_SUCCESS) {                                                         \
            return tinytc_ze_convert_status(result);                                               \
        }                                                                                          \
    } while (0)

////////////////////////////
//////// Device info ///////
////////////////////////////

/**
 * @brief Query core info from level zero runtime
 *
 * @param info [out] pointer to the core_info object created
 * @param device [in] device handle
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_ze_core_info_create(tinytc_core_info_t *info,
                                                         ze_device_handle_t device);

////////////////////////////
////////// Kernel //////////
////////////////////////////

/**
 * @brief Create a level zero module from a tinytc binary
 *
 * @param mod [out] pointer to the module object created
 * @param context [in] context handle
 * @param device [in] device handle
 * @param bin [in] binary object
 * @param build_log [out][optional] handle to store build log; caller needs to clean up the build
 * log with zeModuleBuildLogDestroy; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_ze_module_create(ze_module_handle_t *mod,
                                                      ze_context_handle_t context,
                                                      ze_device_handle_t device,
                                                      const_tinytc_binary_t bin,
                                                      ze_module_build_log_handle_t *build_log);

/**
 * @brief Create a kernel and set group size
 *
 * @param krnl [out] pointer to the kernel object created
 * @param mod [in] module handle
 * @param name [in] kernel name
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_ze_kernel_create(ze_kernel_handle_t *krnl,
                                                      ze_module_handle_t mod, char const *name);

/**
 * @brief Get work group size for kernel
 *
 * @param kernel [in] kernel handle
 * @param x [out] pointer to group size in x dimension
 * @param y [out] pointer to group size in y dimension
 * @param z [out] pointer to group size in z dimension
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_ze_get_group_size(ze_kernel_handle_t kernel, uint32_t *x,
                                                       uint32_t *y, uint32_t *z);

/**
 * @brief Convert group size to level zero group count
 *
 * @param howmany group size
 *
 * @return group count
 */
TINYTC_EXPORT ze_group_count_t tinytc_ze_get_group_count(uint32_t howmany);

////////////////////////////
////////// Recipe //////////
////////////////////////////

/**
 * @brief Create kernel object for recipe
 *
 * @param handler [out] pointer to recipe handler object
 * @param recipe [in] recipe object
 * @param context [in] context handle
 * @param device [in] device handle
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_ze_recipe_handler_create(tinytc_recipe_handler_t *handler,
                                                              tinytc_recipe_t recipe,
                                                              ze_context_handle_t context,
                                                              ze_device_handle_t device);

/**
 * @brief Submit recipe to device
 *
 * @param handler [in] recipe handler object
 * @param list [in] command list handle
 * @param signal_event [inout][optional] event to be signalled on completion; can be nullptr
 * @param num_wait_events [in][optional] number of events the kernel depends on
 * @param wait_events [in][optional][range(0,num_wait_events)] array of events to wait on; can be
 * nullptr if num_wait_events is 0
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_ze_recipe_handler_submit(tinytc_recipe_handler_t handler,
                                                              ze_command_list_handle_t list,
                                                              ze_event_handle_t signal_event,
                                                              uint32_t num_wait_events,
                                                              ze_event_handle_t *wait_events);

#ifdef __cplusplus
}
#endif

#endif // TINYTC_ZE_20240416_H
