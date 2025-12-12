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

auto get_use_permutation_functional(core_config const &cfg, coopmatrix_type const *at,
                                    coopmatrix_type const *rt) -> use_permutation_functional {
    if (rt->use() == matrix_use::b && at->use() == matrix_use::acc) {
        auto al = get_layout(cfg, at);
        auto rl = get_layout(cfg, rt);
        return [al, rl](std::int64_t v) -> std::int64_t {
            /**
             * Using that M >= S we have for matrix_b
             * L_b(i,k_1,j,k_2) = i + k_1*S + j*S*K_1 + k_2*S*K_1*J.
             *
             * We have
             * p_b + v_bS = L_b
             *
             * Recovering i,k_1,j,k_2 from L_b we have
             *   i = L_b%S = p_b
             * k_1 = L_b/S%K_1 = v_b%K_1
             *   j = L_b/(SK_1)%J = v_b/K_1%J
             * k_2 = L_b/(SK_1J) = v_b/(K_1J)
             *
             * Let k=k_1 + k_2K_1, and L_1, L_2 be the block sizes of matrix
             * acc. We have L_{acc} = i + (k%L_1)*S + j*S*L_1 +
             * (k/L_1)*S*L_1*J.
             *
             * Recovering p_{acc}, v_{acc} from
             * p_{acc} + v_{acc}S = L_{acc}
             * we have
             * p_{acc} = L_{acc}%S = p_b
             * v_{acc} = L_{acc}/S = k%L_1 + j*L_1 + (k/L_1)*L_1*J
             *
             * If M < S, then we have K_1=K_2=L_1=L_2=1, and there is no layout
             * transformation. The code below just returns v - the identity -
             * if M < S.
             */
            auto const k_1 = v % rl.blocks1;
            auto const j = v / rl.blocks1 % rl.cols;
            auto const k_2 = v / (rl.blocks1 * rl.cols);
            auto const k = k_1 + k_2 * rl.blocks1;
            return k % al.blocks1 + j * al.blocks1 + (k / al.blocks1) * al.blocks1 * al.cols;
        };
    }
    return [](std::int64_t v) -> std::int64_t { return v; };
}

} // namespace tinytc
