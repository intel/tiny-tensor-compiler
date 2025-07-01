// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef WALK_20250613_HPP
#define WALK_20250613_HPP

#include "object.hpp"

#include <functional>

namespace mochi {

enum class walk_order { pre_order, post_order };

template <walk_order Order, typename T, bool OnlyLeaves = false>
void walk_down(T *in, std::function<void(T *)> callback) {
    const bool invoke_callback = !OnlyLeaves || !in->has_children();
    if constexpr (Order == walk_order::pre_order) {
        if (invoke_callback) {
            callback(in);
        }
    }
    for (auto &child : in->children()) {
        walk_down<Order>(child.get(), callback);
    }
    if constexpr (Order == walk_order::post_order) {
        if (invoke_callback) {
            callback(in);
        }
    }
}

template <walk_order Order, typename T>
void walk_down(T *in, std::function<void(T *)> callback,
               std::function<void(T *)> prepost_callback) {
    if constexpr (Order == walk_order::pre_order) {
        callback(in);
    } else {
        prepost_callback(in);
    }
    for (auto &child : in->children()) {
        walk_down<Order>(child.get(), callback);
    }
    if constexpr (Order == walk_order::post_order) {
        callback(in);
    } else {
        prepost_callback(in);
    }
}

template <walk_order Order, typename T> void walk_up(T *in, std::function<void(T *)> callback) {
    if constexpr (Order == walk_order::pre_order) {
        callback(in);
    }
    if (in->parent()) {
        walk_up<Order>(in->parent(), callback);
    }
    if constexpr (Order == walk_order::post_order) {
        callback(in);
    }
}

} // namespace mochi

#endif // WALK_20250613_HPP
