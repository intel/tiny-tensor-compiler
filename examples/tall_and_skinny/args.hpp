// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ARGS_20240215_HPP
#define ARGS_20240215_HPP

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
    bool double_precision;
    bool help;
    bool verify;
    double beta;
    bool specialize_M;
    bool specialize_ld;
    bool large_GRF;
};

class arg_parser {
  public:
    static args parse_args(int argc, char **argv);
    static void show_help(std::ostream &os);
};

#endif // ARGS_20240215_HPP
