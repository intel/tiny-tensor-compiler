// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "argparser.hpp"
#include "codegen.hpp"
#include "parser.hpp"

#include <exception>
#include <iostream>
#include <optional>

using namespace mochi;

int main(int argc, char **argv) {
    char const *filename = nullptr;
    bool help = false;

    auto parser = tinytc::cmd::arg_parser{};
    try {
        parser.set_short_opt('h', &help, "Show help");
        parser.set_long_opt("help", &help, "Show help");
        parser.add_positional_arg("file-name", &filename,
                                  "Path to source code; leave empty to read from stdin");

        parser.parse(argc, argv);
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    if (help) {
        parser.print_help(std::cout, "mochi", "");

        return 0;
    }

    try {
        auto obj = parse_file(filename);
        if (obj) {
            generate_inst_header(std::cout, *obj);
            generate_inst_cpp(std::cout, *obj);
        } else {
            return 1;
        }
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
