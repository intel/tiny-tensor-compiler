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

auto parse_file(char const *filename) -> std::optional<objects> {
    auto code_stream = std::ifstream(filename);
    if (!code_stream.good()) {
        throw std::runtime_error("Could not open source file");
    }
    auto code = std::string(std::istreambuf_iterator<char>{code_stream}, {});

    auto lex = lexer(code.size(), code.data(), filename);
    auto obj = objects();
    auto p = parser(lex, obj);
    if (p() == 0) {
        return obj;
    }
    return std::nullopt;
}

} // namespace mochi

