// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "argparser.hpp"
#include "argparser_common.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <cstdint>
#include <cstring>
#include <exception>
#include <iostream>
#include <vector>

using namespace tinytc;

int main(int argc, char **argv) {
    auto pass_names = std::vector<char const *>{};
    char const *filename = nullptr;
    auto info = core_info{};
    tinytc_core_feature_flags_t core_features = 0;
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
        parser.set_short_opt('p', &pass_names, "Run pass");
        parser.set_short_opt('h', &help, "Show help");
        parser.set_long_opt("help", &help, "Show help");
        parser.add_positional_arg("file-name", &filename,
                                  "Path to source code; leave empty to read from stdin");
        cmd::add_optflag_states(parser, flags);
        cmd::add_core_feature_flags(parser, core_features);

        parser.parse(argc, argv);
    } catch (status const &st) {
        std::cerr << "Error (" << static_cast<int>(st) << "): " << error_string(st) << std::endl;
        return -1;
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    if (help) {
        parser.print_help(std::cout, "tinytc-opt", "");

        std::uint32_t names_size = 0;
        char const *const *names = nullptr;
        list_function_passes(names_size, names);

        std::cout << std::endl << "Passes:" << std::endl;
        for (std::uint32_t i = 0; i < names_size; ++i) {
            for (int i = 0; i < cmd::arg_parser::optindent; ++i) {
                std::cout << ' ';
            }
            std::cout << names[i] << std::endl;
        }

        std::cout << std::endl;
        cmd::list_optimization_flags(std::cout);

        std::cout << std::endl;
        cmd::list_core_feature_flags(std::cout);

        return 0;
    }

    if (pass_names.empty() || std::strncmp(pass_names.back(), "dump", 4) != 0) {
        pass_names.emplace_back("dump-ir");
    }

    auto ctx = compiler_context{};
    try {
        ctx = make_compiler_context();
        set_error_reporter(ctx, [](char const *what, const tinytc_location_t *, void *) {
            std::cerr << what << std::endl;
        });
        set_optimization_level(ctx, opt_level);
        cmd::set_optflags(ctx, flags);
        set_core_features(info, core_features);
        auto p = prog{};
        if (!filename) {
            p = parse_stdin(ctx);
        } else {
            p = parse_file(filename, ctx);
        }

        for (auto const &pass_name : pass_names) {
            run_function_pass(pass_name, p, info);
        }
    } catch (status const &st) {
        std::cerr << "Error (" << static_cast<int>(st) << "): " << error_string(st) << std::endl;
        return 1;
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
