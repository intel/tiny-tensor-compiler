// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tall_and_skinny.hpp"
#include "device_info.hpp"
#include "error.hpp"
#include "parser.hpp"
#include "recipe.hpp"
#include "reference_counted.hpp"
#include "tiling.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/types.h"
#include "util.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <source_location>
#include <utility>
#include <vector>

namespace tinytc {

auto tall_and_skinny_kernel_name(tall_and_skinny_kernel k) -> char const * {
    switch (k) {
    case tall_and_skinny_kernel::gemm:
        return "gemm";
    case tall_and_skinny_kernel::gemm_beta0:
        return "gemm_beta0";
    case tall_and_skinny_kernel::num_kernels:
        break;
    }
    throw status::invalid_arguments;
}
tall_and_skinny_recipe::tall_and_skinny_recipe(prog prg, binary bin, scalar_type ty,
                                               std::uint32_t M_block_size)
    : ::tinytc_recipe(std::move(prg), std::move(bin)), ty_(ty), M_block_size_(M_block_size) {}
auto tall_and_skinny_recipe::num_kernels() const -> std::uint32_t {
    return static_cast<std::uint32_t>(tall_and_skinny_kernel::num_kernels);
}
auto tall_and_skinny_recipe::kernel_name(std::uint32_t kernel_num) const -> char const * {
    return tall_and_skinny_kernel_name(static_cast<tall_and_skinny_kernel>(kernel_num));
}

} // namespace tinytc

using namespace tinytc;

extern "C" {
tinytc_status_t tinytc_recipe_tall_and_skinny_create(tinytc_recipe_t *recipe,
                                                     const_tinytc_core_info_t info,
                                                     tinytc_scalar_type_t ty, uint32_t N,
                                                     uint32_t K, uint32_t M_block_size,
                                                     tinytc_source_context_t ctx) {
    if (recipe == nullptr || info == nullptr) {
        return tinytc_status_invalid_arguments;
    }

    std::int32_t source_id = 0;
    if (ctx) {
        TINYTC_CHECK_STATUS(
            tinytc_source_context_add_source(ctx, "tall and skinny recipe", "", &source_id));
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

    if (M_block_size == 0u) {
        TINYTC_CHECK_STATUS(tinytc_recipe_tall_and_skinny_suggest_block_size(info, &M_block_size));
    }

    return exception_to_status_code(
        [&] {
            auto const ty_ = enum_cast<scalar_type>(ty);

            auto const shapes = std::vector{blas_shape{ty_, {M_block_size, N}}};
            auto [sgs, tiling] = suggest_subgroup_size_and_tiling(shapes, *info);

            // We want to avoid working on too many columns in parallel as there is a high
            // chance to trash the cache due to the large pitch
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
                auto alpha = fb.argument(ty_, "alpha", my_loc());
                auto A = fb.argument(create_memref(ty_, {dynamic, K}, {1, dynamic}, my_loc()), "A",
                                     my_loc());
                auto B =
                    fb.argument(create_memref(ty_, {K, N}, {1, dynamic}, my_loc()), "B", my_loc());
                auto beta_arg = fb.argument(ty_, "beta", my_loc());
                auto C = fb.argument(create_memref(ty_, {dynamic, N}, {1, dynamic}, my_loc()), "C",
                                     my_loc());
                fb.subgroup_size(sgs);
                auto const wgs = tiling.work_group_size(sgs);
                fb.work_group_size(wgs[0], wgs[1]);

                auto beta = is_beta_nonzero ? beta_arg : value(0.0, ty_, my_loc());
                fb.body([&](region_builder &bb) { body(bb, alpha, A, B, beta, C); }, my_loc());
            };

            auto pb = program_builder{};
            pb.create(
                tall_and_skinny_kernel_name(tall_and_skinny_kernel::gemm),
                [&](function_builder &fb) { kernel(fb, true); }, my_loc());
            pb.create(
                tall_and_skinny_kernel_name(tall_and_skinny_kernel::gemm_beta0),
                [&](function_builder &fb) { kernel(fb, false); }, my_loc());

            auto p = pb.get_product(my_loc());
            tinytc_binary_t bin;
            CHECK_STATUS(tinytc_prog_compile_to_binary(&bin, p.get(), info,
                                                       tinytc_bundle_format_native, ctx));
            *recipe = std::make_unique<tall_and_skinny_recipe>(std::move(p), binary(bin), ty_,
                                                               M_block_size)
                          .release();
        },
        ctx, my_loc());
}

tinytc_status_t tinytc_recipe_tall_and_skinny_suggest_block_size(const_tinytc_core_info_t info,
                                                                 uint32_t *M_block_size) {
    if (info == nullptr || M_block_size == nullptr) {
        return tinytc_status_invalid_arguments;
    }

    return tinytc::exception_to_status_code(
        [&] { *M_block_size = std::min(128u, info->minmax_number_of_work_items()); });
}

tinytc_status_t tinytc_recipe_tall_and_skinny_set_args(tinytc_recipe_handler_t handler, uint32_t M,
                                                       size_t alpha_size, const void *alpha_value,
                                                       tinytc_mem_t A, uint32_t ldA, tinytc_mem_t B,
                                                       uint32_t ldB, size_t beta_size,
                                                       const void *beta_value, tinytc_mem_t C,
                                                       uint32_t ldC) {
    if (handler == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    auto recipe = dynamic_cast<tall_and_skinny_recipe const *>(handler->get_recipe().get());
    if (recipe == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return tinytc::exception_to_status_code([&] {
        if (tinytc::is_argument_zero(recipe->ty(), beta_size, beta_value)) {
            handler->active_kernel(static_cast<std::uint32_t>(tall_and_skinny_kernel::gemm_beta0));
        } else {
            handler->active_kernel(static_cast<std::uint32_t>(tall_and_skinny_kernel::gemm));
        }
        handler->arg(0, alpha_size, alpha_value);
        handler->mem_arg(1, A);
        handler->arg(2, sizeof(uint32_t), &M);
        handler->arg(3, sizeof(uint32_t), &ldA);
        handler->mem_arg(4, B);
        handler->arg(5, sizeof(uint32_t), &ldB);
        handler->arg(6, beta_size, beta_value);
        handler->mem_arg(7, C);
        handler->arg(8, sizeof(uint32_t), &M);
        handler->arg(9, sizeof(uint32_t), &ldC);

        std::uint32_t howmany = 1 + (M - 1) / recipe->M_block_size();
        handler->howmany(howmany);
    });
}
}
