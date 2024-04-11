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
///////// Data type ////////
////////////////////////////

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
 * @param memref_ty [in] memref data type object
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_group_type_create(tinytc_data_type_t *dt,
                                                       tinytc_data_type_t memref_ty);

/**
 * @brief Release data type object
 *
 * Decreases reference count by 1, free memory if reference count is 0.
 *
 * @param dt [inout] data type object
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_data_type_release(tinytc_data_type_t dt);

/**
 * @brief Increase reference count of data type object by 1
 *
 * @param dt [inout] data type object
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_data_type_retain(tinytc_data_type_t dt);

////////////////////////////
/////////// Value //////////
////////////////////////////

/**
 * @brief Create value
 *
 * @param val [out] pointer to the value object created
 * @param type [in] data type object
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_value_create(tinytc_value_t *val, tinytc_data_type_t type);

/**
 * @brief Create floating point immediate value
 *
 * @param val [out] pointer to the value object created
 * @param imm [in] immediate value
 * @param type [in] type of immediate value
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_float_imm_create(tinytc_value_t *val, double imm,
                                                      tinytc_scalar_type_t type);
/**
 * @brief Create integer immediate value
 *
 * @param val [out] pointer to the value object created
 * @param imm [in] immediate value
 * @param type [in] type of immediate value
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_int_imm_create(tinytc_value_t *val, int64_t imm,
                                                    tinytc_scalar_type_t type);

/**
 * @brief Release value handle
 *
 * Decreases reference count by 1, free memory if reference count is 0.
 *
 * @param val [inout] value object
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_value_release(tinytc_value_t val);

/**
 * @brief Increase reference count of value handle by 1
 *
 * @param val [inout] value object
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_value_retain(tinytc_value_t val);

/**
 * @brief Set name of value
 *
 * @param val [inout] value object
 * @param name [in] name; null-terminated string
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_value_set_name(tinytc_value_t val, char const *name);

/**
 * @brief Get name of value
 *
 * The returned pointer may be invalidated if the value or any node in the abstract syntax
 * tree referencing the value is modified.
 *
 * @param val [in] value object
 * @param name [out] pointer to C string
 *
 * @return tinytc_success on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_value_get_name(tinytc_value_t val, char const **name);

#ifdef __cplusplus
}
#endif

#endif // TINYTC_20240409_H
