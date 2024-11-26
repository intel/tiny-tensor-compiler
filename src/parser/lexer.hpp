// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef LEXER_20230614_HPP
#define LEXER_20230614_HPP

#include "parser/parser_impl.hpp"
#include "tinytc/types.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

namespace tinytc {

class lexer {
  public:
    lexer(std::uint64_t input_size, char const *input, location const &start_loc);
    parser::symbol_type operator()();

    void error(location const &l, std::string const &m);
    auto input() const -> char const * { return input_; }
    auto input_size() const -> std::size_t { return YYLIMIT >= input_ ? YYLIMIT - input_ : 0u; }

  private:
    std::uint64_t lex_number(char const *s, char const *e);
    std::int64_t lex_integer_constant(char const *s, char const *e);
    double lex_floating_constant(char const *s, char const *e);

    char const *input_;
    std::size_t len_;
    char const *YYCURSOR, *YYLIMIT;
    location loc_;
};

inline parser::symbol_type yylex(lexer &lex) { return lex(); }

} // namespace tinytc

#endif // LEXER_20230614_HPP
