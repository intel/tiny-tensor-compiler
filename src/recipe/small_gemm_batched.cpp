// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "small_gemm_batched.hpp"
#include "compiler_context.hpp"
#include "error.hpp"
#include "recipe.hpp"
#include "reference_counted.hpp"
#include "support/util.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <source_location>
#include <utility>

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
tinytc_status_t tinytc_recipe_small_gemm_batched_create(
    tinytc_recipe_t *recipe, const_tinytc_core_info_t info, tinytc_scalar_type_t ty,
    tinytc_transpose_t tA, tinytc_transpose_t tB, int64_t M, int64_t N, int64_t K, int64_t ldA,
    int64_t strideA, int64_t ldB, int64_t strideB, int64_t ldC, int64_t strideC,
    tinytc_compiler_context_t ctx) {
    if (recipe == nullptr || info == nullptr || M == TINYTC_DYNAMIC || N == TINYTC_DYNAMIC ||
        K == TINYTC_DYNAMIC || ldA == TINYTC_DYNAMIC || strideA == TINYTC_DYNAMIC ||
        ldB == TINYTC_DYNAMIC || strideB == TINYTC_DYNAMIC || ldC == TINYTC_DYNAMIC ||
        strideC == TINYTC_DYNAMIC) {
        return tinytc_status_invalid_arguments;
    }

    auto ctx_ = ctx ? compiler_context{ctx} : make_compiler_context();
    std::int32_t source_id = 0;
    TINYTC_CHECK_STATUS(tinytc_compiler_context_add_source(
        ctx_.get(), "recipe/small_gemm_batched.cpp", "", &source_id));

    auto const my_loc = [&](std::source_location const loc = std::source_location::current()) {
        auto l = location{};
        l.begin.source_id = source_id;
        l.begin.line = loc.line();
        l.begin.column = loc.column();
        l.end = l.begin;
        ++l.end.column;
        return l;
    };
    auto const make_static_sizes = [](transpose t, std::int64_t A, std::int64_t B) {
        auto s = std::array<std::int64_t, 3u>{A, B, 0};
        if (t == transpose::T) {
            std::swap(s[0], s[1]);
        }
        return s;
    };

    return exception_to_status_code(
        [&] {
            auto const ty_ = get_scalar(ctx_, enum_cast<scalar_type>(ty));
            auto const tA_ = enum_cast<transpose>(tA);
            auto const tB_ = enum_cast<transpose>(tB);

            auto const kernel = [&](char const *name, bool is_beta_nonzero) {
                auto const static_offsets = std::array<std::int64_t, 3u>{0, 0, dynamic};
                auto const A_static_sizes = make_static_sizes(tA_, M, K);
                auto const B_static_sizes = make_static_sizes(tB_, K, N);
                auto const C_static_sizes = make_static_sizes(transpose::N, M, N);

                auto A_ty = get_memref(ty_, {A_static_sizes[0], A_static_sizes[1], dynamic},
                                       {1, ldA, strideA}, address_space::global, my_loc());
                auto B_ty = get_memref(ty_, {B_static_sizes[0], B_static_sizes[1], dynamic},
                                       {1, ldB, strideB}, address_space::global, my_loc());
                auto C_ty = get_memref(ty_, {M, N, dynamic}, {1, ldC, strideC},
                                       address_space::global, my_loc());
                auto f = make_func(name, {ty_, A_ty, B_ty, ty_, C_ty}, my_loc());
                auto fn_body = f.get_body();
                auto params = std::array<value, 5u>{};
                fn_body.get_parameters(params);
                params[0].set_name("alpha");
                params[1].set_name("A");
                params[2].set_name("B");
                params[3].set_name("beta");
                params[4].set_name("C");

                auto bb = region_builder{fn_body};

                auto gid = bb.add(make_group_id(ctx_, my_loc()));
                auto at =
                    get_memref(ty_, A_static_sizes, {1, ldA}, address_space::global, my_loc());
                auto bt =
                    get_memref(ty_, B_static_sizes, {1, ldB}, address_space::global, my_loc());
                auto ct =
                    get_memref(ty_, C_static_sizes, {1, ldC}, address_space::global, my_loc());
                auto a = bb.add(make_subview(params[1], static_offsets, A_static_sizes,
                                             array_view<value>{gid}, {}, at, my_loc()));
                auto b = bb.add(make_subview(params[2], static_offsets, B_static_sizes,
                                             array_view<value>{gid}, {}, bt, my_loc()));
                auto c = bb.add(make_subview(params[4], static_offsets, C_static_sizes,
                                             array_view<value>{gid}, {}, ct, my_loc()));
                auto beta = is_beta_nonzero ? params[3] : bb.add(make_constant_zero(ty_, my_loc()));
                bb.add(make_gemm(tA_, tB_, false, params[0], std::move(a), std::move(b), beta,
                                 std::move(c), my_loc()));

                return f;
            };
            auto p = make_prog(ctx_, my_loc());
            p.add_function(
                kernel(small_gemm_batched_kernel_name(small_gemm_batched_kernel::gemm), true));
            p.add_function(kernel(
                small_gemm_batched_kernel_name(small_gemm_batched_kernel::gemm_beta0), false));
            tinytc_source_t src;
            CHECK_STATUS(tinytc_prog_compile_to_opencl(&src, p.get(), info));
            *recipe = std::make_unique<small_gemm_batched_recipe>(std::move(p), source(src),
                                                                  enum_cast<scalar_type>(ty))
                          .release();
        },
        ctx_.get());
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
