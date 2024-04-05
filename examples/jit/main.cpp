// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include <tinytc/tinytc.hpp>

#include <functional>
#include <iostream>
#include <string>
#include <utility>

using namespace tinytc;

int main(int argc, char **argv) {
    if (argc < 2) {
        return -1;
    }

    auto srcman = source_manager(&std::cerr);
    auto info = get_core_info_intel_gpu(intel_gpu_architecture::pvc);
    auto prog = srcman.parse_file(argv[1]);
    if (!prog) {
        return -1;
    }
    auto bin = optimize_and_make_binary(std::move(prog), bundle_format::spirv, std::move(info),
                                        srcman.error_reporter());
    if (!bin) {
        return -1;
    }

    return 0;
}
