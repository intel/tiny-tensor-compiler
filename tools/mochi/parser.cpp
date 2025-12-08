// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "parser.hpp"

#include "lexer.hpp"
#include "parser_impl.hpp"

#include <fstream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace fs = std::filesystem;

namespace mochi {

auto find_path_to(std::string const &filename, std::vector<char const *> const &search_paths)
    -> fs::path {
    auto path = fs::path{filename};
    for (auto &s : search_paths) {
        auto candidate_path = fs::path{s} / path;
        if (fs::exists(candidate_path)) {
            return candidate_path;
        }
    }
    return path;
}

auto open_file(fs::path const &p) -> std::ifstream {
    auto code_stream = std::ifstream(p, std::ios_base::in);
    if (!code_stream.good()) {
        auto err = std::ostringstream{} << "Could not open " << p << " for reading.";
        throw std::runtime_error(std::move(err).str());
    }
    return code_stream;
}

auto parse_file(std::size_t input_size, char const *input, char const *filename,
                std::vector<char const *> const &search_paths) -> std::optional<objects> {
    auto lex = lexer(input_size, input, filename);
    auto obj = objects();
    auto p = parser(lex, obj, search_paths);
    if (p() == 0) {
        return obj;
    }
    return std::nullopt;
}

auto parse_file(std::string const &filename, std::vector<char const *> const &search_paths)
    -> std::optional<objects> {
    auto code_stream = open_file(find_path_to(filename, search_paths));
    auto code = std::string(std::istreambuf_iterator<char>{code_stream}, {});

    return parse_file(code.size(), code.c_str(), filename.c_str(), search_paths);
}

} // namespace mochi

