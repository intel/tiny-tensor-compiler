// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "args.hpp"

#include <sycl/sycl.hpp>
#include <tinytc/tinytc.hpp>
#include <tinytc/tinytc_sycl.hpp>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>

using namespace sycl;
using namespace tinytc;

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

template <typename T> void test(queue q, args &a) {
    auto const fill = [](std::vector<T> &x) {
        for (std::size_t i = 0; i < x.size(); ++i) {
            x[i] = i % 101;
        }
    };
    std::int64_t na_max = 0;
    std::int64_t nb_max = 0;
    std::int64_t nc_max = 0;
    for (auto &c : a.tc) {
        na_max = std::max(na_max, c.m * c.k);
        nb_max = std::max(nb_max, c.k * c.n);
        nc_max = std::max(nc_max, c.m * c.n);
    }
    auto A_host = std::vector<T>(na_max);
    auto B_host = std::vector<T>(nb_max);
    auto C_host = std::vector<T>(nc_max);
    auto C_ref_host = std::vector<T>(nc_max);
    T *C_ref = malloc_device<T>(nc_max, q);
    T *A = malloc_device<T>(na_max, q);
    T *B = malloc_device<T>(nb_max, q);
    T *C = malloc_device<T>(nc_max, q);
    fill(A_host);
    fill(B_host);
    q.copy(A_host.data(), A, na_max).wait();
    q.copy(B_host.data(), B, nb_max).wait();

    auto const check = [&](std::int64_t M, std::int64_t N) {
        q.copy(C_ref, C_ref_host.data(), M * N).wait();
        q.copy(C, C_host.data(), M * N).wait();
        std::size_t num_err = 0;
        for (std::int64_t i = 0; i < M * N; ++i) {
            auto err = std::abs(C_host[i] - C_ref_host[i]);
            if (err > 10.0 * std::numeric_limits<T>::epsilon()) {
                if (num_err < 10) {
                    std::cout << i << " " << err << " " << C_host[i] << " " << C_ref_host[i]
                              << std::endl;
                }
                ++num_err;
            }
        }
        if (num_err > 10) {
            std::cout << "and " << num_err - 10 << " further errors." << std::endl;
        }
    };

    auto const &type = typeid(T);
    for (auto &c : a.tc) {
        if (a.verify) {
            q.memset(C, 0, c.m * c.n * sizeof(T)).wait();
            q.memset(C_ref, 0, c.m * c.n * sizeof(T)).wait();
            q.submit([&](auto &h) {
                 auto beta = a.beta;
                 h.parallel_for(range{static_cast<std::size_t>(c.n), static_cast<std::size_t>(c.m)},
                                [=](id<2> it) {
                                    auto m = it[1];
                                    auto n = it[0];
                                    auto c_acc = T(0.0);
                                    for (std::int64_t k = 0; k < c.k; ++k) {
                                        c_acc += A[m + k * c.m] * B[k + n * c.k];
                                    }
                                    C_ref[m + n * c.m] = c_acc + T(beta) * C_ref[m + n * c.m];
                                });
             }).wait();
        }

        auto source_ctx = make_source_context();
        try {
            auto info = make_core_info(q.get_device());
            info.set_core_features(tinytc_core_feature_flag_large_register_file);

            auto tas = make_recipe_handler(
                q, make_tall_and_skinny(info, to_scalar_type_v<T>, c.n, c.k, 0, source_ctx));

            tall_and_skinny::set_args(tas, c.m, T(1.0), A, c.m, B, c.k, T(a.beta), C, c.m);
            tas.submit(q).wait();
            if (a.verify) {
                check(c.m, c.n);
            }
            double min_exec_time_ns = bench([&]() { tas.submit(q).wait(); });

            auto bw_C_factor = a.beta != 0.0 ? 2 : 1;
            auto bw =
                sizeof(T) * (c.m * c.n * bw_C_factor + c.m * c.k + c.k * c.n) / min_exec_time_ns;
            auto gflops = 2 * c.m * c.n * c.k / min_exec_time_ns;
            std::cout << type.name() << "," << c.m << "," << c.n << "," << c.k << "," << a.beta
                      << "," << min_exec_time_ns / 1e9 << "," << bw << "," << gflops << std::endl;
        } catch (status const &st) {
            std::cerr << "Error (" << static_cast<int>(st) << "): " << tinytc::error_string(st)
                      << std::endl;
            if (source_ctx.get_error_log()[0] != '\0') {
                std::cerr << "Error log: " << std::endl << source_ctx.get_error_log() << std::endl;
            }
        } catch (std::exception const &e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    free(A, q);
    free(B, q);
    free(C, q);
    free(C_ref, q);
};

int main(int argc, char **argv) {
    auto a = args{};
    try {
        a = arg_parser::parse_args(argc, argv);
    } catch (std::runtime_error const &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    if (a.help || a.tc.empty()) {
        arg_parser::show_help(std::cout);
        return 0;
    }

    auto q = queue{};

    std::cout << "precision,m,n,k,beta,time,bandwidth,gflops" << std::endl;
    try {
        if (a.double_precision) {
            test<double>(std::move(q), a);
        } else {
            test<float>(std::move(q), a);
        }
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}
