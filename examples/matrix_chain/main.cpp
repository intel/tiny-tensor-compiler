// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "test.hpp"
#include "test_multi.hpp"

#include <argparser.hpp>
#include <sycl/sycl.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <utility>

using namespace sycl;
using namespace tinytc;

int main(int argc, char **argv) {
    bool dump = false;
    std::int64_t N = 5, P = 9, howmany;
    std::size_t alignment = 0;
    char precision = 's';
    test_case tc = test_case::volume;
    bool help = false;

    auto parser = cmd::arg_parser{};
    try {
        parser.set_short_opt('a', &alignment, "Alignment (in number of bytes)");
        parser.set_short_opt('d', &dump, "Dump IR to stdout");
        parser.set_short_opt('f', &precision, "Data type (s or d)").validator([](char f) {
            return f == 's' || f == 'd';
        });
        parser.set_short_opt('h', &help, "Show help");
        parser.set_short_opt('N', &N, "Polynomial degree").validator([](std::int64_t p) {
            return p > 0;
        });
        parser.set_short_opt('P', &P, "Number of quantities").validator([](std::int64_t n) {
            return n > 0;
        });
        parser.set_long_opt("help", &help, "Show help");
        parser.add_positional_arg("test_case", &tc, "Test case (volume or ader)", true)
            .converter([](char const *str, test_case &val) -> cmd::parser_status {
                if (strcmp(str, "volume") == 0) {
                    val = test_case::volume;
                } else if (strcmp(str, "ader") == 0) {
                    val = test_case::ader;
                } else {
                    return cmd::parser_status::invalid_argument;
                }
                return cmd::parser_status::success;
            });
        parser.add_positional_arg("howmany", &howmany, "Batch size", true)
            .validator([](std::int64_t h) { return h > 0; });

        parser.parse(argc, argv);
    } catch (std::exception const &e) {
        if (!help) {
            std::cerr << e.what() << std::endl;
        }
        parser.print_help(std::cout, "matrix_chain", "");
        return help ? 0 : -1;
    }
    if (help) {
        parser.print_help(std::cout, "matrix_chain", "");
        return 0;
    }

    auto devices = platform{}.get_devices();
    auto sub_devices = std::vector<device>{};
    for (auto &device : devices) {
        if (device.get_info<info::device::partition_max_sub_devices>() > 1) {
            auto subs =
                device.create_sub_devices<info::partition_property::partition_by_affinity_domain>(
                    info::partition_affinity_domain::next_partitionable);
            sub_devices.insert(sub_devices.end(), subs.begin(), subs.end());
        } else {
            sub_devices.push_back(device);
        }
    }
    auto q = std::vector<queue>();
    for (auto &device : sub_devices) {
        q.emplace_back(queue(device));
    }

    auto run_test_multi = [&](auto precision) {
        using T = decltype(precision);
        auto t = test_multi<T>(N, P, howmany, alignment, tc, q, dump);
        if (!t.check()) {
            std::cerr << "Result mismatch between reference and optimized!" << std::endl;
            // return;
        }
        t.print_header();
        t.reference();
        t.optimized();
    };
    if (precision == 's') {
        run_test_multi(1.0f);
    } else {
        run_test_multi(1.0);
    }

    return 0;
}
