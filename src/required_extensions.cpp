// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "required_extensions.hpp"

#include <clir/builtin_function.hpp>
#include <clir/visitor/required_extensions.hpp>

#include <utility>

namespace tinytc {

auto ext_list(std::vector<clir::extension> const &ext) -> std::vector<char const *> {
    auto result = std::vector<char const *>{};
    result.reserve(ext.size() + 1);
    for (auto const &e : ext) {
        result.emplace_back(clir::to_string(e));
    }
    result.emplace_back("cl_khr_fp64");
    return result;
}

auto required_extensions(clir::func f) -> std::vector<char const *> {
    return ext_list(clir::get_required_extensions(std::move(f)));
}
auto required_extensions(clir::prog p) -> std::vector<char const *> {
    return ext_list(clir::get_required_extensions(std::move(p)));
}

} // namespace tinytc
