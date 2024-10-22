// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SOURCE_20240412_HPP
#define SOURCE_20240412_HPP

#include "compiler_context.hpp"
#include "reference_counted.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"

#include <cstddef>
#include <string>
#include <vector>

struct tinytc_source : tinytc::reference_counted {
  public:
    tinytc_source(tinytc::compiler_context ctx, std::string code, tinytc_location const &code_loc,
                  std::vector<char const *> required_extensions,
                  tinytc_core_feature_flags_t core_features);

    inline auto code() const -> char const * { return code_.c_str(); }
    inline auto code_loc() const -> tinytc_location const & { return code_loc_; }
    inline auto context() const -> tinytc_compiler_context_t { return ctx_.get(); }
    inline auto share_context() const -> tinytc::compiler_context { return ctx_; }
    inline auto size() const -> std::size_t { return code_.size(); }
    inline auto const &required_extensions() const { return required_extensions_; }
    inline auto core_features() const noexcept -> tinytc_core_feature_flags_t {
        return core_features_;
    }

  private:
    tinytc::compiler_context ctx_;
    std::string code_;
    tinytc_location code_loc_;
    std::vector<char const *> required_extensions_;
    tinytc_core_feature_flags_t core_features_;
};

#endif // SOURCE_20240412_HPP
