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

/**
 * @brief Status codes
 */
typedef enum {
    tinytc_status_success = 0x0,                     ///< Success
    tinytc_status_bad_alloc = 0x1,                   ///< Failure to allocate storage
    tinytc_status_invalid_arguments = 0x2,           ///< Function got invalid arguments
    tinytc_status_out_of_range = 0x3,                ///< Element access out of bounds
    tinytc_status_runtime_error = 0x4,               ///< Runtime error
    tinytc_status_internal_compiler_error = 0x5,     ///< Internal compiler error
    tinytc_status_unsupported_subgroup_size = 0x6,   ///< Device does not support subgroup size
    tinytc_status_unsupported_work_group_size = 0x7, ///< Device does not support work-group size
    tinytc_status_compilation_error = 0x8,           ///< Compilation error
    tinytc_status_file_io_error = 0x9,               ///< Error during File I/O
    tinytc_status_parse_error = 0xa,                 ///< Error during parsing
    tinytc_status_unavailable_extension = 0xb,       ///< Unavailable runtime extension
    tinytc_status_unsupported_backend = 0xc,         ///< Unsupported backend (SYCL runtime)
    tinytc_status_invalid_kernel_arguments = 0xd,    ///< Kernel got invalid arguments
    tinytc_status_unsupported_device = 0xe,          ///< Unsupported device
    tinytc_status_invalid_core_info = 0xf,           ///< Invalid core info object
    // IR errors
    tinytc_status_ir_out_of_bounds = 0x100,             ///< Out of bounds access
    tinytc_status_ir_invalid_shape = 0x101,             ///< Invalid tensor shape
    tinytc_status_ir_incompatible_shapes = 0x102,       ///< Tensor shape requirements not satisfied
    tinytc_status_ir_shape_stride_mismatch = 0x103,     ///< Mismatch of shape and stride
    tinytc_status_ir_scalar_mismatch = 0x104,           ///< Mismatch of scalar types
    tinytc_status_ir_invalid_number_of_indices = 0x105, /// Invalid number of indices
    tinytc_status_ir_expected_scalar = 0x106,           ///< Expected a value of scalar type
    tinytc_status_ir_expected_memref = 0x107,           ///< Expected a value of memref type
    tinytc_status_ir_expected_memref_or_scalar = 0x108, ///< Expected memref or scalar type
    tinytc_status_ir_expected_memref_or_group = 0x109, ///< Expected a value of memref or group type
    tinytc_status_ir_expected_vector_or_matrix = 0x10a,    ///< Expected a vector or marix
    tinytc_status_ir_unexpected_yield = 0x10b,             ///< Unexpected yield instruction
    tinytc_status_ir_yield_mismatch = 0x10c,               ///< Wrong number of yielded values
    tinytc_status_ir_multiple_dynamic_modes = 0x10d,       ///< At most one mode must be dynamic
    tinytc_status_ir_invalid_slice = 0x10e,                ///< Invalid slice
    tinytc_status_ir_expand_shape_order_too_small = 0x10f, ///< Expand shape too small
    tinytc_status_ir_expand_shape_mismatch = 0x110,        ///< Invalid expand shape
    tinytc_status_ir_collective_called_from_spmd = 0x111,  ///< Collective instruction from SPMD
    tinytc_status_ir_fp_unsupported = 0x112, ///< Instruction does not support floating type
    tinytc_status_ir_spmd_called_from_collective = 0x113, ///< SPMD instruction from collective
    // Level zero errors
    tinytc_status_ze_result_not_ready = 0x10000,         ///< ZE_RESULT_NOT_READY
    tinytc_status_ze_result_error_device_lost = 0x10001, ///< ZE_RESULT_ERROR_DEVICE_LOST
    tinytc_status_ze_result_error_out_of_host_memory =
        0x10002, ///< ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY
    tinytc_status_ze_result_error_out_of_device_memory =
        0x10003, ///< ZE_RESULT_ERROR_OUT_OF_DEVICE_MEMORY
    tinytc_status_ze_result_error_module_build_failure =
        0x10004, ///< ZE_RESULT_ERROR_MODULE_BUILD_FAILURE
    tinytc_status_ze_result_error_module_link_failure =
        0x10005, ///< ZE_RESULT_ERROR_MODULE_LINK_FAILURE
    tinytc_status_ze_result_error_device_requires_reset =
        0x10006, ///< ZE_RESULT_ERROR_DEVICE_REQUIRES_RESET
    tinytc_status_ze_result_error_device_in_low_power_state =
        0x10007, ///< ZE_RESULT_ERROR_DEVICE_IN_LOW_POWER_STATE
    tinytc_status_ze_result_exp_error_device_is_not_vertex =
        0x10008, ///< ZE_RESULT_EXP_ERROR_DEVICE_IS_NOT_VERTEX
    tinytc_status_ze_result_exp_error_vertex_is_not_device =
        0x10009, ///< ZE_RESULT_EXP_ERROR_VERTEX_IS_NOT_DEVICE
    tinytc_status_ze_result_exp_error_remote_device =
        0x1000A, ///< ZE_RESULT_EXP_ERROR_REMOTE_DEVICE
    tinytc_status_ze_result_exp_error_operands_incompatible =
        0x1000B, ///< ZE_RESULT_EXP_ERROR_OPERANDS_INCOMPATIBLE
    tinytc_status_ze_result_exp_rtas_build_retry = 0x1000C, ///< ZE_RESULT_EXP_RTAS_BUILD_RETRY
    tinytc_status_ze_result_exp_rtas_build_deferred =
        0x1000D, ///< ZE_RESULT_EXP_RTAS_BUILD_DEFERRED
    tinytc_status_ze_result_error_insufficient_permissions =
        0x1000E, ///< ZE_RESULT_ERROR_INSUFFICIENT_PERMISSIONS
    tinytc_status_ze_result_error_not_available = 0x1000F, ///< ZE_RESULT_ERROR_NOT_AVAILABLE
    tinytc_status_ze_result_error_dependency_unavailable =
        0x10010, ///< ZE_RESULT_ERROR_DEPENDENCY_UNAVAILABLE
    tinytc_status_ze_result_warning_dropped_data = 0x10011, ///< ZE_RESULT_WARNING_DROPPED_DATA
    tinytc_status_ze_result_error_uninitialized = 0x10012,  ///< ZE_RESULT_ERROR_UNINITIALIZED
    tinytc_status_ze_result_error_unsupported_version =
        0x10013, ///< ZE_RESULT_ERROR_UNSUPPORTED_VERSION
    tinytc_status_ze_result_error_unsupported_feature =
        0x10014, ///< ZE_RESULT_ERROR_UNSUPPORTED_FEATURE
    tinytc_status_ze_result_error_invalid_argument = 0x10015, ///< ZE_RESULT_ERROR_INVALID_ARGUMENT
    tinytc_status_ze_result_error_invalid_null_handle =
        0x10016, ///< ZE_RESULT_ERROR_INVALID_NULL_HANDLE
    tinytc_status_ze_result_error_handle_object_in_use =
        0x10017, ///< ZE_RESULT_ERROR_HANDLE_OBJECT_IN_USE
    tinytc_status_ze_result_error_invalid_null_pointer =
        0x10018,                                          ///< ZE_RESULT_ERROR_INVALID_NULL_POINTER
    tinytc_status_ze_result_error_invalid_size = 0x10019, ///< ZE_RESULT_ERROR_INVALID_SIZE
    tinytc_status_ze_result_error_unsupported_size = 0x1001A, ///< ZE_RESULT_ERROR_UNSUPPORTED_SIZE
    tinytc_status_ze_result_error_unsupported_alignment =
        0x1001B, ///< ZE_RESULT_ERROR_UNSUPPORTED_ALIGNMENT
    tinytc_status_ze_result_error_invalid_synchronization_object =
        0x1001C, ///< ZE_RESULT_ERROR_INVALID_SYNCHRONIZATION_OBJECT
    tinytc_status_ze_result_error_invalid_enumeration =
        0x1001D, ///< ZE_RESULT_ERROR_INVALID_ENUMERATION
    tinytc_status_ze_result_error_unsupported_enumeration =
        0x1001E, ///< ZE_RESULT_ERROR_UNSUPPORTED_ENUMERATION
    tinytc_status_ze_result_error_unsupported_image_format =
        0x1001F, ///< ZE_RESULT_ERROR_UNSUPPORTED_IMAGE_FORMAT
    tinytc_status_ze_result_error_invalid_native_binary =
        0x10020, ///< ZE_RESULT_ERROR_INVALID_NATIVE_BINARY
    tinytc_status_ze_result_error_invalid_global_name =
        0x10021, ///< ZE_RESULT_ERROR_INVALID_GLOBAL_NAME
    tinytc_status_ze_result_error_invalid_kernel_name =
        0x10022, ///< ZE_RESULT_ERROR_INVALID_KERNEL_NAME
    tinytc_status_ze_result_error_invalid_function_name =
        0x10023, ///< ZE_RESULT_ERROR_INVALID_FUNCTION_NAME
    tinytc_status_ze_result_error_invalid_group_size_dimension =
        0x10024, ///< ZE_RESULT_ERROR_INVALID_GROUP_SIZE_DIMENSION
    tinytc_status_ze_result_error_invalid_global_width_dimension =
        0x10025, ///< ZE_RESULT_ERROR_INVALID_GLOBAL_WIDTH_DIMENSION
    tinytc_status_ze_result_error_invalid_kernel_argument_index =
        0x10026, ///< ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_INDEX
    tinytc_status_ze_result_error_invalid_kernel_argument_size =
        0x10027, ///< ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_SIZE
    tinytc_status_ze_result_error_invalid_kernel_attribute_value =
        0x10028, ///< ZE_RESULT_ERROR_INVALID_KERNEL_ATTRIBUTE_VALUE
    tinytc_status_ze_result_error_invalid_module_unlinked =
        0x10029, ///< ZE_RESULT_ERROR_INVALID_MODULE_UNLINKED
    tinytc_status_ze_result_error_invalid_command_list_type =
        0x1002A, ///< ZE_RESULT_ERROR_INVALID_COMMAND_LIST_TYPE
    tinytc_status_ze_result_error_overlapping_regions =
        0x1002B, ///< ZE_RESULT_ERROR_OVERLAPPING_REGIONS
    tinytc_status_ze_result_warning_action_required =
        0x1002C,                                     ///< ZE_RESULT_WARNING_ACTION_REQUIRED
    tinytc_status_ze_result_error_unknown = 0x1002D, ///< ZE_RESULT_ERROR_UNKNOWN
    // OpenCL errors
    tinytc_status_cl_build_program_failure = 0x20000,   ///< CL_BUILD_PROGRAM_FAILURE
    tinytc_status_cl_compile_program_failure = 0x20001, ///< CL_COMPILE_PROGRAM_FAILURE
    tinytc_status_cl_compiler_not_available = 0x20002,  ///< CL_COMPILER_NOT_AVAILABLE
    tinytc_status_cl_device_not_found = 0x20003,        ///< CL_DEVICE_NOT_FOUND
    tinytc_status_cl_device_not_available = 0x20004,    ///< CL_DEVICE_NOT_AVAILABLE
    tinytc_status_cl_device_partition_failed = 0x20005, ///< CL_DEVICE_PARTITION_FAILED
    tinytc_status_cl_exec_status_error_for_events_in_wait_list =
        0x20006, ///< CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST
    tinytc_status_cl_image_format_mismatch = 0x20007,      ///< CL_IMAGE_FORMAT_MISMATCH
    tinytc_status_cl_image_format_not_supported = 0x20008, ///< CL_IMAGE_FORMAT_NOT_SUPPORTED
    tinytc_status_cl_invalid_arg_index = 0x20009,          ///< CL_INVALID_ARG_INDEX
    tinytc_status_cl_invalid_arg_size = 0x2000A,           ///< CL_INVALID_ARG_SIZE
    tinytc_status_cl_invalid_arg_value = 0x2000B,          ///< CL_INVALID_ARG_VALUE
    tinytc_status_cl_invalid_binary = 0x2000C,             ///< CL_INVALID_BINARY
    tinytc_status_cl_invalid_buffer_size = 0x2000D,        ///< CL_INVALID_BUFFER_SIZE
    tinytc_status_cl_invalid_build_options = 0x2000E,      ///< CL_INVALID_BUILD_OPTIONS
    tinytc_status_cl_invalid_command_queue = 0x2000F,      ///< CL_INVALID_COMMAND_QUEUE
    tinytc_status_cl_invalid_compiler_options = 0x20010,   ///< CL_INVALID_COMPILER_OPTIONS
    tinytc_status_cl_invalid_context = 0x20011,            ///< CL_INVALID_CONTEXT
    tinytc_status_cl_invalid_device = 0x20012,             ///< CL_INVALID_DEVICE
    tinytc_status_cl_invalid_device_partition_count =
        0x20013,                                         ///< CL_INVALID_DEVICE_PARTITION_COUNT
    tinytc_status_cl_invalid_device_queue = 0x20014,     ///< CL_INVALID_DEVICE_QUEUE
    tinytc_status_cl_invalid_device_type = 0x20015,      ///< CL_INVALID_DEVICE_TYPE
    tinytc_status_cl_invalid_event = 0x20016,            ///< CL_INVALID_EVENT
    tinytc_status_cl_invalid_event_wait_list = 0x20017,  ///< CL_INVALID_EVENT_WAIT_LIST
    tinytc_status_cl_invalid_global_offset = 0x20018,    ///< CL_INVALID_GLOBAL_OFFSET
    tinytc_status_cl_invalid_global_work_size = 0x20019, ///< CL_INVALID_GLOBAL_WORK_SIZE
    tinytc_status_cl_invalid_host_ptr = 0x2001A,         ///< CL_INVALID_HOST_PTR
    tinytc_status_cl_invalid_image_descriptor = 0x2001B, ///< CL_INVALID_IMAGE_DESCRIPTOR
    tinytc_status_cl_invalid_image_format_descriptor =
        0x2001C,                                           ///< CL_INVALID_IMAGE_FORMAT_DESCRIPTOR
    tinytc_status_cl_invalid_image_size = 0x2001D,         ///< CL_INVALID_IMAGE_SIZE
    tinytc_status_cl_invalid_kernel = 0x2001E,             ///< CL_INVALID_KERNEL
    tinytc_status_cl_invalid_kernel_args = 0x2001F,        ///< CL_INVALID_KERNEL_ARGS
    tinytc_status_cl_invalid_kernel_definition = 0x20020,  ///< CL_INVALID_KERNEL_DEFINITION
    tinytc_status_cl_invalid_kernel_name = 0x20021,        ///< CL_INVALID_KERNEL_NAME
    tinytc_status_cl_invalid_linker_options = 0x20022,     ///< CL_INVALID_LINKER_OPTIONS
    tinytc_status_cl_invalid_mem_object = 0x20023,         ///< CL_INVALID_MEM_OBJECT
    tinytc_status_cl_invalid_operation = 0x20024,          ///< CL_INVALID_OPERATION
    tinytc_status_cl_invalid_pipe_size = 0x20025,          ///< CL_INVALID_PIPE_SIZE
    tinytc_status_cl_invalid_platform = 0x20026,           ///< CL_INVALID_PLATFORM
    tinytc_status_cl_invalid_program = 0x20027,            ///< CL_INVALID_PROGRAM
    tinytc_status_cl_invalid_program_executable = 0x20028, ///< CL_INVALID_PROGRAM_EXECUTABLE
    tinytc_status_cl_invalid_property = 0x20029,           ///< CL_INVALID_PROPERTY
    tinytc_status_cl_invalid_queue_properties = 0x2002A,   ///< CL_INVALID_QUEUE_PROPERTIES
    tinytc_status_cl_invalid_sampler = 0x2002B,            ///< CL_INVALID_SAMPLER
    tinytc_status_cl_invalid_spec_id = 0x2002C,            ///< CL_INVALID_SPEC_ID
    tinytc_status_cl_invalid_value = 0x2002D,              ///< CL_INVALID_VALUE
    tinytc_status_cl_invalid_work_dimension = 0x2002E,     ///< CL_INVALID_WORK_DIMENSION
    tinytc_status_cl_invalid_work_group_size = 0x2002F,    ///< CL_INVALID_WORK_GROUP_SIZE
    tinytc_status_cl_invalid_work_item_size = 0x20030,     ///< CL_INVALID_WORK_ITEM_SIZE
    tinytc_status_cl_kernel_arg_info_not_available = 0x20031, ///< CL_KERNEL_ARG_INFO_NOT_AVAILABLE
    tinytc_status_cl_link_program_failure = 0x20032,          ///< CL_LINK_PROGRAM_FAILURE
    tinytc_status_cl_linker_not_available = 0x20033,          ///< CL_LINKER_NOT_AVAILABLE
    tinytc_status_cl_map_failure = 0x20034,                   ///< CL_MAP_FAILURE
    tinytc_status_cl_mem_copy_overlap = 0x20035,              ///< CL_MEM_COPY_OVERLAP
    tinytc_status_cl_mem_object_allocation_failure = 0x20036, ///< CL_MEM_OBJECT_ALLOCATION_FAILURE
    tinytc_status_cl_misaligned_sub_buffer_offset = 0x20037,  ///< CL_MISALIGNED_SUB_BUFFER_OFFSET
    tinytc_status_cl_out_of_host_memory = 0x20038,            ///< CL_OUT_OF_HOST_MEMORY
    tinytc_status_cl_out_of_resources = 0x20039,              ///< CL_OUT_OF_RESOURCES
    tinytc_status_cl_max_size_restriction_exceeded = 0x2003A, ///< CL_MAX_SIZE_RESTRICTION_EXCEEDED
    tinytc_status_cl_profiling_info_not_available = 0x2003B,  ///< CL_PROFILING_INFO_NOT_AVAILABLE
    // The unknown error comes last
    tinytc_status_unknown = 0x7fffffff ///< Unknown error occured
} tinytc_status_t;

