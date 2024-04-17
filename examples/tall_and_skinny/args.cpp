// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "args.hpp"

#include <cstdlib>
#include <cstring>
#include <ostream>
#include <stdexcept>
#include <string>

args arg_parser::parse_args(int argc, char **argv) {
    args a = {};
    a.beta = 0.0;
    auto num = std::vector<std::int64_t>(3);
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            auto const fail = [&]() {
                throw std::runtime_error("==> Error: unrecognized argument " +
                                         std::string(argv[i]));
            };
            if (std::strcmp(argv[i], "-h") == 0 || std::strcmp(argv[i], "--help") == 0) {
                a.help = true;
            } else if (std::strcmp(argv[i], "-v") == 0 || std::strcmp(argv[i], "--verify") == 0) {
                a.verify = true;
            } else if (i + 1 < argc) {
                if (std::strcmp(argv[i], "-b") == 0 || std::strcmp(argv[i], "--beta") == 0) {
                    ++i;
                    a.beta = atof(argv[i]);
                } else if (std::strcmp(argv[i], "-p") == 0 ||
                           std::strcmp(argv[i], "--precision") == 0) {
                    ++i;
                    if (argv[i][0] == 'd') {
                        a.double_precision = true;
                    } else if (argv[i][0] == 's') {
                        a.double_precision = false;
                    } else {
                        fail();
                    }
                } else {
                    fail();
                }
            } else {
                fail();
            }
        } else {
            num.clear();
            char const *delim = "x";
            auto arg = std::string(argv[i]);
            char *token = std::strtok(argv[i], delim);
            while (token) {
                num.emplace_back(atoi(token));
                token = std::strtok(nullptr, delim);
            }
            if (num.size() != 3) {
                throw std::runtime_error("==> Could not parse test case: " + arg);
            }
            a.tc.push_back({num[0], num[1], num[2]});
        }
    }

    return a;
}

void arg_parser::show_help(std::ostream &os) {
    os << "usage: tall_and_skinny test-case1 test-case2 ..." << std::endl
       << R"HELP(
positional arguments:
    test-caseN          MxNxK triplet (e.g. 300000x64x64)

optional arguments:
    -h, --help          Show help and quit
    -p, --precision     Precision (single = s, double = d)
    -v, --verify        Verify optimized implementation
)HELP";
}
