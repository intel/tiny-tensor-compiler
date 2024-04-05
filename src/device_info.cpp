// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/device_info.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace tinytc {

core_info::~core_info() {}

auto get_core_info_intel_gpu(intel_gpu_architecture arch) -> std::shared_ptr<core_info> {
    switch (arch) {
    case intel_gpu_architecture::pvc:
        return std::make_shared<core_info_intel>(static_cast<std::uint32_t>(arch), 8, 8, 128 * 1024,
                                                 std::vector<std::uint32_t>{16, 32});
    }
    return nullptr;
}

core_info_intel::core_info_intel(std::uint32_t ip_version, std::uint32_t num_eus_per_subslice,
                                 std::uint32_t num_threads_per_eu, std::uint32_t local_memory_size,
                                 std::vector<std::uint32_t> subgroup_sizes)
    : ip_version_(ip_version), num_eus_per_subslice_(num_eus_per_subslice),
      num_threads_per_eu_(num_threads_per_eu), local_memory_size_(local_memory_size),
      subgroup_sizes_(std::move(subgroup_sizes)), core_features_(0) {
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

void core_info_intel::set_core_feature(core_feature_flag flag) {
    core_features_ |= static_cast<std::uint32_t>(flag);
    if (flag == core_feature_flag::large_register_file) {
        num_registers_per_thread_ = num_reg_large_grf();
    }
}

void core_info_intel::clear_core_feature(core_feature_flag flag) {
    core_features_ &= ~static_cast<std::uint32_t>(flag);
    if (flag == core_feature_flag::large_register_file) {
        num_registers_per_thread_ = num_reg_small_grf();
    }
}

auto core_info_intel::core_features() const -> std::uint32_t { return core_features_; }

auto core_info_intel::get_core_config(std::uint32_t subgroup_size) const -> core_config {
    if (std::find(subgroup_sizes_.begin(), subgroup_sizes_.end(), subgroup_size) ==
        subgroup_sizes_.end()) {
        throw std::out_of_range("Requested subgroup size not available");
    }

    auto const num_threads_per_eu_due_to_register_use =
        num_threads_per_eu_ * num_reg_small_grf() / num_registers_per_thread_;
    auto const num_threads_per_eu_due_to_subgroup_size =
        num_threads_per_eu_ * subgroup_sizes_.front() / subgroup_size;
    auto const num_threads_per_eu =
        std::min(num_threads_per_eu_due_to_register_use, num_threads_per_eu_due_to_subgroup_size);

    auto max_number_of_work_items = num_threads_per_eu * num_eus_per_subslice_ * subgroup_size;

    return core_config{subgroup_size,      max_number_of_work_items,
                       local_memory_size_, register_size_ * num_registers_per_thread_,
                       ip_version_,        core_features_};
}

} // namespace tinytc
