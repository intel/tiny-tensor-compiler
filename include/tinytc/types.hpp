// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TYPES_20240410_HPP
#define TYPES_20240410_HPP

#include "tinytc/types.h"

namespace tinytc {

//! Status codes
enum class status {
    success = tinytc_success,
    bad_alloc = tinytc_bad_alloc,
    invalid_arguments = tinytc_invalid_arguments,
    runtime_error = tinytc_runtime_error,
    internal_compiler_error = tinytc_internal_compiler_error,
    unsupported_subgroup_size = tinytc_unsupported_subgroup_size,
    unsupported_work_group_size = tinytc_unsupported_work_group_size,
    // IR errors
    ir_out_of_bounds = tinytc_ir_out_of_bounds,
    ir_invalid_shape = tinytc_ir_invalid_shape,
    ir_incompatible_shapes = tinytc_ir_incompatible_shapes,
    ir_shape_stride_mismatch = tinytc_ir_shape_stride_mismatch,
    ir_scalar_mismatch = tinytc_ir_scalar_mismatch,
    ir_invalid_number_of_indices = tinytc_ir_invalid_number_of_indices,
    ir_expected_scalar = tinytc_ir_expected_scalar,
    ir_expected_memref = tinytc_ir_expected_memref,
    ir_expected_memref_or_scalar = tinytc_ir_expected_memref_or_scalar,
    ir_expected_memref_or_group = tinytc_ir_expected_memref_or_group,
    ir_expected_vector_or_matrix = tinytc_ir_expected_vector_or_matrix,
    ir_unexpected_yield = tinytc_ir_unexpected_yield,
    ir_yield_mismatch = tinytc_ir_yield_mismatch,
    ir_multiple_dynamic_modes = tinytc_ir_multiple_dynamic_modes,
    ir_invalid_slice = tinytc_ir_invalid_slice,
    ir_expand_shape_order_too_small = tinytc_ir_expand_shape_order_too_small,
    ir_expand_shape_mismatch = tinytc_ir_expand_shape_mismatch,
};

//! @brief Alias for tinytc_position in namespace tinytc
using position = ::tinytc_position;
//! @brief Alias for tinytc_location in namespace tinytc
using location = ::tinytc_location;

} // namespace tinytc

#endif // TYPES_20240410_HPP
