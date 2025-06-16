// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "parser.hpp"

#include "lexer.hpp"
#include "parser_impl.hpp"

#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace mochi {

auto parse_file(std::size_t input_size, char const *input, char const *filename)
    -> std::optional<objects> {
    auto lex = lexer(input_size, input, filename);
    auto obj = objects();
    auto p = parser(lex, obj);
    if (p() == 0) {
        return obj;
    }
    return std::nullopt;
}

} // namespace mochi

