// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "test.hpp"
#include "test_multi.hpp"

#include <sycl/sycl.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <utility>

using namespace sycl;

int main(int argc, char **argv) {
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

    std::int64_t N = 5, P = 9, howmany;
    std::size_t alignment = 0;
    char precision = 's';
    test_case tc = test_case::volume;

    if (argc < 5) {
        std::cerr << "Usage: matrix_chain <test_case> <N> <P> <howmany> [alignment] [s/d]"
                  << std::endl;
        return -1;
    }
    if (strcmp(argv[1], "volume") == 0) {
        tc = test_case::volume;
    } else if (strcmp(argv[1], "ader") == 0) {
        tc = test_case::ader;
    } else {
        std::cerr << "Unknown test case " << argv[1] << ". Available are: ader, volume."
                  << std::endl;
        return -1;
    }
    N = static_cast<std::int64_t>(std::atol(argv[2]));
    P = static_cast<std::int64_t>(std::atol(argv[3]));
    howmany = static_cast<std::int64_t>(std::atol(argv[4]));
    if (argc >= 6) {
        alignment = static_cast<std::size_t>(std::atol(argv[5]));
    }
    if (argc >= 7) {
        precision = argv[6][0];
        if (precision != 's' && precision != 'd') {
            std::cerr << "Precision must be single (s) or double (d)" << std::endl;
            return -1;
        }
    }
    auto run_test_multi = [&](auto precision) {
        using T = decltype(precision);
        auto t = test_multi<T>(N, P, howmany, alignment, tc, q);
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
