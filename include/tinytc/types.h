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
    tinytc_status_compilation_error = 0x8,
    tinytc_status_file_io_error = 0x9,
    tinytc_status_parse_error = 0xa,
    tinytc_status_unavailable_extension = 0xb,
    tinytc_status_unsupported_backend = 0xc,
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
    tinytc_status_ir_collective_called_from_spmd = 0x111,
    // Level zero errors
    tinytc_status_ze_result_not_ready = 0x10000,
    tinytc_status_ze_result_error_device_lost = 0x10001,
    tinytc_status_ze_result_error_out_of_host_memory = 0x10002,
    tinytc_status_ze_result_error_out_of_device_memory = 0x10003,
    tinytc_status_ze_result_error_module_build_failure = 0x10004,
    tinytc_status_ze_result_error_module_link_failure = 0x10005,
    tinytc_status_ze_result_error_device_requires_reset = 0x10006,
    tinytc_status_ze_result_error_device_in_low_power_state = 0x10007,
    tinytc_status_ze_result_exp_error_device_is_not_vertex = 0x10008,
    tinytc_status_ze_result_exp_error_vertex_is_not_device = 0x10009,
    tinytc_status_ze_result_exp_error_remote_device = 0x1000A,
    tinytc_status_ze_result_exp_error_operands_incompatible = 0x1000B,
    tinytc_status_ze_result_exp_rtas_build_retry = 0x1000C,
    tinytc_status_ze_result_exp_rtas_build_deferred = 0x1000D,
    tinytc_status_ze_result_error_insufficient_permissions = 0x1000E,
    tinytc_status_ze_result_error_not_available = 0x1000F,
    tinytc_status_ze_result_error_dependency_unavailable = 0x10010,
    tinytc_status_ze_result_warning_dropped_data = 0x10011,
    tinytc_status_ze_result_error_uninitialized = 0x10012,
    tinytc_status_ze_result_error_unsupported_version = 0x10013,
    tinytc_status_ze_result_error_unsupported_feature = 0x10014,
    tinytc_status_ze_result_error_invalid_argument = 0x10015,
    tinytc_status_ze_result_error_invalid_null_handle = 0x10016,
    tinytc_status_ze_result_error_handle_object_in_use = 0x10017,
    tinytc_status_ze_result_error_invalid_null_pointer = 0x10018,
    tinytc_status_ze_result_error_invalid_size = 0x10019,
    tinytc_status_ze_result_error_unsupported_size = 0x1001A,
    tinytc_status_ze_result_error_unsupported_alignment = 0x1001B,
    tinytc_status_ze_result_error_invalid_synchronization_object = 0x1001C,
    tinytc_status_ze_result_error_invalid_enumeration = 0x1001D,
    tinytc_status_ze_result_error_unsupported_enumeration = 0x1001E,
    tinytc_status_ze_result_error_unsupported_image_format = 0x1001F,
    tinytc_status_ze_result_error_invalid_native_binary = 0x10020,
    tinytc_status_ze_result_error_invalid_global_name = 0x10021,
    tinytc_status_ze_result_error_invalid_kernel_name = 0x10022,
    tinytc_status_ze_result_error_invalid_function_name = 0x10023,
    tinytc_status_ze_result_error_invalid_group_size_dimension = 0x10024,
    tinytc_status_ze_result_error_invalid_global_width_dimension = 0x10025,
    tinytc_status_ze_result_error_invalid_kernel_argument_index = 0x10026,
    tinytc_status_ze_result_error_invalid_kernel_argument_size = 0x10027,
    tinytc_status_ze_result_error_invalid_kernel_attribute_value = 0x10028,
    tinytc_status_ze_result_error_invalid_module_unlinked = 0x10029,
    tinytc_status_ze_result_error_invalid_command_list_type = 0x1002A,
    tinytc_status_ze_result_error_overlapping_regions = 0x1002B,
    tinytc_status_ze_result_warning_action_required = 0x1002C,
    tinytc_status_ze_result_error_unknown = 0x1002D,
    // OpenCL errors
    tinytc_status_cl_build_program_failure = 0x20000,
    tinytc_status_cl_compile_program_failure = 0x20001,
    tinytc_status_cl_compiler_not_available = 0x20002,
    tinytc_status_cl_device_not_found = 0x20003,
    tinytc_status_cl_device_not_available = 0x20004,
    tinytc_status_cl_device_partition_failed = 0x20005,
    tinytc_status_cl_exec_status_error_for_events_in_wait_list = 0x20006,
    tinytc_status_cl_image_format_mismatch = 0x20007,
    tinytc_status_cl_image_format_not_supported = 0x20008,
    tinytc_status_cl_invalid_arg_index = 0x20009,
    tinytc_status_cl_invalid_arg_size = 0x2000A,
    tinytc_status_cl_invalid_arg_value = 0x2000B,
    tinytc_status_cl_invalid_binary = 0x2000C,
    tinytc_status_cl_invalid_buffer_size = 0x2000D,
    tinytc_status_cl_invalid_build_options = 0x2000E,
    tinytc_status_cl_invalid_command_queue = 0x2000F,
    tinytc_status_cl_invalid_compiler_options = 0x20010,
    tinytc_status_cl_invalid_context = 0x20011,
    tinytc_status_cl_invalid_device = 0x20012,
    tinytc_status_cl_invalid_device_partition_count = 0x20013,
    tinytc_status_cl_invalid_device_queue = 0x20014,
    tinytc_status_cl_invalid_device_type = 0x20015,
    tinytc_status_cl_invalid_event = 0x20016,
    tinytc_status_cl_invalid_event_wait_list = 0x20017,
    tinytc_status_cl_invalid_global_offset = 0x20018,
    tinytc_status_cl_invalid_global_work_size = 0x20019,
    tinytc_status_cl_invalid_host_ptr = 0x2001A,
    tinytc_status_cl_invalid_image_descriptor = 0x2001B,
    tinytc_status_cl_invalid_image_format_descriptor = 0x2001C,
    tinytc_status_cl_invalid_image_size = 0x2001D,
    tinytc_status_cl_invalid_kernel = 0x2001E,
    tinytc_status_cl_invalid_kernel_args = 0x2001F,
    tinytc_status_cl_invalid_kernel_definition = 0x20020,
    tinytc_status_cl_invalid_kernel_name = 0x20021,
    tinytc_status_cl_invalid_linker_options = 0x20022,
    tinytc_status_cl_invalid_mem_object = 0x20023,
    tinytc_status_cl_invalid_operation = 0x20024,
    tinytc_status_cl_invalid_pipe_size = 0x20025,
    tinytc_status_cl_invalid_platform = 0x20026,
    tinytc_status_cl_invalid_program = 0x20027,
    tinytc_status_cl_invalid_program_executable = 0x20028,
    tinytc_status_cl_invalid_property = 0x20029,
    tinytc_status_cl_invalid_queue_properties = 0x2002A,
    tinytc_status_cl_invalid_sampler = 0x2002B,
    tinytc_status_cl_invalid_spec_id = 0x2002C,
    tinytc_status_cl_invalid_value = 0x2002D,
    tinytc_status_cl_invalid_work_dimension = 0x2002E,
    tinytc_status_cl_invalid_work_group_size = 0x2002F,
    tinytc_status_cl_invalid_work_item_size = 0x20030,
    tinytc_status_cl_kernel_arg_info_not_available = 0x20031,
    tinytc_status_cl_link_program_failure = 0x20032,
    tinytc_status_cl_linker_not_available = 0x20033,
    tinytc_status_cl_map_failure = 0x20034,
    tinytc_status_cl_mem_copy_overlap = 0x20035,
    tinytc_status_cl_mem_object_allocation_failure = 0x20036,
    tinytc_status_cl_misaligned_sub_buffer_offset = 0x20037,
    tinytc_status_cl_out_of_host_memory = 0x20038,
    tinytc_status_cl_out_of_resources = 0x20039,
    tinytc_status_cl_max_size_restriction_exceeded = 0x2003A,
    tinytc_status_cl_profiling_info_not_available = 0x2003B,
    // SYCL errors
    tinytc_status_sycl_runtime = 0x30000,
    tinytc_status_sycl_kernel = 0x30001,
    tinytc_status_sycl_accessor = 0x30002,
    tinytc_status_sycl_nd_range = 0x30003,
    tinytc_status_sycl_event = 0x30004,
    tinytc_status_sycl_kernel_argument = 0x30005,
    tinytc_status_sycl_build = 0x30006,
    tinytc_status_sycl_invalid = 0x30007,
    tinytc_status_sycl_memory_allocation = 0x30008,
    tinytc_status_sycl_platform = 0x30009,
    tinytc_status_sycl_profiling = 0x3000A,
    tinytc_status_sycl_feature_not_supported = 0x3000B,
    tinytc_status_sycl_kernel_not_supported = 0x3000C,
    tinytc_status_sycl_backend_mismatch = 0x3000D,
    // The unknown error comes last
    tinytc_status_unknown = 0x7fffffff
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

