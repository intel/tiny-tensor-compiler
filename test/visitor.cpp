// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include <doctest/doctest.h>
#include <tinytc/tinytc.hpp>

#include <algorithm>

using namespace tinytc::ir;

TEST_CASE("is_equal") {
    CHECK(is_equal(void_type(), void_type()));
    CHECK(is_equal(data_type(scalar_type::f32), data_type(scalar_type::f32)));
    CHECK(!is_equal(data_type(scalar_type::i32), data_type(scalar_type::u16)));
    CHECK(!is_equal(data_type(scalar_type::i32), void_type()));
    CHECK(!is_equal(void_type(), data_type(scalar_type::i32)));
    auto a = memref_type(scalar_type::f32, {1, 2});
    auto b = memref_type(scalar_type::f32, {2, 3});
    auto c = memref_type(scalar_type::f64, {1, 2});
    CHECK(is_equal(a, a));
    CHECK(!is_equal(a, b));
    CHECK(!is_equal(a, c));
    CHECK(is_equal(group_type(a), group_type(a)));
    CHECK(!is_equal(group_type(a), group_type(b)));
    CHECK(!is_equal(group_type(a), group_type(c)));
    CHECK(!is_equal(group_type(a), a));
}
