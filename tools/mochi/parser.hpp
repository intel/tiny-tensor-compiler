// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PARSER_20250611_HPP
#define PARSER_20250611_HPP

#include "objects.hpp"

#include <cstddef>
#include <optional>

namespace mochi {

auto parse_file(std::size_t input_size, char const *input, char const *filename = nullptr)
    -> std::optional<objects>;

} // namespace mochi

#endif // PARSER_20250611_HPP
