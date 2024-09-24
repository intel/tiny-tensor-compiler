// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "device_info.hpp"
#include "error.hpp"
#include "node/program_node.hpp"
#include "pass/check_ir.hpp"
#include "pass/convert_to_opencl.hpp"
#include "pass/dump_cfg.hpp"
#include "pass/dump_ir.hpp"
#include "pass/insert_barrier.hpp"
#include "pass/insert_lifetime_stop.hpp"
#include "pass/stack.hpp"
#include "pass/work_group_size.hpp"
#include "passes.hpp"
#include "reference_counted.hpp"
#include "required_extensions.hpp"
#include "source.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <clir/visitor/codegen_opencl.hpp>
#include <clir/visitor/unique_names.hpp>

#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

using namespace tinytc;

extern "C" {

tinytc_status_t tinytc_run_function_pass(char const *pass_name, tinytc_prog_t prg,
                                         const_tinytc_core_info_t info) {
    if (prg == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] {
#define FUNCTION_PASS(NAME, CREATE_PASS)                                                           \
    if (strcmp(NAME, pass_name) == 0) {                                                            \
        return run_function_pass(CREATE_PASS, *prg);                                               \
    }
#define FUNCTION_PASS_WITH_INFO(NAME, CREATE_PASS)                                                 \
    if (strcmp(NAME, pass_name) == 0) {                                                            \
        return run_function_pass(CREATE_PASS(info), *prg);                                         \
    }
#include "passes.def"
#undef FUNCTION_PASS
#undef FUNCTION_PASS_WITH_INFO
            throw status::unknown_pass_name;
        },
        prg->get_context());
}

tinytc_status_t tinytc_list_function_passes(uint32_t *names_size, char const *const **names) {
    if (names_size == nullptr || names == nullptr) {
        return tinytc_status_invalid_arguments;
    }
#define FUNCTION_PASS(NAME, CREATE_PASS) NAME,
#define FUNCTION_PASS_WITH_INFO(NAME, CREATE_PASS) NAME,
    static char const *const pass_names[] = {
#include "passes.def"
    };
#undef FUNCTION_PASS
#undef FUNCTION_PASS_WITH_INFO
    *names_size = sizeof(pass_names) / sizeof(char const *);
    *names = pass_names;

    return tinytc_status_success;
}

tinytc_status_t tinytc_prog_compile_to_opencl(tinytc_source_t *src, tinytc_prog_t prg,
                                              const_tinytc_core_info_t info) {
    if (src == nullptr || prg == nullptr || info == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] {
            // passes
            run_function_pass(check_ir_pass{}, *prg);
            run_function_pass(insert_lifetime_stop_pass{}, *prg);
            run_function_pass(set_stack_ptr_pass{}, *prg);
            //  insert_barriers(*prg);
            run_function_pass(work_group_size_pass{info}, *prg);
            //  lower_linalg(*prg, *info);
            run_function_pass(dump_ir_pass{std::cout}, *prg);
            // propagate_constants(*prg);
            // dump_ir(std::cout, *prg);
            //  opencl
            auto ast = convert_to_opencl_pass{info}.run_on_program(*prg);
            clir::make_names_unique(ast);

            auto oss = std::ostringstream{};
            auto ext = required_extensions(ast);
            for (auto const &e : ext) {
                oss << "#pragma OPENCL EXTENSION " << e << " : enable" << std::endl;
            }

            clir::generate_opencl(oss, std::move(ast));

            *src = std::make_unique<::tinytc_source>(oss.str(), prg->loc(), std::move(ext),
                                                     info->core_features())
                       .release();
        },
        prg->get_context());
}
}
