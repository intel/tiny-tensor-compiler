// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/recipe/tall_and_skinny.hpp"

#include "tinytc/binary.hpp"
#include "tinytc/bundle_format.hpp"
#include "tinytc/ir/builder.hpp"
#include "tinytc/ir/data_type.hpp"
#include "tinytc/ir/inst.hpp"
#include "tinytc/ir/prog.hpp"
#include "tinytc/ir/scalar_type.hpp"
#include "tinytc/ir/slice.hpp"
#include "tinytc/ir/tiling.hpp"
#include "tinytc/ir/value.hpp"

#include <algorithm>
#include <array>
#include <functional>
#include <utility>

namespace tinytc::recipe {

auto generate_tall_and_skinny_binary(ir::gemm_scalar_type ty, std::uint32_t M_block_size,
                                     std::uint32_t N, std::uint32_t K,
                                     std::shared_ptr<core_info> info,
                                     ir::error_reporter_function err) -> std::shared_ptr<binary> {
    auto const shapes = std::vector{ir::blas_shape{ty.C, {M_block_size, N}}};
    auto [sgs, tiling] = ir::suggest_subgroup_size_and_tiling(shapes, *info);

    // We want to avoid working on too many columns in parallel as there is a high chance
    // to trash the cache due to the large pitch
    while (tiling[0] < tiling[1] && tiling[1] > 1) {
        tiling[1] /= 2;
    }

    auto pb = ir::program_builder{};
    try {
        auto const kernel = [&](ir::function_builder &fb, bool is_beta_nonzero) {
            auto alpha = fb.argument(ty.alpha, "alpha");
            auto A = fb.argument(ir::memref_type(ty.A, {ir::dynamic, K}, {1, ir::dynamic}), "A");
            auto B = fb.argument(ir::memref_type(ty.B, {K, N}, {1, ir::dynamic}), "B");
            auto beta_arg = fb.argument(ty.beta, "beta");
            auto C = fb.argument(ir::memref_type(ty.C, {ir::dynamic, N}, {1, ir::dynamic}), "C");
            fb.subgroup_size(sgs);
            auto const wgs = tiling.work_group_size(sgs);
            fb.work_group_size(wgs[0], wgs[1]);

            auto beta = is_beta_nonzero ? beta_arg : ir::value(0.0, ty.beta);
            fb.body([&](ir::region_builder &bb) {
                auto gid = bb.create_group_id();
                auto m = bb.create_mul(gid, ir::value(M_block_size, ir::scalar_type::index));
                auto M = bb.create_size(C, 0);
                auto M_m = bb.create_sub(M, m);
                auto cond = bb.create_cmp(ir::cmp_condition::lt, M_m,
                                          ir::value(M_block_size, ir::scalar_type::index));
                bb.create_ifelse(
                    std::move(cond),
                    [&](ir::region_builder &bb) {
                        auto a =
                            bb.create_subview(A, {ir::slice(m, ir::dynamic),
                                                  ir::slice(static_cast<std::uint32_t>(0), K)});
                        auto c =
                            bb.create_subview(C, {ir::slice(m, ir::dynamic),
                                                  ir::slice(static_cast<std::uint32_t>(0), N)});
                        bb.create_gemm(ir::transpose::N, ir::transpose::N, alpha, a, B, beta, c);
                    },
                    [&](ir::region_builder &bb) {
                        auto a =
                            bb.create_subview(A, {ir::slice(m, M_block_size),
                                                  ir::slice(static_cast<std::uint32_t>(0), K)});
                        auto c =
                            bb.create_subview(C, {ir::slice(m, M_block_size),
                                                  ir::slice(static_cast<std::uint32_t>(0), N)});
                        bb.create_gemm(ir::transpose::N, ir::transpose::N, alpha, a, B, beta, c);
                    });
            });
        };
        pb.create("gemm", [&](ir::function_builder &fb) { kernel(fb, true); });
        pb.create("gemm_beta0", [&](ir::function_builder &fb) { kernel(fb, false); });
    } catch (ir::compilation_error const &e) {
        err(e.loc(), e.what());
        return nullptr;
    }

    return optimize_and_make_binary(pb.get_product(), bundle_format::spirv, std::move(info),
                                    std::move(err));
}

} // namespace tinytc::recipe
