// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "parser/lexer.hpp"

#include "doctest/doctest.h"

#include <cstdint>
#include <limits>
#include <variant>

using namespace tinytc::ir;

TEST_CASE("lex integer") {
    token tok;
    CHECK(lexer("9223372036854775807")(tok) == token_kind::integer_constant);
    CHECK(std::get<std::int64_t>(tok.val) == std::numeric_limits<std::int64_t>::max());

    CHECK(lexer("9223372036854775808")(tok) == token_kind::unknown);
    CHECK(std::get<lexer_error>(tok.val) == lexer_error::integer_overflow);

    CHECK(lexer("-9223372036854775807")(tok) == token_kind::integer_constant);
    CHECK(std::get<std::int64_t>(tok.val) == -std::numeric_limits<std::int64_t>::max());

    // While -9223372036854775808 is representable by int64 the literal
    // to be out of range in analogy to -9223372036854775808ll in C++
    CHECK(lexer("-9223372036854775808")(tok) == token_kind::unknown);
    CHECK(std::get<lexer_error>(tok.val) == lexer_error::integer_overflow);
}

TEST_CASE("lex float") {
    token tok;
    auto check = [&tok](char const *string, double ref) {
        CHECK(lexer(string)(tok) == token_kind::floating_constant);
        CHECK(std::get<double>(tok.val) == ref);
    };

    check("123.456", 123.456);
    check("53111251581212893.120591209512095102", 53111251581212893.120591209512095102);
    check(".42", 0.42);
    check("42.", 42.0);
    check("+1e1", +1e+1);
    check("1e-1", 1e-1);
    check(".1e-1", .1e-1);
    check("-1.e-1", -1.e-1);
    check("0x1.4p3", 0x1.4p3);
    check("-0x1.4p3", -0x1.4p3);
    check("0x.412341p-15", 0x.412341p-15);
    check("0x412341p+42", 0x412341p+42);

    CHECK(lexer("1e1000000000")(tok) == token_kind::unknown);
    CHECK(std::get<lexer_error>(tok.val) == lexer_error::floating_out_of_range);
}
