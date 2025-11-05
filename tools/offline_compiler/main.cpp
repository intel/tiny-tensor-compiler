// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "argparser.hpp"
#include "argparser_common.hpp"
#include "tinytc/core.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <cstdint>
#include <exception>
#include <fstream>
#include <iostream>

using namespace tinytc;

void write_asm(tinytc_spv_mod_t mod, char const *output_filename) {
    if (output_filename) {
        print_to_file(mod, output_filename);
    } else {
        auto spvasm = print_to_string(mod);
        std::cout << spvasm.get();
    }
}

void write_bin(tinytc_spv_mod_t mod, char const *output_filename) {
    auto bin = spirv_assemble(mod);
    auto raw_data = get_raw(bin.get());
    if (output_filename) {
        auto stream = std::ofstream(output_filename, std::ios::binary);
        if (!stream.good()) {
            throw status::file_io_error;
        }
        stream.write(reinterpret_cast<char const *>(raw_data.data), raw_data.data_size);
    } else {
        std::cout.write(reinterpret_cast<char const *>(raw_data.data), raw_data.data_size);
    }
}

int main(int argc, char **argv) {
    char const *input_filename = nullptr;
    char const *output_filename = nullptr;
    auto info = shared_handle<tinytc_core_info_t>{};
    tinytc_core_feature_flags_t core_features = 0;
    std::int32_t opt_level = 2;
    auto flags = cmd::optflag_states{};
    bool emit_asm = false;
    bool help = false;

    auto parser = cmd::arg_parser{};
    try {
        info = create_core_info_intel_from_arch(intel_gpu_architecture::pvc);

        parser.set_short_opt('O', &opt_level, "Optimization level, default is -O2")
            .validator([](std::int32_t level) { return 0 <= level; });
        parser
            .set_short_opt('d', &info,
                           "Device name (cf. intel_gpu_architecture enum), default is \"pvc\"")
            .converter(
                [](char const *str, shared_handle<tinytc_core_info_t> &val) -> cmd::parser_status {
                    val = create_core_info_intel_from_name(str);
                    if (!val) {
                        return cmd::parser_status::invalid_argument;
                    }
                    return cmd::parser_status::success;
                });
        parser.set_short_opt('o', &output_filename,
                             "Path to output file; leave empty to print to stdout");
        parser.set_short_opt('S', &emit_asm, "Compile only; do not assemble");
        parser.set_short_opt('h', &help, "Show help");
        parser.set_long_opt("help", &help, "Show help");
        parser.add_positional_arg("file-name", &input_filename,
                                  "Path to source code; leave empty to read from stdin");
        cmd::add_optflag_states(parser, flags);
        cmd::add_core_feature_flags(parser, core_features);

        parser.parse(argc, argv);
    } catch (status const &st) {
        std::cerr << "Error (" << static_cast<int>(st) << "): " << to_string(st) << std::endl;
        return -1;
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    if (help) {
        parser.print_help(std::cout, "tinytc", "");

        std::cout << std::endl;
        cmd::list_optimization_flags(std::cout);

        std::cout << std::endl;
        cmd::list_core_feature_flags(std::cout);

        return 0;
    }

    try {
        auto ctx = create_compiler_context();
        set_error_reporter(ctx.get(), [](char const *what, const tinytc_location_t *, void *) {
            std::cerr << what << std::endl;
        });
        set_optimization_level(ctx.get(), opt_level);
        cmd::set_optflags(ctx.get(), flags);
        set_core_features(info.get(), core_features);
        auto p = [&] {
            if (!input_filename) {
                return parse_stdin(ctx.get());
            }
            return parse_file(input_filename, ctx.get());
        }();

        auto mod = compile_to_spirv(p.get(), info.get());
        if (emit_asm) {
            write_asm(mod.get(), output_filename);
        } else {
            write_bin(mod.get(), output_filename);
        }
    } catch (status const &st) {
        std::cerr << "Error (" << static_cast<int>(st) << "): " << to_string(st) << std::endl;
        return 1;
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
