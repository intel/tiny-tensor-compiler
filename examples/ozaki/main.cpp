// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "../gemm_common.hpp"
#include "memory.hpp"
#include "oz_int8.hpp"

#include <argparser.hpp>
#include <oneapi/mkl.hpp>
#include <sycl/sycl.hpp>
#include <tinytc/core.hpp>

#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>
#include <random>
#include <utility>
#include <vector>

using namespace sycl;
using namespace tinytc;

struct args {
    std::int64_t N = 4096;
    std::int64_t repeat = 10;
    std::int64_t s = 3;
    double phi = 0.1;
    bool no_check = false;
    examples::test_type ty = examples::test_type::f64;
};

template <typename T> void init_matrix(std::int64_t M, std::int64_t N, T *A, double phi, queue q) {
    std::random_device rd;
    auto gen = std::mt19937(rd());
    auto uniform = std::uniform_real_distribution<>(-0.5, 0.5);
    auto normal = std::normal_distribution<>(0.0, 1.0);
    auto vec = std::vector<T>(M * N);
    for (std::int64_t i = 0; i < M * N; ++i) {
        vec[i] = uniform(gen) * std::exp(phi * normal(gen));
    }
    q.copy(vec.data(), A, vec.size()).wait();
}

template <typename T> auto l2_rel_err(std::int64_t N, T *C, T *C_ref, queue q) {
    double err_sum = 0;
    auto err_sum_buf = buffer<double>{&err_sum, 1};
    double C_ref_sum = 0;
    auto C_ref_sum_buf = buffer<double>{&C_ref_sum, 1};
    q.submit([&](handler &cgh) {
         auto err_sum_reduction = reduction(err_sum_buf, cgh, plus<>());
         auto C_ref_sum_reduction = reduction(C_ref_sum_buf, cgh, plus<>());

         cgh.parallel_for(range<1>{static_cast<std::size_t>(N * N)}, err_sum_reduction,
                          C_ref_sum_reduction, [=](id<1> idx, auto &esum, auto &csum) {
                              double a = C[idx];
                              double b = C_ref[idx];
                              double d = a - b;
                              esum += d * d;
                              csum += b * b;
                          });
     }).wait();
    err_sum = err_sum_buf.get_host_access()[0];
    C_ref_sum = C_ref_sum_buf.get_host_access()[0];
    return std::sqrt(err_sum / C_ref_sum);
}

template <typename T> void test(queue q, args const &a) {
    auto A = malloc_device_unique<T>(a.N * a.N, q);
    auto B = malloc_device_unique<T>(a.N * a.N, q);
    auto C = malloc_device_unique<T>(a.N * a.N, q);

    const auto bench_print = [&](char const *name, auto &&run) {
        auto C_tc = malloc_device_unique<T>(a.N * a.N, q);
        run(A.get(), B.get(), C_tc.get());

        double err = 0.0;
        if (!a.no_check) {
            err = l2_rel_err(a.N, C_tc.get(), C.get(), q);
        }
        double min_exec_time_ns =
            examples::bench([&]() { run(A.get(), B.get(), C_tc.get()); }, a.repeat);

        auto gflops = 2 * a.N * a.N * a.N / min_exec_time_ns;
        std::cout << to_string(a.ty) << "," << name << "," << a.N << "," << a.s << ","
                  << min_exec_time_ns / 1e6 << "," << gflops << "," << err << std::endl;
    };

    try {
        init_matrix(a.N, a.N, A.get(), a.phi, q);
        init_matrix(a.N, a.N, B.get(), a.phi, q);

        auto oz = oz_int8(a.ty, a.N, a.s, q);

        oz.ref(A.get(), B.get(), C.get());

        std::cout << "precision,impl,N,num_splits,time_ms,gflops,l2_rel_err" << std::endl;

        bench_print("gemm", [&](T *A, T *B, T *C) {
            oz.ref(A, B, C);
            q.wait();
        });

        bench_print("oz_int8", [&](T *A, T *B, T *C) {
            oz(A, B, C);
            q.wait();
        });
    } catch (status const &st) {
        std::cerr << "Error (" << static_cast<int>(st) << "): " << to_string(st) << std::endl;
    } catch (std::exception const &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

int main(int argc, char **argv) {
    auto a = args{};
    bool help = false;

    auto parser = cmd::arg_parser{};
    try {
        parser.set_short_opt('N', &a.N, "A,B,C matrix size (NxN)");
        parser.set_short_opt('r', &a.repeat, "Number of repeats");
        parser.set_short_opt('f', &a.ty, "Data type (f32, f64)")
            .converter(examples::convert_data_type)
            .validator([](examples::test_type const &t) {
                return t == examples::test_type::f32 || t == examples::test_type::f64;
            });
        parser.set_short_opt('s', &a.s, "Number of splits");
        parser.set_short_opt('h', &help, "Show help");
        parser.set_long_opt("phi", &a.phi, "Tweak exponent range");
        parser.set_long_opt("no-check", &a.no_check, "Skip error checking");
        parser.set_long_opt("help", &help, "Show help");

        parser.parse(argc, argv);
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    if (help) {
        parser.print_help(std::cout, "ozaki",
                          "NxNxN DGEMM using Ozaki/Ootomo scheme on INT8 tensor cores");
        return !help ? -1 : 0;
    }

    if (a.N % 256 != 0) {
        std::cerr << "Boundary check is not implemented; N must be divisible by 256" << std::endl;
        return -1;
    }

    auto q = queue{property::queue::in_order()};

    try {
        if (a.ty == examples::test_type::f64) {
            test<double>(q, a);
        } else {
            test<float>(q, a);
        }
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}
