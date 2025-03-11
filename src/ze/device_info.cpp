// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "../device_info.hpp"
#include "error.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc.hpp"
#include "tinytc/tinytc_ze.h"
#include "tinytc/tinytc_ze.hpp"
#include "tinytc/types.h"

#include <level_zero/ze_api.h>
#include <vector>

namespace tinytc {
void set_spirv_features(tinytc_core_info_t info, ze_device_handle_t device) {
    auto const set_feature = [&info](tinytc_spirv_feature_t feature, bool available) {
        CHECK_STATUS(tinytc_core_info_set_spirv_feature(info, feature, available));
    };

    auto float_atomics = ze_float_atomic_ext_properties_t{};
    float_atomics.stype = ZE_STRUCTURE_TYPE_FLOAT_ATOMIC_EXT_PROPERTIES;
    auto module_props = ze_device_module_properties_t{};
    module_props.stype = ZE_STRUCTURE_TYPE_DEVICE_MODULE_PROPERTIES;
    module_props.pNext = &float_atomics;
    ZE_CHECK_STATUS(zeDeviceGetModuleProperties(device, &module_props));

    const bool has_f16 = module_props.flags & ZE_DEVICE_MODULE_FLAG_FP16;
    const bool has_f64 = module_props.flags & ZE_DEVICE_MODULE_FLAG_FP64;

    set_feature(tinytc_spirv_feature_float16, has_f16);
    set_feature(tinytc_spirv_feature_float64, has_f64);
    set_feature(tinytc_spirv_feature_groups, true);
    set_feature(tinytc_spirv_feature_subgroup_dispatch, true);
    set_feature(tinytc_spirv_feature_int64_atomics,
                module_props.flags & ZE_DEVICE_MODULE_FLAG_INT64_ATOMICS);
    if (has_f16) {

        set_feature(tinytc_spirv_feature_atomic_float16_add_local,
                    float_atomics.fp16Flags & ZE_DEVICE_FP_ATOMIC_EXT_FLAG_LOCAL_ADD);

        set_feature(tinytc_spirv_feature_atomic_float16_add_global,
                    float_atomics.fp16Flags & ZE_DEVICE_FP_ATOMIC_EXT_FLAG_GLOBAL_ADD);
    }

    set_feature(tinytc_spirv_feature_atomic_float32_add_local,
                float_atomics.fp32Flags & ZE_DEVICE_FP_ATOMIC_EXT_FLAG_LOCAL_ADD);

    set_feature(tinytc_spirv_feature_atomic_float32_add_global,
                float_atomics.fp32Flags & ZE_DEVICE_FP_ATOMIC_EXT_FLAG_GLOBAL_ADD);
    if (has_f64) {

        set_feature(tinytc_spirv_feature_atomic_float64_add_local,
                    float_atomics.fp64Flags & ZE_DEVICE_FP_ATOMIC_EXT_FLAG_LOCAL_ADD);

        set_feature(tinytc_spirv_feature_atomic_float64_add_global,
                    float_atomics.fp64Flags & ZE_DEVICE_FP_ATOMIC_EXT_FLAG_GLOBAL_ADD);
    }
}
} // namespace tinytc

extern "C" {
tinytc_status_t tinytc_ze_get_support_level(ze_device_handle_t device,
                                            tinytc_support_level_t *level) {
    if (level == nullptr) {
        return tinytc_status_invalid_arguments;
    }

    auto dev_ip_ver = ze_device_ip_version_ext_t{};
    dev_ip_ver.stype = ZE_STRUCTURE_TYPE_DEVICE_IP_VERSION_EXT;
    dev_ip_ver.pNext = nullptr;
    auto dev_props = ze_device_properties_t{};
    dev_props.stype = ZE_STRUCTURE_TYPE_DEVICE_PROPERTIES;
    dev_props.pNext = &dev_ip_ver;
    TINYTC_ZE_CHECK_STATUS(zeDeviceGetProperties(device, &dev_props));

    if (dev_props.vendorId != 0x8086) {
        *level = tinytc_support_level_none;
        return tinytc_status_success;
    }
    *level = tinytc_support_level_basic;

    const auto is_arch = [&dev_ip_ver](auto arch) {
        return arch <= dev_ip_ver.ipVersion &&
               dev_ip_ver.ipVersion <= arch + TINYTC_INTEL_GPU_ARCHITECTURE_SUB_VERSION_BITS;
    };
    if (is_arch(tinytc_intel_gpu_architecture_pvc) || is_arch(tinytc_intel_gpu_architecture_bmg)) {
        *level = tinytc_support_level_tuned;
    }
    return tinytc_status_success;
}

tinytc_status_t tinytc_ze_core_info_create(tinytc_core_info_t *info, ze_device_handle_t device) {
    if (info == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    auto dev_ip_ver = ze_device_ip_version_ext_t{};
    dev_ip_ver.stype = ZE_STRUCTURE_TYPE_DEVICE_IP_VERSION_EXT;
    dev_ip_ver.pNext = nullptr;
    auto dev_props = ze_device_properties_t{};
    dev_props.stype = ZE_STRUCTURE_TYPE_DEVICE_PROPERTIES;
    dev_props.pNext = &dev_ip_ver;
    TINYTC_ZE_CHECK_STATUS(zeDeviceGetProperties(device, &dev_props));
    auto compute_props = ze_device_compute_properties_t{};
    compute_props.stype = ZE_STRUCTURE_TYPE_DEVICE_COMPUTE_PROPERTIES;
    compute_props.pNext = nullptr;
    TINYTC_ZE_CHECK_STATUS(zeDeviceGetComputeProperties(device, &compute_props));

    auto subgroup_sizes = std::vector<std::int32_t>(
        compute_props.subGroupSizes, compute_props.subGroupSizes + compute_props.numSubGroupSizes);

    TINYTC_CHECK_STATUS(tinytc_core_info_intel_create(
        info, dev_ip_ver.ipVersion, dev_props.numEUsPerSubslice, dev_props.numThreadsPerEU,
        subgroup_sizes.size(), subgroup_sizes.data()));

    return tinytc::exception_to_status_code_ze([&] { set_spirv_features(*info, device); });
}
}

