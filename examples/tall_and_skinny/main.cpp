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
    std::int32_t alignment = 0;
    std::int32_t M_block_size = 0;
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
    auto const alloc_device = [&a, &q](std::size_t num_bytes) {
        if (a.alignment == 0) {
            return malloc_device<T>(num_bytes, q);
        } else {
            return aligned_alloc_device<T>(a.alignment, num_bytes, q);
        }
    };
    T *A = alloc_device(na_max);
    T *B = alloc_device(nb_max);
    T *C = alloc_device(nc_max);

    auto const check = [&](std::int64_t M, std::int64_t N, std::int64_t K) {
        q.copy(C, C_host.data(), M * N).wait();
        std::size_t num_err = 0;
        const auto error_bound = examples::test_gemm_error_bound<T>(K);
        for (std::int64_t j = 0; j < N; ++j) {
            for (std::int64_t i = 0; i < M; ++i) {
                const auto relerr = examples::test_gemm_rel_error<T>(C_host.data(), i, j, M);
                if (relerr > error_bound) {
                    if (num_err < 10) {
                        std::cout << "C_{" << i << "," << j << "}=" << C_host[i + j * M]
                                  << ", relative_error=" << relerr
                                  << ", error_bound=" << error_bound << std::endl;
                    }
                    ++num_err;
                }
            }
        }
        if (num_err > 10) {
            std::cout << "and " << num_err - 10 << " further errors." << std::endl;
        }
    };

    for (auto &c : a.tc) {
        examples::test_gemm_matrix<T, matrix_use::a>(A_host.data(), c.m, c.k);
        examples::test_gemm_matrix<T, matrix_use::b>(B_host.data(), c.k, c.n);
        q.copy(A_host.data(), A, c.m * c.k).wait();
        q.copy(B_host.data(), B, c.k * c.n).wait();
        q.memset(C, 0, c.m * c.n * sizeof(T)).wait();

        auto beta = a.update ? T{1} : T{0};
        try {
            auto info = make_core_info(q.get_device());
            set_core_features(info, tinytc_core_feature_flag_large_register_file);

            std::int64_t M = a.specialize_M ? c.m : dynamic;
            std::int64_t ldA = dynamic, ldB = dynamic, ldC = dynamic;
            if (a.specialize_ld) {
                ldA = c.m;
                ldB = c.k;
                ldC = c.m;
            }
            auto ctx = make_compiler_context();
            set_error_reporter(ctx, [](char const *what, const tinytc_location_t *, void *) {
                std::cerr << what << std::endl;
            });
            auto r = make_tall_and_skinny_specialized(info, a.ty, M, c.n, c.k, ldA, ldB, ldC,
                                                      a.alignment, a.alignment, a.alignment,
                                                      a.M_block_size, ctx);
            if (a.dump) {
                dump(get_prog(r));
            }
            auto tas = make_recipe_handler(q, r);

            tall_and_skinny::set_args(tas, c.m, T{1}, mem(A, mem_type::usm_pointer), c.m,
                                      mem(B, mem_type::usm_pointer), c.k, beta,
                                      mem(C, mem_type::usm_pointer), c.m);
            tas.submit(q).wait();
            if (a.verify) {
                check(c.m, c.n, c.k);
            }
            double min_exec_time_ns = bench([&]() { tas.submit(q).wait(); });

            const auto ops_per_mnk = [&] {
                switch (a.ty) {
                case scalar_type::c32:
                case scalar_type::c64:
                    return 8;
                default:
                    return 2;
                }
            }();

            auto bw_C_factor = a.update ? 2 : 1;
            auto bw =
                sizeof(T) * (c.m * c.n * bw_C_factor + c.m * c.k + c.k * c.n) / min_exec_time_ns;
            auto gflops = ops_per_mnk * c.m * c.n * c.k / min_exec_time_ns;
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
};

int main(int argc, char **argv) {
    auto a = args{};
    bool help = false;

    auto parser = cmd::arg_parser{};
    try {
        parser.set_short_opt('a', &a.alignment, "Override memory alignment");
        parser.set_short_opt('d', &a.dump, "Dump IR to stdout");
        parser.set_short_opt('f', &a.ty, "Data type (f32, f64, c32, c64)")
            .converter(examples::convert_data_type);
        parser.set_short_opt('h', &help, "Show help");
        parser.set_short_opt('u', &a.update,
                             "Add A*B to C (beta=1) instead of overwriting C (beta=0)");
        parser.set_short_opt('v', &a.verify, "Verify optimized implementation");
        parser.set_long_opt("help", &help, "Show help");
        parser.set_long_opt("m-block-size", &a.M_block_size,
                            "Set block size for M mode (one work-group per block)");
        parser.set_long_opt("specialize-m", &a.specialize_M,
                            "Specialize M instead of using dynamic value");
        parser.set_long_opt("specialize-ld", &a.specialize_ld,
                            "Specialize ldA, ldB, ldC instead of using dynamic value");
        parser.add_positional_arg("test-case", &a.tc, "MxNxK triplet (e.g. 300000x64x64)")
            .converter(examples::convert_test_case)
            .validator(examples::validate_test_case);

        parser.parse(argc, argv);
    } catch (std::exception const &e) {
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
        case scalar_type::bf16:
            test<tinytc::bfloat16>(std::move(q), a);
            break;
        case scalar_type::f16:
            test<tinytc::half>(std::move(q), a);
            break;
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
