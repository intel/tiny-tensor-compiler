// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/parser.hpp"

#include "tinytc/device_info.hpp"
#include "tinytc/ir/error.hpp"
#include "tinytc/ir/passes.hpp"
#include "tinytc/ir/prog.hpp"
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
    if (!check_ir(p, [&](location const &loc, std::string const &what) {
            srcman.report_error(loc, what);
        })) {
        return 1;
    }
    try {
        auto info = get_core_info_intel_gpu(intel_gpu_architecture::pvc);
        insert_barriers(p);
        insert_lifetime_stop_inst(p);
        set_stack_ptrs(p);
        set_work_group_size(p, info);
        auto ocl_prog = generate_opencl_ast(std::move(p), info);
        clir::make_names_unique(ocl_prog);
        clir::generate_opencl(std::cout, std::move(ocl_prog));
    } catch (status const &st) {
        std::cerr << error_string(st) << std::endl;
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
