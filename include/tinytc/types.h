// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TYPES_20240410_H
#define TYPES_20240410_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////
///////// Constants ////////
////////////////////////////

#define TINYTC_DYNAMIC INT64_MIN

////////////////////////////
/////// Enumerations ///////
////////////////////////////

//! Status codes
typedef enum {
    tinytc_status_success = 0x0,           ///< Operation was successful
    tinytc_status_bad_alloc = 0x1,         ///< Failure to allocate storage
    tinytc_status_invalid_arguments = 0x2, ///< Operation got invalid arguments
    tinytc_status_out_of_range = 0x3,
    tinytc_status_runtime_error = 0x4, ///< Runtime error
    tinytc_status_internal_compiler_error = 0x5,
    tinytc_status_unsupported_subgroup_size = 0x6,
    tinytc_status_unsupported_work_group_size = 0x7,
    // IR errors
    tinytc_status_ir_out_of_bounds = 0x100,
    tinytc_status_ir_invalid_shape = 0x101,
    tinytc_status_ir_incompatible_shapes = 0x102,
    tinytc_status_ir_shape_stride_mismatch = 0x103,
    tinytc_status_ir_scalar_mismatch = 0x104,
    tinytc_status_ir_invalid_number_of_indices = 0x105,
    tinytc_status_ir_expected_scalar = 0x106,
    tinytc_status_ir_expected_memref = 0x107,
    tinytc_status_ir_expected_memref_or_scalar = 0x108,
    tinytc_status_ir_expected_memref_or_group = 0x109,
    tinytc_status_ir_expected_vector_or_matrix = 0x10a,
    tinytc_status_ir_unexpected_yield = 0x10b,
    tinytc_status_ir_yield_mismatch = 0x10c,
    tinytc_status_ir_multiple_dynamic_modes = 0x10d,
    tinytc_status_ir_invalid_slice = 0x10e,
    tinytc_status_ir_expand_shape_order_too_small = 0x10f,
    tinytc_status_ir_expand_shape_mismatch = 0x110,
} tinytc_status_t;

//! Scalar types
typedef enum {
    tinytc_scalar_type_bool = 0,  ///< boolean
    tinytc_scalar_type_index = 1, ///< Unsigned integer type for indices
    tinytc_scalar_type_i8 = 2,    ///< Signed 8 bit integer
    tinytc_scalar_type_i16 = 3,   ///< Signed 16 bit integer
    tinytc_scalar_type_i32 = 4,   ///< Signed 32 bit integer
    tinytc_scalar_type_i64 = 5,   ///< Signed 64 bit integer
    tinytc_scalar_type_u8 = 6,    ///< Unsigned 8 bit integer
    tinytc_scalar_type_u16 = 7,   ///< Unsigned 16 bit integer
    tinytc_scalar_type_u32 = 8,   ///< Unsigned 32 bit integer
    tinytc_scalar_type_u64 = 9,   ///< Unsigned 64 bit integer
    tinytc_scalar_type_f32 = 10,  ///< Single precision floating point (32 bit)
    tinytc_scalar_type_f64 = 11   ///< Double precision floating point (64 bit)
} tinytc_scalar_type_t;

//! Binary operations
typedef enum {
    tinytc_binary_op_add = 0, ///< add
    tinytc_binary_op_sub = 1, ///< subtract
    tinytc_binary_op_mul = 2, ///< multiply
    tinytc_binary_op_div = 3, ///< divide
    tinytc_binary_op_rem = 4  ///< division remainder
} tinytc_binary_op_t;

//! Compare operation
typedef enum {
    tinytc_cmp_condition_eq = 0, ///< equals
    tinytc_cmp_condition_ne = 1, ///< not equal
    tinytc_cmp_condition_gt = 2, ///< greater than
    tinytc_cmp_condition_ge = 3, ///< greather or equal than
    tinytc_cmp_condition_lt = 4, ///< less than
    tinytc_cmp_condition_le = 5  ///< less or equal than
} tinytc_cmp_condition_t;

//! Transpose
typedef enum {
    tinytc_transpose_N = 0, ///< no transpose
    tinytc_transpose_T = 1  ///< transpose
} tinytc_transpose_t;

////////////////////////////
/////////// Types //////////
////////////////////////////

typedef uint8_t tinytc_bool_t;

//! @struct tinytc_data_type
//! @brief Opaque struct for a data type
struct tinytc_data_type;
//! @brief data_type handle
typedef struct tinytc_data_type *tinytc_data_type_t;

//! @struct tinytc_value
//! @brief Opaque struct for a value
struct tinytc_value;
//! @brief value handle
typedef struct tinytc_value *tinytc_value_t;

//! @struct tinytc_inst
//! @brief Opaque struct for an instruction
struct tinytc_inst;
//! @brief inst handle
typedef struct tinytc_inst *tinytc_inst_t;

//! @struct tinytc_region
//! @brief Opaque struct for a region
struct tinytc_region;
//! @brief region handle
typedef struct tinytc_region *tinytc_region_t;

//! @struct tinytc_func
//! @brief Opaque struct for a function
struct tinytc_func;
//! @brief func handle
typedef struct tinytc_func *tinytc_func_t;

//! @struct tinytc_prog
//! @brief Opaque struct for a program
struct tinytc_prog;
//! @brief prog handle
typedef struct tinytc_prog *tinytc_prog_t;

////////////////////////////
////////// Structs /////////
////////////////////////////

//! @brief Source code position
typedef struct tinytc_position {
    int32_t source_id; ///< Source file identifier; 0 is "unknown source"
    int32_t line;      ///< Line number; counting starts at 1
    int32_t column;    ///< Column number; counting start at 1
} tinytc_position_t;

//! @brief Source code location
typedef struct tinytc_location {
    tinytc_position begin; ///< Starting position
    tinytc_position end;   ///< End position
} tinytc_location_t;

#ifdef __cplusplus
}
#endif

#endif // TYPES_20240410_H
