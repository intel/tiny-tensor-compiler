// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef WALK_20240911_HPP
#define WALK_20240911_HPP

#include "node/function_node.hpp"
#include "node/inst_node.hpp"
#include "node/region_node.hpp"
#include "support/ilist_base.hpp"

#include <functional>
#include <type_traits>
#include <utility>

namespace tinytc {

enum class walk_order { pre_order, post_order };

class walk_stage {
  public:
    walk_stage(inst_node &i);

    inline bool is_before_all_regions() const { return next_region_ == 0; }
    inline bool is_after_all_regions() const { return next_region_ == num_regions_; }

    void advance() { ++next_region_; }

  private:
    int const num_regions_;
    int next_region_ = 0;
};

template <walk_order Order>
void walk(inst_node const &i, std::function<void(inst_node const &i)> callback) {
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
template <walk_order Order> void walk(inst_node &i, std::function<void(inst_node &i)> callback) {
    walk<Order>(const_cast<inst_node const &>(i),
                [c = std::move(callback)](inst_node const &i) { c(const_cast<inst_node &>(i)); });
}

template <walk_order Order>
void walk(inst_node const &i, std::function<void(region_node const &reg)> callback) {
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
template <walk_order Order>
void walk(inst_node &i, std::function<void(region_node &reg)> callback) {
    walk<Order>(
        const_cast<inst_node const &>(i),
        [c = std::move(callback)](region_node const &reg) { c(const_cast<region_node &>(reg)); });
}

void walk(inst_node &i, std::function<void(inst_node &i, walk_stage const &stage)> callback);

template <walk_order Order>
void walk(function_node const &fn, std::function<void(inst_node const &i)> callback) {
    for (auto &i : fn.body()) {
        walk<Order>(i, callback);
    }
}
template <walk_order Order>
void walk(function_node &fn, std::function<void(inst_node &i)> callback) {
    for (auto &i : fn.body()) {
        walk<Order>(i, callback);
    }
}

template <walk_order Order>
void walk(function_node const &fn, std::function<void(region_node const &reg)> callback) {
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
template <walk_order Order>
void walk(function_node &i, std::function<void(region_node &reg)> callback) {
    walk<Order>(
        const_cast<function_node const &>(i),
        [c = std::move(callback)](region_node const &reg) { c(const_cast<region_node &>(reg)); });
}

inline void walk(function_node &fn,
                 std::function<void(inst_node &i, walk_stage const &stage)> callback) {
    for (auto &i : fn.body()) {
        walk(i, callback);
    }
}

} // namespace tinytc

#endif // WALK_20240911_HPP
