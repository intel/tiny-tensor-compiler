// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "device_info.hpp"
#include "error.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <algorithm>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

tinytc_core_info::~tinytc_core_info() {}

namespace tinytc {

core_info_intel::core_info_intel(std::uint32_t ip_version, std::uint32_t num_eus_per_subslice,
                                 std::uint32_t num_threads_per_eu, std::uint32_t local_memory_size,
                                 std::vector<std::uint32_t> subgroup_sizes)
    : ip_version_(ip_version), num_eus_per_subslice_(num_eus_per_subslice),
      num_threads_per_eu_(num_threads_per_eu), local_memory_size_(local_memory_size),
      subgroup_sizes_(std::move(subgroup_sizes)), core_features_(0u) {
    std::sort(subgroup_sizes_.begin(), subgroup_sizes_.end());

    register_size_ = 32;
    if (ip_version_ >= static_cast<std::uint32_t>(intel_gpu_architecture::pvc)) {
        register_size_ = 64;
    }
    num_registers_per_thread_ = num_reg_small_grf();
}

auto core_info_intel::num_reg_small_grf() const -> std::uint32_t { return 128; }

auto core_info_intel::num_reg_large_grf() const -> std::uint32_t {
    return ip_version_ >= static_cast<std::uint32_t>(intel_gpu_architecture::pvc)
               ? 256
               : num_reg_small_grf();
}

auto core_info_intel::ip_version() const -> std::uint32_t { return ip_version_; }

auto core_info_intel::subgroup_sizes() const -> std::vector<std::uint32_t> const & {
    return subgroup_sizes_;
}

auto core_info_intel::register_size() const -> std::uint32_t { return register_size_; }

auto core_info_intel::num_registers_per_thread() const -> std::uint32_t {
    return num_registers_per_thread_;
}

auto core_info_intel::core_features() const -> tinytc_core_feature_flags_t {
    return core_features_;
}

void core_info_intel::core_features(tinytc_core_feature_flags_t flags) {
    if (flags & tinytc_core_feature_flag_large_register_file) {
        num_registers_per_thread_ = num_reg_large_grf();
    } else {
        num_registers_per_thread_ = num_reg_small_grf();
    }
}

auto core_info_intel::max_number_of_work_items(std::uint32_t subgroup_size) const -> std::uint32_t {
    auto const num_threads_per_eu_due_to_register_use =
        num_threads_per_eu_ * num_reg_small_grf() / num_registers_per_thread_;
    auto const num_threads_per_eu_due_to_subgroup_size =
        num_threads_per_eu_ * subgroup_sizes_.front() / subgroup_size;
    auto const num_threads_per_eu =
        std::min(num_threads_per_eu_due_to_register_use, num_threads_per_eu_due_to_subgroup_size);

    return num_threads_per_eu * num_eus_per_subslice_ * subgroup_size;
}

auto core_info_intel::minmax_number_of_work_items() const -> std::uint32_t {
    std::uint32_t minmax = std::numeric_limits<std::uint32_t>::max();
    for (auto const &sgs : subgroup_sizes_) {
        minmax = std::min(minmax, max_number_of_work_items(sgs));
    }
    return minmax;
}

auto core_info_intel::get_core_config(std::uint32_t subgroup_size) const -> core_config {
    if (std::find(subgroup_sizes_.begin(), subgroup_sizes_.end(), subgroup_size) ==
        subgroup_sizes_.end()) {
        throw std::out_of_range("Requested subgroup size not available");
    }

    bool block_read_write_supported = !(subgroup_size == 32 && register_size_ == 32);

    return core_config{subgroup_size,
                       max_number_of_work_items(subgroup_size),
                       local_memory_size_,
                       register_size_ * num_registers_per_thread_,
                       ip_version_,
                       core_features_,
                       block_read_write_supported};
}

} // namespace tinytc

using namespace tinytc;

extern "C" {
tinytc_status_t tinytc_core_info_intel_create_from_arch(tinytc_core_info_t *info,
                                                        tinytc_intel_gpu_architecture_t arch) {
    if (info == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        switch (arch) {
        case tinytc_intel_gpu_architecture_pvc:
            *info =
                std::make_unique<core_info_intel>(static_cast<std::uint32_t>(arch), 8, 8,
                                                  128 * 1024, std::vector<std::uint32_t>{16, 32})
                    .release();
            break;
        default:
            *info = nullptr;
            throw status::invalid_arguments;
        }
    });
}

tinytc_status_t tinytc_core_info_intel_create(tinytc_core_info_t *info, uint32_t ip_version,
                                              uint32_t num_eus_per_subslice,
                                              uint32_t num_threads_per_eu,
                                              uint32_t local_memory_size, uint32_t sgs_size,
                                              uint32_t const *sgs) {
    if (info == nullptr || sgs == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *info = std::make_unique<core_info_intel>(ip_version, num_eus_per_subslice,
                                                  num_threads_per_eu, local_memory_size,
                                                  std::vector<std::uint32_t>(sgs, sgs + sgs_size))
                    .release();
    });
}

tinytc_status_t tinytc_core_info_get_ip_version(const_tinytc_core_info_t info,
                                                uint32_t *ip_version) {
    if (info == nullptr || ip_version == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *ip_version = info->ip_version(); });
}

tinytc_status_t tinytc_core_info_get_subgroup_sizes(const_tinytc_core_info_t info,
                                                    uint32_t *sgs_size, uint32_t const **sgs) {

    if (info == nullptr || sgs_size == nullptr || sgs == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        auto const &sgs_sizes = info->subgroup_sizes();
        if (sgs_sizes.size() > std::numeric_limits<std::uint32_t>::max()) {
            throw std::out_of_range("too many subgroup sizes");
        }
        *sgs_size = sgs_sizes.size();
        *sgs = sgs_sizes.data();
    });
}

tinytc_status_t tinytc_core_info_get_register_size(const_tinytc_core_info_t info, uint32_t *size) {

    if (info == nullptr || size == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *size = info->register_size(); });
}

tinytc_status_t tinytc_core_info_get_num_registers_per_thread(const_tinytc_core_info_t info,
                                                              uint32_t *num) {

    if (info == nullptr || num == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *num = info->num_registers_per_thread(); });
}

tinytc_status_t tinytc_core_info_set_core_features(tinytc_core_info_t info,
                                                   tinytc_core_feature_flags_t flags) {

    if (info == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { info->core_features(flags); });
}

tinytc_status_t tinytc_core_info_get_core_features(tinytc_core_info_t info,
                                                   tinytc_core_feature_flags_t *flags) {
    if (info == nullptr || flags == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *flags = info->core_features(); });
}

tinytc_status_t tinytc_core_info_release(tinytc_core_info_t obj) {
    if (obj == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    auto ref_count = obj->dec_ref();
    if (ref_count == 0) {
        delete obj;
    }
    return tinytc_status_success;
}

tinytc_status_t tinytc_core_info_retain(tinytc_core_info_t obj) {
    if (obj == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    obj->inc_ref();
    return tinytc_status_success;
}
}
