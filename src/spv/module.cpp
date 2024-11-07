// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "spv/module.hpp"
#include "spv/instructions.hpp"
#include "support/ilist_base.hpp"

namespace tinytc {
void ilist_callbacks<spv::spv_inst>::node_added(spv::spv_inst *) {}
void ilist_callbacks<spv::spv_inst>::node_removed(spv::spv_inst *node) { delete node; }
} // namespace tinytc

namespace tinytc::spv {
mod::mod(std::int32_t major_version, std::int32_t minor_version)
    : major_version_{major_version}, minor_version_{minor_version} {}
mod::~mod() {}

auto mod::bound() const -> std::int32_t {
    std::int32_t bnd = 0;
    for (auto const &sec : insts_) {
        for (auto const &i : sec) {
            if (i.has_result_id()) {
                ++bnd;
            }
        }
    }
    return bnd;
}

} // namespace tinytc::spv

