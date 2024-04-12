// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "binary.hpp"
#include "device_info.hpp"
#include "error.hpp"
#include "opencl_cc.hpp"
#include "passes.hpp"
#include "source.hpp"
#include "tinytc/internal/compiler_options.hpp"
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
                                              tinytc_error_handler_t err_handler,
                                              void *err_handler_data) {
    if (src == nullptr || prg == nullptr || info == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] {
            auto p = prog(prg, true);
            // passes
            check_ir(p);
            insert_barriers(p);
            insert_lifetime_stop_inst(p);
            set_stack_ptrs(p);
            set_work_group_size(p, *info);
            auto metadata = get_metadata(p);
            // opencl
            auto ast = generate_opencl_ast(std::move(p), *info);
            clir::make_names_unique(ast);
            auto oss = std::ostringstream{};
            clir::generate_opencl(oss, ast);

            // Compile
            auto ext = internal::required_extensions(std::move(ast));
            auto compiler_options = internal::default_compiler_options;

            *src =
                std::make_unique<source>(oss.str(), std::move(metadata), std::move(ext)).release();
        },
        err_handler, err_handler_data);
}

tinytc_status_t tinytc_source_compile_to_binary(tinytc_binary_t *bin, const_tinytc_source_t src,
                                                const_tinytc_core_info_t info,
                                                tinytc_bundle_format_t format,
                                                tinytc_error_handler_t err_handler,
                                                void *err_handler_data) {

    if (bin == nullptr || src == nullptr || info == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] {
            auto compiler_options = internal::default_compiler_options;
            auto const core_features = info->core_features();
            if (core_features &
                static_cast<std::uint32_t>(core_feature_flag::large_register_file)) {
                compiler_options.push_back(internal::large_register_file_compiler_option_ze);
            }
            auto fmt = enum_cast<bundle_format>(format);
            auto bin_data = compile_opencl_c(src->code(), fmt, info->ip_version(), compiler_options,
                                             src->required_extensions());
            *bin =
                std::make_unique<binary>(std::move(bin_data), fmt, src->metadata(), core_features)
                    .release();
        },
        err_handler, err_handler_data);
}

tinytc_status_t tinytc_prog_compile_to_binary(tinytc_binary_t *bin, tinytc_prog_t prg,
                                              const_tinytc_core_info_t info,
                                              tinytc_bundle_format_t format,
                                              tinytc_error_handler_t err_handler,
                                              void *err_handler_data) {
    if (bin == nullptr || prg == nullptr || info == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code(
        [&] {
            auto p = prog(prg, true);
            auto src = compile_to_opencl(p, *info, err_handler, err_handler_data);
            *bin = compile_to_binary(*src, *info, enum_cast<bundle_format>(format)).release();
        },
        err_handler, err_handler_data);
}

tinytc_status_t tinytc_source_get_code(const_tinytc_source_t src, char const **code) {
    if (src == nullptr || code == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *code = src->code(); });
}

tinytc_status_t tinytc_source_destroy(tinytc_source_t src) {
    return exception_to_status_code([&] { delete src; });
}

tinytc_status_t tinytc_binary_destroy(tinytc_binary_t bin) {
    return exception_to_status_code([&] { delete bin; });
}
}
