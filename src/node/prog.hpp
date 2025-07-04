// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PROG_20250626_HPP
#define PROG_20250626_HPP

#include "reference_counted.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"
#include "util/iterator.hpp"

#include <utility>
#include <vector>

struct tinytc_prog final : tinytc::reference_counted {
  public:
    using container_t = std::vector<tinytc::unique_handle<tinytc_func_t>>;

    using iterator = tinytc::indirect_random_access_iterator<container_t::iterator>;
    using const_iterator = tinytc::indirect_random_access_iterator<container_t::const_iterator>;

    tinytc_prog(tinytc::shared_handle<tinytc_compiler_context_t> ctx,
                tinytc_location const &lc = {});

    inline auto context() const -> tinytc_compiler_context_t { return ctx_.get(); }
    inline auto share_context() const -> tinytc::shared_handle<tinytc_compiler_context_t> {
        return ctx_;
    }

    inline auto loc() const noexcept -> tinytc_location const & { return loc_; }
    inline void loc(tinytc_location const &loc) noexcept { loc_ = loc; }

    inline auto begin() -> iterator { return iterator{funcs_.begin()}; }
    inline auto end() -> iterator { return iterator{funcs_.end()}; }
    inline auto begin() const -> const_iterator { return const_iterator{funcs_.begin()}; }
    inline auto end() const -> const_iterator { return const_iterator{funcs_.end()}; }
    inline void push_back(tinytc::unique_handle<tinytc_func_t> &&fun) {
        funcs_.push_back(std::move(fun));
    }

  private:
    tinytc::shared_handle<tinytc_compiler_context_t> ctx_;
    container_t funcs_;
    tinytc_location loc_;
};

namespace tinytc {

using program_node = ::tinytc_prog;

} // namespace tinytc

#endif // PROG_20250626_HPP
