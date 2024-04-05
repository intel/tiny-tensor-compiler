// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef BINARY_20240308_HPP
#define BINARY_20240308_HPP

#include "tinytc/export.hpp"
#include "tinytc/ir/error.hpp"
#include "tinytc/ir/prog.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace tinytc {

class core_info;
enum class bundle_format;
namespace ir {
class location;
}

//! Kernel metadata
struct TINYTC_EXPORT kernel_metadata {
    std::uint32_t subgroup_size;                  ///< Subgroup size
    std::array<std::uint32_t, 2> work_group_size; ///< Work-group size
};

/**
 * @brief Container encapsulating a SPIR-V or native device binary
 */
class TINYTC_EXPORT binary {
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
TINYTC_EXPORT auto
optimize_and_make_binary(ir::prog prog, bundle_format format, std::shared_ptr<core_info> info,
                         ir::error_reporter_function err = ir::null_error_reporter())
    -> std::shared_ptr<binary>;

} // namespace tinytc

#endif // BINARY_20240308_HPP
