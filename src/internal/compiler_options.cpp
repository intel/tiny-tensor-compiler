// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/internal/compiler_options.hpp"

#include <clir/builtin_function.hpp>
#include <clir/func.hpp>
#include <clir/prog.hpp>
#include <clir/visitor/required_extensions.hpp>

#include <utility>

namespace tinytc::internal {

const std::vector<std::string> default_compiler_options{"-cl-std=CL2.0", "-cl-mad-enable"};
const char *large_register_file_compiler_option_ze = "-ze-opt-large-register-file";
const char *large_register_file_compiler_option_cl = "-cl-intel-256-GRF-per-thread";

std::vector<std::string> ext_list(std::vector<clir::extension> const &ext) {
    auto result = std::vector<std::string>{};
    result.reserve(ext.size() + 1);
    for (auto const &e : ext) {
        result.emplace_back(clir::to_string(e));
    }
    result.emplace_back("cl_khr_fp64");
    return result;
}

std::vector<std::string> required_extensions(clir::func f) {
    return ext_list(clir::get_required_extensions(std::move(f)));
}
std::vector<std::string> required_extensions(clir::prog p) {
    return ext_list(clir::get_required_extensions(std::move(p)));
}

} // namespace tinytc::internal
