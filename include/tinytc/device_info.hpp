// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DEVICE_INFO_20240304_HPP
#define DEVICE_INFO_20240304_HPP

#include "tinytc/export.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace tinytc {

//! Core features that may be optionally enabled
enum class TINYTC_EXPORT core_feature_flag : std::uint32_t {
    /**
     * Request a large register file.
     * On PVC this doubles the number of registers per vector engine
     * but halves the number of available hardware threads.
     * When this feature is activated, the kernel is compiled with
     * the "-ze-opt-large-register-file" option.
     */
    large_register_file = 0x1
};

//! Core parameters for a specific choice of subgroup size and core feature flags
class TINYTC_EXPORT core_config {
  public:
    std::uint32_t subgroup_size; ///< Smallest unit of execution
    std::uint32_t
        max_number_of_work_items;    ///< Maximum size of local work group in number of works items
    std::uint32_t local_memory_size; ///< Maximum size of shared local memory in byte
    std::uint32_t register_space;    ///< Size of register file in bytes
    std::uint32_t ip_version;        ///< Device ip version
    std::uint32_t core_features;     ///< Required core features / compilation flags
};

//! Set of core configurations
class TINYTC_EXPORT core_info {
  public:
    //! empty dtor
    virtual ~core_info();
    //! Returns IP version
    virtual auto ip_version() const -> std::uint32_t = 0;
    //! Returns available subgroup sizes
    virtual auto subgroup_sizes() const -> std::vector<std::uint32_t> const & = 0;
    //! Returns size of one register size in bytes
    virtual auto register_size() const -> std::uint32_t = 0;
    //! Returns available number of registers per subgroup
    virtual auto num_registers_per_thread() const -> std::uint32_t = 0;
    //! Request core feature
    virtual void set_core_feature(core_feature_flag flag) = 0;
    //! Clear core feature request
    virtual void clear_core_feature(core_feature_flag flag) = 0;
    //! Get core features
    virtual auto core_features() const -> std::uint32_t = 0;
    //! Return core config for specific subgroup size and number of registers per tile
    virtual auto get_core_config(std::uint32_t subgroup_size) const -> core_config = 0;
};

/**
 * @brief IP versions for Intel GPUs
 *
 * Note: IP versions are extracted from
 * * https://github.com/intel/compute-runtime/blob/4b5d5f235abf0ff67c9188f8096afd4da2e0574d/third_party/aot_config_headers/platforms.h
 * * https://github.com/intel/llvm/blob/56e9067ba69809fb6ea1fd4328456ca3a009f984/sycl/source/detail/device_info.hpp#L619
 */
enum class TINYTC_EXPORT intel_gpu_architecture : std::uint32_t {
    pvc = 0x030f0007 ///< PVC
};

//! Look up core info for Intel GPU architecture
TINYTC_EXPORT auto get_core_info_intel_gpu(intel_gpu_architecture arch)
    -> std::shared_ptr<core_info>;

//! Set of core configurations for Intel GPUs
class TINYTC_EXPORT core_info_intel : public core_info {
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

    //! @copydoc core_info::ip_version
    auto ip_version() const -> std::uint32_t override;
    //! @copydoc core_info::subgroup_sizes
    auto subgroup_sizes() const -> std::vector<std::uint32_t> const & override;
    //! @copydoc core_info::register_size
    auto register_size() const -> std::uint32_t override;
    //! @copydoc core_info::num_registers_per_thread
    auto num_registers_per_thread() const -> std::uint32_t override;
    //! @copydoc core_info::set_core_feature
    void set_core_feature(core_feature_flag flag) override;
    //! @copydoc core_info::clear_core_feature
    void clear_core_feature(core_feature_flag flag) override;
    //! @copydoc core_info::core_features
    auto core_features() const -> std::uint32_t override;
    //! @copydoc core_info::get_core_config
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
