// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <doctest/doctest.h>
#include <passes.hpp>

using namespace tinytc;

TEST_CASE("is_equal") {
    CHECK(is_equal(*make_scalar(scalar_type::f32), *make_scalar(scalar_type::f32)));
    CHECK(!is_equal(*make_scalar(scalar_type::i32), *make_scalar(scalar_type::u16)));
    auto a = make_memref(scalar_type::f32, {1, 2});
    auto b = make_memref(scalar_type::f32, {2, 3});
    auto c = make_memref(scalar_type::f64, {1, 2});
    CHECK(is_equal(*a, *a));
    CHECK(!is_equal(*a, *b));
    CHECK(!is_equal(*a, *c));
    CHECK(is_equal(*make_group(a), *make_group(a)));
    CHECK(!is_equal(*make_group(a), *make_group(b)));
    CHECK(!is_equal(*make_group(a), *make_group(c)));
    CHECK(!is_equal(*make_group(a), *a));
}
