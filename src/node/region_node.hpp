// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef REGION_NODE_20230908_HPP
#define REGION_NODE_20230908_HPP

#include "node/inst_node.hpp"
#include "support/ilist.hpp"
#include "support/util.hpp"
#include "tinytc/tinytc.hpp"

#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

namespace tinytc {

//! Instruction classification
enum class region_kind { mixed = 0x0, collective = 0x1, spmd = 0x2 };

template <> struct ilist_traits<inst_node> {
    auto get_parent_region() -> tinytc_region *;
    void node_added(inst_node *node) { node->parent(get_parent_region()); }
    void node_removed(inst_node *node) { tinytc_inst_destroy(node); }
};

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
    inline auto insts() -> tinytc::ilist<tinytc_inst> & { return insts_; }
    inline auto begin() const -> const_iterator { return insts_.cbegin(); }
    inline auto end() const -> const_iterator { return insts_.cend(); }
    inline auto insts() const -> tinytc::ilist<tinytc_inst> const & { return insts_; }
    inline auto empty() const -> bool { return insts_.empty(); }

  private:
    static auto inst_list_offset() -> std::size_t {
        static_assert(std::is_standard_layout_v<tinytc_region>, "offsetof not guaranteed to work");
        return offsetof(tinytc_region, insts_);
    }
    friend struct tinytc::ilist_traits<tinytc_inst>;

    tinytc::region_kind kind_;
    tinytc::ilist<tinytc_inst> insts_;
    tinytc::location loc_;
};

namespace tinytc {

using region_node = ::tinytc_region;

} // namespace tinytc

#endif // REGION_NODE_20230908_HPP
