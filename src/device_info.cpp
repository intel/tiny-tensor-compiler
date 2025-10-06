// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "device_info.hpp"
#include "error.hpp"
#include "tinytc/core.h"
#include "tinytc/core.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"
#include "util/casting.hpp"
#include "util/fnv1a.hpp"

#include <algorithm>
#include <cstring>
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

auto core_info_generic::subgroup_sizes() const -> array_view<std::int32_t> {
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
    return core_config{subgroup_size, max_work_group_size_, register_space_, &matrix_};
}
auto core_info_generic::matrix() const -> matrix_ext_info const & { return matrix_; }

core_info_intel::core_info_intel(std::uint32_t ip_version, std::int32_t num_eus_per_subslice,
                                 std::int32_t num_threads_per_eu,
                                 std::vector<std::int32_t> subgroup_sizes)
    : ip_version_(ip_version), num_eus_per_subslice_(num_eus_per_subslice),
      num_threads_per_eu_(num_threads_per_eu), subgroup_sizes_(std::move(subgroup_sizes)),
      core_features_(0u) {
    std::sort(subgroup_sizes_.begin(), subgroup_sizes_.end());

    register_size_ = 32;
    if (is_arch(tinytc_intel_gpu_architecture_pvc)) {
        register_size_ = 64;
        set_spirv_feature(spirv_feature::bfloat16_conversion, true);

        const auto block_info = matrix_ext_block_io_info{.base_address_alignment = 64,
                                                         .min_stride = 64,
                                                         .max_stride = (1 << 24) - 1,
                                                         .pos0_alignment = 4,
                                                         .stride_alignment = 8,
                                                         .width_alignment = 4};
        matrix_ = matrix_ext_info(16, block_info, pvc_matrix_ext_types);
    } else if (is_arch(tinytc_intel_gpu_architecture_bmg)) {
        register_size_ = 64;
        set_spirv_feature(spirv_feature::bfloat16_conversion, true);

        const auto block_info = matrix_ext_block_io_info{.base_address_alignment = 64,
                                                         .min_stride = 64,
                                                         .max_stride = (1 << 24) - 1,
                                                         .pos0_alignment = 4,
                                                         .stride_alignment = 16,
                                                         .width_alignment = 4};
        matrix_ = matrix_ext_info(16, block_info, pvc_matrix_ext_types);
    }
}

auto core_info_intel::num_reg_small_grf() const -> std::int32_t { return 128; }

auto core_info_intel::num_reg_large_grf() const -> std::int32_t {
    if (is_arch(tinytc_intel_gpu_architecture_pvc) || is_arch(tinytc_intel_gpu_architecture_bmg)) {
        return 256;
    }
    return num_reg_small_grf();
}

auto core_info_intel::num_reg() const -> std::int32_t {
    return core_features_ & tinytc_core_feature_flag_large_register_file ? num_reg_large_grf()
                                                                         : num_reg_small_grf();
}

auto core_info_intel::subgroup_sizes() const -> array_view<std::int32_t> { return subgroup_sizes_; }

auto core_info_intel::register_space() const -> std::int32_t { return register_size_ * num_reg(); }

auto core_info_intel::core_features() const -> tinytc_core_feature_flags_t {
    return core_features_;
}

void core_info_intel::core_features(tinytc_core_feature_flags_t flags) { core_features_ = flags; }

auto core_info_intel::max_work_group_size(std::int32_t subgroup_size) const -> std::int32_t {
    auto const num_threads_per_eu_due_to_register_use =
        num_threads_per_eu_ * num_reg_small_grf() / num_reg();
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

    return core_config{subgroup_size, max_work_group_size(subgroup_size), register_space(),
                       &matrix_};
}

auto core_info_intel::matrix() const -> matrix_ext_info const & { return matrix_; }

} // namespace tinytc

using namespace tinytc;

