// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DEVICE_INFO_20240304_HPP
#define DEVICE_INFO_20240304_HPP

#include "matrix_ext_info.hpp"
#include "reference_counted.hpp"
#include "tinytc/core.hpp"
#include "tinytc/types.h"

#include <array>
#include <cstdint>
#include <vector>

namespace tinytc {

enum class spirv_feature;

//! Core parameters for a specific choice of subgroup size and core feature flags
class core_config {
  public:
    std::int32_t subgroup_size;       ///< Smallest unit of execution
    std::int32_t max_work_group_size; ///< Maximum size of local work group in number of works items
    std::int32_t register_space;      ///< Size of register file in bytes
    matrix_ext_info const *matrix;
};

} // namespace tinytc

struct tinytc_core_info : tinytc::reference_counted {
    //! empty dtor
    virtual ~tinytc_core_info();
    virtual auto subgroup_sizes() const -> tinytc::array_view<std::int32_t> = 0;
    //! Returns availabe register space per subgroup
    virtual auto register_space() const -> std::int32_t = 0;
    //! Get core features
    virtual auto core_features() const -> tinytc_core_feature_flags_t = 0;
    //! Set core features
    virtual void core_features(tinytc_core_feature_flags_t flags) = 0;
    //! Returns the minimum of the maximum work group size over all subgroup sizes; selected
    //! core features are respected
    virtual auto minmax_work_group_size() const -> std::int32_t = 0;
    //! Return core config for specific subgroup size and number of registers per tile
    virtual auto get_core_config(std::int32_t subgroup_size) const -> tinytc::core_config = 0;
    virtual void set_spirv_feature(tinytc::spirv_feature f, bool available) = 0;
    virtual auto have_spirv_feature(tinytc::spirv_feature f) const -> bool = 0;
    virtual auto matrix() const -> tinytc::matrix_ext_info const & = 0;
    virtual auto alignment() const -> std::int32_t = 0;
    virtual void alignment(std::int32_t alignment) = 0;
};

namespace tinytc {

class core_info_common : public ::tinytc_core_info {
  public:
    inline void set_spirv_feature(spirv_feature f, bool available) override {
        spv_feature_[static_cast<int>(f)] = available;
    }
    inline auto have_spirv_feature(spirv_feature f) const -> bool override {
        return spv_feature_[static_cast<int>(f)];
    }
    inline auto alignment() const -> std::int32_t override { return alignment_; }
    inline void alignment(std::int32_t alignment) override { alignment_ = alignment; }

  private:
    std::array<bool, TINYTC_ENUM_NUM_SPIRV_FEATURE> spv_feature_ = {};
    std::int32_t alignment_ = 128;
};

class core_info_generic : public core_info_common {
  public:
    core_info_generic(std::int32_t register_space, std::int32_t max_work_group_size,
                      std::vector<std::int32_t> subgroup_sizes);
    auto subgroup_sizes() const -> array_view<std::int32_t> override;
    auto register_space() const -> std::int32_t override;
    auto core_features() const -> tinytc_core_feature_flags_t override;
    void core_features(tinytc_core_feature_flags_t flags) override;
    auto minmax_work_group_size() const -> std::int32_t override;
    auto get_core_config(std::int32_t subgroup_size) const -> tinytc::core_config override;
    auto matrix() const -> matrix_ext_info const & override;

  private:
    std::int32_t register_space_;
    std::int32_t max_work_group_size_;
    std::vector<std::int32_t> subgroup_sizes_;
    matrix_ext_info matrix_;
};

//! Set of core configurations for Intel GPUs
class core_info_intel : public core_info_common {
  public:
    /**
     * @brief ctor
     *
     * @param ip_version IP version of architecture
     * @param num_eus_per_subslice Number of Execution Units (Xe Vector Engines) per subslice (Xe
     * Core)
     * @param num_threads_per_eu Number of threads per Execution Unit (Xe Vector Engine)
     * @param subgroup_sizes Allowed subgroup sizes
     */
    core_info_intel(std::uint32_t ip_version, std::int32_t num_eus_per_subslice,
                    std::int32_t num_threads_per_eu, std::vector<std::int32_t> subgroup_sizes);

    auto subgroup_sizes() const -> array_view<std::int32_t> override;
    //! @copydoc ::tinytc_core_info::register_space
    auto register_space() const -> std::int32_t override;
    //! @copydoc ::tinytc_core_info::core_features()
    auto core_features() const -> tinytc_core_feature_flags_t override;
    //! @copydoc ::tinytc_core_info::core_features(tinytc_core_feature_flags_t)
    void core_features(tinytc_core_feature_flags_t flags) override;
    //! @copydoc ::tinytc_core_info::minmax_work_group_size
    auto minmax_work_group_size() const -> std::int32_t override;
    //! @copydoc ::tinytc_core_info::get_core_config
    auto get_core_config(std::int32_t subgroup_size) const -> core_config override;
    auto matrix() const -> matrix_ext_info const & override;

  private:
    inline auto is_arch(tinytc_intel_gpu_architecture_t arch) const -> bool {
        return arch <= ip_version_ &&
               ip_version_ <= arch + TINYTC_INTEL_GPU_ARCHITECTURE_SUB_VERSION_BITS;
    }
    auto num_reg_small_grf() const -> std::int32_t;
    auto num_reg_large_grf() const -> std::int32_t;
    auto num_reg() const -> std::int32_t;
    auto max_work_group_size(std::int32_t subgroup_size) const -> std::int32_t;

    std::uint32_t ip_version_;
    std::int32_t num_eus_per_subslice_;
    std::int32_t num_threads_per_eu_;
    std::vector<std::int32_t> subgroup_sizes_;
    std::int32_t register_size_;
    tinytc_core_feature_flags_t core_features_;
    matrix_ext_info matrix_;
};

} // namespace tinytc

#endif // DEVICE_INFO_20240304_HPP
