// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

// Code COPIED from Double-Batched FFT Library
// Copyright (C) 2022 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "opencl_cc.hpp"

#include "ocloc_api.h"

#include <cstdio>
#include <cstring>
#include <limits>
#include <sstream>

namespace tinytc {

std::vector<std::uint8_t> compile_opencl_c(std::size_t source_length, char const *source_text,
                                           bundle_format format, std::uint32_t ip_version,
                                           std::uint32_t options_size, char const *const *options,
                                           std::uint32_t extensions_size,
                                           char const *const *extensions) {
    auto const format_options = [](std::uint32_t options_size,
                                   char const *const *options) -> std::string {
        if (options_size == 0) {
            return {};
        }
        auto oss = std::ostringstream{};
        std::uint32_t opt_it = 0;
        oss << options[opt_it++];
        for (; opt_it < options_size; ++opt_it) {
            oss << " " << options[opt_it];
        }
        return oss.str();
    };
    auto const format_ext_list = [](std::uint32_t extensions_size,
                                    char const *const *extensions) -> std::string {
        if (extensions_size == 0) {
            return {};
        }
        auto oss = std::ostringstream{};
        std::uint32_t ext_it = 0;
        oss << "-cl-ext=+" << extensions[ext_it++];
        for (; ext_it < extensions_size; ++ext_it) {
            oss << ",+" << extensions[ext_it];
        }
        return oss.str();
    };
    unsigned int num_args = 2;
    constexpr unsigned int max_num_args = 11;
    char const *argv[max_num_args] = {"ocloc", "compile"};
    auto const ext_list = format_ext_list(extensions_size, extensions);
    if (!ext_list.empty()) {
        argv[num_args++] = "-internal_options";
        argv[num_args++] = ext_list.c_str();
    }
    auto const cl_options = format_options(options_size, options);
    if (!cl_options.empty()) {
        argv[num_args++] = "-options";
        argv[num_args++] = cl_options.c_str();
    }
    char device[16];
    snprintf(device, sizeof(device), "%d", ip_version);
    if (ip_version != 0) {
        argv[num_args++] = "-device";
        argv[num_args++] = device;
    }
    if (format == bundle_format::spirv) {
        argv[num_args++] = "-spv_only";
    }
    argv[num_args++] = "-file";
    argv[num_args++] = "kernel.cl";

    const std::uint32_t num_sources = 1;
    const std::uint8_t *data_sources = reinterpret_cast<const std::uint8_t *>(source_text);
    const std::uint64_t len_sources = source_length + 1;
    char const *name_sources = argv[num_args - 1];
    std::uint32_t num_input_headers = 0;
    std::uint32_t num_outputs = 0;
    std::uint8_t **data_outputs = nullptr;
    std::uint64_t *len_outputs = nullptr;
    char **name_outputs = nullptr;
    oclocInvoke(num_args, argv, num_sources, &data_sources, &len_sources, &name_sources,
                num_input_headers, nullptr, nullptr, nullptr, &num_outputs, &data_outputs,
                &len_outputs, &name_outputs);

    auto const ends_with = [](char const *str, char const *ending) {
        auto lstr = strlen(str);
        auto lend = strlen(ending);
        if (lend > lstr) {
            return false;
        }
        return strncmp(str + (lstr - lend), ending, lend) == 0;
    };

    constexpr std::uint32_t invalid_index = std::numeric_limits<std::uint32_t>::max();
    std::uint32_t log_file = invalid_index;
    std::uint32_t bin_file = invalid_index;
    for (std::uint32_t o = 0; o < num_outputs; ++o) {
        if (strcmp(name_outputs[o], "stdout.log") == 0) {
            log_file = o;
        } else if (format == bundle_format::spirv && ends_with(name_outputs[o], ".spv")) {
            bin_file = o;
        } else if (format == bundle_format::native &&
                   (ends_with(name_outputs[o], ".bin") || ends_with(name_outputs[o], ".ar"))) {
            bin_file = o;
        }
    }
    if (bin_file == invalid_index) {
        if (log_file != invalid_index) {
            char *log_ptr = reinterpret_cast<char *>(data_outputs[log_file]);
            auto log = std::string(log_ptr, len_outputs[log_file]);
            throw opencl_c_compilation_error(std::move(log));
        }
        throw opencl_c_compilation_error("OpenCL-C compilation failed (no log available)");
    }

    auto result = std::vector<std::uint8_t>(data_outputs[bin_file],
                                            data_outputs[bin_file] + len_outputs[bin_file]);
    oclocFreeOutput(&num_outputs, &data_outputs, &len_outputs, &name_outputs);
    return result;
}

} // namespace tinytc
