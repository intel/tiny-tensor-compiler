// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef REQUIRED_EXTENSIONS_20240416_HPP
#define REQUIRED_EXTENSIONS_20240416_HPP

#include <vector>

#include <clir/func.hpp>
#include <clir/prog.hpp>

namespace tinytc {

auto required_extensions(clir::func f) -> std::vector<char const *>;
auto required_extensions(clir::prog p) -> std::vector<char const *>;

} // namespace tinytc

#endif // REQUIRED_EXTENSIONS_20240416_HPP
