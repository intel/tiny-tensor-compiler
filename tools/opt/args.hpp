// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ARGS_20240911_HPP
#define ARGS_20240911_HPP

#include "tinytc/tinytc.hpp"

#include <iosfwd>
#include <string>
#include <vector>

struct args {
    std::vector<std::string> pass_names;
    char const *filename;
    tinytc::core_info info;
    bool help;
};

class arg_parser {
  public:
    static args parse_args(int argc, char **argv);
    static void show_help(std::ostream &os);
};

#endif // ARGS_20240911_HPP
