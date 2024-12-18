// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef LUT_20241216_HPP
#define LUT_20241216_HPP

namespace tinytc::spv {

class spv_inst;

template <typename Map, typename Key, typename Maker>
auto lookup(Map &map, Key &&key, Maker &&maker) {
    auto it = map.find(key);
    if (it == map.end()) {
        map[key] = maker(key);
        return map[key];
    }
    return it->second;
}
template <typename Maker> auto lookup(spv_inst *&var, Maker &&maker) -> spv_inst * {
    if (!var) {
        var = maker();
    }
    return var;
}

} // namespace tinytc::spv

#endif // LUT_20241216_HPP
