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

/**
 * @brief Cf. @ref tinytc_status_t
 *
 * A status is typically thrown as exception, hence one should wrap calls as following:
 *
 * @code{.cpp}
 * try {
 *   ...
 * } catch (tinytc::status const& st) {
 *   ...
 * }
 * @endcode
 */
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
    unsupported_backend = tinytc_status_unsupported_backend,
    invalid_kernel_arguments = tinytc_status_invalid_kernel_arguments,
    unsupported_device = tinytc_status_unsupported_device,
    invalid_core_info = tinytc_status_invalid_core_info,
    unknown_pass_name = tinytc_status_unknown_pass_name,
    not_implemented = tinytc_status_not_implemented,
    // IR errors
    ir_out_of_bounds = tinytc_status_ir_out_of_bounds,
    ir_invalid_shape = tinytc_status_ir_invalid_shape,
    ir_incompatible_shapes = tinytc_status_ir_incompatible_shapes,
    ir_shape_stride_mismatch = tinytc_status_ir_shape_stride_mismatch,
    ir_scalar_mismatch = tinytc_status_ir_scalar_mismatch,
    ir_invalid_number_of_indices = tinytc_status_ir_invalid_number_of_indices,
    ir_expected_boolean = tinytc_status_ir_expected_boolean,
    ir_expected_scalar = tinytc_status_ir_expected_scalar,
    ir_expected_int = tinytc_status_ir_expected_int,
    ir_expected_float = tinytc_status_ir_expected_float,
    ir_expected_complex = tinytc_status_ir_expected_complex,
    ir_expected_i32 = tinytc_status_ir_expected_i32,
    ir_expected_index = tinytc_status_ir_expected_index,
    ir_expected_coopmatrix = tinytc_status_ir_expected_coopmatrix,
    ir_expected_coopmatrix_or_scalar = tinytc_status_ir_expected_coopmatrix_or_scalar,
    ir_expected_coopmatrix_scalar_or_boolean =
        tinytc_status_ir_expected_coopmatrix_scalar_or_boolean,
    ir_expected_memref = tinytc_status_ir_expected_memref,
    ir_expected_memref_or_scalar = tinytc_status_ir_expected_memref_or_scalar,
    ir_expected_memref_or_group = tinytc_status_ir_expected_memref_or_group,
    ir_expected_memref_order_0 = tinytc_status_ir_expected_memref_order_0,
    ir_expected_memref_order_1 = tinytc_status_ir_expected_memref_order_1,
    ir_expected_memref_order_2 = tinytc_status_ir_expected_memref_order_2,
    ir_expected_memref_order_0_or_1 = tinytc_status_ir_expected_memref_order_0_or_1,
    ir_expected_memref_order_1_or_2 = tinytc_status_ir_expected_memref_order_1_or_2,
    ir_expected_memref_order_0_1_or_2 = tinytc_status_ir_expected_memref_order_0_1_or_2,
    ir_unexpected_yield = tinytc_status_ir_unexpected_yield,
    ir_yield_mismatch = tinytc_status_ir_yield_mismatch,
    ir_subview_mismatch = tinytc_status_ir_subview_mismatch,
    ir_invalid_slice = tinytc_status_ir_invalid_slice,
    ir_expand_shape_order_too_small = tinytc_status_ir_expand_shape_order_too_small,
    ir_expand_shape_mismatch = tinytc_status_ir_expand_shape_mismatch,
    ir_collective_called_from_spmd = tinytc_status_ir_collective_called_from_spmd,
    ir_fp_unsupported = tinytc_status_ir_fp_unsupported,
    ir_spmd_called_from_collective = tinytc_status_ir_spmd_called_from_collective,
    ir_expected_local_address_space = tinytc_status_ir_expected_local_address_space,
    ir_expected_global_address_space = tinytc_status_ir_expected_global_address_space,
    ir_address_space_mismatch = tinytc_status_ir_address_space_mismatch,
    ir_invalid_offset = tinytc_status_ir_invalid_offset,
    ir_int_unsupported = tinytc_status_ir_int_unsupported,
    ir_boolean_unsupported = tinytc_status_ir_boolean_unsupported,
    ir_complex_unsupported = tinytc_status_ir_complex_unsupported,
    ir_coopmatrix_unsupported = tinytc_status_ir_coopmatrix_unsupported,
    ir_forbidden_cast = tinytc_status_ir_forbidden_cast,
    ir_invalid_beta = tinytc_status_ir_invalid_beta,
    ir_init_return_mismatch = tinytc_status_ir_init_return_mismatch,
    ir_invalid_matrix_use = tinytc_status_ir_invalid_matrix_use,
    ir_unsupported_coopmatrix_shape = tinytc_status_ir_unsupported_coopmatrix_shape,
    ir_forbidden_promotion = tinytc_status_ir_forbidden_promotion,
    ir_constant_mismatch = tinytc_status_ir_constant_mismatch,
    ir_insufficient_alignment = tinytc_status_ir_insufficient_alignment,
    ir_must_have_yield = tinytc_status_ir_must_have_yield,
    ir_yield_in_else_branch_missing = tinytc_status_ir_yield_in_else_branch_missing,
    ir_from_to_mismatch = tinytc_status_ir_from_to_mismatch,
    ir_operand_type_must_match_return_type = tinytc_status_ir_operand_type_must_match_return_type,
    ir_invalid_stride = tinytc_status_ir_invalid_stride,
    ir_init_return_type_mismatch = tinytc_status_ir_init_return_type_mismatch,
    ir_invalid_alignment = tinytc_status_ir_invalid_alignment,
    ir_value_still_has_uses = tinytc_status_ir_value_still_has_uses,
    ir_expected_array_attribute = tinytc_status_ir_expected_array_attribute,
    ir_expected_boolean_attribute = tinytc_status_ir_expected_boolean_attribute,
    ir_expected_dictionary_attribute = tinytc_status_ir_expected_dictionary_attribute,
    ir_expected_integer_attribute = tinytc_status_ir_expected_integer_attribute,
    ir_expected_string_attribute = tinytc_status_ir_expected_string_attribute,
    ir_duplicate_key_in_dictionary = tinytc_status_ir_duplicate_key_in_dictionary,
    ir_unexpected_array_attribute_size = tinytc_status_ir_unexpected_array_attribute_size,
    spirv_forbidden_forward_declaration = tinytc_status_spirv_forbidden_forward_declaration,
    spirv_undefined_value = tinytc_status_spirv_undefined_value,
    spirv_missing_dope_vector = tinytc_status_spirv_missing_dope_vector,
    spirv_unsupported_atomic_data_type = tinytc_status_spirv_unsupported_atomic_data_type,
    spirv_required_feature_unavailable = tinytc_status_spirv_required_feature_unavailable,
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
    i8 = tinytc_scalar_type_i8,       ///< Signed 8 bit integer
    i16 = tinytc_scalar_type_i16,     ///< Signed 16 bit integer
    i32 = tinytc_scalar_type_i32,     ///< Signed 32 bit integer
    i64 = tinytc_scalar_type_i64,     ///< Signed 64 bit integer
    index = tinytc_scalar_type_index, ///< Unsigned Integer type for indices
    f16 = tinytc_scalar_type_f16,     ///< Half precision floating point (16 bit)
    bf16 = tinytc_scalar_type_bf16,   ///< Brain floating point format with 16 bits
    f32 = tinytc_scalar_type_f32,     ///< Single precision floating point (32 bit)
    f64 = tinytc_scalar_type_f64,     ///< Double precision floating point (64 bit)
    c32 = tinytc_scalar_type_c32,     ///< Single precision complex (2x32 bit)
    c64 = tinytc_scalar_type_c64      ///< Double precision complex (2x64 bit)
};

