// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "coopmatrix_layout.hpp"
#include "device_info.hpp"
#include "node/type.hpp"
#include "number.hpp"
#include "tinytc/types.hpp"

#include <algorithm>

namespace tinytc {

auto get_layout(core_config const &cfg, coopmatrix_type const *ct) -> coopmatrix_layout {
    auto l = coopmatrix_layout{};
    l.sty = ct->component_ty();
    l.rows = std::min(ct->rows(), static_cast<std::int64_t>(cfg.subgroup_size));
    l.cols = (1 + (l.rows * ct->cols() - 1) / cfg.subgroup_size) * cfg.subgroup_size / l.rows;
    l.blocks = ct->rows() / l.rows;
    l.length = l.rows * l.cols * l.blocks / cfg.subgroup_size;
    l.shape1 = ct->cols();
    l.blocks1 = 1;
    auto sty_size = size(l.sty);
    if (ct->use() == matrix_use::b && l.blocks > 1) {
        const auto omega_b = std::max(1, static_cast<int>(2 / sty_size));
        l.blocks1 = omega_b;
    }
    l.ops_per_chan = 1;
    if (ct->use() == matrix_use::a) {
        const auto omega = std::max(1, static_cast<int>(4 / sty_size));
        if (l.cols % l.ops_per_chan == 0) {
            l.ops_per_chan = omega;
        }
    }

    return l;
}

} // namespace tinytc
