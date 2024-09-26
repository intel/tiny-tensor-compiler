// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tall_and_skinny.hpp"
#include "compiler_context.hpp"
#include "device_info.hpp"
#include "error.hpp"
#include "recipe.hpp"
#include "reference_counted.hpp"
#include "support/util.hpp"
#include "tiling.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/types.h"

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
tall_and_skinny_recipe::tall_and_skinny_recipe(prog prg, source src, scalar_type ty, std::int64_t M,
                                               std::int64_t ldA, std::int64_t ldB, std::int64_t ldC,
                                               std::int32_t M_block_size)
    : ::tinytc_recipe(std::move(prg), std::move(src)), ty_(ty), M_dyn_(is_dynamic_value(M)),
      ldA_dyn_(is_dynamic_value(ldA)), ldB_dyn_(is_dynamic_value(ldB)),
      ldC_dyn_(is_dynamic_value(ldC)), M_block_size_(M_block_size) {}
auto tall_and_skinny_recipe::num_kernels() const -> int {
    return static_cast<int>(tall_and_skinny_kernel::num_kernels);
}
auto tall_and_skinny_recipe::kernel_name(int kernel_num) const -> char const * {
    return tall_and_skinny_kernel_name(static_cast<tall_and_skinny_kernel>(kernel_num));
}

} // namespace tinytc

using namespace tinytc;

