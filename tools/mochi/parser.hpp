// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PARSER_20250611_HPP
#define PARSER_20250611_HPP

#include "objects.hpp"

#include <optional>

namespace mochi {

auto parse_file(char const *filename) -> std::optional<objects>;

} // namespace mochi

#endif // PARSER_20250611_HPP
