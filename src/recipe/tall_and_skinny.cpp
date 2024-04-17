// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "error.hpp"
#include "tiling.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc.hpp"
#include "util.hpp"

#include <cstdint>
#include <source_location>

using namespace tinytc;

extern "C" {
tinytc_status_t tinytc_recipe_tall_and_skinny_create(tinytc_binary_t *bin, tinytc_core_info_t info,
                                                     tinytc_scalar_type_t ty, uint32_t M_block_size,
                                                     uint32_t N, uint32_t K,
                                                     tinytc_source_context_t ctx) {
    if (bin == nullptr || info == nullptr) {
        return tinytc_status_invalid_arguments;
    }

    std::int32_t source_id = 0;
    if (ctx) {
        TINYTC_CHECK(
            tinytc_source_context_add_source(ctx, "tall and skinny receipe", "", &source_id));
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

    return exception_to_status_code(
        [&] {
            auto const ty_ = enum_cast<scalar_type>(ty);

            auto const shapes = std::vector{blas_shape{ty_, {M_block_size, N}}};
            auto [sgs, tiling] = suggest_subgroup_size_and_tiling(shapes, *info);

            // We want to avoid working on too many columns in parallel as there is a high chance
            // to trash the cache due to the large pitch
            while (tiling[0] < tiling[1] && tiling[1] > 1) {
                tiling[1] /= 2;
            }

            auto const body = [&](region_builder &bb, value &alpha, value &A, value &B, value &beta,
                                  value &C) {
                auto gid = bb.add(create_group_id(my_loc()));
                auto m = bb.add(
                    create_binary_op(binary_op::mul, gid, value(M_block_size, my_loc()), my_loc()));
                auto M = bb.add(create_size(C, 0, my_loc()));
                auto M_m = bb.add(create_binary_op(binary_op::sub, M, m, my_loc()));
                auto cond = bb.add(
                    create_cmp(cmp_condition::lt, M_m, value(M_block_size, my_loc()), my_loc()));
                bb.create_ifelse(
                    cond,
                    [&](region_builder &bb) {
                        auto a = bb.add(create_subview(A, {m, 0u}, {dynamic, K}, my_loc()));
                        auto c = bb.add(create_subview(C, {m, 0u}, {dynamic, N}, my_loc()));
                        bb.add(create_gemm(transpose::N, transpose::N, false, alpha, a, B, beta, c,
                                           my_loc()));
                    },
                    [&](region_builder &bb) {
                        auto a = bb.add(create_subview(A, {m, 0u}, {M_block_size, K}, my_loc()));
                        auto c = bb.add(create_subview(C, {m, 0u}, {M_block_size, N}, my_loc()));
                        bb.add(create_gemm(transpose::N, transpose::N, false, alpha, a, B, beta, c,
                                           my_loc()));
                    },
                    {}, my_loc());
            };

            auto const kernel = [&](function_builder &fb, bool is_beta_nonzero) {
                auto alpha = fb.argument(ty_, "alpha");
                auto A = fb.argument(create_memref(ty_, {dynamic, K}, {1, dynamic}), "A");
                auto B = fb.argument(create_memref(ty_, {K, N}, {1, dynamic}), "B");
                auto beta_arg = fb.argument(ty_, "beta");
                auto C = fb.argument(create_memref(ty_, {dynamic, N}, {1, dynamic}), "C");
                fb.subgroup_size(sgs);
                auto const wgs = tiling.work_group_size(sgs);
                fb.work_group_size(wgs[0], wgs[1]);

                auto beta = is_beta_nonzero ? beta_arg : value(0.0, ty_, my_loc());
                fb.body([&](region_builder &bb) { body(bb, alpha, A, B, beta, C); }, my_loc());
            };

            auto pb = program_builder{};
            pb.create(
                "gemm", [&](function_builder &fb) { kernel(fb, true); }, my_loc());
            pb.create(
                "gemm_beta0", [&](function_builder &fb) { kernel(fb, false); }, my_loc());

            auto p = pb.get_product(my_loc());
            CHECK(
                tinytc_prog_compile_to_binary(bin, p.get(), info, tinytc_bundle_format_spirv, ctx));
        },
        ctx, my_loc());
}
}
