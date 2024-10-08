// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "args.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <utility>

using namespace tinytc;

int main(int argc, char **argv) {
    auto a = args{};
    try {
        a = arg_parser::parse_args(argc, argv);
    } catch (status const &st) {
        std::cerr << "Error (" << static_cast<int>(st) << "): " << error_string(st) << std::endl;
        return -1;
    } catch (std::runtime_error const &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    if (a.help) {
        arg_parser::show_help(std::cout);
        return 0;
    }

    auto ctx = compiler_context{};
    try {
        ctx = make_compiler_context();
        ctx.set_optimization_level(a.opt_level);
        auto p = prog{};
        if (!a.filename) {
            p = parse_stdin(ctx);
        } else {
            p = parse_file(a.filename, ctx);
        }

        auto src = compile_to_opencl(std::move(p), a.info);
        std::cout << src.get_code();
    } catch (status const &st) {
        std::cerr << "Error (" << static_cast<int>(st) << "): " << error_string(st) << std::endl;
        return 1;
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