//! Scalar types
typedef enum {
    tinytc_scalar_type_i1 = 0,    ///< Signed 1 bit integer (boolean)
    tinytc_scalar_type_i8 = 1,    ///< Signed 8 bit integer
    tinytc_scalar_type_i16 = 2,   ///< Signed 16 bit integer
    tinytc_scalar_type_i32 = 3,   ///< Signed 32 bit integer
    tinytc_scalar_type_i64 = 4,   ///< Signed 64 bit integer
    tinytc_scalar_type_index = 5, ///< Integer type for indices
    tinytc_scalar_type_f32 = 6,   ///< Single precision floating point (32 bit)
    tinytc_scalar_type_f64 = 7,   ///< Double precision floating point (64 bit)
    tinytc_scalar_type_c32 = 8,   ///< Single precision complex (2x32 bit)
    tinytc_scalar_type_c64 = 9    ///< Double precision complex (2x64 bit)
} tinytc_scalar_type_t;

//! Arithmetic operations
typedef enum {
    tinytc_arithmetic_add = 0, ///< add
    tinytc_arithmetic_sub = 1, ///< subtract
    tinytc_arithmetic_mul = 2, ///< multiply
    tinytc_arithmetic_div = 3, ///< divide
    tinytc_arithmetic_rem = 4, ///< division remainder
    tinytc_arithmetic_shl = 5, ///< left shift
    tinytc_arithmetic_shr = 6, ///< arithmetic right shift
    tinytc_arithmetic_and = 7, ///< bitwise and
    tinytc_arithmetic_or = 8,  ///< bitwise or
    tinytc_arithmetic_xor = 9  ///< bitwise xor
} tinytc_arithmetic_t;

