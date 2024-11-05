// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "error.hpp"
#include "location.hpp"
#include "tinytc/tinytc.h"

#include <cctype>
#include <sstream>
#include <utility>

namespace tinytc {

compilation_error::compilation_error(location const &loc, status code, std::string extra_info)
    : loc_(loc), code_(code), extra_info_(std::move(extra_info)) {}

auto report_error_with_context(char const *code, std::size_t code_len, std::string const &file_name,
                               location const &l, std::string const &what) -> std::string {
    constexpr int additional_context_lines = 2;

    int cur_line = 1;
    const char *begin = code;
    const char *limit = begin + code_len;
    while (cur_line + additional_context_lines < l.begin.line && *begin != '\0' && begin <= limit) {
        if (*begin == '\n') {
            ++cur_line;
        }
        ++begin;
    }
    auto oerr = std::ostringstream{};
    char const *end = begin;
    int start_col = -1;
    while (cur_line <= l.end.line && *end != '\0' && end <= limit) {
        if (!std::isspace(*end) && start_col < 0) {
            start_col = static_cast<int>(end - begin);
        }
        if (*end == '\n') {
            // start_col < 0 => only white-space
            if (start_col < 0) {
                start_col = static_cast<int>(end - begin);
            }
            oerr << std::string(begin, end) << std::endl;
            if (cur_line >= l.begin.line) {
                int col_begin = 0;
                int num_col = 0;
                if (l.begin.line != l.end.line) {
                    if (cur_line == l.begin.line) {
                        col_begin = l.begin.column > 1 ? l.begin.column - 1 : 0;
                        num_col = static_cast<int>(end - begin) - col_begin;
                    } else if (cur_line == l.end.line) {
                        num_col = l.end.column > 1 ? l.end.column - 1 : 0;
                        if (num_col >= start_col) {
                            num_col -= start_col;
                            col_begin = start_col;
                        } else {
                            col_begin = 0;
                        }
                    } else {
                        col_begin = start_col;
                        num_col = static_cast<int>(end - begin) - col_begin;
                    }
                } else {
                    col_begin = l.begin.column > 1 ? l.begin.column - 1 : 0;
                    num_col = l.end.column > l.begin.column ? l.end.column - l.begin.column : 1;
                }
                oerr << std::string(col_begin, ' ') << std::string(num_col, '~') << std::endl;
            }
            ++cur_line;
            start_col = -1;
            begin = end + 1;
        }
        ++end;
    }
    oerr << file_name << ":";
    print_range(oerr, l.begin, l.end);
    oerr << ": " << what;
    return std::move(oerr).str();
}

} // namespace tinytc

