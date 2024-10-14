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

            auto const shapes =
                std::vector{blas_shape{enum_cast<scalar_type>(ty), {M_block_size, N}}};
            auto [sgs, tiling] = suggest_subgroup_size_and_tiling(shapes, *info);

            // We want to avoid working on too many columns in parallel as there is a high
            // chance to trash the cache due to the large pitch
            while (tiling[0] < tiling[1] && tiling[1] > 1) {
                tiling[1] /= 2;
            }

            auto const body = [&](region_builder &bb, value alpha, value A, value B,
                                  bool is_beta_nonzero, value beta_arg, value C) {
                auto c_M_block_size = bb.add(make_constant(M_block_size, index_ty, my_loc()));
                auto gid = bb.add(make_group_id(ctx_, my_loc()));
                auto m = bb.add(make_arith(arithmetic::mul, gid, c_M_block_size, my_loc()));
                auto beta = is_beta_nonzero ? beta_arg : bb.add(make_constant_zero(ty_, my_loc()));

                auto const static_offsets = std::array<std::int64_t, 2u>{dynamic, 0};
                auto const offsets = array_view<value>(m);

                auto const static_gemm = [&](region_builder &bb) {
                    auto const A_static_sizes = std::array<std::int64_t, 2u>{M_block_size, K};
                    auto const C_static_sizes = std::array<std::int64_t, 2u>{M_block_size, N};
                    auto a = bb.add(
                        make_subview(A, static_offsets, A_static_sizes, offsets, {}, my_loc()));
                    auto c = bb.add(
                        make_subview(C, static_offsets, C_static_sizes, offsets, {}, my_loc()));
                    bb.add(make_gemm(transpose::N, transpose::N, false, alpha, a, B, beta, c,
                                     my_loc()));
                };
                auto const dynamic_gemm = [&](region_builder &bb, value dyn_block_size) {
                    auto const A_static_sizes = std::array<std::int64_t, 2u>{dynamic, K};
                    auto const C_static_sizes = std::array<std::int64_t, 2u>{dynamic, N};
                    auto const sizes = array_view<value>(dyn_block_size);
                    auto a = bb.add(
                        make_subview(A, static_offsets, A_static_sizes, offsets, sizes, my_loc()));
                    auto c = bb.add(
                        make_subview(C, static_offsets, C_static_sizes, offsets, sizes, my_loc()));
                    bb.add(make_gemm(transpose::N, transpose::N, false, alpha, a, B, beta, c,
                                     my_loc()));
                };

                if (!is_dynamic_value(M) && M % M_block_size == 0) {
                    static_gemm(bb);
                } else {
                    auto M_val = is_dynamic_value(M) ? bb.add(make_size(C, 0, my_loc()))
                                                     : bb.add(make_constant(M, index_ty));
                    auto M_val_sub_m = bb.add(make_arith(arithmetic::sub, M_val, m, my_loc()));
                    auto cond =
                        bb.add(make_cmp(cmp_condition::lt, M_val_sub_m, c_M_block_size, my_loc()));
                    bb.ifelse(
                        cond, [&](region_builder &bb) { dynamic_gemm(bb, M_val_sub_m); },
                        [&](region_builder &bb) { static_gemm(bb); }, {}, my_loc());
                }
            };

            auto const kernel = [&](char const *name, bool is_beta_nonzero) {
                auto A_ty = get_memref(ty_, {M, K}, {1, ldA}, address_space::global, my_loc());
                auto B_ty = get_memref(ty_, {K, N}, {1, ldB}, address_space::global, my_loc());
                auto C_ty = get_memref(ty_, {M, N}, {1, ldC}, address_space::global, my_loc());
                auto f = make_func(name, {ty_, A_ty, B_ty, ty_, C_ty}, my_loc());
                auto fn_body = f.get_body();
                auto params = std::array<value, 5u>{};
                fn_body.get_parameters(params);
                params[0].set_name("alpha");
                params[1].set_name("A");
                params[2].set_name("B");
                params[3].set_name("beta");
                params[4].set_name("C");
                auto const wgs = tiling.work_group_size(sgs);
                f.set_work_group_size(wgs[0], wgs[1]);

                auto bb = region_builder{fn_body};
                body(bb, params[0], params[1], params[2], is_beta_nonzero, params[3], params[4]);
                return f;
            };

            auto p = make_prog(ctx_, my_loc());
            p.add_function(kernel(tall_and_skinny_kernel_name(tall_and_skinny_kernel::gemm), true));
            p.add_function(
                kernel(tall_and_skinny_kernel_name(tall_and_skinny_kernel::gemm_beta0), false));
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
