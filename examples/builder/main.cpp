// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/builder.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <array>
#include <cstdint>
#include <iostream>
#include <utility>

using namespace tinytc;

int main() {
    int64_t M = 64;
    int64_t N = 32;

    try {
        auto ctx = make_compiler_context();
        auto element_ty = get<f32_type>(ctx.get());
        auto ty = get<memref_type>(element_ty, array_view{M, N}, array_view<std::int64_t>{},
                                   address_space::global);

        auto void_ty = get<void_type>(ctx.get());
        auto f = make_func("copy", {ty, ty}, void_ty);

        auto body = get_body(f.get());
        std::array<tinytc_value_t, 2u> params;
        get_parameters(body, params);

        auto bb = region_builder{body};
        auto alpha = bb.constant_one(element_ty);
        auto beta = bb.constant_zero(element_ty);
        bb.create<axpby_inst>(false, transpose::N, alpha, params[0], beta, params[1]);

        auto p = make_prog(ctx.get());
        add_function(p.get(), std::move(f));

        dump(p.get());
    } catch (builder_error const &e) {
        std::cerr << "Error  " << static_cast<int>(e.code()) << std::endl;
    } catch (status const &st) {
        std::cerr << "Error " << static_cast<int>(st) << std::endl;
    }

    return 0;
}