extern "C" {
tinytc_status_t tinytc_core_info_generic_create(tinytc_core_info_t *info, int32_t register_space,
                                                int32_t max_work_group_size, size_t sgs_size,
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
            (*info)->set_spirv_feature(spirv_feature::float16, true);
            (*info)->set_spirv_feature(spirv_feature::float64, false);
            (*info)->set_spirv_feature(spirv_feature::int64_atomics, true);
            (*info)->set_spirv_feature(spirv_feature::groups, true);
            (*info)->set_spirv_feature(spirv_feature::subgroup_dispatch, true);
            (*info)->set_spirv_feature(spirv_feature::atomic_float16_add_local, false);
            (*info)->set_spirv_feature(spirv_feature::atomic_float16_add_global, false);
            (*info)->set_spirv_feature(spirv_feature::atomic_float32_add_local, true);
            (*info)->set_spirv_feature(spirv_feature::atomic_float32_add_global, true);
            (*info)->set_spirv_feature(spirv_feature::atomic_float64_add_local, false);
            (*info)->set_spirv_feature(spirv_feature::atomic_float64_add_global, false);
            (*info)->set_spirv_feature(spirv_feature::atomic_float16_min_max_local, true);
            (*info)->set_spirv_feature(spirv_feature::atomic_float16_min_max_global, true);
            (*info)->set_spirv_feature(spirv_feature::atomic_float32_min_max_local, true);
            (*info)->set_spirv_feature(spirv_feature::atomic_float32_min_max_global, true);
            (*info)->set_spirv_feature(spirv_feature::atomic_float64_min_max_local, false);
            (*info)->set_spirv_feature(spirv_feature::atomic_float64_min_max_global, false);
            (*info)->set_spirv_feature(spirv_feature::bfloat16_conversion, false);
            (*info)->set_spirv_feature(spirv_feature::subgroup_buffer_block_io, true);
            break;
        case tinytc_intel_gpu_architecture_pvc:
        case tinytc_intel_gpu_architecture_bmg:
            *info = std::make_unique<core_info_intel>(static_cast<std::uint32_t>(arch), 8, 8,
                                                      std::vector<std::int32_t>{16, 32})
                        .release();
            (*info)->set_spirv_feature(spirv_feature::float16, true);
            (*info)->set_spirv_feature(spirv_feature::float64, true);
            (*info)->set_spirv_feature(spirv_feature::int64_atomics, true);
            (*info)->set_spirv_feature(spirv_feature::groups, true);
            (*info)->set_spirv_feature(spirv_feature::subgroup_dispatch, true);
            (*info)->set_spirv_feature(spirv_feature::atomic_float16_add_local, false);
            (*info)->set_spirv_feature(spirv_feature::atomic_float16_add_global, false);
            (*info)->set_spirv_feature(spirv_feature::atomic_float32_add_local, true);
            (*info)->set_spirv_feature(spirv_feature::atomic_float32_add_global, true);
            (*info)->set_spirv_feature(spirv_feature::atomic_float64_add_local, true);
            (*info)->set_spirv_feature(spirv_feature::atomic_float64_add_global, true);
            (*info)->set_spirv_feature(spirv_feature::atomic_float16_min_max_local, true);
            (*info)->set_spirv_feature(spirv_feature::atomic_float16_min_max_global, true);
            (*info)->set_spirv_feature(spirv_feature::atomic_float32_min_max_local, true);
            (*info)->set_spirv_feature(spirv_feature::atomic_float32_min_max_global, true);
            (*info)->set_spirv_feature(spirv_feature::atomic_float64_min_max_local, true);
            (*info)->set_spirv_feature(spirv_feature::atomic_float64_min_max_global, true);
            (*info)->set_spirv_feature(spirv_feature::bfloat16_conversion, true);
            (*info)->set_spirv_feature(spirv_feature::subgroup_buffer_block_io, true);
            break;
        default:
            *info = nullptr;
            throw status::invalid_arguments;
        }
    });
}

tinytc_status_t tinytc_core_info_intel_create_from_name(tinytc_core_info_t *info,
                                                        char const *name) {
    if (info == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        switch (fnv1a(name, std::strlen(name))) {
        case "tgl"_fnv1a:
            CHECK_STATUS(
                tinytc_core_info_intel_create_from_arch(info, tinytc_intel_gpu_architecture_tgl));
            break;
        case "pvc"_fnv1a:
            CHECK_STATUS(
                tinytc_core_info_intel_create_from_arch(info, tinytc_intel_gpu_architecture_pvc));
            break;
        case "bmg"_fnv1a:
            CHECK_STATUS(
                tinytc_core_info_intel_create_from_arch(info, tinytc_intel_gpu_architecture_bmg));
            break;
        default:
            *info = nullptr;
            throw status::invalid_arguments;
        }
    });
}

tinytc_status_t tinytc_core_info_intel_create(tinytc_core_info_t *info, uint32_t ip_version,
                                              int32_t num_eus_per_subslice,
                                              int32_t num_threads_per_eu, size_t sgs_size,
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

tinytc_status_t tinytc_core_info_get_subgroup_sizes(const_tinytc_core_info_t info, size_t *sgs_size,
                                                    int32_t const **sgs) {

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

tinytc_status_t tinytc_core_info_get_core_features(const_tinytc_core_info_t info,
                                                   tinytc_core_feature_flags_t *flags) {
    if (info == nullptr || flags == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *flags = info->core_features(); });
}

tinytc_status_t tinytc_core_info_set_spirv_feature(tinytc_core_info_t info,
                                                   tinytc_spirv_feature_t feature,
                                                   tinytc_bool_t available) {

    if (info == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { info->set_spirv_feature(enum_cast<spirv_feature>(feature), available); });
}

tinytc_status_t tinytc_core_info_have_spirv_feature(const_tinytc_core_info_t info,
                                                    tinytc_spirv_feature_t feature,
                                                    tinytc_bool_t *available) {
    if (info == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] { *available = info->have_spirv_feature(enum_cast<spirv_feature>(feature)); });
}

tinytc_status_t tinytc_core_info_get_default_alignment(const_tinytc_core_info_t info,
                                                       int32_t *alignment) {
    if (info == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *alignment = info->alignment(); });
}

tinytc_status_t tinytc_core_info_set_default_alignment(tinytc_core_info_t info, int32_t alignment) {
    if (info == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { info->alignment(alignment); });
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
