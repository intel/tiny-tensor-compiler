// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "args.hpp"

#include <cstdlib>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <string>

args arg_parser::parse_args(int argc, char **argv) {
    args a = {};
    a.internal_repetitions = 1;
    a.ty = tinytc::scalar_type::f32;
    a.transA = tinytc::transpose::N;
    a.transB = tinytc::transpose::N;
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
            } else if (std::strcmp(argv[i], "--trans-a") == 0) {
                a.transA = tinytc::transpose::T;
            } else if (std::strcmp(argv[i], "--trans-b") == 0) {
                a.transB = tinytc::transpose::T;
            } else if (std::strcmp(argv[i], "-v") == 0 || std::strcmp(argv[i], "--verify") == 0) {
                a.verify = true;
            } else if (std::strcmp(argv[i], "-a") == 0 || std::strcmp(argv[i], "--atomic") == 0) {
                a.atomic = true;
            } else if (i + 1 < argc) {
                if (std::strcmp(argv[i], "-i") == 0 ||
                    std::strcmp(argv[i], "--internal-reps") == 0) {
                    a.internal_repetitions = atoi(argv[++i]);
                } else if (std::strcmp(argv[i], "-b") == 0 || std::strcmp(argv[i], "--beta") == 0) {
                    ++i;
                    a.beta = atof(argv[i]);
                } else if (std::strcmp(argv[i], "-p") == 0 ||
                           std::strcmp(argv[i], "--precision") == 0) {
                    ++i;
                    if (argv[i][0] == 'd' || strcmp(argv[i], "f64") == 0) {
                        a.ty = tinytc::scalar_type::f64;
                    } else if (argv[i][0] == 's' || strcmp(argv[i], "f32") == 0) {
                        a.ty = tinytc::scalar_type::f32;
                    } else if (strcmp(argv[i], "c64") == 0) {
                        a.ty = tinytc::scalar_type::c64;
                    } else if (strcmp(argv[i], "c32") == 0) {
                        a.ty = tinytc::scalar_type::c32;
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
    os << "usage: tinytcbench test-case1 test-case2 ..." << std::endl
       << R"HELP(
positional arguments:
    test-caseN          MxNxK triplet (e.g. 64x64x64)

optional arguments:
    -h, --help          Show help and quit
    -i, --internal-reps Number of GEMM repetitions inside kernel (default: 1)
    -p, --precision     Precision (single = s or f32, double = d or f64, complex = c32, long complex = c64)
    --trans-a           Transpose A matrix
    --trans-b           Transpose B matrix
    -v, --verify        Verify optimized implementation
    -a, --atomic        Update C atomically
)HELP";
}
