// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef VALUE_NODE_20230309_HPP
#define VALUE_NODE_20230309_HPP

#include "location.hpp"
#include "node/data_type_node.hpp"
#include "tinytc/types.h"

#include <cstdint>
#include <string>
#include <utility>

struct tinytc_value final {
  public:
    tinytc_value(tinytc_data_type_t ty = nullptr, tinytc::location const &lc = {});

    inline auto loc() const noexcept -> tinytc::location const & { return loc_; }
    inline void loc(tinytc::location const &loc) noexcept { loc_ = loc; }

    inline auto ty() const -> tinytc_data_type_t { return ty_; }

    inline auto context() const -> tinytc_compiler_context_t { return ty_->context(); }

    inline auto name() const -> char const * { return name_.c_str(); }
    inline void name(std::string name) { name_ = std::move(name); }
    auto has_name() const -> bool { return !name_.empty(); }

  private:
    tinytc_data_type_t ty_;
    tinytc::location loc_;
    std::string name_;
};

namespace tinytc {

using value_node = ::tinytc_value;

} // namespace tinytc

#endif // VALUE_NODE_20230309_HPP
