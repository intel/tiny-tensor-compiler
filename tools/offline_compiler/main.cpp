// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "argparser.hpp"
#include "argparser_common.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <cstdint>
#include <exception>
#include <iostream>
#include <string_view>
#include <utility>

using namespace tinytc;

int main(int argc, char **argv) {
    char const *filename = nullptr;
    auto info = core_info{};
    std::int32_t opt_level = 2;
    auto flags = cmd::optflag_states{};
    bool help = false;

    auto parser = cmd::arg_parser{};
    try {
        info = make_core_info_intel_from_arch(intel_gpu_architecture::pvc);

        parser.set_short_opt('O', &opt_level, "Optimization level, default is -O2")
            .validator([](std::int32_t level) { return 0 <= level; });
        parser
            .set_short_opt('d', &info,
                           "Device name (cf. intel_gpu_architecture enum), default is \"pvc\"")
            .converter([](char const *str, core_info &val) -> cmd::parser_status {
                val = make_core_info_intel_from_name(str);
                if (!val) {
                    return cmd::parser_status::invalid_argument;
                }
                return cmd::parser_status::success;
            });
        parser.set_short_opt('h', &help, "Show help");
        parser.set_long_opt("help", &help, "Show help");
        parser.add_positional_arg("file-name", &filename,
                                  "Path to source code; leave empty to read from stdin");
        cmd::add_optflag_states(parser, flags);

        parser.parse(argc, argv);
    } catch (status const &st) {
        std::cerr << "Error (" << static_cast<int>(st) << "): " << error_string(st) << std::endl;
        return -1;
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    if (help) {
        parser.print_help(std::cout, "tinytc", "");

        std::cout << std::endl;
        cmd::list_optimization_flags(std::cout);

        return 0;
    }

    auto ctx = compiler_context{};
    try {
        ctx = make_compiler_context();
        ctx.set_optimization_level(opt_level);
        cmd::set_optflags(ctx, flags);
        auto p = prog{};
        if (!filename) {
            p = parse_stdin(ctx);
        } else {
            p = parse_file(filename, ctx);
        }

        auto src = compile_to_opencl(std::move(p), info);
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
