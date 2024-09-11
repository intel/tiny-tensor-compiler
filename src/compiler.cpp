// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "device_info.hpp"
#include "error.hpp"
#include "node/program_node.hpp"
#include "parser.hpp"
#include "pass/check_ir.hpp"
#include "pass/dump_ir.hpp"
#include "passes.hpp"
#include "reference_counted.hpp"
#include "required_extensions.hpp"
#include "source.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/types.h"

#include <clir/visitor/codegen_opencl.hpp>
#include <clir/visitor/unique_names.hpp>

#include <cstring>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>

#include <iostream>

using namespace tinytc;

extern "C" {

tinytc_status_t tinytc_run_function_pass(char const *pass_name, tinytc_prog_t prg,
                                         const_tinytc_core_info_t info,
                                         tinytc_source_context_t ctx) {
    if (prg == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] {
#define FUNCTION_PASS(NAME, CREATE_PASS)                                                           \
    if (strcmp(NAME, pass_name) == 0) {                                                            \
        return run_function_pass(CREATE_PASS, *prg);                                               \
    }
#include "passes.def"
#undef FUNCTION_PASS
        },
        ctx);
}

tinytc_status_t tinytc_list_function_passes(uint32_t *names_size, char const *const **names) {
    if (names_size == nullptr || names == nullptr) {
        return tinytc_status_invalid_arguments;
    }
#define FUNCTION_PASS(NAME, CREATE_PASS) NAME,
    static char const *const pass_names[] = {
#include "passes.def"
    };
#undef FUNCTION_PASS
    *names_size = sizeof(pass_names) / sizeof(char const *);
    *names = pass_names;

    return tinytc_status_success;
}

tinytc_status_t tinytc_prog_compile_to_opencl(tinytc_source_t *src, tinytc_prog_t prg,
                                              const_tinytc_core_info_t info,
                                              tinytc_source_context_t ctx) {
    if (src == nullptr || prg == nullptr || info == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] {
            // passes
            run_function_pass(check_ir_pass{}, *prg);
            // insert_lifetime_stop_inst(*prg);
            //  set_stack_ptrs(*prg);
            //  insert_barriers(*prg);
            //  set_work_group_size(*prg, *info);
            //  lower_linalg(*prg, *info);
            run_function_pass(dump_ir_pass{std::cout}, *prg);
            // propagate_constants(*prg);
            // dump_ir(std::cout, *prg);
            //  opencl
            /*auto ast = generate_opencl_ast(*prg, *info);
            clir::make_names_unique(ast);

            auto oss = std::ostringstream{};
            auto ext = required_extensions(ast);
            for (auto const &e : ext) {
                oss << "#pragma OPENCL EXTENSION " << e << " : enable" << std::endl;
            }

            clir::generate_opencl(oss, std::move(ast));

            *src = std::make_unique<::tinytc_source>(oss.str(), prg->loc(), std::move(ext),
                                                     info->core_features())
                       .release();*/
        },
        ctx);
}
}
