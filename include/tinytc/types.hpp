// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TYPES_20240410_HPP
#define TYPES_20240410_HPP

#include "tinytc/types.h"

#include <cstdint>

namespace tinytc {

////////////////////////////
///////// Constants ////////
////////////////////////////

constexpr static std::int64_t dynamic = TINYTC_DYNAMIC;

////////////////////////////
/////// Enumerations ///////
////////////////////////////

//! Status codes
enum class status {
    success = tinytc_status_success,
    bad_alloc = tinytc_status_bad_alloc,
    invalid_arguments = tinytc_status_invalid_arguments,
    out_of_range = tinytc_status_out_of_range,
    runtime_error = tinytc_status_runtime_error,
    internal_compiler_error = tinytc_status_internal_compiler_error,
    unsupported_subgroup_size = tinytc_status_unsupported_subgroup_size,
    unsupported_work_group_size = tinytc_status_unsupported_work_group_size,
    // IR errors
    ir_out_of_bounds = tinytc_status_ir_out_of_bounds,
    ir_invalid_shape = tinytc_status_ir_invalid_shape,
    ir_incompatible_shapes = tinytc_status_ir_incompatible_shapes,
    ir_shape_stride_mismatch = tinytc_status_ir_shape_stride_mismatch,
    ir_scalar_mismatch = tinytc_status_ir_scalar_mismatch,
    ir_invalid_number_of_indices = tinytc_status_ir_invalid_number_of_indices,
    ir_expected_scalar = tinytc_status_ir_expected_scalar,
    ir_expected_memref = tinytc_status_ir_expected_memref,
    ir_expected_memref_or_scalar = tinytc_status_ir_expected_memref_or_scalar,
    ir_expected_memref_or_group = tinytc_status_ir_expected_memref_or_group,
    ir_expected_vector_or_matrix = tinytc_status_ir_expected_vector_or_matrix,
    ir_unexpected_yield = tinytc_status_ir_unexpected_yield,
    ir_yield_mismatch = tinytc_status_ir_yield_mismatch,
    ir_multiple_dynamic_modes = tinytc_status_ir_multiple_dynamic_modes,
    ir_invalid_slice = tinytc_status_ir_invalid_slice,
    ir_expand_shape_order_too_small = tinytc_status_ir_expand_shape_order_too_small,
    ir_expand_shape_mismatch = tinytc_status_ir_expand_shape_mismatch,
};

//! Scalar types
enum class scalar_type {
    bool_ = tinytc_scalar_type_bool,  ///< boolean
    index = tinytc_scalar_type_index, ///< Unsigned integer type for indices
    i8 = tinytc_scalar_type_i8,       ///< Signed 8 bit integer
    i16 = tinytc_scalar_type_i16,     ///< Signed 16 bit integer
    i32 = tinytc_scalar_type_i32,     ///< Signed 32 bit integer
    i64 = tinytc_scalar_type_i64,     ///< Signed 64 bit integer
    u8 = tinytc_scalar_type_u8,       ///< Unsigned 8 bit integer
    u16 = tinytc_scalar_type_u16,     ///< Unsigned 16 bit integer
    u32 = tinytc_scalar_type_u32,     ///< Unsigned 32 bit integer
    u64 = tinytc_scalar_type_u64,     ///< Unsigned 64 bit integer
    f32 = tinytc_scalar_type_f32,     ///< Single precision floating point (32 bit)
    f64 = tinytc_scalar_type_f64      ///< Double precision floating point (64 bit)
};

////////////////////////////
/////// Type aliases ///////
////////////////////////////

//! @brief Alias for tinytc_position in namespace tinytc
using position = ::tinytc_position;
//! @brief Alias for tinytc_location in namespace tinytc
using location = ::tinytc_location;

} // namespace tinytc

#endif // TYPES_20240410_HPP
