// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "../device_info.hpp"
#include "device_info_helper.hpp"
#include "error.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc.hpp"
#include "tinytc/tinytc_cl.h"
#include "tinytc/types.h"

#include <CL/cl.h>
#include <CL/cl_ext.h>
#include <CL/cl_platform.h>
#include <cstddef>
#include <cstring>
#include <vector>

#ifndef CL_DEVICE_SINGLE_FP_ATOMIC_CAPABILITIES_EXT
#define CL_DEVICE_SINGLE_FP_ATOMIC_CAPABILITIES_EXT 0x4231
#endif
#ifndef CL_DEVICE_DOUBLE_FP_ATOMIC_CAPABILITIES_EXT
#define CL_DEVICE_DOUBLE_FP_ATOMIC_CAPABILITIES_EXT 0x4232
#endif
#ifndef CL_DEVICE_HALF_FP_ATOMIC_CAPABILITIES_EXT
#define CL_DEVICE_HALF_FP_ATOMIC_CAPABILITIES_EXT 0x4233
#endif

#ifndef CL_DEVICE_GLOBAL_FP_ATOMIC_ADD_EXT
#define CL_DEVICE_GLOBAL_FP_ATOMIC_ADD_EXT (1 << 1)
#endif
#ifndef CL_DEVICE_LOCAL_FP_ATOMIC_ADD_EXT
#define CL_DEVICE_LOCAL_FP_ATOMIC_ADD_EXT (1 << 17)
#endif

namespace tinytc {
void set_spirv_features(tinytc_core_info_t info, cl_device_id device) {
    auto const set_feature = [&info](tinytc_spirv_feature_t feature, bool available) {
        CHECK_STATUS(tinytc_core_info_set_spirv_feature(info, feature, available));
    };

    const auto ocl_exts = get_opencl_extensions(device);
    const auto ocl_version = get_opencl_version(device);
    const auto max_num_subgroups = device_info<cl_uint>(device, CL_DEVICE_MAX_NUM_SUB_GROUPS);
    const auto double_fp_config =
        device_info<cl_device_fp_config>(device, CL_DEVICE_DOUBLE_FP_CONFIG);

    set_feature(tinytc_spirv_feature_float16, ocl_exts & opencl_ext_cl_khr_fp16);
    set_feature(tinytc_spirv_feature_float64,
                double_fp_config != 0 || ocl_exts & opencl_ext_cl_khr_fp64);
    set_feature(tinytc_spirv_feature_groups, ocl_version.major >= 2 && max_num_subgroups != 0);
    set_feature(tinytc_spirv_feature_subgroup_dispatch,
                ocl_version.major >= 2 && max_num_subgroups != 0);
    set_feature(tinytc_spirv_feature_subgroup_buffer_block_io,
                ocl_exts & opencl_ext_cl_intel_spirv_subgroups);
    set_feature(tinytc_spirv_feature_int64_atomics,
                ocl_exts & (opencl_ext_cl_khr_int64_base_atomics |
                            opencl_ext_cl_khr_int64_extended_atomics));
    if (ocl_exts & opencl_ext_cl_ext_float_atomics) {
        auto f16_flags =
            device_info<cl_bitfield>(device, CL_DEVICE_HALF_FP_ATOMIC_CAPABILITIES_EXT);
        auto f32_flags =
            device_info<cl_bitfield>(device, CL_DEVICE_SINGLE_FP_ATOMIC_CAPABILITIES_EXT);
        auto f64_flags =
            device_info<cl_bitfield>(device, CL_DEVICE_DOUBLE_FP_ATOMIC_CAPABILITIES_EXT);
        set_feature(tinytc_spirv_feature_atomic_float16_add_local,
                    f16_flags & CL_DEVICE_LOCAL_FP_ATOMIC_ADD_EXT);
        set_feature(tinytc_spirv_feature_atomic_float32_add_local,
                    f32_flags & CL_DEVICE_LOCAL_FP_ATOMIC_ADD_EXT);
        set_feature(tinytc_spirv_feature_atomic_float64_add_local,
                    f64_flags & CL_DEVICE_LOCAL_FP_ATOMIC_ADD_EXT);
        set_feature(tinytc_spirv_feature_atomic_float16_add_global,
                    f16_flags & CL_DEVICE_GLOBAL_FP_ATOMIC_ADD_EXT);
        set_feature(tinytc_spirv_feature_atomic_float32_add_global,
                    f32_flags & CL_DEVICE_GLOBAL_FP_ATOMIC_ADD_EXT);
        set_feature(tinytc_spirv_feature_atomic_float64_add_global,
                    f64_flags & CL_DEVICE_GLOBAL_FP_ATOMIC_ADD_EXT);
    }
}
} // namespace tinytc

