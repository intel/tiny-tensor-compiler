// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef COMPILER_OPTIONS_20230621_HPP
#define COMPILER_OPTIONS_20230621_HPP

#include <array>

namespace tinytc {

[[maybe_unused]] constexpr std::array default_compiler_options = {"-cl-std=CL2.0",
                                                                  "-cl-mad-enable"};

constexpr char large_register_file_compiler_option_ze[] = "-ze-opt-large-register-file";
constexpr char large_register_file_compiler_option_cl[] = "-cl-intel-256-GRF-per-thread";

} // namespace tinytc

#endif // COMPILER_OPTIONS_20230621_HPP