//! Arithmetic operations
enum class arithmetic {
    add = tinytc_arithmetic_add,  ///< add
    sub = tinytc_arithmetic_sub,  ///< subtract
    mul = tinytc_arithmetic_mul,  ///< multiply
    div = tinytc_arithmetic_div,  ///< divide
    rem = tinytc_arithmetic_rem,  ///< division remainder
    shl = tinytc_arithmetic_shl,  ///< left shift
    shr = tinytc_arithmetic_shr,  ///< arithmetic right shift
    and_ = tinytc_arithmetic_and, ///< bitwise and
    or_ = tinytc_arithmetic_or,   ///< bitwise or
    xor_ = tinytc_arithmetic_xor, ///< bitwise xor
    min = tinytc_arithmetic_min,  ///< minimum
    max = tinytc_arithmetic_max   ///< maximum
};

//! Arithmetic operations (unary)
enum class arithmetic_unary {
    neg = tinytc_arithmetic_unary_neg,   ///< negation
    not_ = tinytc_arithmetic_unary_not,  ///< bitwise not
    abs = tinytc_arithmetic_unary_abs,   ///< absolute value
    conj = tinytc_arithmetic_unary_conj, ///< complex conjugate
    im = tinytc_arithmetic_unary_im,     ///< imaginary part
    re = tinytc_arithmetic_unary_re      ///< real part
};

