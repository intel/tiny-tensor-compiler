// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef REGION_NODE_20230908_HPP
#define REGION_NODE_20230908_HPP

#include "reference_counted.hpp"
#include "support/util.hpp"
#include "tinytc/tinytc.hpp"

#include <cstdint>
#include <utility>
#include <vector>

namespace tinytc {

//! Instruction classification
enum class region_kind { mixed, collective, spmd };

} // namespace tinytc

struct tinytc_region : tinytc::reference_counted {
  public:
    using iterator = std::vector<tinytc::inst>::iterator;
    using const_iterator = std::vector<tinytc::inst>::const_iterator;

    inline tinytc_region(std::vector<tinytc::inst> insts = {}, tinytc::location const &lc = {})
        : insts_(std::move(insts)), kind_(tinytc::region_kind::mixed) {
        loc(lc);
    }

    inline auto kind() const noexcept -> tinytc::region_kind { return kind_; }
    inline void kind(tinytc::region_kind kind) noexcept { kind_ = kind; }

    inline auto loc() const noexcept -> tinytc::location const & { return loc_; }
    inline void loc(tinytc::location const &loc) noexcept { loc_ = loc; }

    inline auto begin() -> iterator { return insts_.begin(); }
    inline auto end() -> iterator { return insts_.end(); }
    inline auto insts() -> tinytc::iterator_range_wrapper<iterator> { return {begin(), end()}; }
    inline auto begin() const -> const_iterator { return insts_.cbegin(); }
    inline auto end() const -> const_iterator { return insts_.cend(); }
    inline auto insts() const -> tinytc::iterator_range_wrapper<const_iterator> {
        return {begin(), end()};
    }
    inline void insts(std::vector<tinytc::inst> insts) { insts_ = std::move(insts); }
    inline auto erase(iterator pos) -> iterator { return insts_.erase(pos); }
    inline auto insert(iterator pos, tinytc::inst const &i) -> iterator {
        return insts_.insert(pos, i);
    }
    inline auto insert(iterator pos, tinytc::inst &&i) -> iterator {
        return insts_.insert(pos, std::move(i));
    }
    inline auto empty() const -> bool { return insts_.empty(); }

  private:
    std::vector<tinytc::inst> insts_;
    tinytc::region_kind kind_;
    tinytc::location loc_;
};

namespace tinytc {
using rgn = ::tinytc_region;
} // namespace tinytc

#endif // REGION_NODE_20230908_HPP
