// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "coopmatrix_info.hpp"

namespace tinytc {

auto accelerated_coopmatrix_info::have_precision(scalar_type A, scalar_type B,
                                                 scalar_type D) const -> bool {
    for (auto const &type : types_) {
        if (type.A() == A && type.B() == B && type.D() == D) {
            return true;
        }
    }
    return false;
}

const std::array<accelerated_coopmatrix_type, 5u> pvc_accelerated_coopmatrix_types = {{
    {scalar_type::i8,
     scalar_type::i8,
     {scalar_type::i32},
     scalar_type::i32,
     {{16, 1, 32}, {16, 2, 32}, {16, 4, 32}, {16, 8, 32}}},
    {scalar_type::f16,
     scalar_type::f16,
     {scalar_type::f16, scalar_type::f32},
     scalar_type::f32,
     {{16, 1, 16}, {16, 2, 16}, {16, 4, 16}, {16, 8, 16}}},
    {scalar_type::f16,
     scalar_type::f16,
     {scalar_type::f16, scalar_type::f32},
     scalar_type::f16,
     {{16, 1, 16}, {16, 2, 16}, {16, 4, 16}, {16, 8, 16}}},
    {scalar_type::bf16,
     scalar_type::bf16,
     {scalar_type::bf16, scalar_type::f32},
     scalar_type::f32,
     {{16, 1, 16}, {16, 2, 16}, {16, 4, 16}, {16, 8, 16}}},
    {scalar_type::bf16,
     scalar_type::bf16,
     {scalar_type::bf16, scalar_type::f32},
     scalar_type::bf16,
     {{16, 1, 16}, {16, 2, 16}, {16, 4, 16}, {16, 8, 16}}},
}};

} // namespace tinytc