//! Builtin values
enum class builtin {
    group_id = tinytc_builtin_group_id,                   ///< group id
    group_size = tinytc_builtin_group_size,               ///< group size
    num_subgroups = tinytc_builtin_num_subgroups,         ///< number of subgroups
    subgroup_size = tinytc_builtin_subgroup_size,         ///< subgroup size
    subgroup_id = tinytc_builtin_subgroup_id,             ///< subgroup id
    subgroup_local_id = tinytc_builtin_subgroup_local_id, ///< subgroup local id
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

//! Group operation
enum class group_operation {
    exclusive_scan = tinytc_group_operation_exclusive_scan, ///< Exclusive scan
    inclusive_scan = tinytc_group_operation_inclusive_scan, ///< Inclusive scan
    reduce = tinytc_group_operation_reduce                  ///< Reduction
};

//! Transpose
enum class transpose {
    N = tinytc_transpose_N, ///< no transpose
    T = tinytc_transpose_T  ///< transpose
};

//! Address space
enum class address_space {
    global = tinytc_address_space_global, ///< Global memory
    local = tinytc_address_space_local    ///< Local memory, returned by alloca
};

//! Checked flag
enum class checked_flag {
    none = tinytc_checked_flag_none, ///< Perform no checks
    rows = tinytc_checked_flag_rows, ///< Check for out-of-bound rows
    cols = tinytc_checked_flag_cols, ///< Check for out-of-bound cols
    both = tinytc_checked_flag_both  ///< Check for out-of-bound rows and cols
};

//! Store flag
enum class store_flag {
    regular = tinytc_store_flag_regular,      ///< Non-atomic non-block store
    atomic = tinytc_store_flag_atomic,        ///< Atomic store
    atomic_add = tinytc_store_flag_atomic_add ///< Atomic fetch add
};

//! Matrix use
enum class matrix_use {
    a = tinytc_matrix_use_a,    ///< matrix_a
    b = tinytc_matrix_use_b,    ///< matrix_b
    acc = tinytc_matrix_use_acc ///< matrix_acc
};

//! @brief Cf. @ref tinytc_spirv_feature_t
enum class spirv_feature {
    float16 = tinytc_spirv_feature_float16,
    float64 = tinytc_spirv_feature_float64,
    int64_atomics = tinytc_spirv_feature_int64_atomics,
    groups = tinytc_spirv_feature_groups,
    subgroup_dispatch = tinytc_spirv_feature_subgroup_dispatch,
    atomic_float16_add_local = tinytc_spirv_feature_atomic_float16_add_local,
    atomic_float16_add_global = tinytc_spirv_feature_atomic_float16_add_global,
    atomic_float32_add_local = tinytc_spirv_feature_atomic_float32_add_local,
    atomic_float32_add_global = tinytc_spirv_feature_atomic_float32_add_global,
    atomic_float64_add_local = tinytc_spirv_feature_atomic_float64_add_local,
    atomic_float64_add_global = tinytc_spirv_feature_atomic_float64_add_global,
    bfloat16_conversion = tinytc_spirv_feature_bfloat16_conversion,
};

//! @brief Cf. @ref tinytc_core_feature_flag_t
enum class core_feature_flag { large_register_file = tinytc_core_feature_flag_large_register_file };

//! @brief Cf. @ref tinytc_intel_gpu_architecture_t
enum class intel_gpu_architecture {
    tgl = tinytc_intel_gpu_architecture_tgl,
    pvc = tinytc_intel_gpu_architecture_pvc
};

//! Target binary format
enum class bundle_format {
    spirv = tinytc_bundle_format_spirv,  ///< SPIR-V
    native = tinytc_bundle_format_native ///< Native device binary
};

//! Flags for optimizer
enum class optflag { unsafe_fp_math = tinytc_optflag_unsafe_fp_math };

//! Memory object type
enum class mem_type {
    buffer = tinytc_mem_type_buffer,           ///< Buffer object (e.g. cl_mem)
    usm_pointer = tinytc_mem_type_usm_pointer, ///< Unified shared memory pointer
    svm_pointer = tinytc_mem_type_svm_pointer, ///< Shared virtual memory pointer
};

//! Support level of a device
enum class support_level {
    //! Device is unsupported (e.g. subgroups feature missing in OpenCL-C)
    none = tinytc_support_level_none,
    //! Device provides necessary features but is not well tested
    basic = tinytc_support_level_basic,
    //! Device provides necessary features and is well tested
    tuned = tinytc_support_level_tuned
};

////////////////////////////
/////// Type aliases ///////
////////////////////////////

//! @brief Alias for tinytc_named_attr in namespace tinytc
using named_attr = ::tinytc_named_attr;
//! @brief Alias for tinytc_position in namespace tinytc
using position = ::tinytc_position;
//! @brief Alias for tinytc_location in namespace tinytc
using location = ::tinytc_location;
//! @brief Alias for tinytc_error_reporter_t in namespace tinytc
using error_reporter_t = ::tinytc_error_reporter_t;

} // namespace tinytc

#endif // TYPES_20240410_HPP
