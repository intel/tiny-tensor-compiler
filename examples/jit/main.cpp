// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <exception>
#include <iostream>
#include <utility>

using namespace tinytc;

int main(int argc, char **argv) {
    if (argc < 2) {
        return -1;
    }

    auto ctx = create_source_context();
    try {
        auto info = create_core_info_intel_from_arch(intel_gpu_architecture::pvc);
        auto prog = parse_file(argv[1], ctx);
        if (!prog) {
            return -1;
        }
        compile_to_binary(std::move(prog), info, bundle_format::spirv, ctx);
    } catch (status const &st) {
        std::cerr << ctx.get_error_log() << std::endl;
        return 1;
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
