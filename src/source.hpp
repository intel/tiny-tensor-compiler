// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SOURCE_20240412_HPP
#define SOURCE_20240412_HPP

#include <string>
#include <utility>

struct tinytc_source {
  public:
    inline tinytc_source(std::string code) : code_(std::move(code)) {}

    auto code() const -> char const * { return code_.c_str(); }
    auto size() const -> std::size_t { return code_.size(); }

  private:
    std::string code_;
};

namespace tinytc {
using source = ::tinytc_source;
}

#endif // SOURCE_20240412_HPP
