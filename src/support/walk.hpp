// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef WALK_20240911_HPP
#define WALK_20240911_HPP

#include "node/func.hpp"
#include "node/inst.hpp"
#include "node/region.hpp"
#include "util/ilist_base.hpp"

#include <functional>

namespace tinytc {

enum class walk_order { pre_order, post_order };

class walk_stage {
  public:
    walk_stage(tinytc_inst &i);

    inline bool is_before_all_regions() const { return next_region_ == 0; }
    inline bool is_after_all_regions() const { return next_region_ == num_regions_; }

    void advance() { ++next_region_; }

  private:
    int const num_regions_;
    int next_region_ = 0;
};

template <walk_order Order>
void walk(tinytc_inst &i, std::function<void(tinytc_inst &i)> callback) {
    if constexpr (Order == walk_order::pre_order) {
        callback(i);
    }
    for (auto &reg : i.child_regions()) {
        for (auto &j : reg) {
            walk<Order>(j, callback);
        }
    }
    if constexpr (Order == walk_order::post_order) {
        callback(i);
    }
}

template <walk_order Order>
void walk(tinytc_inst &i, std::function<void(tinytc_region &reg)> callback) {
    for (auto &reg : i.child_regions()) {
        if constexpr (Order == walk_order::pre_order) {
            callback(reg);
        }
        for (auto &j : reg) {
            walk<Order>(j, callback);
        }
        if constexpr (Order == walk_order::post_order) {
            callback(reg);
        }
    }
}

void walk(tinytc_inst &i, std::function<void(tinytc_inst &i, walk_stage const &stage)> callback);

template <walk_order Order>
void walk(tinytc_func &fn, std::function<void(tinytc_inst &i)> callback) {
    for (auto &i : fn.body()) {
        walk<Order>(i, callback);
    }
}

template <walk_order Order>
void walk(tinytc_func &fn, std::function<void(tinytc_region &reg)> callback) {
    if constexpr (Order == walk_order::pre_order) {
        callback(fn.body());
    }
    for (auto &j : fn.body()) {
        walk<Order>(j, callback);
    }
    if constexpr (Order == walk_order::post_order) {
        callback(fn.body());
    }
}

inline void walk(tinytc_func &fn,
                 std::function<void(tinytc_inst &i, walk_stage const &stage)> callback) {
    for (auto &i : fn.body()) {
        walk(i, callback);
    }
}

} // namespace tinytc

#endif // WALK_20240911_HPP
