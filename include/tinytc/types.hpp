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
    compilation_error = tinytc_status_compilation_error,
    file_io_error = tinytc_status_file_io_error,
    parse_error = tinytc_status_parse_error,
    unavailable_extension = tinytc_status_unavailable_extension,
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
    ir_collective_called_from_spmd = tinytc_status_ir_collective_called_from_spmd,
    // Level Zero errors
    ze_result_not_ready = tinytc_status_ze_result_not_ready,
    ze_result_error_device_lost = tinytc_status_ze_result_error_device_lost,
    ze_result_error_out_of_host_memory = tinytc_status_ze_result_error_out_of_host_memory,
    ze_result_error_out_of_device_memory = tinytc_status_ze_result_error_out_of_device_memory,
    ze_result_error_module_build_failure = tinytc_status_ze_result_error_module_build_failure,
    ze_result_error_module_link_failure = tinytc_status_ze_result_error_module_link_failure,
    ze_result_error_device_requires_reset = tinytc_status_ze_result_error_device_requires_reset,
    ze_result_error_device_in_low_power_state =
        tinytc_status_ze_result_error_device_in_low_power_state,
    ze_result_exp_error_device_is_not_vertex =
        tinytc_status_ze_result_exp_error_device_is_not_vertex,
    ze_result_exp_error_vertex_is_not_device =
        tinytc_status_ze_result_exp_error_vertex_is_not_device,
    ze_result_exp_error_remote_device = tinytc_status_ze_result_exp_error_remote_device,
    ze_result_exp_error_operands_incompatible =
        tinytc_status_ze_result_exp_error_operands_incompatible,
    ze_result_exp_rtas_build_retry = tinytc_status_ze_result_exp_rtas_build_retry,
    ze_result_exp_rtas_build_deferred = tinytc_status_ze_result_exp_rtas_build_deferred,
    ze_result_error_insufficient_permissions =
        tinytc_status_ze_result_error_insufficient_permissions,
    ze_result_error_not_available = tinytc_status_ze_result_error_not_available,
    ze_result_error_dependency_unavailable = tinytc_status_ze_result_error_dependency_unavailable,
    ze_result_warning_dropped_data = tinytc_status_ze_result_warning_dropped_data,
    ze_result_error_uninitialized = tinytc_status_ze_result_error_uninitialized,
    ze_result_error_unsupported_version = tinytc_status_ze_result_error_unsupported_version,
    ze_result_error_unsupported_feature = tinytc_status_ze_result_error_unsupported_feature,
    ze_result_error_invalid_argument = tinytc_status_ze_result_error_invalid_argument,
    ze_result_error_invalid_null_handle = tinytc_status_ze_result_error_invalid_null_handle,
    ze_result_error_handle_object_in_use = tinytc_status_ze_result_error_handle_object_in_use,
    ze_result_error_invalid_null_pointer = tinytc_status_ze_result_error_invalid_null_pointer,
    ze_result_error_invalid_size = tinytc_status_ze_result_error_invalid_size,
    ze_result_error_unsupported_size = tinytc_status_ze_result_error_unsupported_size,
    ze_result_error_unsupported_alignment = tinytc_status_ze_result_error_unsupported_alignment,
    ze_result_error_invalid_synchronization_object =
        tinytc_status_ze_result_error_invalid_synchronization_object,
    ze_result_error_invalid_enumeration = tinytc_status_ze_result_error_invalid_enumeration,
    ze_result_error_unsupported_enumeration = tinytc_status_ze_result_error_unsupported_enumeration,
    ze_result_error_unsupported_image_format =
        tinytc_status_ze_result_error_unsupported_image_format,
    ze_result_error_invalid_native_binary = tinytc_status_ze_result_error_invalid_native_binary,
    ze_result_error_invalid_global_name = tinytc_status_ze_result_error_invalid_global_name,
    ze_result_error_invalid_kernel_name = tinytc_status_ze_result_error_invalid_kernel_name,
    ze_result_error_invalid_function_name = tinytc_status_ze_result_error_invalid_function_name,
    ze_result_error_invalid_group_size_dimension =
        tinytc_status_ze_result_error_invalid_group_size_dimension,
    ze_result_error_invalid_global_width_dimension =
        tinytc_status_ze_result_error_invalid_global_width_dimension,
    ze_result_error_invalid_kernel_argument_index =
        tinytc_status_ze_result_error_invalid_kernel_argument_index,
    ze_result_error_invalid_kernel_argument_size =
        tinytc_status_ze_result_error_invalid_kernel_argument_size,
    ze_result_error_invalid_kernel_attribute_value =
        tinytc_status_ze_result_error_invalid_kernel_attribute_value,
    ze_result_error_invalid_module_unlinked = tinytc_status_ze_result_error_invalid_module_unlinked,
    ze_result_error_invalid_command_list_type =
        tinytc_status_ze_result_error_invalid_command_list_type,
    ze_result_error_overlapping_regions = tinytc_status_ze_result_error_overlapping_regions,
    ze_result_warning_action_required = tinytc_status_ze_result_warning_action_required,
    ze_result_error_unknown = tinytc_status_ze_result_error_unknown,
    // OpenCL errors
    cl_build_program_failure = tinytc_status_cl_build_program_failure,
    cl_compile_program_failure = tinytc_status_cl_compile_program_failure,
    cl_compiler_not_available = tinytc_status_cl_compiler_not_available,
    cl_device_not_found = tinytc_status_cl_device_not_found,
    cl_device_not_available = tinytc_status_cl_device_not_available,
    cl_device_partition_failed = tinytc_status_cl_device_partition_failed,
    cl_exec_status_error_for_events_in_wait_list =
        tinytc_status_cl_exec_status_error_for_events_in_wait_list,
    cl_image_format_mismatch = tinytc_status_cl_image_format_mismatch,
    cl_image_format_not_supported = tinytc_status_cl_image_format_not_supported,
    cl_invalid_arg_index = tinytc_status_cl_invalid_arg_index,
    cl_invalid_arg_size = tinytc_status_cl_invalid_arg_size,
    cl_invalid_arg_value = tinytc_status_cl_invalid_arg_value,
    cl_invalid_binary = tinytc_status_cl_invalid_binary,
    cl_invalid_buffer_size = tinytc_status_cl_invalid_buffer_size,
    cl_invalid_build_options = tinytc_status_cl_invalid_build_options,
    cl_invalid_command_queue = tinytc_status_cl_invalid_command_queue,
    cl_invalid_compiler_options = tinytc_status_cl_invalid_compiler_options,
    cl_invalid_context = tinytc_status_cl_invalid_context,
    cl_invalid_device = tinytc_status_cl_invalid_device,
    cl_invalid_device_partition_count = tinytc_status_cl_invalid_device_partition_count,
    cl_invalid_device_queue = tinytc_status_cl_invalid_device_queue,
    cl_invalid_device_type = tinytc_status_cl_invalid_device_type,
    cl_invalid_event = tinytc_status_cl_invalid_event,
    cl_invalid_event_wait_list = tinytc_status_cl_invalid_event_wait_list,
    cl_invalid_global_offset = tinytc_status_cl_invalid_global_offset,
    cl_invalid_global_work_size = tinytc_status_cl_invalid_global_work_size,
    cl_invalid_host_ptr = tinytc_status_cl_invalid_host_ptr,
    cl_invalid_image_descriptor = tinytc_status_cl_invalid_image_descriptor,
    cl_invalid_image_format_descriptor = tinytc_status_cl_invalid_image_format_descriptor,
    cl_invalid_image_size = tinytc_status_cl_invalid_image_size,
    cl_invalid_kernel = tinytc_status_cl_invalid_kernel,
    cl_invalid_kernel_args = tinytc_status_cl_invalid_kernel_args,
    cl_invalid_kernel_definition = tinytc_status_cl_invalid_kernel_definition,
    cl_invalid_kernel_name = tinytc_status_cl_invalid_kernel_name,
    cl_invalid_linker_options = tinytc_status_cl_invalid_linker_options,
    cl_invalid_mem_object = tinytc_status_cl_invalid_mem_object,
    cl_invalid_operation = tinytc_status_cl_invalid_operation,
    cl_invalid_pipe_size = tinytc_status_cl_invalid_pipe_size,
    cl_invalid_platform = tinytc_status_cl_invalid_platform,
    cl_invalid_program = tinytc_status_cl_invalid_program,
    cl_invalid_program_executable = tinytc_status_cl_invalid_program_executable,
    cl_invalid_property = tinytc_status_cl_invalid_property,
    cl_invalid_queue_properties = tinytc_status_cl_invalid_queue_properties,
    cl_invalid_sampler = tinytc_status_cl_invalid_sampler,
    cl_invalid_spec_id = tinytc_status_cl_invalid_spec_id,
    cl_invalid_value = tinytc_status_cl_invalid_value,
    cl_invalid_work_dimension = tinytc_status_cl_invalid_work_dimension,
    cl_invalid_work_group_size = tinytc_status_cl_invalid_work_group_size,
    cl_invalid_work_item_size = tinytc_status_cl_invalid_work_item_size,
    cl_kernel_arg_info_not_available = tinytc_status_cl_kernel_arg_info_not_available,
    cl_link_program_failure = tinytc_status_cl_link_program_failure,
    cl_linker_not_available = tinytc_status_cl_linker_not_available,
    cl_map_failure = tinytc_status_cl_map_failure,
    cl_mem_copy_overlap = tinytc_status_cl_mem_copy_overlap,
    cl_mem_object_allocation_failure = tinytc_status_cl_mem_object_allocation_failure,
    cl_misaligned_sub_buffer_offset = tinytc_status_cl_misaligned_sub_buffer_offset,
    cl_out_of_host_memory = tinytc_status_cl_out_of_host_memory,
    cl_out_of_resources = tinytc_status_cl_out_of_resources,
    cl_max_size_restriction_exceeded = tinytc_status_cl_max_size_restriction_exceeded,
    cl_profiling_info_not_available = tinytc_status_cl_profiling_info_not_available,
    // The unknown error comes last
    unknown = tinytc_status_unknown
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

