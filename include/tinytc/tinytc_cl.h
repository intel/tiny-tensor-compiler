// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TINYTC_CL_20240416_H
#define TINYTC_CL_20240416_H

#include "tinytc/export.h"
#include "tinytc/types.h"

#include <CL/cl.h>

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////
/////////// Error //////////
////////////////////////////

#define TINYTC_CL_CHECK_STATUS(X)                                                                  \
    do {                                                                                           \
        cl_int stat = X;                                                                           \
        if (stat != CL_SUCCESS) {                                                                  \
            return tinytc_status_compute_runtime_error;                                            \
        }                                                                                          \
    } while (0)

////////////////////////////
//////// Device info ///////
////////////////////////////

/**
 * @brief Get support level of OpenCL device
 *
 * @param device [in] Device handle
 * @param level [out] Pointer to support level
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_cl_get_support_level(cl_device_id device,
                                                          tinytc_support_level_t *level);

/**
 * @brief Query core info from OpenCL runtime
 *
 * @param info [out] pointer to the core_info object created
 * @param device [in] device handle
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_cl_core_info_create(tinytc_core_info_t *info,
                                                         cl_device_id device);

////////////////////////////
////////// Kernel //////////
////////////////////////////

/**
 * @brief Compile tensor program
 *
 * @param bundle [out] pointer to the kernel bundle (cl_program) object created
 * @param context [in] context handle
 * @param device [in] device handle
 * @param prg [inout] tensor program; modified as compiler passes are run
 * @param core_features [in][optional] requested core features; must be 0 (default) or a combination
 * of tinytc_core_feature_flag_t
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_cl_kernel_bundle_create_with_program(
    cl_program *bundle, cl_context context, cl_device_id device, tinytc_prog_t prg,
    tinytc_core_feature_flags_t core_features);

/**
 * @brief Create an OpenCL program from a tinytc binary
 *
 * @param bundle [out] pointer to the kernel bundle (cl_program) object created
 * @param context [in] context handle
 * @param device [in] device handle
 * @param bin [in] binary object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_cl_kernel_bundle_create_with_binary(cl_program *bundle,
                                                                         cl_context context,
                                                                         cl_device_id device,
                                                                         const_tinytc_binary_t bin);

/**
 * @brief Get work group size for kernel
 *
 * @param kernel [in] kernel handle
 * @param local_size [out][range(0,3)] pointer to local size array of size >= 3
 * entries
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_cl_get_group_size(cl_kernel kernel, size_t *local_size);

/**
 * @brief Convert group size to opencl global range
 *
 * @param num_groups [in][range(0,3)] pointer to number of groups of size >= 3
 * @param local_size [in][range(0,3)] pointer to local size array of size >= 3
 * @param global_size [out][range(0,3)] pointer to global size array of size >= 3
 */
TINYTC_EXPORT void tinytc_cl_get_global_size(const size_t *num_groups, const size_t *local_size,
                                             size_t *global_size);

////////////////////////////
////////// Recipe //////////
////////////////////////////

/**
 * @brief Create kernel object for recipe
 *
 * @param handler [out] pointer to recipe handler object
 * @param context [in] context handle
 * @param device [in] device handle
 * @param recipe [in] recipe object
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_cl_recipe_handler_create(tinytc_recipe_handler_t *handler,
                                                              cl_context context,
                                                              cl_device_id device,
                                                              tinytc_recipe_t recipe);

/**
 * @brief Submit recipe to device
 *
 * @param handler [in] recipe handler object
 * @param queue [in] command queue handle
 * @param num_wait_events [in][optional] number of events the kernel depends on
 * @param wait_events [in][optional][range(0,num_wait_events)] array of events to wait on; can be
 * nullptr if num_wait_events is 0
 * @param event [out][optional] pointer to event object created; can be nullptr
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_cl_recipe_handler_submit(tinytc_recipe_handler_t handler,
                                                              cl_command_queue queue,
                                                              cl_uint num_wait_events,
                                                              const cl_event *wait_events,
                                                              cl_event *event);

#ifdef __cplusplus
}
#endif

#endif // TINYTC_CL_20240416_H
