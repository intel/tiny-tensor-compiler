// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PARSER_20250611_HPP
#define PARSER_20250611_HPP

#include "objects.hpp"

#include <cstddef>
#include <filesystem>
#include <iosfwd>
#include <optional>
#include <string>
#include <vector>

namespace mochi {

auto find_path_to(std::string const &filename, std::vector<char const *> const &search_paths)
    -> std::filesystem::path;
auto open_file(std::filesystem::path const &p) -> std::ifstream;

auto parse_file(std::size_t input_size, char const *input, char const *filename = nullptr,
                std::vector<char const *> const &search_paths = {}) -> std::optional<objects>;
auto parse_file(std::string const &filename, std::vector<char const *> const &search_paths = {})
    -> std::optional<objects>;

} // namespace mochi

#endif // PARSER_20250611_HPP
