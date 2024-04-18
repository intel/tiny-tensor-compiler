// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "binary.hpp"
#include "compiler_options.hpp"
#include "device_info.hpp"
#include "error.hpp"
#include "node/program_node.hpp"
#include "opencl_cc.hpp"
#include "passes.hpp"
#include "required_extensions.hpp"
#include "source.hpp"
#include "tinytc/tinytc.h"
#include "util.hpp"

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
            *src =
                std::make_unique<::tinytc_source>(oss.str(), prg->loc(), std::move(ext)).release();
        },
        ctx, prg->loc());
}

tinytc_status_t tinytc_source_compile_to_binary(tinytc_binary_t *bin, const_tinytc_source_t src,
                                                const_tinytc_core_info_t info,
                                                tinytc_bundle_format_t format,
                                                tinytc_source_context_t ctx) {

    if (bin == nullptr || src == nullptr || info == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] {
            auto compiler_options =
                std::vector(default_compiler_options.begin(), default_compiler_options.end());
            auto const core_features = info->core_features();
            if (core_features &
                static_cast<std::uint32_t>(core_feature_flag::large_register_file)) {
                compiler_options.push_back(large_register_file_compiler_option_ze);
            }
            auto fmt = enum_cast<bundle_format>(format);
            auto bin_data = compile_opencl_c(src->code(), fmt, info->ip_version(), compiler_options,
                                             src->required_extensions());
            *bin = std::make_unique<::tinytc_binary>(std::move(bin_data), fmt, core_features)
                       .release();
        },
        ctx, src->code_loc());
}

tinytc_status_t tinytc_prog_compile_to_binary(tinytc_binary_t *bin, tinytc_prog_t prg,
                                              const_tinytc_core_info_t info,
                                              tinytc_bundle_format_t format,
                                              tinytc_source_context_t ctx) {
    if (bin == nullptr || prg == nullptr || info == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    tinytc_source_t src;
    TINYTC_CHECK_STATUS(tinytc_prog_compile_to_opencl(&src, prg, info, ctx));
    if (auto status = tinytc_source_compile_to_binary(bin, src, info, format, ctx);
        status != tinytc_status_success) {
        tinytc_source_destroy(src);
        return status;
    }
    return tinytc_status_success;
}

tinytc_status_t tinytc_source_get_code(const_tinytc_source_t src, char const **code) {
    if (src == nullptr || code == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *code = src->code(); });
}

void tinytc_source_destroy(tinytc_source_t src) { delete src; }
}
