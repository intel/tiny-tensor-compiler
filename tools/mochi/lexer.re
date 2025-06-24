// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "lexer.hpp"

#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <utility>

namespace mochi {

lexer::lexer(std::size_t input_size, char const *input, char const *filename)
    : input_{input}, len_(input_size), filename_(filename), YYCURSOR{input_},
      YYLIMIT{input_ + len_} {
    loc_.initialize(&filename_);
}

auto lexer::operator()() -> parser::symbol_type {
    char const *YYMARKER;
lex:
    char const *b = YYCURSOR;
    loc_.step();
    auto const adv_loc = [&]() { loc_.columns(YYCURSOR - b); };
    // clang-format off
    /*!re2c
        re2c:define:YYCTYPE = char;
        re2c:yyfill:enable = 0;
        re2c:eof = 0;

        // strings
         "\"" [^\"]* "\"" | "'" [^']* "'" {
             char const* begin = b;
             char const* mark = begin;
             for (; mark != YYCURSOR; ++mark) {
                if (*mark == '\n') {
                    loc_.lines(1);
                    begin = mark + 1;
                }
             }
             loc_.columns(YYCURSOR - begin);
             auto str = std::string(b + 1, YYCURSOR - 1);
             return parser::make_STRING(std::move(str), loc_);
         }

        // id
        id = [a-zA-Z_][a-zA-Z0-9_]*;
        "@" id {
            adv_loc();
            auto id = std::string(b+1, YYCURSOR);
            return parser::make_GLOBAL_ID(std::move(id), loc_);
        }
        "%" id {
            adv_loc();
            auto id = std::string(b+1, YYCURSOR);
            return parser::make_LOCAL_ID(std::move(id), loc_);
        }

        // numbers
        dec = [1-9[0-9]*;
        hex = "0x" [a-fA-F0-9]+;
        oct = "0" [0-7]*;
        dec | hex | oct {
            adv_loc();
            auto i = lex_number(b, YYCURSOR);
            return parser::make_NUMBER(i, loc_);
        }

        // keywords
        "case"              { adv_loc(); return parser::make_CASE(loc_); }
        "collective"        { adv_loc(); return parser::make_COLLECTIVE(loc_); }
        "cxx"               { adv_loc(); return parser::make_CXX(loc_); }
        "doc_to_string"     { adv_loc(); return parser::make_DOC_TO_STRING(loc_); }
        "enum"              { adv_loc(); return parser::make_ENUM(loc_); }
        "inst"              { adv_loc(); return parser::make_INST(loc_); }
        "mixed"             { adv_loc(); return parser::make_MIXED(loc_); }
        "op"                { adv_loc(); return parser::make_OP(loc_); }
        "private"           { adv_loc(); return parser::make_PRIVATE(loc_); }
        "prop"              { adv_loc(); return parser::make_PROP(loc_); }
        "reg"               { adv_loc(); return parser::make_REG(loc_); }
        "ret"               { adv_loc(); return parser::make_RET(loc_); }
        "spmd"              { adv_loc(); return parser::make_SPMD(loc_); }

        // punctuation
        "=>"                { adv_loc(); return parser::make_ARROW(loc_); }
        ":"                 { adv_loc(); return parser::make_COLON(loc_); }
        "{"                 { adv_loc(); return parser::make_LBRACE(loc_); }
        "}"                 { adv_loc(); return parser::make_RBRACE(loc_); }
        "?"                 { adv_loc(); return parser::make_QUESTION(loc_); }
        "*"                 { adv_loc(); return parser::make_STAR(loc_); }

        // whitespace
        whitespace            = [ \t\v\r]+;
        comment               = ";" [^\n]*;
        newline               = "\r"? "\n";
        whitespace | comment { adv_loc(); goto lex; }
        newline              { loc_.lines(1); goto lex; }

        [\x00]              { adv_loc(); return parser::make_YYEOF(loc_); }
        $                   { adv_loc(); return parser::make_YYEOF(loc_); }
        *                   {
            adv_loc();
            throw parser::syntax_error(loc_, "Unknown token");
        }
     */
    // clang-format on
}

auto lexer::lex_number(char const *s, char const *e) -> std::int64_t {
    auto number = std::string(s, e);
    const std::int64_t i = strtol(number.c_str(), nullptr, 0);
    if (errno == ERANGE) {
        throw parser::syntax_error(loc_, "Integer value out of range: " + number);
    }
    return i;
}

} // namespace mochi
