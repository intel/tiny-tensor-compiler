// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/recipe/small_gemm_batched.hpp"

#include "tinytc/binary.hpp"
#include "tinytc/bundle_format.hpp"
#include "tinytc/ir/builder.hpp"
#include "tinytc/ir/data_type.hpp"
#include "tinytc/ir/inst.hpp"
#include "tinytc/ir/prog.hpp"
#include "tinytc/ir/slice.hpp"
#include "tinytc/ir/value.hpp"

#include <algorithm>
#include <functional>
#include <utility>

namespace tinytc::recipe {

auto generate_small_gemm_batched_binary(gemm_scalar_type ty, transpose tA, transpose tB,
                                        std::uint32_t M, std::uint32_t N, std::uint32_t K,
                                        std::uint32_t ldA, std::uint32_t strideA, std::uint32_t ldB,
                                        std::uint32_t strideB, std::uint32_t ldC,
                                        std::uint32_t strideC, std::shared_ptr<core_info> info,
                                        error_reporter_function err) -> std::shared_ptr<binary> {
    auto const selA = [&](std::uint32_t N1, std::uint32_t N2) {
        return tA == transpose::T ? N2 : N1;
    };
    auto const selB = [&](std::uint32_t N1, std::uint32_t N2) {
        return tB == transpose::T ? N2 : N1;
    };
    auto pb = program_builder{};
    try {
        auto const kernel = [&](function_builder &fb, bool is_beta_nonzero) {
            auto alpha = fb.argument(ty.alpha, "alpha");
            auto A = fb.argument(
                memref_type(ty.A, {selA(M, K), selA(K, M), dynamic}, {1, ldA, strideA}), "A");
            auto B = fb.argument(
                memref_type(ty.B, {selB(K, N), selB(N, K), dynamic}, {1, ldB, strideB}), "B");
            auto beta_arg = fb.argument(ty.beta, "beta");
            auto C = fb.argument(memref_type(ty.C, {M, N, dynamic}, {1, ldC, strideC}), "C");

            auto beta = is_beta_nonzero ? std::move(beta_arg) : value(0.0, ty.beta);
            fb.body([&](region_builder &bb) {
                auto gid = bb.create_group_id();
                auto all = slice(static_cast<std::int64_t>(0), dynamic);
                auto a = bb.create_subview(A, {all, all, gid});
                auto b = bb.create_subview(B, {all, all, gid});
                auto c = bb.create_subview(C, {all, all, std::move(gid)});
                bb.create_gemm(tA, tB, alpha, std::move(a), std::move(b), beta, std::move(c));
            });
        };
        pb.create("gemm", [&](function_builder &fb) { kernel(fb, true); });
        pb.create("gemm_beta0", [&](function_builder &fb) { kernel(fb, false); });
    } catch (compilation_error const &e) {
        err(e.loc(), e.what());
        return nullptr;
    }

    return optimize_and_make_binary(pb.get_product(), bundle_format::spirv, std::move(info),
                                    std::move(err));
}

} // namespace tinytc::recipe
