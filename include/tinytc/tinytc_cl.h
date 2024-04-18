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

TINYTC_EXPORT tinytc_status_t tinytc_cl_convert_status(cl_int status);

#define TINYTC_CL_CHECK_STATUS(X)                                                                  \
    do {                                                                                           \
        cl_int stat = X;                                                                           \
        if (stat != CL_SUCCESS) {                                                                  \
            return tinytc_cl_convert_status(stat);                                                 \
        }                                                                                          \
    } while (0)

////////////////////////////
//////// Device info ///////
////////////////////////////

/**
 * @brief Query core info from OpenCL runtime
 *
 * @param info [out] pointer to the core_info object created
 * @param device [in] device handle
 *
 * @return CL_SUCCESS on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_cl_core_info_create(tinytc_core_info_t *info,
                                                         cl_device_id device);

////////////////////////////
////////// Kernel //////////
////////////////////////////

/**
 * @brief Create an OpenCL program from a tinytc binary
 *
 * @param mod [out] pointer to the program object created
 * @param context [in] context handle
 * @param device [in] device handle
 * @param bin [in] binary object
 *
 * @return CL_SUCCESS on success and error otherwise
 */
TINYTC_EXPORT tinytc_status_t tinytc_cl_program_create(cl_program *mod, cl_context context,
                                                       cl_device_id device, tinytc_binary_t bin);

/**
 * @brief Get work group size for kernel
 *
 * @param kernel [in] kernel handle
 * @param dev [in] device handle
 * @param x [out] pointer to group size in x dimension
 * @param y [out] pointer to group size in y dimension
 * @param z [out] pointer to group size in z dimension
 *
 * @return group count
 */
TINYTC_EXPORT tinytc_status_t tinytc_cl_get_group_size(cl_kernel kernel, cl_device_id dev,
                                                       size_t *x, size_t *y, size_t *z);

/**
 * @brief Convert group size to opencl global range
 *
 * @param howmany group size
 * @param local_size_x [in] group size in x dimension
 * @param local_size_y [in] group size in y dimension
 * @param local_size_z [in] group size in z dimension
 * @param global_size_x [out] pointer to global size in x dimension
 * @param global_size_y [out] pointer to globap size in y dimension
 * @param global_size_z [out] pointer to globap size in z dimension
 */
TINYTC_EXPORT void tinytc_cl_get_global_size(size_t howmany, size_t local_size_x,
                                             size_t local_size_y, size_t local_size_z,
                                             size_t *global_size_x, size_t *global_size_y,
                                             size_t *global_size_z);

#ifdef __cplusplus
}
#endif

#endif // TINYTC_CL_20240416_H
