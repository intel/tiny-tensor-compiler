// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TYPES_20240410_H
#define TYPES_20240410_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////
/////// Enumerations ///////
////////////////////////////

//! Status codes
typedef enum {
    tinytc_success = 0x0,           ///< Operation was successful
    tinytc_bad_alloc = 0x1,         ///< Failure to allocate storage
    tinytc_invalid_arguments = 0x2, ///< Operation got invalid arguments
    tinytc_runtime_error = 0x3,     ///< Runtime error
    tinytc_internal_compiler_error = 0x4,
    tinytc_unsupported_subgroup_size = 0x5,
    tinytc_unsupported_work_group_size = 0x6,
    // IR errors
    tinytc_ir_out_of_bounds = 0x100,
    tinytc_ir_invalid_shape = 0x101,
    tinytc_ir_incompatible_shapes = 0x102,
    tinytc_ir_shape_stride_mismatch = 0x103,
    tinytc_ir_scalar_mismatch = 0x104,
    tinytc_ir_invalid_number_of_indices = 0x105,
    tinytc_ir_expected_scalar = 0x106,
    tinytc_ir_expected_memref = 0x107,
    tinytc_ir_expected_memref_or_scalar = 0x108,
    tinytc_ir_expected_memref_or_group = 0x109,
    tinytc_ir_expected_vector_or_matrix = 0x10a,
    tinytc_ir_unexpected_yield = 0x10b,
    tinytc_ir_yield_mismatch = 0x10c,
    tinytc_ir_multiple_dynamic_modes = 0x10d,
    tinytc_ir_invalid_slice = 0x10e,
    tinytc_ir_expand_shape_order_too_small = 0x10f,
    tinytc_ir_expand_shape_mismatch = 0x110,
} tinytc_status_t;

//! Scalar types
typedef enum {
    tinytc_bool = 0,  ///< boolean
    tinytc_index = 1, ///< Unsigned integer type for indices
    tinytc_i8 = 2,    ///< Signed 8 bit integer
    tinytc_i16 = 3,   ///< Signed 16 bit integer
    tinytc_i32 = 4,   ///< Signed 32 bit integer
    tinytc_i64 = 5,   ///< Signed 64 bit integer
    tinytc_u8 = 6,    ///< Unsigned 8 bit integer
    tinytc_u16 = 7,   ///< Unsigned 16 bit integer
    tinytc_u32 = 8,   ///< Unsigned 32 bit integer
    tinytc_u64 = 9,   ///< Unsigned 64 bit integer
    tinytc_f32 = 10,  ///< Single precision floating point (32 bit)
    tinytc_f64 = 11   ///< Double precision floating point (64 bit)
} tinytc_scalar_type_t;

////////////////////////////
////////// Structs /////////
////////////////////////////

/// @brief Source code position
typedef struct tinytc_position {
    int32_t source_id; ///< Source file identifier; 0 is "unknown source"
    int32_t line;      ///< Line number; counting starts at 1
    int32_t column;    ///< Column number; counting start at 1
} tinytc_position_t;

/// @brief Source code location
typedef struct tinytc_location {
    tinytc_position begin; ///< Starting position
    tinytc_position end;   ///< End position
} tinytc_location_t;

////////////////////////////
////////// Handles /////////
////////////////////////////

//! @struct tinytc_prog
//! @brief Opaque struct for a program
struct tinytc_prog;
//! @brief prog handle
typedef struct tinytc_prog *tinytc_prog_t;

//! @struct tinytc_func
//! @brief Opaque struct for a function
struct tinytc_func;
//! @brief func handle
typedef struct tinytc_func *tinytc_func_t;

//! @struct tinytc_region
//! @brief Opaque struct for a region
struct tinytc_region;
//! @brief region handle
typedef struct tinytc_region *tinytc_region_t;

//! @struct tinytc_inst
//! @brief Opaque struct for an instruction
struct tinytc_inst;
//! @brief inst handle
typedef struct tinytc_inst *tinytc_inst_t;

//! @struct tinytc_value
//! @brief Opaque struct for a value
struct tinytc_value;
//! @brief value handle
typedef struct tinytc_value *tinytc_value_t;

//! @struct tinytc_data_type
//! @brief Opaque struct for a data type
struct tinytc_data_type;
//! @brief data_type handle
typedef struct tinytc_data_type *tinytc_data_type_t;

#ifdef __cplusplus
}
#endif

#endif // TYPES_20240410_H
