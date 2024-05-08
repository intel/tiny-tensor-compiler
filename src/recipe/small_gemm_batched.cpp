// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "small_gemm_batched.hpp"
#include "error.hpp"
#include "parser.hpp"
#include "recipe.hpp"
#include "reference_counted.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"
#include "util.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <source_location>
#include <utility>
#include <vector>

namespace tinytc {

auto small_gemm_batched_kernel_name(small_gemm_batched_kernel k) -> char const * {
    switch (k) {
    case small_gemm_batched_kernel::gemm:
        return "gemm";
    case small_gemm_batched_kernel::gemm_beta0:
        return "gemm_beta0";
    case small_gemm_batched_kernel::num_kernels:
        break;
    }
    throw status::invalid_arguments;
}
small_gemm_batched_recipe::small_gemm_batched_recipe(prog prg, source src, scalar_type ty)
    : ::tinytc_recipe(std::move(prg), std::move(src)), ty_(ty) {}
auto small_gemm_batched_recipe::num_kernels() const -> int {
    return static_cast<int>(small_gemm_batched_kernel::num_kernels);
}
auto small_gemm_batched_recipe::kernel_name(int kernel_num) const -> char const * {
    return small_gemm_batched_kernel_name(static_cast<small_gemm_batched_kernel>(kernel_num));
}

} // namespace tinytc

using namespace tinytc;

extern "C" {
tinytc_status_t
tinytc_recipe_small_gemm_batched_create(tinytc_recipe_t *recipe, const_tinytc_core_info_t info,
                                        tinytc_scalar_type_t ty, tinytc_transpose_t tA,
                                        tinytc_transpose_t tB, int64_t M, int64_t N, int64_t K,
                                        int64_t ldA, int64_t strideA, int64_t ldB, int64_t strideB,
                                        int64_t ldC, int64_t strideC, tinytc_source_context_t ctx) {
    if (recipe == nullptr || info == nullptr || M == TINYTC_DYNAMIC || N == TINYTC_DYNAMIC ||
        K == TINYTC_DYNAMIC || ldA == TINYTC_DYNAMIC || strideA == TINYTC_DYNAMIC ||
        ldB == TINYTC_DYNAMIC || strideB == TINYTC_DYNAMIC || ldC == TINYTC_DYNAMIC ||
        strideC == TINYTC_DYNAMIC) {
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

    auto const selA = [&](std::int64_t N1, std::int64_t N2) {
        return tA == tinytc_transpose_T ? N2 : N1;
    };
    auto const selB = [&](std::int64_t N1, std::int64_t N2) {
        return tB == tinytc_transpose_T ? N2 : N1;
    };
    return exception_to_status_code(
        [&] {
            auto const ty_ = enum_cast<scalar_type>(ty);
            auto const tA_ = enum_cast<transpose>(tA);
            auto const tB_ = enum_cast<transpose>(tB);

            auto const kernel = [&](function_builder &fb, bool is_beta_nonzero) {
                auto alpha = fb.argument(make_scalar(ty_, my_loc()), "alpha", my_loc());
                auto A = fb.argument(make_memref(ty_, {selA(M, K), selA(K, M), dynamic},
                                                 {1, ldA, strideA}, my_loc()),
                                     "A", my_loc());
                auto B = fb.argument(make_memref(ty_, {selB(K, N), selB(N, K), dynamic},
                                                 {1, ldB, strideB}, my_loc()),
                                     "B", my_loc());
                auto beta_arg = fb.argument(make_scalar(ty_, my_loc()), "beta", my_loc());
                auto C = fb.argument(make_memref(ty_, {M, N, dynamic}, {1, ldC, strideC}, my_loc()),
                                     "C", my_loc());

                auto beta = is_beta_nonzero ? std::move(beta_arg) : make_imm(0.0, ty_, my_loc());
                fb.body(
                    [&](region_builder &bb) {
                        auto gid = bb.add(make_group_id(my_loc()));
                        auto offsets = std::vector<value>{make_index(0, my_loc()),
                                                          make_index(0, my_loc()), gid};
                        auto size = std::vector<value>{make_dynamic(my_loc()),
                                                       make_dynamic(my_loc()), value{}};
                        auto a = bb.add(make_subview(A, offsets, size, my_loc()));
                        auto b = bb.add(make_subview(B, offsets, size, my_loc()));
                        auto c = bb.add(make_subview(C, offsets, size, my_loc()));
                        bb.add(make_gemm(tA_, tB_, false, alpha, std::move(a), std::move(b), beta,
                                         std::move(c), my_loc()));
                    },
                    my_loc());
            };
            auto pb = program_builder{};
            pb.create(
                small_gemm_batched_kernel_name(small_gemm_batched_kernel::gemm),
                [&](function_builder &fb) { kernel(fb, true); }, my_loc());
            pb.create(
                small_gemm_batched_kernel_name(small_gemm_batched_kernel::gemm_beta0),
                [&](function_builder &fb) { kernel(fb, false); }, my_loc());
            auto p = pb.get_product(my_loc());
            tinytc_source_t src;
            CHECK_STATUS(tinytc_prog_compile_to_opencl(&src, p.get(), info, ctx));
            *recipe = std::make_unique<small_gemm_batched_recipe>(std::move(p), source(src), ty_)
                          .release();
        },
        ctx);
}

tinytc_status_t tinytc_recipe_small_gemm_batched_set_args(
    tinytc_recipe_handler_t handler, int64_t howmany, size_t alpha_size, const void *alpha_value,
    tinytc_mem_type_t A_type, const void *A_value, tinytc_mem_type_t B_type, const void *B_value,
    size_t beta_size, const void *beta_value, tinytc_mem_type_t C_type, const void *C_value) {
    if (handler == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    auto recipe = dynamic_cast<small_gemm_batched_recipe const *>(handler->get_recipe().get());
    if (recipe == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return tinytc::exception_to_status_code([&] {
        auto scalar_size = size(recipe->ty());
        if (scalar_size != alpha_size || scalar_size != beta_size) {
            throw status::invalid_kernel_arguments;
        }
        if (tinytc::is_argument_zero(recipe->ty(), beta_size, beta_value)) {
            handler->active_kernel(
                static_cast<std::uint32_t>(small_gemm_batched_kernel::gemm_beta0));
        } else {
            handler->active_kernel(static_cast<std::uint32_t>(small_gemm_batched_kernel::gemm));
        }
        handler->arg(0, alpha_size, alpha_value);
        handler->mem_arg(1, A_value, A_type);
        handler->arg(2, sizeof(int64_t), &howmany);
        handler->mem_arg(3, B_value, B_type);
        handler->arg(4, sizeof(int64_t), &howmany);
        handler->arg(5, beta_size, beta_value);
        handler->mem_arg(6, C_value, C_type);
        handler->arg(7, sizeof(int64_t), &howmany);
        handler->howmany(howmany);
    });
}
}
