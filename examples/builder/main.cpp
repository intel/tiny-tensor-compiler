// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <array>
#include <cstdint>
#include <iostream>
#include <utility>

using namespace tinytc;

int main() {
    scalar_type sty = scalar_type::f32;
    int64_t M = 64;
    int64_t N = 32;

    try {
        auto ctx = make_compiler_context();
        auto element_ty = get_scalar(ctx, sty);
        auto ty = get_memref(element_ty, {M, N});

        auto f = make_func("copy", {ty, ty}, get_void(ctx));

        auto body = f.get_body();
        std::array<value, 2u> params;
        body.get_parameters(params);

        auto bb = region_builder{body};
        auto alpha = bb.add(make_constant_one(element_ty));
        auto beta = bb.add(make_constant_zero(element_ty));
        bb.add(make_axpby(false, transpose::N, alpha, params[0], beta, params[1]));

        auto p = make_prog(ctx);
        p.add_function(std::move(f));

        p.dump();
    } catch (builder_error const &e) {
        std::cerr << "Error  " << static_cast<int>(e.code()) << std::endl;
    } catch (status const &st) {
        std::cerr << "Error " << static_cast<int>(st) << std::endl;
    }

    return 0;
}
