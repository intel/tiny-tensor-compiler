// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "linalg_blas_a2.hpp"

#include <array>

namespace tinytc::test {

auto make_blas_a2_prog(compiler_context const &ctx, char const *name, tensor_layout const &layoutA,
                       tensor_layout const &layoutB, tinytc_type_t alpha_ty, tinytc_type_t A_ty,
                       tinytc_type_t beta_ty, tinytc_type_t B_ty,
                       std::function<void(region_builder &, array_view<tinytc_value_t>)> make_op,
                       std::int32_t work_group_size) -> prog {
    auto p = make_prog(ctx);

    auto At = get<memref_type>(A_ty, layoutA.static_shape(), layoutA.static_stride(),
                               address_space::global);
    auto Bt = get<memref_type>(B_ty, layoutB.static_shape(), layoutB.static_stride(),
                               address_space::global);

    auto void_ty = get<void_type>(ctx.get());
    auto f = make_func(name, {alpha_ty, At, beta_ty, Bt}, void_ty);
    if (work_group_size) {
        auto const wgs_attr =
            named_attr{get_string_attr(ctx.get(), "work_group_size"),
                       get_array_attr(ctx.get(), {get_integer_attr(ctx.get(), work_group_size),
                                                  get_integer_attr(ctx.get(), 1)})};
        set_attr(f, get_dictionary_attr_with_sorted(ctx.get(), wgs_attr));
    }

    auto fn_body = get_body(f);
    auto params = std::array<tinytc_value_t, 4u>{};
    get_parameters(fn_body, params);
    set_name(params[0], "alpha");
    set_name(params[1], "A");
    set_name(params[2], "beta");
    set_name(params[3], "B");

    auto bb = region_builder{fn_body};

    make_op(bb, params);

    add_function(p, std::move(f));

    return p;
}

} // namespace tinytc::test