extern "C" {
tinytc_status_t tinytc_recipe_tall_and_skinny_create(tinytc_recipe_t *recipe,
                                                     const_tinytc_core_info_t info,
                                                     tinytc_scalar_type_t ty, int64_t N, int64_t K,
                                                     int32_t M_block_size,
                                                     tinytc_compiler_context_t ctx) {
    return tinytc_recipe_tall_and_skinny_create_specialized(recipe, info, ty, TINYTC_DYNAMIC, N, K,
                                                            TINYTC_DYNAMIC, TINYTC_DYNAMIC,
                                                            TINYTC_DYNAMIC, M_block_size, ctx);
}

tinytc_status_t tinytc_recipe_tall_and_skinny_create_specialized(
    tinytc_recipe_t *recipe, const_tinytc_core_info_t info, tinytc_scalar_type_t ty, int64_t M,
    int64_t N, int64_t K, int64_t ldA, int64_t ldB, int64_t ldC, int32_t M_block_size,
    tinytc_compiler_context_t ctx) {
    if (recipe == nullptr || info == nullptr || N == TINYTC_DYNAMIC || K == TINYTC_DYNAMIC) {
        return tinytc_status_invalid_arguments;
    }

    auto ctx_ = ctx ? compiler_context{ctx} : make_compiler_context();
    std::int32_t source_id = 0;
    TINYTC_CHECK_STATUS(tinytc_compiler_context_add_source(ctx_.get(), "recipe/tall_and_skinny.cpp",
                                                           "", &source_id));

    auto const my_loc = [&](std::source_location const loc = std::source_location::current()) {
        auto l = location{};
        l.begin.source_id = source_id;
        l.begin.line = loc.line();
        l.begin.column = loc.column();
        l.end = l.begin;
        ++l.end.column;
        return l;
    };

    if (M_block_size == 0) {
        TINYTC_CHECK_STATUS(tinytc_recipe_tall_and_skinny_suggest_block_size(info, &M_block_size));
    }

    return exception_to_status_code(
        [&] {
            auto const ty_ = get_scalar(ctx_, enum_cast<scalar_type>(ty));
            auto const index_ty = get_scalar(ctx_, scalar_type::index);
            auto const i64_ty = get_scalar(ctx_, scalar_type::i64);

            auto const shapes =
                std::vector{blas_shape{enum_cast<scalar_type>(ty), {M_block_size, N}}};
            auto [sgs, tiling] = suggest_subgroup_size_and_tiling(shapes, *info);

            // We want to avoid working on too many columns in parallel as there is a high
            // chance to trash the cache due to the large pitch
            while (tiling[0] < tiling[1] && tiling[1] > 1) {
                tiling[1] /= 2;
            }

            auto const body = [&](region_builder &bb, value &alpha, value &A, value &B, value &beta,
                                  value &C) {
                auto const gemm = [&](region_builder &bb, std::vector<value> const &offsets,
                                      value const &block_size) {
                    auto a = bb.add(make_subview(
                        A, offsets, {block_size, make_imm(K, index_ty, my_loc())}, my_loc()));
                    auto c = bb.add(make_subview(
                        C, offsets, {block_size, make_imm(N, index_ty, my_loc())}, my_loc()));
                    bb.add(make_gemm(transpose::N, transpose::N, false, alpha, a, B, beta, c,
                                     my_loc()));
                };

                auto const block_size_imm = make_imm(M_block_size, index_ty, my_loc());
                auto gid = bb.add(make_group_id(ctx_, my_loc()));
                auto m = bb.add(make_arith(arithmetic::mul, gid,
                                           make_imm(M_block_size, index_ty, my_loc()), my_loc()));
                auto const offsets = std::vector<value>{m, make_imm(0, index_ty, my_loc())};

                if (!is_dynamic_value(M) && M % M_block_size == 0) {
                    gemm(bb, offsets, block_size_imm);
                } else {
                    auto M_val = is_dynamic_value(M) ? bb.add(make_size(C, 0, my_loc()))
                                                     : make_imm(M, index_ty);
                    auto M_val_sub_m = bb.add(make_arith(arithmetic::sub, M_val, m, my_loc()));
                    auto cond =
                        bb.add(make_cmp(cmp_condition::lt, M_val_sub_m,
                                        make_imm(M_block_size, index_ty, my_loc()), my_loc()));
                    auto const dynamic_imm = make_imm(dynamic, i64_ty, my_loc());
                    bb.ifelse(
                        cond, [&](region_builder &bb) { gemm(bb, offsets, dynamic_imm); },
                        [&](region_builder &bb) { gemm(bb, offsets, block_size_imm); }, {},
                        my_loc());
                }
            };

            auto const kernel = [&](function_builder &fb, bool is_beta_nonzero) {
                auto alpha = fb.argument(ty_, "alpha", my_loc());
                auto A = fb.argument(get_memref(ctx_, enum_cast<scalar_type>(ty), {M, K}, {1, ldA},
                                                address_space::global, my_loc()),
                                     "A", my_loc());
                auto B = fb.argument(get_memref(ctx_, enum_cast<scalar_type>(ty), {K, N}, {1, ldB},
                                                address_space::global, my_loc()),
                                     "B", my_loc());
                auto beta_arg = fb.argument(ty_, "beta", my_loc());
                auto C = fb.argument(get_memref(ctx_, enum_cast<scalar_type>(ty), {M, N}, {1, ldC},
                                                address_space::global, my_loc()),
                                     "C", my_loc());
                fb.subgroup_size(sgs);
                auto const wgs = tiling.work_group_size(sgs);
                fb.work_group_size(wgs[0], wgs[1]);

                auto beta = is_beta_nonzero ? beta_arg : make_fimm(0.0, ty_, my_loc());
                fb.body([&](region_builder &bb) { body(bb, alpha, A, B, beta, C); }, my_loc());
            };

            auto p = [&] {
                auto pb = program_builder{ctx_, my_loc()};
                pb.create(
                    tall_and_skinny_kernel_name(tall_and_skinny_kernel::gemm),
                    [&](function_builder &fb) { kernel(fb, true); }, my_loc());
                pb.create(
                    tall_and_skinny_kernel_name(tall_and_skinny_kernel::gemm_beta0),
                    [&](function_builder &fb) { kernel(fb, false); }, my_loc());
                return std::move(pb).get_product();
            }();
            tinytc_source_t src;
            CHECK_STATUS(tinytc_prog_compile_to_opencl(&src, p.get(), info));
            *recipe = std::make_unique<tall_and_skinny_recipe>(std::move(p), source(src),
                                                               enum_cast<scalar_type>(ty), M, ldA,
                                                               ldB, ldC, M_block_size)
                          .release();
        },
        ctx_.get());
}

tinytc_status_t tinytc_recipe_tall_and_skinny_suggest_block_size(const_tinytc_core_info_t info,
                                                                 int32_t *M_block_size) {
    if (info == nullptr || M_block_size == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return tinytc::exception_to_status_code([&] {
        if (info->minmax_work_group_size() <= 0) {
            throw tinytc::status::invalid_core_info;
        }
        *M_block_size = std::min(128, info->minmax_work_group_size());
    });
}

tinytc_status_t tinytc_recipe_tall_and_skinny_set_args(
    tinytc_recipe_handler_t handler, int64_t M, size_t alpha_size, const void *alpha_value,
    tinytc_mem_type_t A_type, const void *A_value, int64_t ldA, tinytc_mem_type_t B_type,
    const void *B_value, int64_t ldB, size_t beta_size, const void *beta_value,
    tinytc_mem_type_t C_type, const void *C_value, int64_t ldC) {
    if (handler == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    auto recipe = dynamic_cast<tall_and_skinny_recipe const *>(handler->get_recipe().get());
    if (recipe == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return tinytc::exception_to_status_code([&] {
        auto scalar_size = size(recipe->ty());
        if (scalar_size != alpha_size || scalar_size != beta_size) {
            throw status::invalid_kernel_arguments;
        }
        if (tinytc::is_argument_zero(recipe->ty(), beta_size, beta_value)) {
            handler->active_kernel(static_cast<std::uint32_t>(tall_and_skinny_kernel::gemm_beta0));
        } else {
            handler->active_kernel(static_cast<std::uint32_t>(tall_and_skinny_kernel::gemm));
        }
        std::uint32_t arg_index = 0;
        handler->arg(arg_index++, alpha_size, alpha_value);
        handler->mem_arg(arg_index++, A_value, A_type);
        if (recipe->is_M_dynamic()) {
            handler->arg(arg_index++, sizeof(int64_t), &M);
        }
        if (recipe->is_ldA_dynamic()) {
            handler->arg(arg_index++, sizeof(int64_t), &ldA);
        }
        handler->mem_arg(arg_index++, B_value, B_type);
        if (recipe->is_ldB_dynamic()) {
            handler->arg(arg_index++, sizeof(int64_t), &ldB);
        }
        handler->arg(arg_index++, beta_size, beta_value);
        handler->mem_arg(arg_index++, C_value, C_type);
        if (recipe->is_M_dynamic()) {
            handler->arg(arg_index++, sizeof(int64_t), &M);
        }
        if (recipe->is_ldC_dynamic()) {
            handler->arg(arg_index++, sizeof(int64_t), &ldC);
        }

        std::int64_t howmany = 1 + (M - 1) / recipe->M_block_size();
        handler->howmany(howmany);
    });
}
}
