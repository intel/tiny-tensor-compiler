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

core_info_generic::core_info_generic(std::int32_t register_space, std::int32_t max_work_group_size,
                                     std::vector<std::int32_t> subgroup_sizes)
    : register_space_(register_space), max_work_group_size_(max_work_group_size),
      subgroup_sizes_(std::move(subgroup_sizes)) {}

auto core_info_generic::subgroup_sizes() const -> std::vector<std::int32_t> const & {
    return subgroup_sizes_;
}
auto core_info_generic::register_space() const -> std::int32_t { return register_space_; }
auto core_info_generic::core_features() const -> tinytc_core_feature_flags_t { return 0u; }
void core_info_generic::core_features(tinytc_core_feature_flags_t) {}
auto core_info_generic::minmax_work_group_size() const -> std::int32_t {
    return max_work_group_size_;
}
auto core_info_generic::get_core_config(std::int32_t subgroup_size) const -> tinytc::core_config {
    if (std::find(subgroup_sizes_.begin(), subgroup_sizes_.end(), subgroup_size) ==
        subgroup_sizes_.end()) {
        throw std::out_of_range("Requested subgroup size not available");
    }
    return core_config{subgroup_size, max_work_group_size_, register_space_, false};
}

core_info_intel::core_info_intel(std::uint32_t ip_version, std::int32_t num_eus_per_subslice,
                                 std::int32_t num_threads_per_eu,
                                 std::vector<std::int32_t> subgroup_sizes)
    : ip_version_(ip_version), num_eus_per_subslice_(num_eus_per_subslice),
      num_threads_per_eu_(num_threads_per_eu), subgroup_sizes_(std::move(subgroup_sizes)),
      core_features_(0u) {
    std::sort(subgroup_sizes_.begin(), subgroup_sizes_.end());

    register_size_ = 32;
    if (ip_version_ >= static_cast<std::uint32_t>(intel_gpu_architecture::pvc)) {
        register_size_ = 64;
    }
    num_registers_per_thread_ = num_reg_small_grf();
}

auto core_info_intel::num_reg_small_grf() const -> std::int32_t { return 128; }

auto core_info_intel::num_reg_large_grf() const -> std::int32_t {
    return ip_version_ >= static_cast<std::uint32_t>(intel_gpu_architecture::pvc)
               ? 256
               : num_reg_small_grf();
}

auto core_info_intel::subgroup_sizes() const -> std::vector<std::int32_t> const & {
    return subgroup_sizes_;
}

auto core_info_intel::register_space() const -> std::int32_t {
    return register_size_ * num_registers_per_thread_;
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

auto core_info_intel::max_work_group_size(std::int32_t subgroup_size) const -> std::int32_t {
    auto const num_threads_per_eu_due_to_register_use =
        num_threads_per_eu_ * num_reg_small_grf() / num_registers_per_thread_;
    auto const num_threads_per_eu_due_to_subgroup_size =
        num_threads_per_eu_ * subgroup_sizes_.front() / subgroup_size;
    auto const num_threads_per_eu =
        std::min(num_threads_per_eu_due_to_register_use, num_threads_per_eu_due_to_subgroup_size);

    return num_threads_per_eu * num_eus_per_subslice_ * subgroup_size;
}

auto core_info_intel::minmax_work_group_size() const -> std::int32_t {
    std::int32_t minmax = std::numeric_limits<std::int32_t>::max();
    for (auto const &sgs : subgroup_sizes_) {
        minmax = std::min(minmax, max_work_group_size(sgs));
    }
    return minmax;
}

auto core_info_intel::get_core_config(std::int32_t subgroup_size) const -> core_config {
    if (std::find(subgroup_sizes_.begin(), subgroup_sizes_.end(), subgroup_size) ==
        subgroup_sizes_.end()) {
        throw std::out_of_range("Requested subgroup size not available");
    }

    bool block_read_write_supported = !(subgroup_size == 32 && register_size_ == 32);

    return core_config{subgroup_size, max_work_group_size(subgroup_size), register_space(),
                       block_read_write_supported};
}

} // namespace tinytc

using namespace tinytc;

extern "C" {
tinytc_status_t tinytc_core_info_generic_create(tinytc_core_info_t *info, int32_t register_space,
                                                int32_t max_work_group_size, uint32_t sgs_size,
                                                int32_t const *sgs) {
    if (info == nullptr || sgs == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *info = std::make_unique<core_info_generic>(register_space, max_work_group_size,
                                                    std::vector<std::int32_t>(sgs, sgs + sgs_size))
                    .release();
    });
}

tinytc_status_t tinytc_core_info_intel_create_from_arch(tinytc_core_info_t *info,
                                                        tinytc_intel_gpu_architecture_t arch) {
    if (info == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        switch (arch) {
        case tinytc_intel_gpu_architecture_tgl:
            *info = std::make_unique<core_info_intel>(static_cast<std::uint32_t>(arch), 8, 7,
                                                      std::vector<std::int32_t>{8, 16, 32})
                        .release();
            break;
        case tinytc_intel_gpu_architecture_pvc:
            *info = std::make_unique<core_info_intel>(static_cast<std::uint32_t>(arch), 8, 8,
                                                      std::vector<std::int32_t>{16, 32})
                        .release();
            break;
        default:
            *info = nullptr;
            throw status::invalid_arguments;
        }
    });
}

tinytc_status_t tinytc_core_info_intel_create(tinytc_core_info_t *info, uint32_t ip_version,
                                              int32_t num_eus_per_subslice,
                                              int32_t num_threads_per_eu, uint32_t sgs_size,
                                              int32_t const *sgs) {
    if (info == nullptr || sgs == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *info =
            std::make_unique<core_info_intel>(ip_version, num_eus_per_subslice, num_threads_per_eu,
                                              std::vector<std::int32_t>(sgs, sgs + sgs_size))
                .release();
    });
}

tinytc_status_t tinytc_core_info_get_subgroup_sizes(const_tinytc_core_info_t info,
                                                    uint32_t *sgs_size, int32_t const **sgs) {

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

tinytc_status_t tinytc_core_info_get_register_space(const_tinytc_core_info_t info, int32_t *space) {

    if (info == nullptr || space == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *space = info->register_space(); });
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
