// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef BINARY_20240308_HPP
#define BINARY_20240308_HPP

#include "kernel_metadata.hpp"
#include "tinytc/ir/error.hpp"
#include "tinytc/tinytc.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct tinytc_core_info;

namespace tinytc {

enum class bundle_format;

/**
 * @brief Container encapsulating a SPIR-V or native device binary
 */
class binary {
  public:
    /**
     * @brief Create binary
     *
     * @param data Binary data
     * @param format Binary format (SPIR-V or native device binary)
     * @param metadata_map Dictionary kernel name -> kernel metadata
     * @param core_features Required core features
     */
    binary(std::vector<std::uint8_t> data, bundle_format format,
           std::unordered_map<std::string, kernel_metadata> metadata_map,
           std::uint32_t core_features);

    //! Get raw data
    inline auto data() const -> std::uint8_t const * { return data_.data(); }
    //! Get size of raw data
    inline auto size() const -> std::size_t { return data_.size(); }
    //! Get binary format
    inline auto format() const -> bundle_format { return format_; }
    //! Get metadata map
    inline auto metadata() const -> std::unordered_map<std::string, kernel_metadata> const & {
        return metadata_;
    }
    //! Get metadata for kernel name
    inline auto metadata(std::string const &name) const -> kernel_metadata const & {
        return metadata_.at(name);
    }
    //! Get core features
    inline auto core_features() const -> std::uint32_t { return core_features_; }

  private:
    std::vector<std::uint8_t> data_;
    bundle_format format_;
    std::unordered_map<std::string, kernel_metadata> metadata_;
    std::uint32_t core_features_;
};

/**
 * @brief Optimize program and creaty device binary
 *
 * Compiler passes are applied on prog, therefore prog is modified.
 *
 * @param prog Tensor program abstract syntax tree
 * @param format Binary format
 * @param info Core info
 * @param err Error reporting functional
 *
 * @return binary
 */
auto optimize_and_make_binary(prog prog, bundle_format format, tinytc_core_info const &info,
                              error_reporter_function err = null_error_reporter())
    -> std::shared_ptr<binary>;

} // namespace tinytc

#endif // BINARY_20240308_HPP