extern "C" {
char const *tinytc_error_string(tinytc_status_t status) {
    switch (status) {
    case tinytc_status_success:
        return "Success";
    case tinytc_status_bad_alloc:
        return "Bad allocation";
    case tinytc_status_invalid_arguments:
        return "Invalid arguments passed to function";
    case tinytc_status_out_of_range:
        return "Out of range";
    case tinytc_status_runtime_error:
        return "General runtime error";
    case tinytc_status_internal_compiler_error:
        return "Internal compiler error";
    case tinytc_status_unsupported_subgroup_size:
        return "Unsupported subgroup size";
    case tinytc_status_unsupported_work_group_size:
        return "Work group size is larger than maximum work group size supported by device";
    case tinytc_status_compilation_error:
        return "Compilation error";
    case tinytc_status_file_io_error:
        return "I/O error occured in file operation";
    case tinytc_status_parse_error:
        return "Parse error";
    case tinytc_status_unavailable_extension:
        return "Required vendor extension is unavailable";
    case tinytc_status_unsupported_backend:
        return "Unsupport backend";
    case tinytc_status_invalid_kernel_arguments:
        return "Invalid arguments passed to kernel";
    case tinytc_status_unsupported_device:
        return "Unsupported device";
    case tinytc_status_invalid_core_info:
        return "Invalid core info object (e.g. max work group size is 0 or subgroup sizes vector "
               "is empty)";
    case tinytc_status_unknown_pass_name:
        return "Unknown compiler pass name";
    case tinytc_status_not_implemented:
        return "Not implemented";
    // IR
    case tinytc_status_ir_out_of_bounds:
        return "Argument is out of bounds";
    case tinytc_status_ir_invalid_shape:
        return "Mode size must be non-negative";
    case tinytc_status_ir_incompatible_shapes:
        return "Incompatible tensor shapes";
    case tinytc_status_ir_shape_stride_mismatch:
        return "Dimension of shape and stride must match";
    case tinytc_status_ir_scalar_mismatch:
        return "Scalar type mismatch";
    case tinytc_status_ir_invalid_number_of_indices:
        return "Number of indices must match memref order or must be 1 for group types";
    case tinytc_status_ir_expected_scalar:
        return "Expected scalar type";
    case tinytc_status_ir_expected_index:
        return "Expected index type";
    case tinytc_status_ir_expected_coopmatrix:
        return "Expected coopmatrix type";
    case tinytc_status_ir_expected_coopmatrix_or_scalar:
        return "Expected coopmatrix type or scalar type";
    case tinytc_status_ir_expected_memref:
        return "Expected memref type";
    case tinytc_status_ir_expected_memref_or_scalar:
        return "Expected memref type or scalar type";
    case tinytc_status_ir_expected_memref_or_group:
        return "Expected memref or group operand";
    case tinytc_status_ir_expected_matrix:
        return "Expected matrix input";
    case tinytc_status_ir_expected_vector_or_matrix:
        return "Expected vector or matrix input";
    case tinytc_status_ir_unexpected_yield:
        return "Yield encountered in non-yielding region";
    case tinytc_status_ir_yield_mismatch:
        return "Number of yielded values does not match number of values yielded by region";
    case tinytc_status_ir_subview_mismatch:
        return "Number of dynamic offsets and sizes must match number of dynamic operands";
    case tinytc_status_ir_invalid_slice:
        return "Static offset and size must be non-negative or dynamic ('?')";
    case tinytc_status_ir_expand_shape_order_too_small:
        return "Expand shape must have at least 2 entries";
    case tinytc_status_ir_expand_shape_mismatch:
        return "Number of dynamic expand shape operands must equal number of dynamic modes in "
               "static expand shape";
    case tinytc_status_ir_collective_called_from_spmd:
        return "Collective instruction must not be called from SPMD region";
    case tinytc_status_ir_fp_unsupported:
        return "Floating point type unsupported by instruction";
    case tinytc_status_ir_spmd_called_from_collective:
        return "SPMD instruction must not be called from collective region";
    case tinytc_status_ir_expected_local_address_space:
        return "A memref with local address space is expected";
    case tinytc_status_ir_expected_global_address_space:
        return "A memref with global address space is expected";
    case tinytc_status_ir_invalid_offset:
        return "Offset must be non-negative or dynamic";
    case tinytc_status_ir_int_unsupported:
        return "int type unsupported by instruction";
    case tinytc_status_ir_i1_unsupported:
        return "i1 type unsupported by instruction";
    case tinytc_status_ir_complex_unsupported:
        return "complex type unsupported by instruction";
    case tinytc_status_ir_coopmatrix_unsupported:
        return "coopmatrix type unsupported by instruction";
    case tinytc_status_ir_forbidden_cast:
        return "Forbidden cast";
    case tinytc_status_ir_invalid_beta:
        return "beta must be constant and 0 or 1 for atomic linear algebra operations";
    case tinytc_status_ir_init_return_mismatch:
        return "The number or types of the initial values does not match the return type list";
    case tinytc_status_ir_invalid_matrix_use:
        return "Operands have invalid matrix use";
    case tinytc_status_ir_unsupported_coopmatrix_shape:
        return "Unsupported coopmatrix shape for the combination of scalar type, matrix use, and "
               "target architecture";
    case tinytc_status_ir_incompatible_scalar_types:
        return "Scalar types violate compatibility rules";
    case tinytc_status_spirv_forbidden_forward_declaration:
        return "Forward declaration of id is forbidden";
    case tinytc_status_spirv_undefined_value:
        return "Undefined SPIR-V value";
    // Level Zero
    case tinytc_status_ze_result_not_ready:
        return "ZE_RESULT_NOT_READY";
    case tinytc_status_ze_result_error_device_lost:
        return "ZE_RESULT_ERROR_DEVICE_LOST";
    case tinytc_status_ze_result_error_out_of_host_memory:
        return "ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY";
    case tinytc_status_ze_result_error_out_of_device_memory:
        return "ZE_RESULT_ERROR_OUT_OF_DEVICE_MEMORY";
    case tinytc_status_ze_result_error_module_build_failure:
        return "ZE_RESULT_ERROR_MODULE_BUILD_FAILURE";
    case tinytc_status_ze_result_error_module_link_failure:
        return "ZE_RESULT_ERROR_MODULE_LINK_FAILURE";
    case tinytc_status_ze_result_error_device_requires_reset:
        return "ZE_RESULT_ERROR_DEVICE_REQUIRES_RESET";
    case tinytc_status_ze_result_error_device_in_low_power_state:
        return "ZE_RESULT_ERROR_DEVICE_IN_LOW_POWER_STATE";
    case tinytc_status_ze_result_exp_error_device_is_not_vertex:
        return "ZE_RESULT_EXP_ERROR_DEVICE_IS_NOT_VERTEX";
    case tinytc_status_ze_result_exp_error_vertex_is_not_device:
        return "ZE_RESULT_EXP_ERROR_VERTEX_IS_NOT_DEVICE";
    case tinytc_status_ze_result_exp_error_remote_device:
        return "ZE_RESULT_EXP_ERROR_REMOTE_DEVICE";
    case tinytc_status_ze_result_exp_error_operands_incompatible:
        return "ZE_RESULT_EXP_ERROR_OPERANDS_INCOMPATIBLE";
    case tinytc_status_ze_result_exp_rtas_build_retry:
        return "ZE_RESULT_EXP_RTAS_BUILD_RETRY";
    case tinytc_status_ze_result_exp_rtas_build_deferred:
        return "ZE_RESULT_EXP_RTAS_BUILD_DEFERRED";
    case tinytc_status_ze_result_error_insufficient_permissions:
        return "ZE_RESULT_ERROR_INSUFFICIENT_PERMISSIONS";
    case tinytc_status_ze_result_error_not_available:
        return "ZE_RESULT_ERROR_NOT_AVAILABLE";
    case tinytc_status_ze_result_error_dependency_unavailable:
        return "ZE_RESULT_ERROR_DEPENDENCY_UNAVAILABLE";
    case tinytc_status_ze_result_warning_dropped_data:
        return "ZE_RESULT_WARNING_DROPPED_DATA";
    case tinytc_status_ze_result_error_uninitialized:
        return "ZE_RESULT_ERROR_UNINITIALIZED";
    case tinytc_status_ze_result_error_unsupported_version:
        return "ZE_RESULT_ERROR_UNSUPPORTED_VERSION";
    case tinytc_status_ze_result_error_unsupported_feature:
        return "ZE_RESULT_ERROR_UNSUPPORTED_FEATURE";
    case tinytc_status_ze_result_error_invalid_argument:
        return "ZE_RESULT_ERROR_INVALID_ARGUMENT";
    case tinytc_status_ze_result_error_invalid_null_handle:
        return "ZE_RESULT_ERROR_INVALID_NULL_HANDLE";
    case tinytc_status_ze_result_error_handle_object_in_use:
        return "ZE_RESULT_ERROR_HANDLE_OBJECT_IN_USE";
    case tinytc_status_ze_result_error_invalid_null_pointer:
        return "ZE_RESULT_ERROR_INVALID_NULL_POINTER";
    case tinytc_status_ze_result_error_invalid_size:
        return "ZE_RESULT_ERROR_INVALID_SIZE";
    case tinytc_status_ze_result_error_unsupported_size:
        return "ZE_RESULT_ERROR_UNSUPPORTED_SIZE";
    case tinytc_status_ze_result_error_unsupported_alignment:
        return "ZE_RESULT_ERROR_UNSUPPORTED_ALIGNMENT";
    case tinytc_status_ze_result_error_invalid_synchronization_object:
        return "ZE_RESULT_ERROR_INVALID_SYNCHRONIZATION_OBJECT";
    case tinytc_status_ze_result_error_invalid_enumeration:
        return "ZE_RESULT_ERROR_INVALID_ENUMERATION";
    case tinytc_status_ze_result_error_unsupported_enumeration:
        return "ZE_RESULT_ERROR_UNSUPPORTED_ENUMERATION";
    case tinytc_status_ze_result_error_unsupported_image_format:
        return "ZE_RESULT_ERROR_UNSUPPORTED_IMAGE_FORMAT";
    case tinytc_status_ze_result_error_invalid_native_binary:
        return "ZE_RESULT_ERROR_INVALID_NATIVE_BINARY";
    case tinytc_status_ze_result_error_invalid_global_name:
        return "ZE_RESULT_ERROR_INVALID_GLOBAL_NAME";
    case tinytc_status_ze_result_error_invalid_kernel_name:
        return "ZE_RESULT_ERROR_INVALID_KERNEL_NAME";
    case tinytc_status_ze_result_error_invalid_function_name:
        return "ZE_RESULT_ERROR_INVALID_FUNCTION_NAME";
    case tinytc_status_ze_result_error_invalid_group_size_dimension:
        return "ZE_RESULT_ERROR_INVALID_GROUP_SIZE_DIMENSION";
    case tinytc_status_ze_result_error_invalid_global_width_dimension:
        return "ZE_RESULT_ERROR_INVALID_GLOBAL_WIDTH_DIMENSION";
    case tinytc_status_ze_result_error_invalid_kernel_argument_index:
        return "ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_INDEX";
    case tinytc_status_ze_result_error_invalid_kernel_argument_size:
        return "ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_SIZE";
    case tinytc_status_ze_result_error_invalid_kernel_attribute_value:
        return "ZE_RESULT_ERROR_INVALID_KERNEL_ATTRIBUTE_VALUE";
    case tinytc_status_ze_result_error_invalid_module_unlinked:
        return "ZE_RESULT_ERROR_INVALID_MODULE_UNLINKED";
    case tinytc_status_ze_result_error_invalid_command_list_type:
        return "ZE_RESULT_ERROR_INVALID_COMMAND_LIST_TYPE";
    case tinytc_status_ze_result_error_overlapping_regions:
        return "ZE_RESULT_ERROR_OVERLAPPING_REGIONS";
    case tinytc_status_ze_result_warning_action_required:
        return "ZE_RESULT_WARNING_ACTION_REQUIRED";
    case tinytc_status_ze_result_error_unknown:
        return "ZE_RESULT_ERROR_UNKNOWN";
    // OpenCL
    case tinytc_status_cl_build_program_failure:
        return "CL_BUILD_PROGRAM_FAILURE";
    case tinytc_status_cl_compile_program_failure:
        return "CL_COMPILE_PROGRAM_FAILURE";
    case tinytc_status_cl_compiler_not_available:
        return "CL_COMPILER_NOT_AVAILABLE";
    case tinytc_status_cl_device_not_found:
        return "CL_DEVICE_NOT_FOUND";
    case tinytc_status_cl_device_not_available:
        return "CL_DEVICE_NOT_AVAILABLE";
    case tinytc_status_cl_device_partition_failed:
        return "CL_DEVICE_PARTITION_FAILED";
    case tinytc_status_cl_exec_status_error_for_events_in_wait_list:
        return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
    case tinytc_status_cl_image_format_mismatch:
        return "CL_IMAGE_FORMAT_MISMATCH";
    case tinytc_status_cl_image_format_not_supported:
        return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
    case tinytc_status_cl_invalid_arg_index:
        return "CL_INVALID_ARG_INDEX";
    case tinytc_status_cl_invalid_arg_size:
        return "CL_INVALID_ARG_SIZE";
    case tinytc_status_cl_invalid_arg_value:
        return "CL_INVALID_ARG_VALUE";
    case tinytc_status_cl_invalid_binary:
        return "CL_INVALID_BINARY";
    case tinytc_status_cl_invalid_buffer_size:
        return "CL_INVALID_BUFFER_SIZE";
    case tinytc_status_cl_invalid_build_options:
        return "CL_INVALID_BUILD_OPTIONS";
    case tinytc_status_cl_invalid_command_queue:
        return "CL_INVALID_COMMAND_QUEUE";
    case tinytc_status_cl_invalid_compiler_options:
        return "CL_INVALID_COMPILER_OPTIONS";
    case tinytc_status_cl_invalid_context:
        return "CL_INVALID_CONTEXT";
    case tinytc_status_cl_invalid_device:
        return "CL_INVALID_DEVICE";
    case tinytc_status_cl_invalid_device_partition_count:
        return "CL_INVALID_DEVICE_PARTITION_COUNT";
    case tinytc_status_cl_invalid_device_queue:
        return "CL_INVALID_DEVICE_QUEUE";
    case tinytc_status_cl_invalid_device_type:
        return "CL_INVALID_DEVICE_TYPE";
    case tinytc_status_cl_invalid_event:
        return "CL_INVALID_EVENT";
    case tinytc_status_cl_invalid_event_wait_list:
        return "CL_INVALID_EVENT_WAIT_LIST";
    case tinytc_status_cl_invalid_global_offset:
        return "CL_INVALID_GLOBAL_OFFSET";
    case tinytc_status_cl_invalid_global_work_size:
        return "CL_INVALID_GLOBAL_WORK_SIZE";
    case tinytc_status_cl_invalid_host_ptr:
        return "CL_INVALID_HOST_PTR";
    case tinytc_status_cl_invalid_image_descriptor:
        return "CL_INVALID_IMAGE_DESCRIPTOR";
    case tinytc_status_cl_invalid_image_format_descriptor:
        return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
    case tinytc_status_cl_invalid_image_size:
        return "CL_INVALID_IMAGE_SIZE";
    case tinytc_status_cl_invalid_kernel:
        return "CL_INVALID_KERNEL";
    case tinytc_status_cl_invalid_kernel_args:
        return "CL_INVALID_KERNEL_ARGS";
    case tinytc_status_cl_invalid_kernel_definition:
        return "CL_INVALID_KERNEL_DEFINITION";
    case tinytc_status_cl_invalid_kernel_name:
        return "CL_INVALID_KERNEL_NAME";
    case tinytc_status_cl_invalid_linker_options:
        return "CL_INVALID_LINKER_OPTIONS";
    case tinytc_status_cl_invalid_mem_object:
        return "CL_INVALID_MEM_OBJECT";
    case tinytc_status_cl_invalid_operation:
        return "CL_INVALID_OPERATION";
    case tinytc_status_cl_invalid_pipe_size:
        return "CL_INVALID_PIPE_SIZE";
    case tinytc_status_cl_invalid_platform:
        return "CL_INVALID_PLATFORM";
    case tinytc_status_cl_invalid_program:
        return "CL_INVALID_PROGRAM";
    case tinytc_status_cl_invalid_program_executable:
        return "CL_INVALID_PROGRAM_EXECUTABLE";
    case tinytc_status_cl_invalid_property:
        return "CL_INVALID_PROPERTY";
    case tinytc_status_cl_invalid_queue_properties:
        return "CL_INVALID_QUEUE_PROPERTIES";
    case tinytc_status_cl_invalid_sampler:
        return "CL_INVALID_SAMPLER";
    case tinytc_status_cl_invalid_spec_id:
        return "CL_INVALID_SPEC_ID";
    case tinytc_status_cl_invalid_value:
        return "CL_INVALID_VALUE";
    case tinytc_status_cl_invalid_work_dimension:
        return "CL_INVALID_WORK_DIMENSION";
    case tinytc_status_cl_invalid_work_group_size:
        return "CL_INVALID_WORK_GROUP_SIZE";
    case tinytc_status_cl_invalid_work_item_size:
        return "CL_INVALID_WORK_ITEM_SIZE";
    case tinytc_status_cl_kernel_arg_info_not_available:
        return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";
    case tinytc_status_cl_link_program_failure:
        return "CL_LINK_PROGRAM_FAILURE";
    case tinytc_status_cl_linker_not_available:
        return "CL_LINKER_NOT_AVAILABLE";
    case tinytc_status_cl_map_failure:
        return "CL_MAP_FAILURE";
    case tinytc_status_cl_mem_copy_overlap:
        return "CL_MEM_COPY_OVERLAP";
    case tinytc_status_cl_mem_object_allocation_failure:
        return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
    case tinytc_status_cl_misaligned_sub_buffer_offset:
        return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
    case tinytc_status_cl_out_of_host_memory:
        return "CL_OUT_OF_HOST_MEMORY";
    case tinytc_status_cl_out_of_resources:
        return "CL_OUT_OF_RESOURCES";
    case tinytc_status_cl_max_size_restriction_exceeded:
        return "CL_MAX_SIZE_RESTRICTION_EXCEEDED";
    case tinytc_status_cl_profiling_info_not_available:
        return "CL_PROFILING_INFO_NOT_AVAILABLE";
    case tinytc_status_unknown:
        return "Unknown error";
    }
    return "Unknown status code";
}
}
