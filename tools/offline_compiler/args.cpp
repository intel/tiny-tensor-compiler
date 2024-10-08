// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "args.hpp"
#include "tinytc/types.hpp"

#include <cstring>
#include <sstream>
#include <stdexcept>

using tinytc::core_info;
using tinytc::intel_gpu_architecture;
using tinytc::make_core_info_intel_from_arch;

auto make_core_info_from_string(char const *name) -> core_info {
    if (std::strcmp(name, "pvc") == 0) {
        return make_core_info_intel_from_arch(intel_gpu_architecture::pvc);
    } else if (std::strcmp(name, "tgl") == 0) {
        return make_core_info_intel_from_arch(intel_gpu_architecture::tgl);
    }
    return core_info{};
}

args arg_parser::parse_args(int argc, char **argv) {
    args a = {};
    a.filename = nullptr;
    a.opt_level = 2;

    int npos = 0;
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            auto const fail = [&]() {
                throw std::runtime_error(
                    (std::ostringstream{} << "==> Unrecognized argument: " << argv[i]).str());
            };
            if (std::strcmp(argv[i], "-h") == 0 || std::strcmp(argv[i], "--help") == 0) {
                a.help = true;
            } else if (std::strcmp(argv[i], "-O0") == 0) {
                a.opt_level = 0;
            } else if (std::strcmp(argv[i], "-O1") == 0) {
                a.opt_level = 1;
            } else if (std::strcmp(argv[i], "-O2") == 0) {
                a.opt_level = 2;
            } else if (i + 1 < argc) {
                if (std::strcmp(argv[i], "-d") == 0 || std::strcmp(argv[i], "--device") == 0) {
                    a.info = make_core_info_from_string(argv[++i]);
                    if (!a.info) {
                        throw std::runtime_error(
                            (std::ostringstream{} << "==> Unknown device: " << argv[i]).str());
                    }
                } else {
                    fail();
                }
            } else {
                fail();
            }
        } else {
            if (npos == 0) {
                a.filename = argv[i];
                ++npos;
            } else {
                throw std::runtime_error("==> At most a single positional argument is expected");
            }
        }
    }
    if (!a.info) {
        a.info = make_core_info_intel_from_arch(intel_gpu_architecture::pvc);
    }

    return a;
}

void arg_parser::show_help(std::ostream &os) {
    os << "usage: tinytc [-d <device>] [file-name]" << std::endl
       << R"HELP(
positional arguments:
    file-name           Path to source code; leave empty to read from stdin

optional arguments:
    -d, --device        Device name (cf. intel_gpu_architecture enum), default is "pvc"
    -h, --help          Show help text and exit
)HELP";
}
