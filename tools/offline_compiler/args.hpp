// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ARGS_20240516_HPP
#define ARGS_20240516_HPP

#include "tinytc/tinytc.hpp"

#include <cstdint>
#include <iosfwd>

struct args {
    char const *filename;
    tinytc::core_info info;
    bool help;
    std::int32_t opt_level;
};

class arg_parser {
  public:
    static args parse_args(int argc, char **argv);
    static void show_help(std::ostream &os);
};

#endif // ARGS_20240516_HPP
