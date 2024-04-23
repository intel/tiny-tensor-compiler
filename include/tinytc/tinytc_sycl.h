// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TINYTC_SYCL_20240423_H
#define TINYTC_SYCL_20240423_H

#include "tinytc/export.h"
#include "tinytc/types.h"

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////
/////////// Error //////////
////////////////////////////

TINYTC_EXPORT tinytc_status_t tinytc_sycl_convert_status(int result);

////////////////////////////
//////// Device info ///////
////////////////////////////

/**
 * @brief Query core info from SYCL runtime
 *
 * @param info [out] pointer to the core_info object created
 * @param device [in] pointer to sycl::device
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_sycl_core_info_create(tinytc_core_info_t *info,
                                                           const void *device);

////////////////////////////
////////// Recipe //////////
////////////////////////////

/**
 * @brief Create kernel object for recipe
 *
 * @param handler [out] pointer to recipe handler object
 * @param recipe [in] recipe object
 * @param context [in] pointer to sycl::context
 * @param device [in] pointer to sycl::device
 *
 * @return tinytc_status_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_sycl_recipe_handler_create(tinytc_recipe_handler_t *handler,
                                                                tinytc_recipe_t recipe,
                                                                const void *context,
                                                                const void *device);

#ifdef __cplusplus
}
#endif

#endif // TINYTC_SYCL_20240423_H
