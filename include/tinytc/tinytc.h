// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TINYTC_20240409_H
#define TINYTC_20240409_H

#include "tinytc/export.h"
#include "tinytc/types.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Translate status code to textual description
 *
 * @param status [in] status code
 *
 * @return String
 */
TINYTC_EXPORT char const *tinytc_error_string(tinytc_status_t status);

////////////////////////////
/// Abstract syntax tree ///
////////////////////////////

/**
 * @brief Create program
 *
 * @param program Pointer to program handle
 * @param function_count Number of function handles
 * @param functions Array of function handles
 *
 * @return tinytc_success on success and error otherwise
 */
// tinytc_status_t TINYTC_EXPORT tinytc_prog_create(tinytc_prog_t *program, uint32_t function_count,
// tinytc_func_t *functions);
/**
 * @brief Destroy program
 *
 * @param program Program handle
 *
 * @return tinytc_success on success and error otherwise
 */
// tinytc_status_t TINYTC_EXPORT tinytc_prog_destroy(tinytc_prog_t program);

/**
 * @brief Create scalar data type
 *
 * @param dt [out] pointer to the data type object created
 * @param type [in] scalar type
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_scalar_type_create(tinytc_data_type_t *dt,
                                                        tinytc_scalar_type_t type);

/**
 * @brief Create memref data type
 *
 * @param dt [out] pointer to the data type object created
 * @param scalar_ty [in] element type
 * @param shape_size [in] tensor order; number of elements in shape array, must be 0 if shape ==
 * nullptr
 * @param shape [in][range(0, shape_size)] array of mode sizes
 * @param stride_size [in][optional] number of elements in stride array; must be either 0 for
 * automatic stride calculation or must match shape_size; must be 0 if stride == nullptr
 * @param stride [in][optional][range(0, stride_size)] stride array
 * @param location [in][optional] Source code location; can be nullptr
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_memref_type_create(tinytc_data_type_t *dt,
                                                        tinytc_scalar_type_t scalar_ty,
                                                        uint32_t shape_size, const int64_t *shape,
                                                        uint32_t stride_size, const int64_t *stride,
                                                        const tinytc_location_t *loc);

/**
 * @brief Create group data type
 *
 * @param dt [out] pointer to the data type object created
 * @param memref_ty [in] memref data type handle
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_group_type_create(tinytc_data_type_t *dt,
                                                       tinytc_data_type_t memref_ty);

/**
 * @brief Release data type handle
 *
 * Decreases reference count by 1, free memory if reference count is 0.
 *
 * @param dt [inout] data type handle
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_data_type_release(tinytc_data_type_t dt);

/**
 * @brief Increase reference count of data type handle by 1
 *
 * @param dt [inout] data type handle
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_data_type_retain(tinytc_data_type_t dt);

#ifdef __cplusplus
}
#endif

#endif // TINYTC_20240409_H
