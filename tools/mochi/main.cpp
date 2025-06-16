// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "argparser.hpp"
#include "omochi.hpp"

#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace mochi;

int main(int argc, char **argv) {
    bool help = false;
    generator gen = generator::template_;
    char const *filename = nullptr;
    char const *output_filename = nullptr;
    std::vector<char const *> search_paths;

    auto const gen_converter = [](char const *str, generator &gen) {
        auto g = lex_generator(strlen(str), str);
        if (g) {
            gen = *g;
            return tinytc::cmd::parser_status::success;
        }
        return tinytc::cmd::parser_status::invalid_argument;
    };

    auto parser = tinytc::cmd::arg_parser{};
    try {
        parser.set_short_opt('g', &gen, "Generator").converter(gen_converter);
        parser.set_short_opt('I', &search_paths, "Search path");
        parser.set_short_opt('o', &output_filename, "Output filename");
        parser.set_short_opt('h', &help, "Show help");
        parser.set_long_opt("help", &help, "Show help");
        parser.add_positional_arg("file-name", &filename, "Path to source code");

        parser.parse(argc, argv);
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    if (help) {
        parser.print_help(std::cout, "mochi", "");

        std::cout << std::endl;
        std::cout << "Available generators:" << std::endl;
        const auto print_option = [&](char const *opt) {
            for (int i = 0; i < tinytc::cmd::arg_parser::optindent; ++i) {
                std::cout << ' ';
            }
            std::cout << opt << std::endl;
        };
        print_option("inst_header");
        print_option("inst_cpp");
        print_option("inst_visit_header");
        print_option("template");

        return 0;
    }

    if (!filename) {
        std::cerr << "Empty filename not permitted" << std::endl;
        return 1;
    }

    try {
        if (output_filename) {
            auto code_stream = std::ofstream(output_filename, std::ios_base::out);
            if (!code_stream.good()) {
                auto err = std::ostringstream{} << "Could not open " << output_filename
                                                << " for writing.";
                throw std::runtime_error(std::move(err).str());
            }
            please_do(code_stream, {gen, filename}, search_paths);
        } else {
            please_do(std::cout, {gen, filename}, search_paths);
        }
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
