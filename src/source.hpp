// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SOURCE_20240412_HPP
#define SOURCE_20240412_HPP

#include "reference_counted.hpp"
#include "tinytc/types.h"

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

struct tinytc_source : tinytc::reference_counted {
  public:
    inline tinytc_source(std::string code, tinytc_location const &code_loc,
                         std::vector<char const *> required_extensions)
        : code_(std::move(code)), code_loc_(code_loc),
          required_extensions_(std::move(required_extensions)) {}

    inline auto code() const -> char const * { return code_.c_str(); }
    inline auto code_loc() const -> tinytc_location const & { return code_loc_; }
    inline auto size() const -> std::size_t { return code_.size(); }
    inline auto const &required_extensions() const { return required_extensions_; }

  private:
    std::string code_;
    tinytc_location code_loc_;
    std::vector<char const *> required_extensions_;
};

#endif // SOURCE_20240412_HPP
