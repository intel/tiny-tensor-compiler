// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef OMOCHI_20250616_HPP
#define OMOCHI_20250616_HPP

#include <cstddef>
#include <iosfwd>
#include <optional>
#include <string>
#include <vector>

namespace mochi {

enum class generator {
    api_builder_cpp = 0,
    api_builder_h,
    api_builder_hpp,
    enum_cpp,
    enum_h,
    enum_hpp,
    forward_hpp,
    inst_cpp,
    inst_hpp,
    inst_kind_cpp,
    type_cpp,
    type_hpp,
    visit_hpp,
    template_,
    NUM_GENERATORS
};

auto to_string(generator g) -> char const *;

struct action {
    generator gen;
    std::string filename;
};

auto lex_generator(std::size_t str_length, char const *str) -> std::optional<generator>;
auto lex_omochi(std::size_t str_length, char const *str) -> std::optional<action>;

void please_do(std::ostream &os, action const &a,
               std::vector<char const *> const &search_paths = {});
void please_do(std::ostream &os, std::istream &is, action const &a,
               std::vector<char const *> const &search_paths = {});
void process_template(std::ostream &os, std::string const &filename,
                      std::vector<char const *> const &search_paths = {});

} // namespace mochi

#endif // OMOCHI_20250616_HPP
