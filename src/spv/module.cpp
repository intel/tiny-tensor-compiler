// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "spv/module.hpp"
#include "spv/instructions.hpp"

namespace tinytc {
void ilist_callbacks<spv::spv_inst>::node_added(spv::spv_inst *) {}
void ilist_callbacks<spv::spv_inst>::node_removed(spv::spv_inst *node) { delete node; }
} // namespace tinytc

namespace tinytc::spv {
mod::mod() {}
mod::~mod() {}
} // namespace tinytc::spv

