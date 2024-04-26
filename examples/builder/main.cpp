// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <cstdint>

using namespace tinytc;

int main() {
    scalar_type type = scalar_type::f32;
    int64_t M = 64;
    int64_t N = 32;

    auto pb = program_builder{};
    pb.create("copy", [&](function_builder &fb) {
        auto dt = make_memref(type, {M, N});
        auto A = fb.argument(dt);
        auto B = fb.argument(dt);
        fb.body([&](region_builder &bb) {
            auto alpha = make_imm(1.0, type);
            auto beta = make_imm(0.0, type);
            bb.add(make_axpby(transpose::N, false, alpha, A, beta, B));
        });
    });
    auto program = pb.get_product();

    program.dump();

    return 0;
}