//! Core features that may be optionally enabled
typedef enum {
    /**
     * Request a large register file.
     * On PVC this doubles the number of registers per vector engine
     * but halves the number of available hardware threads.
     * When this feature is activated, the kernel is compiled with
     * the "-ze-opt-large-register-file" option.
     */
    tinytc_core_feature_flag_large_register_file = 0x1
} tinytc_core_feature_flag_t;

/**
 * @brief IP versions for Intel GPUs
 *
 * Note: IP versions are extracted from
 * * https://github.com/intel/compute-runtime/blob/4b5d5f235abf0ff67c9188f8096afd4da2e0574d/third_party/aot_config_headers/platforms.h
 * * https://github.com/intel/llvm/blob/56e9067ba69809fb6ea1fd4328456ca3a009f984/sycl/source/detail/device_info.hpp#L619
 */
typedef enum {
    tinytc_intel_gpu_architecture_pvc = 0x030f0007 ///< PVC
} tinytc_intel_gpu_architecture_t;

//! Target binary format
typedef enum {
    tinytc_bundle_format_spirv = 0, ///< SPIR-V
    tinytc_bundle_format_native = 1 ///< Native device binary
} tinytc_bundle_format_t;

//! Memory object type
typedef enum {
    tinytc_mem_type_buffer = 0x0,      ///< Buffer object (e.g. cl_mem)
    tinytc_mem_type_usm_pointer = 0x1, ///< Unified shared memory pointer
    tinytc_mem_type_svm_pointer = 0x2, ///< Shared virtual memory pointer
} tinytc_mem_type_t;

