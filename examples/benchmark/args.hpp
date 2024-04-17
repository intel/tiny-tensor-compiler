// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ARGS_20230417_HPP
#define ARGS_20230417_HPP

#include <tinytc/types.hpp>

#include <cstdint>
#include <iosfwd>
#include <vector>

struct test_case {
    std::int64_t m;
    std::int64_t n;
    std::int64_t k;
};

struct args {
    std::vector<test_case> tc;
    int internal_repetitions;
    bool double_precision;
    bool help;
    tinytc::transpose transA;
    tinytc::transpose transB;
    bool verify;
};

class arg_parser {
  public:
    static args parse_args(int argc, char **argv);
    static void show_help(std::ostream &os);
};

#endif // ARGS_20230417_HPP
