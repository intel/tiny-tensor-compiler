// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DEVICE_INFO_20240304_HPP
#define DEVICE_INFO_20240304_HPP

#include "reference_counted.hpp"
#include "tinytc/types.h"

#include <cstdint>
#include <vector>

namespace tinytc {

//! Core parameters for a specific choice of subgroup size and core feature flags
class core_config {
  public:
    std::int32_t subgroup_size;       ///< Smallest unit of execution
    std::int32_t max_work_group_size; ///< Maximum size of local work group in number of works items
    std::int32_t register_space;      ///< Size of register file in bytes
    bool block_read_write_supported;  ///< True if block reads / block writes are suppported
};

} // namespace tinytc

struct tinytc_core_info : tinytc::reference_counted {
    //! empty dtor
    virtual ~tinytc_core_info();
    //! Returns available subgroup sizes
    virtual auto subgroup_sizes() const -> std::vector<std::int32_t> const & = 0;
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
};

namespace tinytc {

class core_info_generic : public ::tinytc_core_info {
  public:
    core_info_generic(std::int32_t register_space, std::int32_t max_work_group_size,
                      std::vector<std::int32_t> subgroup_sizes);
    auto subgroup_sizes() const -> std::vector<std::int32_t> const & override;
    auto register_space() const -> std::int32_t override;
    auto core_features() const -> tinytc_core_feature_flags_t override;
    void core_features(tinytc_core_feature_flags_t flags) override;
    auto minmax_work_group_size() const -> std::int32_t override;
    auto get_core_config(std::int32_t subgroup_size) const -> tinytc::core_config override;

  private:
    std::int32_t register_space_;
    std::int32_t max_work_group_size_;
    std::vector<std::int32_t> subgroup_sizes_;
};

//! Set of core configurations for Intel GPUs
class core_info_intel : public ::tinytc_core_info {
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

    //! @copydoc ::tinytc_core_info::subgroup_sizes
    auto subgroup_sizes() const -> std::vector<std::int32_t> const & override;
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

  private:
    auto num_reg_small_grf() const -> std::int32_t;
    auto num_reg_large_grf() const -> std::int32_t;
    auto max_work_group_size(std::int32_t subgroup_size) const -> std::int32_t;

    std::uint32_t ip_version_;
    std::int32_t num_eus_per_subslice_;
    std::int32_t num_threads_per_eu_;
    std::vector<std::int32_t> subgroup_sizes_;
    std::int32_t register_size_;
    std::int32_t num_registers_per_thread_;
    tinytc_core_feature_flags_t core_features_;
};

} // namespace tinytc

#endif // DEVICE_INFO_20240304_HPP
