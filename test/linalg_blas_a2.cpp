// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "linalg_blas_a2.hpp"

#include <array>

namespace tinytc::test {

auto make_blas_a2_prog(char const *name, tensor_layout const &layoutA, tensor_layout const &layoutB,
                       scalar_type alpha_ty, scalar_type A_ty, scalar_type beta_ty,
                       scalar_type B_ty,
                       std::function<void(region_builder &, array_view<value>)> make_op) -> prog {
    auto ctx = make_compiler_context();

    auto const alphat = get_scalar(ctx, alpha_ty);
    auto const at = get_scalar(ctx, A_ty);
    auto const betat = get_scalar(ctx, beta_ty);
    auto const bt = get_scalar(ctx, B_ty);

    auto p = make_prog(ctx);

    auto At =
        get_memref(at, layoutA.static_shape(), layoutA.static_stride(), address_space::global);
    auto Bt =
        get_memref(bt, layoutB.static_shape(), layoutB.static_stride(), address_space::global);

    auto f = make_func(name, {alphat, At, betat, Bt});
    auto fn_body = f.get_body();
    auto params = std::array<value, 4u>{};
    fn_body.get_parameters(params);
    params[0].set_name("alpha");
    params[1].set_name("A");
    params[2].set_name("beta");
    params[3].set_name("B");

    auto bb = region_builder{fn_body};

    make_op(bb, params);

    p.add_function(std::move(f));

    return p;
}

} // namespace tinytc::test
