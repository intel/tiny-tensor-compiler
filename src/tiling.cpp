// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tiling.hpp"
#include "device_info.hpp"
#include "gemm_tools.hpp"
#include "tinytc/tinytc.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace tinytc {

auto blas_shape::operator==(blas_shape const &other) const -> bool {
    return ty == other.ty && shape == other.shape;
}
auto blas_shape::operator!=(blas_shape const &other) const -> bool { return !(*this == other); }

auto suggest_subgroup_size(std::vector<blas_shape> const &shapes,
                           ::tinytc_core_info const &info) -> std::int32_t {
    std::size_t max_size = 1u;
    for (auto &shape : shapes) {
        max_size = std::max(max_size, size(shape.ty));
    }

    auto const &available_subgroup_sizes = info.subgroup_sizes();
    if (available_subgroup_sizes.size() == 0) {
        throw std::out_of_range("Subgroup size vector must have at least one entry");
    }
    auto sensible_subgroup_sizes = std::vector<std::int32_t>{};
    sensible_subgroup_sizes.reserve(available_subgroup_sizes.size());

    auto it = available_subgroup_sizes.begin();
    sensible_subgroup_sizes.push_back(*it++);
    if (max_size < 8u) { // Only consider smallest sub-group size for double precision
        auto const register_space = info.register_space();
        auto const number_of_reals_in_register = (register_space / 2) / max_size;
        auto const number_of_reals_sqrt =
            static_cast<std::int32_t>(std::sqrt(static_cast<double>(number_of_reals_in_register)));
        for (; it != available_subgroup_sizes.end(); ++it) {
            if (*it <= number_of_reals_sqrt) {
                sensible_subgroup_sizes.push_back(*it);
            }
        }
    }
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

auto suggest_local_tiling(std::vector<blas_shape> const &shapes,
                          core_config const &core_cfg) -> local_tiling {
    if (shapes.empty()) {
        return {1, 1};
    }

    auto max_ty_size = std::max_element(shapes.begin(), shapes.end(),
                                        [](blas_shape const &a, blas_shape const &b) {
                                            auto const a0 = size(a.ty);
                                            auto const b0 = size(b.ty);
                                            return a0 < b0;
                                        });

    auto const max_shapei = [](std::vector<blas_shape> const &shapes, std::size_t idx) {
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

    return suggest_local_tiling(blas_shape{max_ty_size->ty, {M, N}}, core_cfg);
}

auto suggest_local_tiling(blas_shape const &bshape, core_config const &core_cfg) -> local_tiling {
    auto [rows, cols] =
        max_register_block_gemm(size(bshape.ty), core_cfg.subgroup_size, core_cfg.register_space);
    auto const num_tile_limit = [](std::int64_t mode, std::int32_t block_size) {
        auto limit = std::numeric_limits<std::int32_t>::max();
        if (!is_dynamic_value(mode)) {
            limit = 1 + (mode - 1) / block_size;
        }
        return limit;
    };
    auto const m_limit = num_tile_limit(bshape.shape[0], rows);
    auto const n_limit = num_tile_limit(bshape.shape[1], cols);

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

auto suggest_subgroup_size_and_tiling(std::vector<blas_shape> const &shapes,
                                      ::tinytc_core_info const &dev_info)
    -> std::tuple<std::int32_t, local_tiling> {
    auto const sgs = suggest_subgroup_size(shapes, dev_info);
    auto const core_cfg = dev_info.get_core_config(sgs);
    auto const tiling = suggest_local_tiling(shapes, core_cfg);
    return std::make_tuple(sgs, tiling);
}

} // namespace tinytc

std::size_t std::hash<tinytc::blas_shape>::operator()(tinytc::blas_shape const &x) const {
    constexpr std::int64_t fnv_prime = 0x100000001B3;
    constexpr std::int64_t fnv_offset = 0xCBF29CE484222325;
    auto hash = (fnv_offset ^ static_cast<std::int64_t>(x.ty)) * fnv_prime;
    hash = (hash ^ x.shape[0]) * fnv_prime;
    hash = (hash ^ x.shape[1]) * fnv_prime;
    return hash;
}

