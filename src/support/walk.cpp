// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "support/walk.hpp"

namespace tinytc {

walk_stage::walk_stage(inst_node &i) : num_regions_(i.num_child_regions()) {}

void walk(inst_node &i, std::function<void(inst_node &i, walk_stage const &stage)> callback) {
    auto stage = walk_stage(i);

    for (auto &reg : i.child_regions()) {
        callback(i, stage);
        stage.advance();

        for (auto &j : *reg) {
            walk(*j, callback);
        }
    }
    callback(i, stage);
}

} // namespace tinytc
