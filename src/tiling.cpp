// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tiling.hpp"
#include "device_info.hpp"
#include "gemm_tools.hpp"
#include "matrix_ext_info.hpp"
#include "support/fnv1a.hpp"
#include "tinytc/tinytc.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

namespace tinytc {

auto blas_shape::operator==(blas_shape const &other) const -> bool {
    return op1_ty == other.op1_ty && op2_ty == other.op2_ty && dst_ty == other.dst_ty &&
           is_gemm == other.is_gemm && shape == other.shape;
}
auto blas_shape::operator!=(blas_shape const &other) const -> bool { return !(*this == other); }

auto suggest_subgroup_size(array_view<blas_shape> const &shapes,
                           ::tinytc_core_info const &info) -> std::int32_t {
    auto const &available_subgroup_sizes = info.subgroup_sizes();
    if (available_subgroup_sizes.size() == 0) {
        throw std::out_of_range("Subgroup size vector must have at least one entry");
    }

    for (auto &shape : shapes) {
        auto const &mext = info.matrix();
        if (shape.is_gemm && mext.have_precision(shape.op1_ty, shape.op2_ty, shape.dst_ty)) {
            return mext.required_subgroup_size();
        }
    }

    auto sensible_subgroup_sizes = [&] {
        auto sgs = std::vector<std::int32_t>{};

        std::size_t max_size = 1u;
        for (auto &shape : shapes) {
            max_size = std::max(max_size, size(shape.dst_ty));
        }
        sgs.reserve(available_subgroup_sizes.size());

        auto it = available_subgroup_sizes.begin();
        sgs.push_back(*it++);
        if (max_size < 8u) { // Only consider smallest sub-group size for double precision
            auto const register_space = info.register_space();
            auto const number_of_reals_in_register = (register_space / 2) / max_size;
            auto const number_of_reals_sqrt = static_cast<std::int32_t>(
                std::sqrt(static_cast<double>(number_of_reals_in_register)));
            for (; it != available_subgroup_sizes.end(); ++it) {
                if (*it <= number_of_reals_sqrt) {
                    sgs.push_back(*it);
                }
            }
        }

        return sgs;
    }();

    if (sensible_subgroup_sizes.size() == 1) {
        return sensible_subgroup_sizes.front();
    }

    auto max_shape0 = std::max_element(
        shapes.begin(), shapes.end(), [](blas_shape const &a, blas_shape const &b) {
            auto const a0 = is_dynamic_value(a.shape[0]) ? 0 : a.shape[0];
            auto const b0 = is_dynamic_value(b.shape[0]) ? 0 : b.shape[0];
            return a0 < b0;
        });
    if (max_shape0 != shapes.end()) {
        for (auto it = sensible_subgroup_sizes.begin(); it != sensible_subgroup_sizes.end(); ++it) {
            if (max_shape0->shape[0] <= *it) {
                return *it;
            }
        }
    }
    return sensible_subgroup_sizes.back();
}

auto suggest_local_tiling(array_view<blas_shape> const &shapes,
                          core_config const &core_cfg) -> local_tiling {
    if (shapes.empty()) {
        return {1, 1};
    }

    auto max_ty_size = std::max_element(shapes.begin(), shapes.end(),
                                        [](blas_shape const &a, blas_shape const &b) {
                                            auto const a0 = size(a.dst_ty);
                                            auto const b0 = size(b.dst_ty);
                                            return a0 < b0;
                                        });

    auto const max_shapei = [](array_view<blas_shape> const &shapes, std::size_t idx) {
        auto max_it = std::max_element(
            shapes.begin(), shapes.end(), [&idx](blas_shape const &a, blas_shape const &b) {
                auto const a0 = is_dynamic_value(a.shape[idx]) ? 0 : a.shape[idx];
                auto const b0 = is_dynamic_value(b.shape[idx]) ? 0 : b.shape[idx];
                return a0 < b0;
            });
        return max_it->shape[idx];
    };

    auto M = max_shapei(shapes, 0);
    auto N = max_shapei(shapes, 1);

    return suggest_local_tiling(size(max_ty_size->dst_ty), {M, N}, core_cfg);
}

auto suggest_local_tiling(std::size_t type_size, std::array<std::int64_t, 2u> const &shape,
                          core_config const &core_cfg) -> local_tiling {
    auto [rows, cols] =
        max_register_block_gemm(type_size, core_cfg.subgroup_size, core_cfg.register_space);
    auto const num_tile_limit = [](std::int64_t mode, std::int32_t block_size) {
        auto limit = std::numeric_limits<std::int32_t>::max();
        if (!is_dynamic_value(mode)) {
            limit = 1 + (mode - 1) / block_size;
        }
        return limit;
    };
    auto const m_limit = num_tile_limit(shape[0], rows);
    auto const n_limit = num_tile_limit(shape[1], cols);

    auto const max_threads = core_cfg.max_work_group_size / core_cfg.subgroup_size;
    if (max_threads == 0) {
        return local_tiling{1, 1};
    }

    double best_ratio = 0.0;
    auto tiling = local_tiling{1, 1};
    for (std::int32_t m = 1; m <= std::min(m_limit, max_threads); m *= 2) {
        std::int32_t n = 1;
        while (2 * n <= std::min(n_limit, max_threads / m)) {
            n *= 2;
        }
        auto const LM = m * rows;
        auto const LN = n * cols;
        double const ratio = LM * LN / static_cast<double>(LM + LN);
        if (ratio > best_ratio) {
            best_ratio = ratio;
            tiling[0] = m;
            tiling[1] = n;
        }
    }

    return tiling;
}

auto suggest_subgroup_size_and_tiling(array_view<blas_shape> const &shapes,
                                      ::tinytc_core_info const &dev_info)
    -> std::tuple<std::int32_t, local_tiling> {
    auto const sgs = suggest_subgroup_size(shapes, dev_info);
    auto const core_cfg = dev_info.get_core_config(sgs);
    auto const tiling = suggest_local_tiling(shapes, core_cfg);
    return std::make_tuple(sgs, tiling);
}

} // namespace tinytc

std::size_t std::hash<tinytc::blas_shape>::operator()(tinytc::blas_shape const &x) const {
    return fnv1a_combine(x.op1_ty, x.op2_ty, x.dst_ty, x.is_gemm, x.shape[0], x.shape[1]);
}

