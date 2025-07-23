// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef REGION_20250626_HPP
#define REGION_20250626_HPP

#include "node/inst.hpp"
#include "node/value.hpp"
#include "tinytc/core.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"
#include "util/ilist.hpp"
#include "util/iterator.hpp"

#include <cstddef>
#include <type_traits>
#include <vector>

namespace tinytc {

//! Instruction classification
enum class region_kind { mixed = 0x0, collective = 0x1, spmd = 0x2 };

template <> struct ilist_callbacks<tinytc_inst> {
    auto get_parent_region() -> tinytc_region *;
    void node_added(tinytc_inst_t node);
    void node_moved(tinytc_inst_t node);
    void node_removed(tinytc_inst_t node);
};

} // namespace tinytc

struct alignas(8) tinytc_region final {
  public:
    using iterator = tinytc::ilist<tinytc_inst>::iterator;
    using const_iterator = tinytc::ilist<tinytc_inst>::const_iterator;

    tinytc_region(tinytc_inst_t def_inst = nullptr);
    ~tinytc_region();

    tinytc_region(tinytc_region const &) = delete;
    tinytc_region(tinytc_region &&) = delete;
    tinytc_region &operator=(tinytc_region const &) = delete;
    tinytc_region &operator=(tinytc_region &&) = delete;

    inline auto kind() const noexcept -> tinytc::region_kind { return kind_; }
    inline void kind(tinytc::region_kind kind) noexcept { kind_ = kind; }

    inline auto loc() const noexcept -> tinytc::location const & { return loc_; }
    void loc(tinytc::location const &loc);

    // Can be nullptr, e.g. if the region is the body of a function
    inline auto defining_inst() const -> tinytc_inst_t { return def_inst_; }
    void defining_inst(tinytc_inst_t def_inst);

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
    inline auto param(std::size_t pos) -> tinytc_value & { return params_[pos]; }
    inline auto param(std::size_t pos) const -> tinytc_value const & { return params_[pos]; }
    inline auto params() const {
        return tinytc::iterator_range_wrapper{param_begin(), param_end()};
    }
    inline auto num_params() const noexcept -> std::size_t { return params_.size(); }
    void set_params(tinytc::array_view<tinytc_type_t> param_types);
    void set_num_params(std::size_t num_params);
    void set_param(std::size_t idx, tinytc_type_t param_type);

  private:
    static auto inst_list_offset() -> std::size_t {
        static_assert(std::is_standard_layout_v<tinytc_region>, "offsetof not guaranteed to work");
        return offsetof(tinytc_region, insts_);
    }
    friend struct tinytc::ilist_callbacks<tinytc_inst>;

    tinytc_inst_t def_inst_;
    tinytc::region_kind kind_;
    tinytc::location loc_;
    // params_ must come before insts_ such that the dtors are called in the correct order
    std::vector<tinytc_value> params_;
    tinytc::ilist<tinytc_inst> insts_;
};

#endif // REGION_20250626_HPP
