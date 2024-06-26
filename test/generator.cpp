// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "device_info.hpp"
#include "gemm_generator.hpp"
#include "reference_counted.hpp"
#include "tiling.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <doctest/doctest.h>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>

using namespace tinytc;

TEST_CASE("suggest work group size") {
    auto info = make_core_info_intel_from_arch(intel_gpu_architecture::pvc);
    info.set_core_features(tinytc_core_feature_flag_large_register_file);
    REQUIRE(info->register_space() == 64 * 256);
    auto check = [&info](std::int64_t M, std::int64_t N, std::size_t sgs, std::size_t m_tiles,
                         std::size_t n_tiles) {
        auto const core_cfg = info->get_core_config(sgs);
        auto const shape = blas_shape{scalar_type::f64, {M, N}};
        auto tiling = suggest_local_tiling(shape, core_cfg);
        CHECK(tiling.m_tiles() == m_tiles);
        CHECK(tiling.n_tiles() == n_tiles);
        CHECK(tiling.number_of_work_items(sgs) <= core_cfg.max_work_group_size);
    };

    check(1, 1, 16, 1, 1);
    check(16, 32, 16, 1, 2);
    check(84, 56, 32, 2, 2);
    check(128, 128, 32, 4, 4);
    check(256, 128, 32, 8, 4);
    check(256, 256, 32, 4, 8);
    check(512, 512, 32, 4, 8);
    check(16123, 9, 32, 32, 1);
    check(461, 283, 32, 4, 8);
    check(dynamic, dynamic, 16, 4, 8);
}

TEST_CASE("routine names") {
    auto cfg = gemm_configuration{gemm_scalar_type{scalar_type::f32, scalar_type::f64},
                                  transpose::N,
                                  transpose::T,
                                  16,
                                  32,
                                  48,
                                  {1, 20},
                                  {1, 40},
                                  {1, 50},
                                  3.14,
                                  std::nullopt};
    CHECK(cfg.identifier("gemm") == "gemm_f32f32f32f64f64_An_Bt_M16_N32_K48_Astride1_20_Bstride1_"
                                    "40_Cstride1_50_alpha40091eb851eb851f_betad");
}

TEST_CASE("max register block") {
    auto s1 = max_register_block_gemm(4, 16, 8192);
    CHECK(s1.first == 2);
    CHECK(s1.second == 19);
    auto s2 = max_register_block_gemm(4, 16, 16384);
    CHECK(s2.first == 2);
    CHECK(s2.second == 44);
    auto s3 = max_register_block_gemm(4, 32, 8192);
    CHECK(s3.first == 1);
    CHECK(s3.second == 19);
    auto s4 = max_register_block_gemm(4, 32, 16384);
    CHECK(s4.first == 1);
    CHECK(s4.second == 44);
    auto d1 = max_register_block_gemm(8, 16, 8192);
    CHECK(d1.first == 1);
    CHECK(d1.second == 16);
    auto d2 = max_register_block_gemm(8, 16, 16384);
    CHECK(d2.first == 2);
    CHECK(d2.second == 19);
}
