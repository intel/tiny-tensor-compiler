// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef BINARY_20240308_HPP
#define BINARY_20240308_HPP

#include "reference_counted.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

struct tinytc_core_info;
namespace tinytc {
enum class bundle_format;
}

/**
 * @brief Container encapsulating a SPIR-V or native device binary
 */
struct tinytc_binary : tinytc::reference_counted {
  public:
    /**
     * @brief Create binary
     *
     * @param data Binary data
     * @param format Binary format (SPIR-V or native device binary)
     * @param metadata_map Dictionary kernel name -> kernel metadata
     * @param core_features Required core features
     */
    tinytc_binary(std::vector<std::uint8_t> data, tinytc::bundle_format format,
                  std::uint32_t core_features);

    //! Get raw data
    inline auto data() const noexcept -> std::uint8_t const * { return data_.data(); }
    //! Get size of raw data
    inline auto size() const noexcept -> std::size_t { return data_.size(); }
    //! Get binary format
    inline auto format() const noexcept -> tinytc::bundle_format { return format_; }
    //! Get core features
    inline auto core_features() const noexcept -> std::uint32_t { return core_features_; }

  private:
    std::vector<std::uint8_t> data_;
    tinytc::bundle_format format_;
    std::uint32_t core_features_;
};

#endif // BINARY_20240308_HPP