extern "C" {
tinytc_status_t tinytc_cl_get_support_level(cl_device_id device, tinytc_support_level_t *level) {
    if (level == nullptr) {
        return tinytc_status_invalid_arguments;
    }

    auto ocl_exts = tinytc::get_opencl_extensions(device);
    bool has_subgroup =
        ocl_exts & (tinytc::opencl_ext_cl_intel_subgroups | tinytc::opencl_ext_cl_khr_subgroups);

    if (!has_subgroup) {
        auto version = tinytc::get_opencl_version(device);
        if (version.major >= 3) {
            std::size_t features_size;
            TINYTC_CL_CHECK_STATUS(
                clGetDeviceInfo(device, CL_DEVICE_OPENCL_C_FEATURES, 0, nullptr, &features_size));
            auto features = std::vector<cl_name_version>(features_size / sizeof(cl_name_version));
            TINYTC_CL_CHECK_STATUS(clGetDeviceInfo(device, CL_DEVICE_OPENCL_C_FEATURES,
                                                   features_size, features.data(), nullptr));
            for (auto &feature : features) {
                if (strncmp("__opencl_c_subgroups", feature.name, CL_NAME_VERSION_MAX_NAME_SIZE) ==
                    0) {
                    has_subgroup = true;
                    break;
                }
            }
        }
    }

    if (!has_subgroup) {
        *level = tinytc_support_level_none;
        return tinytc_status_success;
    }
    *level = tinytc_support_level_basic;

    cl_version ip_ver = 0;
    cl_int err =
        clGetDeviceInfo(device, CL_DEVICE_IP_VERSION_INTEL, sizeof(ip_ver), &ip_ver, nullptr);
    if (err == CL_SUCCESS && ip_ver == tinytc_intel_gpu_architecture_pvc) {
        *level = tinytc_support_level_tuned;
    }

    return tinytc_status_success;
}

tinytc_status_t tinytc_cl_core_info_create(tinytc_core_info_t *info, cl_device_id device) {
    if (info == nullptr) {
        return tinytc_status_invalid_arguments;
    }

    cl_uint vendor_id, mem_base_addr_align;

    TINYTC_CL_CHECK_STATUS(
        clGetDeviceInfo(device, CL_DEVICE_VENDOR_ID, sizeof(vendor_id), &vendor_id, nullptr));

    if (vendor_id == 0x8086) {
        cl_device_type device_type;
        std::size_t subgroup_sizes_size = 0;

        TINYTC_CL_CHECK_STATUS(
            clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(device_type), &device_type, nullptr));

        TINYTC_CL_CHECK_STATUS(clGetDeviceInfo(device, CL_DEVICE_SUB_GROUP_SIZES_INTEL, 0, nullptr,
                                               &subgroup_sizes_size));
        auto subgroup_sizes_long =
            std::vector<std::size_t>(subgroup_sizes_size / sizeof(std::size_t));
        TINYTC_CL_CHECK_STATUS(clGetDeviceInfo(device, CL_DEVICE_SUB_GROUP_SIZES_INTEL,
                                               subgroup_sizes_size, subgroup_sizes_long.data(),
                                               nullptr));
        auto subgroup_sizes =
            std::vector<std::int32_t>(subgroup_sizes_long.begin(), subgroup_sizes_long.end());

        if (device_type == CL_DEVICE_TYPE_GPU) {
            cl_version ip_ver;
            cl_uint num_eus_per_subslice, num_threads_per_eu;

            TINYTC_CL_CHECK_STATUS(clGetDeviceInfo(device, CL_DEVICE_IP_VERSION_INTEL,
                                                   sizeof(ip_ver), &ip_ver, nullptr));
            TINYTC_CL_CHECK_STATUS(clGetDeviceInfo(device, CL_DEVICE_NUM_EUS_PER_SUB_SLICE_INTEL,
                                                   sizeof(num_eus_per_subslice),
                                                   &num_eus_per_subslice, nullptr));
            TINYTC_CL_CHECK_STATUS(clGetDeviceInfo(device, CL_DEVICE_NUM_THREADS_PER_EU_INTEL,
                                                   sizeof(num_threads_per_eu), &num_threads_per_eu,
                                                   nullptr));

            TINYTC_CHECK_STATUS(tinytc_core_info_intel_create(
                info, ip_ver, num_eus_per_subslice, num_threads_per_eu, subgroup_sizes.size(),
                subgroup_sizes.data()));
        } else if (device_type == CL_DEVICE_TYPE_CPU) {
            // 32 zmm registers
            // @todo: need to do something smarter here
            std::uint32_t register_space = 32 * 64;
            size_t max_work_group_size;
            TINYTC_CL_CHECK_STATUS(clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE,
                                                   sizeof(max_work_group_size),
                                                   &max_work_group_size, nullptr));
            TINYTC_CHECK_STATUS(
                tinytc_core_info_generic_create(info, register_space, max_work_group_size,
                                                subgroup_sizes.size(), subgroup_sizes.data()));
        } else {
            return tinytc_status_unsupported_device;
        }
    } else if (vendor_id == 0x1002) {
        // 512 KB / 32 wavefronts
        // @todo: can this info be queried?
        std::uint32_t register_space = 512 * 1024 / 32;
        cl_int subgroup_size;
        size_t max_work_group_size;
        TINYTC_CL_CHECK_STATUS(clGetDeviceInfo(device, CL_DEVICE_WAVEFRONT_WIDTH_AMD,
                                               sizeof(subgroup_size), &subgroup_size, nullptr));
        TINYTC_CL_CHECK_STATUS(clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE,
                                               sizeof(max_work_group_size), &max_work_group_size,
                                               nullptr));
        auto subgroup_sizes = std::vector<std::int32_t>{subgroup_size};
        TINYTC_CHECK_STATUS(
            tinytc_core_info_generic_create(info, register_space, max_work_group_size,
                                            subgroup_sizes.size(), subgroup_sizes.data()));
    } else {
        return tinytc_status_unsupported_device;
    }

    TINYTC_CL_CHECK_STATUS(clGetDeviceInfo(device, CL_DEVICE_MEM_BASE_ADDR_ALIGN,
                                           sizeof(mem_base_addr_align), &mem_base_addr_align,
                                           nullptr));
    // mem_base_addr_align is in bits -> convert to bytes
    TINYTC_CHECK_STATUS(tinytc_core_info_set_default_alignment(*info, mem_base_addr_align / 8));

    return tinytc::exception_to_status_code_cl([&] { set_spirv_features(*info, device); });
}
}

