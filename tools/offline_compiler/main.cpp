// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/parser.hpp"
#include "tinytc/tinytc.hpp"

#include "clir/visitor/codegen_opencl.hpp"
#include "clir/visitor/unique_names.hpp"

#include <cstdint>
#include <cstring>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

using namespace tinytc;

int main(int argc, char **argv) {
    auto p = prog{};
    auto srcman = source_manager(&std::cerr);
    if (argc < 2 || strcmp(argv[1], "-") == 0) {
        p = srcman.parse_stdin();
    } else {
        p = srcman.parse_file(argv[1]);
    }

    if (!p) {
        return 1;
    }
    try {
        auto info = get_core_info_intel_gpu(intel_gpu_architecture::pvc);
        auto src = compile_to_opencl(p, info, nullptr, nullptr);
        std::cout << src.get_code();
    } catch (status const &st) {
        std::cerr << error_string(st) << std::endl;
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