//! Binary operations
enum class binary_op {
    add = tinytc_binary_op_add, ///< add
    sub = tinytc_binary_op_sub, ///< subtract
    mul = tinytc_binary_op_mul, ///< multiply
    div = tinytc_binary_op_div, ///< divide
    rem = tinytc_binary_op_rem  ///< division remainder
};
//! Compare operation
enum class cmp_condition {
    eq = tinytc_cmp_condition_eq, ///< equals
    ne = tinytc_cmp_condition_ne, ///< not equal
    gt = tinytc_cmp_condition_gt, ///< greater than
    ge = tinytc_cmp_condition_ge, ///< greather or equal than
    lt = tinytc_cmp_condition_lt, ///< less than
    le = tinytc_cmp_condition_le  ///< less or equal than
};
//! Transpose
enum class transpose {
    N = tinytc_transpose_N, ///< no transpose
    T = tinytc_transpose_T  ///< transpose
};

enum class core_feature_flag { large_register_file = tinytc_core_feature_flag_large_register_file };

enum class intel_gpu_architecture { pvc = tinytc_intel_gpu_architecture_pvc };

//! Target binary format
enum class bundle_format {
    spirv = tinytc_bundle_format_spirv,  ///< SPIR-V
    native = tinytc_bundle_format_native ///< Native device binary
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
