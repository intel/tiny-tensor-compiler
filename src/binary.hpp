// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef BINARY_20240308_HPP
#define BINARY_20240308_HPP

#include "kernel_metadata.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

struct tinytc_core_info;
namespace tinytc {
enum class bundle_format;
}

/**
 * @brief Container encapsulating a SPIR-V or native device binary
 */
struct tinytc_binary {
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
                  std::unordered_map<std::string, tinytc::kernel_metadata> metadata_map,
                  std::uint32_t core_features);

    //! Get raw data
    inline auto data() const -> std::uint8_t const * { return data_.data(); }
    //! Get size of raw data
    inline auto size() const -> std::size_t { return data_.size(); }
    //! Get binary format
    inline auto format() const -> tinytc::bundle_format { return format_; }
    //! Get metadata map
    inline auto metadata() const
        -> std::unordered_map<std::string, tinytc::kernel_metadata> const & {
        return metadata_;
    }
    //! Get metadata for kernel name
    inline auto metadata(std::string const &name) const -> tinytc::kernel_metadata const & {
        return metadata_.at(name);
    }
    //! Get core features
    inline auto core_features() const -> std::uint32_t { return core_features_; }

  private:
    std::vector<std::uint8_t> data_;
    tinytc::bundle_format format_;
    std::unordered_map<std::string, tinytc::kernel_metadata> metadata_;
    std::uint32_t core_features_;
};

#endif // BINARY_20240308_HPP
