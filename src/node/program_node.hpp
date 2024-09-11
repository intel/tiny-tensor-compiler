// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PROGRAM_NODE_20240208_HPP
#define PROGRAM_NODE_20240208_HPP

#include "location.hpp"
#include "reference_counted.hpp"
#include "support/util.hpp"
#include "tinytc/tinytc.hpp"

#include <cstdint>
#include <utility>
#include <vector>

namespace tinytc {
using func_range = iterator_range_wrapper<func *>;
using const_func_range = iterator_range_wrapper<func const *>;
} // namespace tinytc

struct tinytc_prog : tinytc::reference_counted {
  public:
    inline tinytc_prog(std::vector<tinytc::func> funcs, tinytc::location const &lc = {})
        : funcs_(std::move(funcs)) {
        loc(lc);
    }

    inline auto loc() const noexcept -> tinytc::location const & { return loc_; }
    inline void loc(tinytc::location const &loc) noexcept { loc_ = loc; }

    inline auto begin() -> tinytc::func * { return funcs_.size() > 0 ? funcs_.data() : nullptr; }
    inline auto end() -> tinytc::func * {
        return funcs_.size() > 0 ? funcs_.data() + funcs_.size() : nullptr;
    }
    inline auto functions() -> tinytc::func_range { return tinytc::func_range{begin(), end()}; }
    inline auto begin() const -> tinytc::func const * {
        return funcs_.size() > 0 ? funcs_.data() : nullptr;
    }
    inline auto end() const -> tinytc::func const * {
        return funcs_.size() > 0 ? funcs_.data() + funcs_.size() : nullptr;
    }
    inline auto functions() const -> tinytc::const_func_range {
        return tinytc::const_func_range{begin(), end()};
    }

  private:
    std::vector<tinytc::func> funcs_;
    tinytc::location loc_;
};

namespace tinytc {

using program = ::tinytc_prog;

} // namespace tinytc

#endif // PROGRAM_NODE_20240208_HPP
