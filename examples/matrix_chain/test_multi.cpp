// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "test_multi.hpp"
#include "test_ader.hpp"
#include "test_volume.hpp"

#include <iostream>

using namespace sycl;

template <typename F> double bench(F f, int nrepeat = 10) {
    f();
    double min_exec_time_ns = std::numeric_limits<double>::max();
    for (int i = 0; i < nrepeat; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        f();
        auto end = std::chrono::high_resolution_clock::now();
        double exec_time_ns =
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        min_exec_time_ns = std::min(min_exec_time_ns, exec_time_ns);
    }
    return min_exec_time_ns;
}

template <typename T>
test_multi<T>::test_multi(std::int64_t N, std::int64_t P, std::int64_t howmany,
                          std::size_t alignment, test_case tc, std::vector<queue> const &q,
                          bool dump) {
    for (auto &qu : q) {
        switch (tc) {
        case test_case::ader:
            instances_.emplace_back(
                std::make_unique<test_ader<T>>(N, P, howmany, alignment, qu, dump));
            break;
        case test_case::volume:
            instances_.emplace_back(
                std::make_unique<test_volume<T>>(N, P, howmany, alignment, qu, dump));
            break;
        default:
            break;
        }
    }
}

template <typename T> void test_multi<T>::reference() {
    auto const chain_impl = [&]() {
        std::vector<event> evs;
        for (auto &instance : instances_) {
            auto ev = instance->reference();
            evs.insert(evs.end(), ev.begin(), ev.end());
        }
        for (auto &e : evs) {
            e.wait();
        }
    };
    auto min_exec_time_ns = bench([&]() { chain_impl(); });
    print_performance(min_exec_time_ns);
}

template <typename T> void test_multi<T>::optimized() {
    auto const chain_impl = [&]() {
        std::vector<event> evs;
        for (auto &instance : instances_) {
            auto ev = instance->optimized();
            evs.insert(evs.end(), ev.begin(), ev.end());
        }
        for (auto &e : evs) {
            e.wait();
        }
    };
    auto min_exec_time_ns = bench([&]() { chain_impl(); });
    print_performance(min_exec_time_ns);
}

template <typename T> bool test_multi<T>::check() {
    bool ok = true;
    for (auto &instance : instances_) {
        ok = ok && instance->check();
    }
    return ok;
}

template <typename T> void test_multi<T>::print_header() {
    std::cout << "precision,num_gpu,time,gflops,gflops_aligned,roofline_gflops,percentage_of_"
                 "roofline,bandwidth,roofline_required_bandwidth"
              << std::endl;
}

template <typename T> void test_multi<T>::print_performance(double time_ns) {
    std::int64_t flop = 0;
    std::int64_t flop_aligned = 0;
    std::int64_t bytes = 0;
    for (auto &instance : instances_) {
        flop += instance->flop();
        flop_aligned += instance->flop_aligned();
        bytes += instance->bytes();
    }

    auto const M = instances_.size();
    auto const gflops = flop / time_ns;
    auto const gflops_aligned = flop_aligned / time_ns;
    auto const bw = bytes / time_ns;
    auto const peak_flops = M * 1.6e9 * 512 * 32;
    auto const peak_bw = M * 1.1e12;
    auto const roofline_flops = std::min(peak_flops, flop / (bytes / peak_bw));
    auto const roofline_gflops = roofline_flops * 1e-9;
    auto const required_bw_rw = bytes / (flop / roofline_flops);
    std::cout << typeid(T).name() << "," << M << "," << time_ns / 1e9 << "," << gflops << ","
              << gflops_aligned << "," << roofline_gflops << ","
              << std::round(gflops / roofline_gflops * 100) << "%," << bw << ","
              << required_bw_rw * 1e-9 << std::endl;
}

template class test_multi<float>;
template class test_multi<double>;
