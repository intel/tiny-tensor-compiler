// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/tinytc_ze.h"
#include "tinytc/types.h"

#include <level_zero/ze_api.h>

extern "C" {
tinytc_status_t tinytc_ze_convert_status(ze_result_t result) {
    switch (result) {
    case ZE_RESULT_SUCCESS:
        return tinytc_status_success;
    case ZE_RESULT_NOT_READY:
        return tinytc_status_ze_result_not_ready;
    case ZE_RESULT_ERROR_DEVICE_LOST:
        return tinytc_status_ze_result_error_device_lost;
    case ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY:
        return tinytc_status_ze_result_error_out_of_host_memory;
    case ZE_RESULT_ERROR_OUT_OF_DEVICE_MEMORY:
        return tinytc_status_ze_result_error_out_of_device_memory;
    case ZE_RESULT_ERROR_MODULE_BUILD_FAILURE:
        return tinytc_status_ze_result_error_module_build_failure;
    case ZE_RESULT_ERROR_MODULE_LINK_FAILURE:
        return tinytc_status_ze_result_error_module_link_failure;
    case ZE_RESULT_ERROR_DEVICE_REQUIRES_RESET:
        return tinytc_status_ze_result_error_device_requires_reset;
    case ZE_RESULT_ERROR_DEVICE_IN_LOW_POWER_STATE:
        return tinytc_status_ze_result_error_device_in_low_power_state;
    case ZE_RESULT_EXP_ERROR_DEVICE_IS_NOT_VERTEX:
        return tinytc_status_ze_result_exp_error_device_is_not_vertex;
    case ZE_RESULT_EXP_ERROR_VERTEX_IS_NOT_DEVICE:
        return tinytc_status_ze_result_exp_error_vertex_is_not_device;
    case ZE_RESULT_EXP_ERROR_REMOTE_DEVICE:
        return tinytc_status_ze_result_exp_error_remote_device;
    case ZE_RESULT_EXP_ERROR_OPERANDS_INCOMPATIBLE:
        return tinytc_status_ze_result_exp_error_operands_incompatible;
    case ZE_RESULT_EXP_RTAS_BUILD_RETRY:
        return tinytc_status_ze_result_exp_rtas_build_retry;
    case ZE_RESULT_EXP_RTAS_BUILD_DEFERRED:
        return tinytc_status_ze_result_exp_rtas_build_deferred;
    case ZE_RESULT_ERROR_INSUFFICIENT_PERMISSIONS:
        return tinytc_status_ze_result_error_insufficient_permissions;
    case ZE_RESULT_ERROR_NOT_AVAILABLE:
        return tinytc_status_ze_result_error_not_available;
    case ZE_RESULT_ERROR_DEPENDENCY_UNAVAILABLE:
        return tinytc_status_ze_result_error_dependency_unavailable;
    case ZE_RESULT_WARNING_DROPPED_DATA:
        return tinytc_status_ze_result_warning_dropped_data;
    case ZE_RESULT_ERROR_UNINITIALIZED:
        return tinytc_status_ze_result_error_uninitialized;
    case ZE_RESULT_ERROR_UNSUPPORTED_VERSION:
        return tinytc_status_ze_result_error_unsupported_version;
    case ZE_RESULT_ERROR_UNSUPPORTED_FEATURE:
        return tinytc_status_ze_result_error_unsupported_feature;
    case ZE_RESULT_ERROR_INVALID_ARGUMENT:
        return tinytc_status_ze_result_error_invalid_argument;
    case ZE_RESULT_ERROR_INVALID_NULL_HANDLE:
        return tinytc_status_ze_result_error_invalid_null_handle;
    case ZE_RESULT_ERROR_HANDLE_OBJECT_IN_USE:
        return tinytc_status_ze_result_error_handle_object_in_use;
    case ZE_RESULT_ERROR_INVALID_NULL_POINTER:
        return tinytc_status_ze_result_error_invalid_null_pointer;
    case ZE_RESULT_ERROR_INVALID_SIZE:
        return tinytc_status_ze_result_error_invalid_size;
    case ZE_RESULT_ERROR_UNSUPPORTED_SIZE:
        return tinytc_status_ze_result_error_unsupported_size;
    case ZE_RESULT_ERROR_UNSUPPORTED_ALIGNMENT:
        return tinytc_status_ze_result_error_unsupported_alignment;
    case ZE_RESULT_ERROR_INVALID_SYNCHRONIZATION_OBJECT:
        return tinytc_status_ze_result_error_invalid_synchronization_object;
    case ZE_RESULT_ERROR_INVALID_ENUMERATION:
        return tinytc_status_ze_result_error_invalid_enumeration;
    case ZE_RESULT_ERROR_UNSUPPORTED_ENUMERATION:
        return tinytc_status_ze_result_error_unsupported_enumeration;
    case ZE_RESULT_ERROR_UNSUPPORTED_IMAGE_FORMAT:
        return tinytc_status_ze_result_error_unsupported_image_format;
    case ZE_RESULT_ERROR_INVALID_NATIVE_BINARY:
        return tinytc_status_ze_result_error_invalid_native_binary;
    case ZE_RESULT_ERROR_INVALID_GLOBAL_NAME:
        return tinytc_status_ze_result_error_invalid_global_name;
    case ZE_RESULT_ERROR_INVALID_KERNEL_NAME:
        return tinytc_status_ze_result_error_invalid_kernel_name;
    case ZE_RESULT_ERROR_INVALID_FUNCTION_NAME:
        return tinytc_status_ze_result_error_invalid_function_name;
    case ZE_RESULT_ERROR_INVALID_GROUP_SIZE_DIMENSION:
        return tinytc_status_ze_result_error_invalid_group_size_dimension;
    case ZE_RESULT_ERROR_INVALID_GLOBAL_WIDTH_DIMENSION:
        return tinytc_status_ze_result_error_invalid_global_width_dimension;
    case ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_INDEX:
        return tinytc_status_ze_result_error_invalid_kernel_argument_index;
    case ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_SIZE:
        return tinytc_status_ze_result_error_invalid_kernel_argument_size;
    case ZE_RESULT_ERROR_INVALID_KERNEL_ATTRIBUTE_VALUE:
        return tinytc_status_ze_result_error_invalid_kernel_attribute_value;
    case ZE_RESULT_ERROR_INVALID_MODULE_UNLINKED:
        return tinytc_status_ze_result_error_invalid_module_unlinked;
    case ZE_RESULT_ERROR_INVALID_COMMAND_LIST_TYPE:
        return tinytc_status_ze_result_error_invalid_command_list_type;
    case ZE_RESULT_ERROR_OVERLAPPING_REGIONS:
        return tinytc_status_ze_result_error_overlapping_regions;
    case ZE_RESULT_WARNING_ACTION_REQUIRED:
        return tinytc_status_ze_result_warning_action_required;
    case ZE_RESULT_ERROR_UNKNOWN:
    case ZE_RESULT_FORCE_UINT32:
        return tinytc_status_ze_result_error_unknown;
    }
    return tinytc_status_ze_result_error_unknown;
}
}
