// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef LEXER_20250611_HPP
#define LEXER_20250611_HPP

#include "location.hh"
#include "parser_impl.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

namespace mochi {

class lexer {
  public:
    lexer(std::size_t input_size, char const *input, char const *filename = nullptr);
    auto operator()() -> parser::symbol_type;

    inline auto input() const -> char const * { return input_; }
    inline auto input_size() const -> std::size_t {
        return YYLIMIT >= input_ ? YYLIMIT - input_ : 0u;
    }

  private:
    auto lex_number(char const *s, char const *e) -> std::int64_t;

    char const *input_;
    std::size_t len_;
    std::string filename_;
    char const *YYCURSOR, *YYLIMIT;
    location loc_;
};

inline auto yylex(lexer &lex) -> parser::symbol_type { return lex(); }

} // namespace mochi

#endif // LEXER_20250611_HPP
