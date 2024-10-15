// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef REGION_NODE_20230908_HPP
#define REGION_NODE_20230908_HPP

#include "node/value_node.hpp"
#include "support/ilist.hpp"
#include "support/util.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <vector>

namespace tinytc {

//! Instruction classification
enum class region_kind { mixed = 0x0, collective = 0x1, spmd = 0x2 };

template <> struct ilist_callbacks<tinytc_inst> {
    auto get_parent_region() -> tinytc_region *;
    void node_added(tinytc_inst_t node);
    void node_removed(tinytc_inst_t node);
};

} // namespace tinytc

struct tinytc_region final {
  public:
    using iterator = tinytc::ilist<tinytc_inst>::iterator;
    using const_iterator = tinytc::ilist<tinytc_inst>::const_iterator;

    tinytc_region(tinytc::array_view<tinytc_data_type_t> param_types = {},
                  tinytc::location const &lc = {});
    ~tinytc_region();

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

    inline auto param_begin() { return params_.begin(); }
    inline auto param_end() { return params_.end(); }
    inline auto params() { return tinytc::iterator_range_wrapper{param_begin(), param_end()}; }
    inline auto param_begin() const { return params_.begin(); }
    inline auto param_end() const { return params_.end(); }
    inline auto param(std::int64_t pos) -> tinytc_value & { return params_[pos]; }
    inline auto param(std::int64_t pos) const -> tinytc_value const & { return params_[pos]; }
    inline auto params() const {
        return tinytc::iterator_range_wrapper{param_begin(), param_end()};
    }
    inline auto num_params() const noexcept -> std::int64_t { return params_.size(); }
    void set_params(tinytc::array_view<tinytc_data_type_t> param_types, tinytc::location const &lc);
    void set_num_params(std::size_t num_params);
    void set_param(std::size_t idx, tinytc_data_type_t param_type, tinytc::location const &lc);

  private:
    static auto inst_list_offset() -> std::size_t {
        static_assert(std::is_standard_layout_v<tinytc_region>, "offsetof not guaranteed to work");
        return offsetof(tinytc_region, insts_);
    }
    friend struct tinytc::ilist_callbacks<tinytc_inst>;

    tinytc::region_kind kind_;
    tinytc::ilist<tinytc_inst> insts_;
    std::vector<tinytc_value> params_;
    tinytc::location loc_;
};

namespace tinytc {

using region_node = ::tinytc_region;

} // namespace tinytc

#endif // REGION_NODE_20230908_HPP
