// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef LEXER_20230614_HPP
#define LEXER_20230614_HPP

#include "parser/parser_impl.hpp"
#include "tinytc/ir/location.hpp"

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <string>

namespace tinytc {
enum class scalar_type;
}

namespace tinytc::parser {

class lexer {
  public:
    lexer(std::string const &input, std::ostream *oerr = nullptr, std::string const &filename = "");
    parser::symbol_type operator()();

    void error(location const &l, std::string const &m);

  private:
    std::uint64_t lex_number(char const *s, char const *e);
    std::int64_t lex_integer_constant(char const *s, char const *e);
    double lex_floating_constant(char const *s, char const *e);
    scalar_type lex_integer_type(char const *s, char const *e);
    scalar_type lex_floating_type(char const *s, char const *e);

    char const *input_;
    std::size_t len_;
    char const *YYCURSOR, *YYLIMIT;
    std::ostream *oerr_;
    location loc_;
};

inline parser::symbol_type yylex(lexer &lex) { return lex(); }

} // namespace tinytc::parser

#endif // LEXER_20230614_HPP
