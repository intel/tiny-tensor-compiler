// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "device_info.hpp"
#include "error.hpp"
#include "node/program_node.hpp"
#include "parser.hpp"
#include "passes.hpp"
#include "reference_counted.hpp"
#include "required_extensions.hpp"
#include "source.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/types.h"

#include <clir/visitor/codegen_opencl.hpp>
#include <clir/visitor/unique_names.hpp>

#include <memory>
#include <sstream>
#include <utility>

using namespace tinytc;

extern "C" {

tinytc_status_t tinytc_prog_compile_to_opencl(tinytc_source_t *src, tinytc_prog_t prg,
                                              const_tinytc_core_info_t info,
                                              tinytc_source_context_t ctx) {
    if (src == nullptr || prg == nullptr || info == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] {
            // passes
            check_ir(*prg);
            insert_barriers(*prg);
            insert_lifetime_stop_inst(*prg);
            set_stack_ptrs(*prg);
            set_work_group_size(*prg, *info);
            // opencl
            auto ast = generate_opencl_ast(*prg, *info);
            clir::make_names_unique(ast);
            auto oss = std::ostringstream{};
            clir::generate_opencl(oss, ast);

            auto ext = required_extensions(std::move(ast));
            *src = std::make_unique<::tinytc_source>(oss.str(), prg->loc(), std::move(ext),
                                                     info->core_features())
                       .release();
        },
        ctx);
}
}
