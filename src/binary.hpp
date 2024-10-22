// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef BINARY_20240308_HPP
#define BINARY_20240308_HPP

#include "compiler_context.hpp"
#include "reference_counted.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

/**
 * @brief Container encapsulating a SPIR-V or native device binary
 */
struct tinytc_binary : tinytc::reference_counted {
  public:
    /**
     * @brief Create binary
     *
     * @param ctx Compiler context
     * @param data Binary data
     * @param format Binary format (SPIR-V or native device binary)
     * @param metadata_map Dictionary kernel name -> kernel metadata
     * @param core_features Required core features
     */
    tinytc_binary(tinytc::compiler_context ctx, std::vector<std::uint8_t> data,
                  tinytc::bundle_format format, tinytc_core_feature_flags_t core_features);

    inline auto context() const -> tinytc_compiler_context_t { return ctx_.get(); }
    inline auto share_context() const -> tinytc::compiler_context { return ctx_; }
    //! Get raw data
    inline auto data() const noexcept -> std::uint8_t const * { return data_.data(); }
    //! Get size of raw data
    inline auto size() const noexcept -> std::size_t { return data_.size(); }
    //! Get binary format
    inline auto format() const noexcept -> tinytc::bundle_format { return format_; }
    //! Get core features
    inline auto core_features() const noexcept -> tinytc_core_feature_flags_t {
        return core_features_;
    }

  private:
    tinytc::compiler_context ctx_;
    std::vector<std::uint8_t> data_;
    tinytc::bundle_format format_;
    tinytc_core_feature_flags_t core_features_;
};

#endif // BINARY_20240308_HPP
