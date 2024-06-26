// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "location.hpp"
#include "parser/lexer.hpp"
#include "parser/parser_impl.hpp"
#include "tinytc/types.hpp"

#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <string>
#include <utility>

namespace tinytc {

lexer::lexer(std::uint64_t input_size, char const *input, location const &initial_loc)
    : input_{input}, len_(input_size), YYCURSOR{input_}, YYLIMIT{input_ + len_}, loc_{initial_loc} {
}

parser::symbol_type lexer::operator()() {
    char const *YYMARKER;
lex:
    char const *b = YYCURSOR;
    step(loc_);
    auto const adv_loc = [&]() { columns(loc_, YYCURSOR - b); };
    /*!re2c
        re2c:define:YYCTYPE = char;
        re2c:yyfill:enable = 0;
        re2c:eof = 0;

        // regexp
        comment               = ";" [^\n]*;
        newline               = "\r"? "\n";
        whitespace            = [ \t\v\r]+;

        identifier            = [0-9]+ | ([a-zA-Z] [a-zA-Z0-9_]*);
        local_identifier      = "%" identifier;
        global_identifier     = "@" identifier;

        integer_type          = "i" ("1" | "8" | "16" | "32" | "64") | "index";
        floating_type         = "f" ("32" | "64");

        digit                 = [0-9];
        hexdigit              = [0-9a-fA-F];
        integer_constant      = [-+]? digit+;
        mantissa              = digit* "." digit+ | digit+ ".";
        mantissa_hex          = (hexdigit* "." hexdigit+ | hexdigit+ ".");
        exponent              = [eE] [-+]? digit+;
        exponent_hex          = [pP] [-+]? digit+;
        floating_constant     = [-+]? (mantissa exponent? | digit+ exponent);
        floating_constant_hex = [-+]? "0x" (mantissa_hex exponent_hex? | hexdigit+ exponent_hex);


        // identifier
        local_identifier    {
            adv_loc();
            auto id = std::string(b + 1, YYCURSOR);
            return parser::make_LOCAL_IDENTIFIER(std::move(id), loc_);
        }
        global_identifier   {
            adv_loc();
            auto id = std::string(b + 1, YYCURSOR);
            return parser::make_GLOBAL_IDENTIFIER(std::move(id), loc_);
        }

        // symbols
        "="                 { adv_loc(); return parser::make_EQUALS(loc_); }
        ","                 { adv_loc(); return parser::make_COMMA(loc_); }
        "x"                 { adv_loc(); return parser::make_TIMES(loc_); }
        ":"                 { adv_loc(); return parser::make_COLON(loc_); }
        "("                 { adv_loc(); return parser::make_LPAREN(loc_); }
        ")"                 { adv_loc(); return parser::make_RPAREN(loc_); }
        "{"                 { adv_loc(); return parser::make_LBRACE(loc_); }
        "}"                 { adv_loc(); return parser::make_RBRACE(loc_); }
        "<"                 { adv_loc(); return parser::make_LCHEV(loc_); }
        ">"                 { adv_loc(); return parser::make_RCHEV(loc_); }
        "["                 { adv_loc(); return parser::make_LSQBR(loc_); }
        "]"                 { adv_loc(); return parser::make_RSQBR(loc_); }

        // keywords
        "func"              { adv_loc(); return parser::make_FUNC(loc_); }
        "work_group_size"   { adv_loc(); return parser::make_WORK_GROUP_SIZE(loc_); }
        "subgroup_size"     { adv_loc(); return parser::make_SUBGROUP_SIZE(loc_); }
        "->"                { adv_loc(); return parser::make_RETURNS(loc_); }
        "?"                 { adv_loc(); return parser::make_DYNAMIC(loc_); }
        ".n"                { adv_loc(); return parser::make_NOTRANS(loc_); }
        ".t"                { adv_loc(); return parser::make_TRANS(loc_); }
        ".atomic"           { adv_loc(); return parser::make_ATOMIC(loc_); }

        // constants
        "true"              { adv_loc(); return parser::make_INTEGER_CONSTANT(1, loc_); }
        "false"             { adv_loc(); return parser::make_INTEGER_CONSTANT(0, loc_); }
        integer_constant    {
            adv_loc();
            auto i = lex_integer_constant(b, YYCURSOR);
            return parser::make_INTEGER_CONSTANT(i, loc_);
        }
        floating_constant | floating_constant_hex {
            adv_loc();
            auto f = lex_floating_constant(b, YYCURSOR);
            return parser::make_FLOATING_CONSTANT(f, loc_);
        }

        // types
        integer_type        {
            adv_loc();
            auto t = lex_integer_type(b, YYCURSOR);
            return parser::make_INTEGER_TYPE(t, loc_);
        }
        floating_type       {
            adv_loc();
            auto t = lex_floating_type(b, YYCURSOR);
            return parser::make_FLOATING_TYPE(t, loc_);
        }
        "memref"            { adv_loc(); return parser::make_MEMREF(loc_); }
        "group"             { adv_loc(); return parser::make_GROUP(loc_); }

        // layouts
        "offset"            { adv_loc(); return parser::make_OFFSET(loc_); }
        "strided"           { adv_loc(); return parser::make_STRIDED(loc_); }

        // instructions
        "axpby"             { adv_loc(); return parser::make_AXPBY(loc_); }
        "arith"             { adv_loc(); return parser::make_ARITH(loc_); }
        "gemm"              { adv_loc(); return parser::make_GEMM(loc_); }
        "gemv"              { adv_loc(); return parser::make_GEMV(loc_); }
        "ger"               { adv_loc(); return parser::make_GER(loc_); }
        "hadamard"          { adv_loc(); return parser::make_HADAMARD(loc_); }
        "alloca"            { adv_loc(); return parser::make_ALLOCA(loc_); }
        "cast"              { adv_loc(); return parser::make_CAST(loc_); }
        "cmp"               { adv_loc(); return parser::make_CMP(loc_); }
        "expand"            { adv_loc(); return parser::make_EXPAND(loc_); }
        "fuse"              { adv_loc(); return parser::make_FUSE(loc_); }
        "load"              { adv_loc(); return parser::make_LOAD(loc_); }
        "group_id"          { adv_loc(); return parser::make_GROUP_ID(loc_); }
        "group_size"        { adv_loc(); return parser::make_GROUP_SIZE(loc_); }
        "for"               { adv_loc(); return parser::make_FOR(loc_); }
        "foreach"           { adv_loc(); return parser::make_FOREACH(loc_); }
        "if"                { adv_loc(); return parser::make_IF(loc_); }
        "else"              { adv_loc(); return parser::make_ELSE(loc_); }
        "size"              { adv_loc(); return parser::make_SIZE(loc_); }
        "subview"           { adv_loc(); return parser::make_SUBVIEW(loc_); }
        "store"             { adv_loc(); return parser::make_STORE(loc_); }
        "sum"               { adv_loc(); return parser::make_SUM(loc_); }
        "yield"             { adv_loc(); return parser::make_YIELD(loc_); }

        // binary op
        ".add"              { adv_loc(); return parser::make_ARITHMETIC(arithmetic::add, loc_); }
        ".sub"              { adv_loc(); return parser::make_ARITHMETIC(arithmetic::sub, loc_); }
        ".mul"              { adv_loc(); return parser::make_ARITHMETIC(arithmetic::mul, loc_); }
        ".div"              { adv_loc(); return parser::make_ARITHMETIC(arithmetic::div, loc_); }
        ".rem"              { adv_loc(); return parser::make_ARITHMETIC(arithmetic::rem, loc_); }
        ".shl"              { adv_loc(); return parser::make_ARITHMETIC(arithmetic::shl, loc_); }
        ".shr"              { adv_loc(); return parser::make_ARITHMETIC(arithmetic::shr, loc_); }
        ".and"              { adv_loc(); return parser::make_ARITHMETIC(arithmetic::and_, loc_); }
        ".or"              { adv_loc(); return parser::make_ARITHMETIC(arithmetic::or_, loc_); }
        ".xor"              { adv_loc(); return parser::make_ARITHMETIC(arithmetic::xor_, loc_); }

        // unary op
        ".neg"              { adv_loc(); return parser::make_ARITHMETIC_UNARY(arithmetic_unary::neg, loc_); }
        ".not"              { adv_loc(); return parser::make_ARITHMETIC_UNARY(arithmetic_unary::not_, loc_); }

        // comparison condition
        ".eq"               { adv_loc(); return parser::make_CMP_CONDITION(cmp_condition::eq,
       loc_); }
        ".ne"               { adv_loc(); return parser::make_CMP_CONDITION(cmp_condition::ne,
       loc_); }
        ".gt"               { adv_loc(); return parser::make_CMP_CONDITION(cmp_condition::gt,
       loc_); }
        ".ge"               { adv_loc(); return parser::make_CMP_CONDITION(cmp_condition::ge,
       loc_); }
        ".lt"               { adv_loc(); return parser::make_CMP_CONDITION(cmp_condition::lt,
       loc_); }
        ".le"               { adv_loc(); return parser::make_CMP_CONDITION(cmp_condition::le,
       loc_); }

        whitespace          { adv_loc(); goto lex; }
        comment             { adv_loc(); goto lex; }
        newline             { lines(loc_, 1); goto lex; }
        [\x00]              { adv_loc(); return parser::make_YYEOF(loc_); }
        $                   { adv_loc(); return parser::make_YYEOF(loc_); }
        *                   {
            adv_loc();
            throw parser::syntax_error(loc_, "Unknown token");
        }
     */
}

std::uint64_t lexer::lex_number(char const *s, char const *e) {
    std::uint64_t n = 0;
    for (; s < e; ++s) {
        auto d = *s - '0';
        /*
         * imax >= 10 * i + d
         * floor((imax - d) / 10) >= i
         */
        if ((std::numeric_limits<std::uint64_t>::max() - d) / 10 < n) {
            throw parser::syntax_error(loc_, "Integer overflow: " + std::string(s, e));
        }
        n = 10 * n + d;
    }
    return n;
}

std::int64_t lexer::lex_integer_constant(char const *s, char const *e) {
    std::int64_t i = 0;
    if (*s == '-') {
        ++s;
        std::uint64_t n = lex_number(s, e);
        if (n > static_cast<std::uint64_t>(std::numeric_limits<int64_t>::max())) {
            throw parser::syntax_error(loc_, "Integer overflow: " + std::string(s, e));
        }
        i = -static_cast<std::int64_t>(n);
    } else {
        if (*s == '+') {
            ++s;
        }
        std::uint64_t n = lex_number(s, e);
        if (n > static_cast<std::uint64_t>(std::numeric_limits<int64_t>::max())) {
            throw parser::syntax_error(loc_, "Integer overflow: " + std::string(s, e));
        }
        i = static_cast<std::int64_t>(n);
    }
    return i;
}

double lexer::lex_floating_constant(char const *s, char const *e) {
    auto number = std::string(s, e);
    double d = strtod(number.c_str(), nullptr);
    if (errno == ERANGE) {
        throw parser::syntax_error(loc_, "Floating point value out of range: " + std::string(s, e));
    }
    return d;
}

scalar_type lexer::lex_integer_type(char const *s, char const *) {
    char const *YYMARKER;
    /*!re2c
        re2c:yyfill:enable = 0;
        re2c:define:YYCURSOR = s;

        "i1"    { return scalar_type::i1; }
        "i8"    { return scalar_type::i8; }
        "i16"   { return scalar_type::i16; }
        "i32"   { return scalar_type::i32; }
        "i64"   { return scalar_type::i64; }
        "index" { return scalar_type::index; }
        $       { return {}; }
        *       { return {}; }
    */
    return scalar_type{};
}
scalar_type lexer::lex_floating_type(char const *s, char const *) {
    char const *YYMARKER;
    /*!re2c
        re2c:yyfill:enable = 0;
        re2c:define:YYCURSOR = s;

        "f32"  { return scalar_type::f32; }
        "f64"  { return scalar_type::f64; }
        $      { return {}; }
        *      { return {}; }
    */
    return scalar_type{};
}

} // namespace tinytc
