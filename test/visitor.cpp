// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <doctest/doctest.h>
#include <passes.hpp>

using namespace tinytc;

TEST_CASE("is_equal") {
    CHECK(is_equal(*data_type(scalar_type::f32), *data_type(scalar_type::f32)));
    CHECK(!is_equal(*data_type(scalar_type::i32), *data_type(scalar_type::u16)));
    auto a = create_memref(scalar_type::f32, {1, 2});
    auto b = create_memref(scalar_type::f32, {2, 3});
    auto c = create_memref(scalar_type::f64, {1, 2});
    CHECK(is_equal(*a, *a));
    CHECK(!is_equal(*a, *b));
    CHECK(!is_equal(*a, *c));
    CHECK(is_equal(*create_group(a), *create_group(a)));
    CHECK(!is_equal(*create_group(a), *create_group(b)));
    CHECK(!is_equal(*create_group(a), *create_group(c)));
    CHECK(!is_equal(*create_group(a), *a));
}
