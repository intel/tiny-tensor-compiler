// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "linalg_ops.hpp"

#include <algorithm>
#include <array>

namespace tinytc::test {

auto gemm_mnk(transpose tA, transpose tB, tensor_layout const &A, tensor_layout const &B,
              tensor_layout const &C) -> std::array<std::int64_t, 3u> {
    if (A.dim() != 2 || B.dim() != 2 || C.dim() != 2) {
        throw std::runtime_error("expected matrices");
    }
    const int A_kmode = tA == transpose::T ? 1 : 0;
    const int B_nmode = tB == transpose::T ? 0 : 1;
    const auto M = C.shape(0);
    const auto N = C.shape(1);
    const auto K = A.shape(1 - A_kmode);
    if (M != A.shape(A_kmode) || K != B.shape(1 - B_nmode) || N != B.shape(B_nmode)) {
        throw std::runtime_error("incompatible matmul");
    }
    return {M, N, K};
}

auto make_gemm_prog(char const *name, transpose tA, transpose tB, tensor_layout const &layoutA,
                    tensor_layout const &layoutB, tensor_layout const &layoutC,
                    scalar_type alpha_ty, scalar_type A_ty, scalar_type B_ty, scalar_type beta_ty,
                    scalar_type C_ty) -> prog {
    auto ctx = make_compiler_context();

    auto const alphat = get_scalar(ctx, alpha_ty);
    auto const at = get_scalar(ctx, A_ty);
    auto const bt = get_scalar(ctx, B_ty);
    auto const betat = get_scalar(ctx, beta_ty);
    auto const ct = get_scalar(ctx, C_ty);

    auto p = make_prog(ctx);

    auto At =
        get_memref(at, layoutA.static_shape(), layoutA.static_stride(), address_space::global);
    auto Bt =
        get_memref(bt, layoutB.static_shape(), layoutB.static_stride(), address_space::global);
    auto Ct =
        get_memref(ct, layoutC.static_shape(), layoutC.static_stride(), address_space::global);

    auto f = make_func(name, {alphat, At, Bt, betat, Ct});
    auto fn_body = f.get_body();
    auto params = std::array<value, 5u>{};
    fn_body.get_parameters(params);
    params[0].set_name("alpha");
    params[1].set_name("A");
    params[2].set_name("B");
    params[3].set_name("beta");
    params[4].set_name("C");

    auto bb = region_builder{fn_body};

    bb.add(make_gemm(tA, tB, false, params[0], params[1], params[2], params[3], params[4]));

    p.add_function(std::move(f));

    return p;
}

} // namespace tinytc::test
