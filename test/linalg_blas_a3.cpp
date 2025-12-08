// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "linalg_blas_a3.hpp"
#include "tinytc/types.hpp"

#include <array>
#include <stdexcept>

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

auto gemv_mk(transpose tA, tensor_layout const &A, tensor_layout const &B, tensor_layout const &C)
    -> std::array<std::int64_t, 2u> {
    if (A.dim() != 2 || B.dim() != 1 || C.dim() != 1) {
        throw std::runtime_error("expected vectors and matrix");
    }
    const int A_kmode = tA == transpose::T ? 1 : 0;
    const auto M = C.shape(0);
    const auto K = A.shape(1 - A_kmode);
    if (M != A.shape(A_kmode) || K != B.shape(0)) {
        throw std::runtime_error("incompatible matvec");
    }
    return {M, K};
}

auto ger_mn(tensor_layout const &A, tensor_layout const &B, tensor_layout const &C)
    -> std::array<std::int64_t, 2u> {
    if (A.dim() != 1 || B.dim() != 1 || C.dim() != 2) {
        throw std::runtime_error("expected vectors and matrix");
    }
    const auto M = C.shape(0);
    const auto N = C.shape(1);
    if (M != A.shape(0) || N != B.shape(0)) {
        throw std::runtime_error("incompatible ger");
    }
    return {M, N};
}

auto hadamard_m(tensor_layout const &A, tensor_layout const &B, tensor_layout const &C)
    -> std::int64_t {
    if (A.dim() != 1 || B.dim() != 1 || C.dim() != 1) {
        throw std::runtime_error("expected vectors");
    }
    const auto M = C.shape(0);
    if (M != A.shape(0) || M != B.shape(0)) {
        throw std::runtime_error("incompatible hadamard");
    }
    return M;
}

auto hadamard_mn(tensor_layout const &A, tensor_layout const &B, tensor_layout const &C)
    -> std::array<std::int64_t, 2u> {
    if (A.dim() != 2 || B.dim() != 2 || C.dim() != 2) {
        throw std::runtime_error("expected matrices");
    }
    const auto M = C.shape(0);
    const auto N = C.shape(1);
    if (M != A.shape(0) || N != A.shape(1) || M != B.shape(0) || N != B.shape(1)) {
        throw std::runtime_error("incompatible hadamard");
    }
    return {M, N};
}

auto make_blas_a3_prog(char const *name, tensor_layout const &layoutA, tensor_layout const &layoutB,
                       tensor_layout const &layoutC, tinytc_type_t alpha_ty, tinytc_type_t A_ty,
                       tinytc_type_t B_ty, tinytc_type_t beta_ty, tinytc_type_t C_ty,
                       std::function<void(region_builder &, array_view<tinytc_value_t>)> make_op)
    -> shared_handle<tinytc_prog_t> {
    auto ctx = get_compiler_context(alpha_ty);
    auto p = create_prog(ctx.get());

    auto At = get<memref_type>(A_ty, layoutA.static_shape(), layoutA.static_stride(),
                               address_space::global);
    auto Bt = get<memref_type>(B_ty, layoutB.static_shape(), layoutB.static_stride(),
                               address_space::global);
    auto Ct = get<memref_type>(C_ty, layoutC.static_shape(), layoutC.static_stride(),
                               address_space::global);

    auto void_ty = get<void_type>(ctx.get());
    auto f = create_func(name, {alpha_ty, At, Bt, beta_ty, Ct}, void_ty);
    auto fn_body = get_body(f.get());
    auto params = std::array<tinytc_value_t, 5u>{};
    get_parameters(fn_body, params);
    set_name(params[0], "alpha");
    set_name(params[1], "A");
    set_name(params[2], "B");
    set_name(params[3], "beta");
    set_name(params[4], "C");

    auto bb = region_builder{fn_body};

    make_op(bb, params);

    add_function(p.get(), std::move(f));

    return p;
}

} // namespace tinytc::test
