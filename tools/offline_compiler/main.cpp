// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <cstring>
#include <exception>
#include <iostream>

using namespace tinytc;

int main(int argc, char **argv) {
    auto ctx = source_context();
    try {
        auto p = prog{};
        if (argc < 2 || strcmp(argv[1], "-") == 0) {
            p = ctx.parse_stdin();
        } else {
            p = ctx.parse_file(argv[1]);
        }

        auto info = create_core_info_intel_from_arch(intel_gpu_architecture::pvc);
        auto src = compile_to_opencl(p, info, ctx.get());
        std::cout << src.get_code();
    } catch (status const &st) {
        std::cerr << ctx.get_error_log() << std::endl;
        return 1;
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
