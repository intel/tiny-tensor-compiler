// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DEVICE_INFO_20240304_HPP
#define DEVICE_INFO_20240304_HPP

#include "tinytc/types.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace tinytc {

//! Core parameters for a specific choice of subgroup size and core feature flags
class core_config {
  public:
    std::uint32_t subgroup_size; ///< Smallest unit of execution
    std::uint32_t
        max_number_of_work_items;    ///< Maximum size of local work group in number of works items
    std::uint32_t local_memory_size; ///< Maximum size of shared local memory in byte
    std::uint32_t register_space;    ///< Size of register file in bytes
    std::uint32_t ip_version;        ///< Device ip version
    std::uint32_t core_features;     ///< Required core features / compilation flags
};

} // namespace tinytc

struct tinytc_core_info {
    //! empty dtor
    virtual ~tinytc_core_info();
    //! Returns IP version
    virtual auto ip_version() const -> std::uint32_t = 0;
    //! Returns available subgroup sizes
    virtual auto subgroup_sizes() const -> std::vector<std::uint32_t> const & = 0;
    //! Returns size of one register size in bytes
    virtual auto register_size() const -> std::uint32_t = 0;
    //! Returns available number of registers per subgroup
    virtual auto num_registers_per_thread() const -> std::uint32_t = 0;
    //! Request core feature
    virtual void set_core_feature(tinytc::core_feature_flag flag) = 0;
    //! Clear core feature request
    virtual void clear_core_feature(tinytc::core_feature_flag flag) = 0;
    //! Get core features
    virtual auto core_features() const -> std::uint32_t = 0;
    //! Return core config for specific subgroup size and number of registers per tile
    virtual auto get_core_config(std::uint32_t subgroup_size) const -> tinytc::core_config = 0;
};

namespace tinytc {

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
     * @param local_memory_size Size of shared local memory
     * @param subgroup_sizes Allowed subgroup sizes
     */
    core_info_intel(std::uint32_t ip_version, std::uint32_t num_eus_per_subslice,
                    std::uint32_t num_threads_per_eu, std::uint32_t local_memory_size,
                    std::vector<std::uint32_t> subgroup_sizes);

    //! @copydoc ::tinytc_core_info::ip_version
    auto ip_version() const -> std::uint32_t override;
    //! @copydoc ::tinytc_core_info::subgroup_sizes
    auto subgroup_sizes() const -> std::vector<std::uint32_t> const & override;
    //! @copydoc ::tinytc_core_info::register_size
    auto register_size() const -> std::uint32_t override;
    //! @copydoc ::tinytc_core_info::num_registers_per_thread
    auto num_registers_per_thread() const -> std::uint32_t override;
    //! @copydoc ::tinytc_core_info::set_core_feature
    void set_core_feature(core_feature_flag flag) override;
    //! @copydoc ::tinytc_core_info::clear_core_feature
    void clear_core_feature(core_feature_flag flag) override;
    //! @copydoc ::tinytc_core_info::core_features
    auto core_features() const -> std::uint32_t override;
    //! @copydoc ::tinytc_core_info::get_core_config
    auto get_core_config(std::uint32_t subgroup_size) const -> core_config override;

  private:
    auto num_reg_small_grf() const -> std::uint32_t;
    auto num_reg_large_grf() const -> std::uint32_t;

    std::uint32_t ip_version_;
    std::uint32_t num_eus_per_subslice_;
    std::uint32_t num_threads_per_eu_;
    std::uint32_t local_memory_size_;
    std::vector<std::uint32_t> subgroup_sizes_;
    std::uint32_t register_size_;
    std::uint32_t num_registers_per_thread_;
    std::uint32_t core_features_;
};

} // namespace tinytc

#endif // DEVICE_INFO_20240304_HPP
