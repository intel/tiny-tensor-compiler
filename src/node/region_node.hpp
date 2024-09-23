// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef REGION_NODE_20230908_HPP
#define REGION_NODE_20230908_HPP

#include "node/inst_node.hpp"
#include "support/ilist.hpp"
#include "support/util.hpp"
#include "tinytc/tinytc.hpp"

#include <cstdint>
#include <utility>
#include <vector>

namespace tinytc {

//! Instruction classification
enum class region_kind { mixed, collective, spmd };

} // namespace tinytc

struct tinytc_region final {
  public:
    using iterator = tinytc::ilist<tinytc_inst>::iterator;
    using const_iterator = tinytc::ilist<tinytc_inst>::const_iterator;

    inline tinytc_region(tinytc::location const &lc = {}) : kind_(tinytc::region_kind::mixed) {
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
    inline void push_back(tinytc_inst_t i) { insts_.push_back(i); }
    inline auto erase(iterator pos) -> iterator { return insts_.erase(pos); }
    inline auto insert(iterator pos, tinytc_inst_t i) -> iterator { return insts_.insert(pos, i); }
    inline auto insert_after(iterator pos, tinytc_inst_t i) -> iterator {
        return insts_.insert_after(pos, i);
    }
    inline auto empty() const -> bool { return insts_.empty(); }

  private:
    tinytc::region_kind kind_;
    tinytc::ilist<tinytc_inst> insts_;
    tinytc::location loc_;
};

namespace tinytc {

using region_node = ::tinytc_region;

template <> struct ilist_traits<region_node> {
    static void on_insert(region_node *) {}
    static void on_erase(region_node *node) { tinytc_region_destroy(node); }
};

} // namespace tinytc

#endif // REGION_NODE_20230908_HPP
