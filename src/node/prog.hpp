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
    using iterator = tinytc::indirect_random_access_iterator<std::vector<tinytc::func>::iterator>;
    using const_iterator =
        tinytc::indirect_random_access_iterator<std::vector<tinytc::func>::const_iterator>;

    tinytc_prog(tinytc::compiler_context ctx, tinytc_location const &lc = {});

    inline auto context() const -> tinytc_compiler_context_t { return ctx_.get(); }
    inline auto share_context() const -> tinytc::compiler_context { return ctx_; }

    inline auto loc() const noexcept -> tinytc_location const & { return loc_; }
    inline void loc(tinytc_location const &loc) noexcept { loc_ = loc; }

    inline auto begin() -> iterator { return iterator{funcs_.begin()}; }
    inline auto end() -> iterator { return iterator{funcs_.end()}; }
    inline auto begin() const -> const_iterator { return const_iterator{funcs_.begin()}; }
    inline auto end() const -> const_iterator { return const_iterator{funcs_.end()}; }
    inline void push_back(tinytc::func fun) { funcs_.push_back(std::move(fun)); }

  private:
    tinytc::compiler_context ctx_;
    std::vector<tinytc::func> funcs_;
    tinytc_location loc_;
};

namespace tinytc {

using program_node = ::tinytc_prog;

} // namespace tinytc

#endif // PROG_20250626_HPP
