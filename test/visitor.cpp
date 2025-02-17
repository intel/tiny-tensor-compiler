// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <analysis/equal.hpp>
#include <doctest/doctest.h>

using namespace tinytc;

TEST_CASE("is_equal") {
    auto ctx = make_compiler_context();
    auto f32_ty = get_scalar(ctx, scalar_type::f32);
    auto f64_ty = get_scalar(ctx, scalar_type::f64);
    CHECK(is_equal(*f32_ty, *f32_ty));
    CHECK(!is_equal(*get_scalar(ctx, scalar_type::i32), *get_scalar(ctx, scalar_type::i16)));
    auto a = get_memref(f32_ty, {1, 2});
    auto b = get_memref(f32_ty, {2, 3});
    auto c = get_memref(f64_ty, {1, 2});
    CHECK(is_equal(*a, *a));
    CHECK(!is_equal(*a, *b));
    CHECK(!is_equal(*a, *c));
    CHECK(is_equal(*get_group(a, dynamic), *get_group(a, dynamic)));
    CHECK(!is_equal(*get_group(a, dynamic), *get_group(b, dynamic)));
    CHECK(!is_equal(*get_group(a, dynamic), *get_group(c, dynamic)));
    CHECK(!is_equal(*get_group(a, dynamic), *a));
}