////////////////////////////
/////////// Types //////////
////////////////////////////

typedef uint8_t tinytc_bool_t;

//! @struct tinytc_data_type
//! @brief Opaque struct for a data type
struct tinytc_data_type;
//! @brief data_type handle
typedef struct tinytc_data_type *tinytc_data_type_t;
typedef const struct tinytc_data_type *const_tinytc_data_type_t;

//! @struct tinytc_value
//! @brief Opaque struct for a value
struct tinytc_value;
//! @brief value handle
typedef struct tinytc_value *tinytc_value_t;
typedef const struct tinytc_value *const_tinytc_value_t;

//! @struct tinytc_inst
//! @brief Opaque struct for an instruction
struct tinytc_inst;
//! @brief inst handle
typedef struct tinytc_inst *tinytc_inst_t;
typedef const struct tinytc_inst *const_tinytc_inst_t;

//! @struct tinytc_region
//! @brief Opaque struct for a region
struct tinytc_region;
//! @brief region handle
typedef struct tinytc_region *tinytc_region_t;
typedef const struct tinytc_region *const_tinytc_region_t;

//! @struct tinytc_func
//! @brief Opaque struct for a function
struct tinytc_func;
//! @brief func handle
typedef struct tinytc_func *tinytc_func_t;
typedef const struct tinytc_func *const_tinytc_func_t;

//! @struct tinytc_prog
//! @brief Opaque struct for a program
struct tinytc_prog;
//! @brief prog handle
typedef struct tinytc_prog *tinytc_prog_t;
typedef const struct tinytc_prog *const_tinytc_prog_t;

//! @struct tinytc_core_info;
//! @brief Opaque struct for core information
struct tinytc_core_info;
//! @brief core_info handle
typedef struct tinytc_core_info *tinytc_core_info_t;
typedef const struct tinytc_core_info *const_tinytc_core_info_t;

//! @struct tinytc_source;
//! @brief Opaque struct for source text
struct tinytc_source;
//! @brief source handle
typedef struct tinytc_source *tinytc_source_t;
typedef const struct tinytc_source *const_tinytc_source_t;

//! @struct tintyc_source_context
//! @brief Opaque struct for source context
struct tinytc_source_context;
//! @brief source context handle
typedef struct tinytc_source_context *tinytc_source_context_t;
typedef const struct tinytc_source_context *const_tinytc_source_context_t;

//! @struct tinytc_binary;
//! @brief Opaque struct for a binary
struct tinytc_binary;
//! @brief binary handle
typedef struct tinytc_binary *tinytc_binary_t;
typedef const struct tinytc_binary *const_tinytc_binary_t;

//! @struct tinytc_recipe;
//! @brief Opaque struct for a recipe
struct tinytc_recipe;
//! @brief recipe handle
typedef struct tinytc_recipe *tinytc_recipe_t;
typedef const struct tinytc_recipe *const_tinytc_recipe_t;

//! @struct tinytc_recipe_handler;
//! @brief Opaque struct for a recipe handler
struct tinytc_recipe_handler;
//! @brief recipe_handler handle
typedef struct tinytc_recipe_handler *tinytc_recipe_handler_t;
typedef const struct tinytc_recipe_handler *const_tinytc_recipe_handler_t;

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
    tinytc_position_t begin; ///< Starting position
    tinytc_position_t end;   ///< End position
} tinytc_location_t;

//! @brief Memory object
typedef struct tinytc_mem {
    const void *value;      ///< Pointer value or pointer to buffer object
    tinytc_mem_type_t type; ///< Memory object type
} tinytc_mem_t;

#ifdef __cplusplus
}
#endif

#endif // TYPES_20240410_H
