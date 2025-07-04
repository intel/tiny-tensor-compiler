// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tall_and_skinny.hpp"
#include "device_info.hpp"
#include "error.hpp"
#include "node/type.hpp"
#include "number.hpp"
#include "recipe.hpp"
#include "tiling.hpp"
#include "tinytc/builder.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <source_location>
#include <utility>

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
tall_and_skinny_recipe::tall_and_skinny_recipe(shared_handle<tinytc_prog_t> prg,
                                               shared_handle<tinytc_binary_t> bin, tinytc_type_t ty,
                                               std::int64_t M, std::int64_t ldA, std::int64_t ldB,
                                               std::int64_t ldC, std::int32_t M_block_size)
    : ::tinytc_recipe(std::move(prg), std::move(bin)), ty_(ty), M_dyn_(is_dynamic_value(M)),
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
                                                     tinytc_type_t ty, int64_t N, int64_t K,
                                                     int32_t M_block_size) {
    return tinytc_recipe_tall_and_skinny_create_specialized(recipe, info, ty, TINYTC_DYNAMIC, N, K,
                                                            TINYTC_DYNAMIC, TINYTC_DYNAMIC,
                                                            TINYTC_DYNAMIC, 0, 0, 0, M_block_size);
}

tinytc_status_t tinytc_recipe_tall_and_skinny_create_specialized(
    tinytc_recipe_t *recipe, const_tinytc_core_info_t info, tinytc_type_t ty, int64_t M, int64_t N,
    int64_t K, int64_t ldA, int64_t ldB, int64_t ldC, int32_t alignA, int32_t alignB,
    int32_t alignC, int32_t M_block_size) {
    if (recipe == nullptr || info == nullptr || ty == nullptr || N == TINYTC_DYNAMIC ||
        K == TINYTC_DYNAMIC) {
        return tinytc_status_invalid_arguments;
    }

    auto ctx = ty->context();
    std::int32_t source_id = 0;
    TINYTC_CHECK_STATUS(
        tinytc_compiler_context_add_source(ctx, "recipe/tall_and_skinny.cpp", "", &source_id));

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
            auto const bool_ty = get<boolean_type>(ctx);
            auto const void_ty = get<void_type>(ctx);
            auto const index_ty = get<index_type>(ctx);

            auto const bshape = blas_shape{ty, ty, ty, {M_block_size, N}, true};
            auto [sgs, tiling] = suggest_subgroup_size_and_tiling(array_view(bshape), *info);

            // We want to avoid working on too many columns in parallel as there is a high
            // chance to trash the cache due to the large pitch
            while (tiling[0] < tiling[1] && tiling[1] > 1) {
                tiling[1] /= 2;
            }

            auto const A_stride = std::array<std::int64_t, 2u>{1, ldA};
            auto const B_stride = std::array<std::int64_t, 2u>{1, ldB};
            auto const C_stride = std::array<std::int64_t, 2u>{1, ldC};

            auto const body = [&](region_builder &bb, tinytc_value_t alpha, tinytc_value_t A,
                                  tinytc_value_t B, bool is_beta_nonzero, tinytc_value_t beta_arg,
                                  tinytc_value_t C) {
                auto c_M_block_size = bb.create<constant_inst>(M_block_size, index_ty, my_loc());
                auto gid = bb.create<group_id_inst>(comp3::x, index_ty, my_loc());
                auto m = bb.create<mul_inst>(gid, c_M_block_size, get_type(gid), my_loc());
                auto beta = is_beta_nonzero ? beta_arg : bb.constant_zero(ty, my_loc());

                auto const static_offsets = std::array<std::int64_t, 2u>{dynamic, 0};
                auto const offsets = array_view<tinytc_value_t>(m);

                auto const static_gemm = [&](region_builder &bb) {
                    auto const A_static_sizes = std::array<std::int64_t, 2u>{M_block_size, K};
                    auto const C_static_sizes = std::array<std::int64_t, 2u>{M_block_size, N};
                    auto at = get<memref_type>(ty, A_static_sizes, A_stride, address_space::global);
                    auto ct = get<memref_type>(ty, C_static_sizes, C_stride, address_space::global);
                    auto a = bb.create<subview_inst>(static_offsets, A_static_sizes, A, offsets,
                                                     array_view<tinytc_value_t>{}, at, my_loc());
                    auto c = bb.create<subview_inst>(static_offsets, C_static_sizes, C, offsets,
                                                     array_view<tinytc_value_t>{}, ct, my_loc());
                    bb.create<gemm_inst>(false, transpose::N, transpose::N, alpha, a, B, beta, c,
                                         my_loc());
                };
                auto const dynamic_gemm = [&](region_builder &bb, tinytc_value_t dyn_block_size) {
                    auto const A_static_sizes = std::array<std::int64_t, 2u>{dynamic, K};
                    auto const C_static_sizes = std::array<std::int64_t, 2u>{dynamic, N};
                    auto const sizes = array_view<tinytc_value_t>(dyn_block_size);
                    auto at = get<memref_type>(ty, A_static_sizes, A_stride, address_space::global);
                    auto ct = get<memref_type>(ty, C_static_sizes, C_stride, address_space::global);
                    auto a = bb.create<subview_inst>(static_offsets, A_static_sizes, A, offsets,
                                                     sizes, at, my_loc());
                    auto c = bb.create<subview_inst>(static_offsets, C_static_sizes, C, offsets,
                                                     sizes, ct, my_loc());
                    bb.create<gemm_inst>(false, transpose::N, transpose::N, alpha, a, B, beta, c,
                                         my_loc());
                };

                if (!is_dynamic_value(M) && M % M_block_size == 0) {
                    static_gemm(bb);
                } else {

                    auto M_val = bb.create<size_inst>(0, C, index_ty, my_loc());
                    auto M_val_sub_m = bb.create<sub_inst>(M_val, m, get_type(m), my_loc());
                    auto cond =
                        bb.create<less_than_inst>(M_val_sub_m, c_M_block_size, bool_ty, my_loc());
                    bb.ifelse(
                        cond, [&](region_builder &bb) { dynamic_gemm(bb, M_val_sub_m); },
                        [&](region_builder &bb) { static_gemm(bb); }, {}, my_loc());
                }
            };

            auto const kernel = [&](char const *name, bool is_beta_nonzero) {
                auto A_shape = std::array{M, K};
                auto B_shape = std::array{K, N};
                auto C_shape = std::array{M, N};
                auto A_ty = get<memref_type>(ty, A_shape, A_stride, address_space::global);
                auto B_ty = get<memref_type>(ty, B_shape, B_stride, address_space::global);
                auto C_ty = get<memref_type>(ty, C_shape, C_stride, address_space::global);
                auto f = make_func(name, {ty, A_ty, B_ty, ty, C_ty}, void_ty, my_loc());

                auto alignments = std::array<std::pair<std::int32_t, std::int32_t>, 3u>{
                    {{1, alignA}, {2, alignB}, {4, alignC}}};
                auto align_attr = tinytc_named_attr_t{get_string_attr(ctx, "align"), nullptr};
                for (auto &[param_no, alignment] : alignments) {
                    if (alignment > 0) {
                        align_attr.attr = get_integer_attr(ctx, alignment);
                        set_parameter_attr(f.get(), param_no,
                                           get_dictionary_attr_with_sorted(ctx, align_attr));
                    }
                }

                auto fn_body = get_body(f.get());
                auto params = std::array<tinytc_value_t, 5u>{};
                get_parameters(fn_body, params);
                set_name(params[0], "alpha");
                set_name(params[1], "A");
                set_name(params[2], "B");
                set_name(params[3], "beta");
                set_name(params[4], "C");
                auto const wgs = tiling.work_group_size(sgs);
                auto const wgs_attr =
                    tinytc_named_attr_t{get_string_attr(ctx, "work_group_size"),
                                        get_array_attr(ctx, {get_integer_attr(ctx, wgs[0]),
                                                             get_integer_attr(ctx, wgs[1])})};
                set_attr(f.get(), get_dictionary_attr_with_sorted(ctx, wgs_attr));

                auto bb = region_builder{fn_body};
                body(bb, params[0], params[1], params[2], is_beta_nonzero, params[3], params[4]);
                return f;
            };

            auto p = make_prog(ctx, my_loc());
            add_function(p.get(),
                         kernel(tall_and_skinny_kernel_name(tall_and_skinny_kernel::gemm), true));
            add_function(
                p.get(),
                kernel(tall_and_skinny_kernel_name(tall_and_skinny_kernel::gemm_beta0), false));
            auto bin = compile_to_spirv_and_assemble(p.get(), info);
            *recipe = std::make_unique<tall_and_skinny_recipe>(std::move(p), std::move(bin), ty, M,
                                                               ldA, ldB, ldC, M_block_size)
                          .release();
        },
        ctx);
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
    auto recipe = dynamic_cast<tall_and_skinny_recipe const *>(handler->get_recipe());
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