//! Arithmetic operations (unary)
typedef enum {
    tinytc_arithmetic_unary_neg = 0, ///< negation
    tinytc_arithmetic_unary_not = 1  ///< bitwise not
} tinytc_arithmetic_unary_t;

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
    tinytc_transpose_N = 0, ///< No transpose
    tinytc_transpose_T = 1  ///< Transpose
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

//! Type for combination of core feature flags
typedef uint32_t tinytc_core_feature_flags_t;

/**
 * @brief IP versions for Intel GPUs
 *
 * Note: IP versions are extracted from
 * * https://github.com/intel/compute-runtime/blob/4b5d5f235abf0ff67c9188f8096afd4da2e0574d/third_party/aot_config_headers/platforms.h
 * * https://github.com/intel/llvm/blob/56e9067ba69809fb6ea1fd4328456ca3a009f984/sycl/source/detail/device_info.hpp#L619
 */
typedef enum {
    tinytc_intel_gpu_architecture_tgl = 0x03000000, ///< Tiger Lake
    tinytc_intel_gpu_architecture_pvc = 0x030f0007  ///< Ponte Vecchio
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

//! Support level of a device
typedef enum {
    //! Device is unsupported (e.g. subgroups feature missing in OpenCL-C)
    tinytc_support_level_none = 0x0,
    //! Device provides necessary features but is not well tested
    tinytc_support_level_basic = 0x1,
    //! Device provides necessary features and is well tested
    tinytc_support_level_tuned = 0x2
} tinytc_support_level_t;

////////////////////////////
/////////// Types //////////
////////////////////////////

//! @brief Bool type {0,1}
typedef uint8_t tinytc_bool_t;

//! @struct tinytc_data_type
//! @brief Opaque struct for a data type
struct tinytc_data_type;
//! @brief data_type handle
typedef struct tinytc_data_type *tinytc_data_type_t;
//! @brief const data_type handle
typedef const struct tinytc_data_type *const_tinytc_data_type_t;

//! @struct tinytc_value
//! @brief Opaque struct for a value
struct tinytc_value;
//! @brief value handle
typedef struct tinytc_value *tinytc_value_t;
//! @brief const value handle
typedef const struct tinytc_value *const_tinytc_value_t;

//! @struct tinytc_inst
//! @brief Opaque struct for an instruction
struct tinytc_inst;
//! @brief inst handle
typedef struct tinytc_inst *tinytc_inst_t;
//! @brief const inst handle
typedef const struct tinytc_inst *const_tinytc_inst_t;

//! @struct tinytc_region
//! @brief Opaque struct for a region
struct tinytc_region;
//! @brief region handle
typedef struct tinytc_region *tinytc_region_t;
//! @brief const region handle
typedef const struct tinytc_region *const_tinytc_region_t;

//! @struct tinytc_func
//! @brief Opaque struct for a function
struct tinytc_func;
//! @brief func handle
typedef struct tinytc_func *tinytc_func_t;
//! @brief const func handle
typedef const struct tinytc_func *const_tinytc_func_t;

//! @struct tinytc_prog
//! @brief Opaque struct for a program
struct tinytc_prog;
//! @brief prog handle
typedef struct tinytc_prog *tinytc_prog_t;
//! @brief const prog handle
typedef const struct tinytc_prog *const_tinytc_prog_t;

//! @struct tinytc_core_info;
//! @brief Opaque struct for core information
struct tinytc_core_info;
//! @brief core_info handle
typedef struct tinytc_core_info *tinytc_core_info_t;
//! @brief const core_info handle
typedef const struct tinytc_core_info *const_tinytc_core_info_t;

//! @struct tinytc_source;
//! @brief Opaque struct for source text
struct tinytc_source;
//! @brief source handle
typedef struct tinytc_source *tinytc_source_t;
//! @brief const source handle
typedef const struct tinytc_source *const_tinytc_source_t;

//! @struct tintyc_source_context
//! @brief Opaque struct for source context
struct tinytc_source_context;
//! @brief source_context handle
typedef struct tinytc_source_context *tinytc_source_context_t;
//! @brief const source_context handle
typedef const struct tinytc_source_context *const_tinytc_source_context_t;

//! @struct tinytc_binary;
//! @brief Opaque struct for a binary
struct tinytc_binary;
//! @brief binary handle
typedef struct tinytc_binary *tinytc_binary_t;
//! @brief const binary handle
typedef const struct tinytc_binary *const_tinytc_binary_t;

//! @struct tinytc_recipe;
//! @brief Opaque struct for a recipe
struct tinytc_recipe;
//! @brief recipe handle
typedef struct tinytc_recipe *tinytc_recipe_t;
//! @brief const recipe handle
typedef const struct tinytc_recipe *const_tinytc_recipe_t;

//! @struct tinytc_recipe_handler;
//! @brief Opaque struct for a recipe handler
struct tinytc_recipe_handler;
//! @brief recipe_handler handle
typedef struct tinytc_recipe_handler *tinytc_recipe_handler_t;
//! @brief const recipe_handler handle
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

#ifdef __cplusplus
}
#endif

#endif // TYPES_20240410_H
