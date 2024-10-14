// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "../gemm_common.hpp"

#include <argparser.hpp>
#include <sycl/sycl.hpp>
#include <tinytc/tinytc.hpp>
#include <tinytc/tinytc_sycl.hpp>

#include <algorithm>
#include <chrono>
#include <complex>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>

using namespace sycl;
using namespace tinytc;

struct args {
    bool dump = false;
    bool specialize_M = false;
    bool specialize_ld = false;
    scalar_type ty = scalar_type::f32;
    bool update = false;
    bool verify = false;
    std::vector<examples::test_case> tc;
};

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
            if (err > 10.0 * std::numeric_limits<decltype(err)>::epsilon()) {
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

    for (auto &c : a.tc) {
        auto beta = a.update ? T{1} : T{0};
        if (a.verify) {
            q.memset(C, 0, c.m * c.n * sizeof(T)).wait();
            q.memset(C_ref, 0, c.m * c.n * sizeof(T)).wait();
            q.submit([&](auto &h) {
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

        try {
            auto info = make_core_info(q.get_device());
            info.set_core_features(tinytc_core_feature_flag_large_register_file);

            std::int64_t M = a.specialize_M ? c.m : dynamic;
            std::int64_t ldA = dynamic, ldB = dynamic, ldC = dynamic;
            if (a.specialize_ld) {
                ldA = c.m;
                ldB = c.k;
                ldC = c.m;
            }
            auto r = make_tall_and_skinny_specialized(info, a.ty, M, c.n, c.k, ldA, ldB, ldC, 0);
            if (a.dump) {
                r.get_prog().dump();
            }
            auto tas = make_recipe_handler(q, r);

            tall_and_skinny::set_args(tas, c.m, T{1}, A, c.m, B, c.k, beta, C, c.m);
            tas.submit(q).wait();
            if (a.verify) {
                check(c.m, c.n);
            }
            double min_exec_time_ns = bench([&]() { tas.submit(q).wait(); });

            auto bw_C_factor = a.update ? 2 : 1;
            auto bw =
                sizeof(T) * (c.m * c.n * bw_C_factor + c.m * c.k + c.k * c.n) / min_exec_time_ns;
            auto gflops = 2 * c.m * c.n * c.k / min_exec_time_ns;
            std::cout << to_string(a.ty) << "," << c.m << "," << c.n << "," << c.k << ","
                      << a.update << "," << min_exec_time_ns / 1e9 << "," << bw << "," << gflops
                      << std::endl;
        } catch (status const &st) {
            std::cerr << "Error (" << static_cast<int>(st) << "): " << tinytc::error_string(st)
                      << std::endl;
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
    bool help = false;

    auto parser = cmd::arg_parser{};
    try {
        parser.set_short_opt('d', &a.dump, "Dump IR to stdout");
        parser.set_short_opt('f', &a.ty, "Data type (f32, f64, c32, c64)")
            .converter(examples::convert_data_type);
        parser.set_short_opt('h', &help, "Show help");
        parser.set_short_opt('u', &a.update,
                             "Add A*B to C (beta=1) instead of overwriting C (beta=0)");
        parser.set_short_opt('v', &a.verify, "Verify optimized implementation");
        parser.set_long_opt("help", &help, "Show help");
        parser.set_long_opt("specialize-m", &a.specialize_M,
                            "Specialize M instead of using dynamic value");
        parser.set_long_opt("specialize-ld", &a.specialize_ld,
                            "Specialize ldA, ldB, ldC instead of using dynamic value");
        parser.add_positional_arg("test-case", &a.tc, "MxNxK triplet (e.g. 300000x64x64)")
            .converter(examples::convert_test_case)
            .validator(examples::validate_test_case);

        parser.parse(argc, argv);
    } catch (std::runtime_error const &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    if (help || a.tc.empty()) {
        parser.print_help(std::cout, "tall_and_skinny", "");
        return !help ? -1 : 0;
    }

    auto q = queue{};

    std::cout << "precision,m,n,k,update,time,bandwidth,gflops" << std::endl;
    try {
        switch (a.ty) {
        case scalar_type::f32:
            test<float>(std::move(q), a);
            break;
        case scalar_type::f64:
            test<double>(std::move(q), a);
            break;
        case scalar_type::c32:
            test<std::complex<float>>(std::move(q), a);
            break;
        case scalar_type::c64:
            test<std::complex<double>>(std::move(q), a);
            break;
        default:
            return -1;
        }
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}
