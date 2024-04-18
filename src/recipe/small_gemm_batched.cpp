// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "error.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc.hpp"
#include "util.hpp"

#include <cstdint>
#include <source_location>

using namespace tinytc;

extern "C" {
tinytc_status_t tinytc_recipe_small_gemm_batched_create(
    tinytc_prog_t *prg, tinytc_core_info_t info, tinytc_scalar_type_t ty, tinytc_transpose_t tA,
    tinytc_transpose_t tB, uint32_t M, uint32_t N, uint32_t K, uint32_t ldA, uint32_t strideA,
    uint32_t ldB, uint32_t strideB, uint32_t ldC, uint32_t strideC, tinytc_source_context_t ctx) {
    if (prg == nullptr || info == nullptr) {
        return tinytc_status_invalid_arguments;
    }

    std::int32_t source_id = 0;
    if (ctx) {
        TINYTC_CHECK_STATUS(
            tinytc_source_context_add_source(ctx, "small gemm batched recipe", "", &source_id));
    }

    auto const my_loc = [&](std::source_location const loc = std::source_location::current()) {
        auto l = location{};
        l.begin.source_id = source_id;
        l.begin.line = loc.line();
        l.begin.column = loc.column();
        l.end = l.begin;
        ++l.end.column;
        return l;
    };

    auto const selA = [&](std::uint32_t N1, std::uint32_t N2) {
        return tA == tinytc_transpose_T ? N2 : N1;
    };
    auto const selB = [&](std::uint32_t N1, std::uint32_t N2) {
        return tB == tinytc_transpose_T ? N2 : N1;
    };
    return exception_to_status_code(
        [&] {
            auto const ty_ = enum_cast<scalar_type>(ty);
            auto const tA_ = enum_cast<transpose>(tA);
            auto const tB_ = enum_cast<transpose>(tB);

            auto const kernel = [&](function_builder &fb, bool is_beta_nonzero) {
                auto alpha = fb.argument(ty_, "alpha", my_loc());
                auto A = fb.argument(create_memref(ty_, {selA(M, K), selA(K, M), dynamic},
                                                   {1, ldA, strideA}, my_loc()),
                                     "A", my_loc());
                auto B = fb.argument(create_memref(ty_, {selB(K, N), selB(N, K), dynamic},
                                                   {1, ldB, strideB}, my_loc()),
                                     "B", my_loc());
                auto beta_arg = fb.argument(ty_, "beta", my_loc());
                auto C =
                    fb.argument(create_memref(ty_, {M, N, dynamic}, {1, ldC, strideC}, my_loc()),
                                "C", my_loc());

                auto beta = is_beta_nonzero ? std::move(beta_arg) : value(0.0, ty_, my_loc());
                fb.body(
                    [&](region_builder &bb) {
                        auto gid = bb.add(create_group_id(my_loc()));
                        auto offsets = std::vector<value>{0, 0, gid};
                        auto size = std::vector<value>{dynamic, dynamic, nullptr};
                        auto a = bb.add(create_subview(A, offsets, size, my_loc()));
                        auto b = bb.add(create_subview(B, offsets, size, my_loc()));
                        auto c = bb.add(create_subview(C, offsets, size, my_loc()));
                        bb.add(create_gemm(tA_, tB_, false, alpha, std::move(a), std::move(b), beta,
                                           std::move(c), my_loc()));
                    },
                    my_loc());
            };
            auto pb = program_builder{};
            pb.create(
                "gemm", [&](function_builder &fb) { kernel(fb, true); }, my_loc());
            pb.create(
                "gemm_beta0", [&](function_builder &fb) { kernel(fb, false); }, my_loc());
            auto p = pb.get_product(my_loc());
            *prg = p.release();
        },
        ctx, my_loc());
}
}
