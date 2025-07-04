// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PARSER_20230614_HPP
#define PARSER_20230614_HPP

#include "tinytc/types.hpp"

#include <cstdint>

namespace tinytc {
auto parse(std::uint64_t size, char const *input) -> shared_handle<tinytc_prog_t>;
}

#endif // PARSER_20230614_HPP
