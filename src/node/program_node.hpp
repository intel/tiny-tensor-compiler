// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PROGRAM_NODE_20240208_HPP
#define PROGRAM_NODE_20240208_HPP

#include "location.hpp"
#include "reference_counted.hpp"
#include "support/util.hpp"

#include <cstdint>
#include <utility>
#include <vector>

namespace tinytc {
using func_range = iterator_range_wrapper<tinytc_func_t *>;
using const_func_range = iterator_range_wrapper<const_tinytc_func_t *>;
} // namespace tinytc

struct tinytc_prog final : tinytc::reference_counted {
  public:
    inline tinytc_prog(tinytc_location const &lc = {}) { loc(lc); }
    ~tinytc_prog();

    inline auto loc() const noexcept -> tinytc_location const & { return loc_; }
    inline void loc(tinytc_location const &loc) noexcept { loc_ = loc; }

    inline auto begin() -> tinytc_func_t * { return funcs_.size() > 0 ? funcs_.data() : nullptr; }
    inline auto end() -> tinytc_func_t * {
        return funcs_.size() > 0 ? funcs_.data() + funcs_.size() : nullptr;
    }
    inline auto functions() -> tinytc::func_range { return tinytc::func_range{begin(), end()}; }
    inline auto begin() const -> const_tinytc_func_t * {
        return funcs_.size() > 0 ? const_cast<const_tinytc_func_t *>(funcs_.data()) : nullptr;
    }
    inline auto end() const -> const_tinytc_func_t * {
        return funcs_.size() > 0 ? const_cast<const_tinytc_func_t *>(funcs_.data()) + funcs_.size()
                                 : nullptr;
    }
    inline auto functions() const -> tinytc::const_func_range {
        return tinytc::const_func_range{begin(), end()};
    }
    inline void push_back(tinytc_func_t fun) { funcs_.push_back(fun); }

  private:
    std::vector<tinytc_func_t> funcs_;
    tinytc_location loc_;
};

namespace tinytc {

using program_node = ::tinytc_prog;

} // namespace tinytc

#endif // PROGRAM_NODE_20240208_HPP
