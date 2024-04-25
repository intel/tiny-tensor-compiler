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
#include <source_location>
#include <sstream>
#include <stdexcept>

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

auto gemm_kernel_with_inner_repetition(scalar_type ty, transpose tA, transpose tB, std::int64_t M,
                                       std::int64_t N, std::int64_t K,
                                       std::array<std::int64_t, 2> A_stride,
                                       std::array<std::int64_t, 2> B_stride,
                                       std::array<std::int64_t, 2> C_stride,
                                       std::uint32_t repetitions, queue q) -> binary {
    auto ctx = create_source_context();
    char const *file_name = std::source_location::current().file_name();
    auto const source_id = ctx.add_source(file_name, "");

    auto const my_loc = [&](std::source_location const loc = std::source_location::current()) {
        auto l = location{};
        l.begin.source_id = source_id;
        l.begin.line = loc.line();
        l.begin.column = loc.column();
        l.end = l.begin;
        ++l.end.column;
        return l;
    };

    auto kernel = [&](function_builder &fb) {
        auto A = fb.argument(
            create_group(create_memref(
                ty, {M, K}, std::vector<std::int64_t>(A_stride.begin(), A_stride.end()), my_loc())),
            "A", my_loc());
        auto B = fb.argument(
            create_group(create_memref(
                ty, {K, N}, std::vector<std::int64_t>(B_stride.begin(), B_stride.end()), my_loc())),
            "B", my_loc());
        auto C = fb.argument(
            create_group(create_memref(
                ty, {M, N}, std::vector<std::int64_t>(C_stride.begin(), C_stride.end()), my_loc())),
            "C", my_loc());
        fb.body(
            [&](region_builder &bb) {
                auto gid = bb.add(create_group_id(my_loc()));
                auto a = bb.add(create_load(A, {gid}, my_loc()));
                auto b = bb.add(create_load(B, {gid}, my_loc()));
                auto c = bb.add(create_load(C, {gid}, my_loc()));
                bb.create_for(
                    scalar_type::index, value(0u, my_loc()), value(repetitions, my_loc()),
                    [&](region_builder &bb) {
                        bb.add(create_gemm(tA, tB, false, value(1.0, ty, my_loc()), a, b,
                                           value(0.0, ty, my_loc()), c, my_loc()));
                    },
                    "r", my_loc());
            },
            my_loc());
    };

    try {
        auto pb = program_builder{};
        pb.create("gemm", kernel, my_loc());
        auto p = pb.get_product(my_loc());

        auto info = create_core_info(q.get_device());
        info.set_core_feature(core_feature_flag::large_register_file);
        return compile_to_binary(p, info, bundle_format::native, ctx);
    } catch (builder_error const &e) {
        ctx.report_error(e.loc(), e.what());
        std::cerr << "Error  (" << static_cast<int>(e.code()) << "): " << std::endl
                  << ctx.get_error_log() << std::endl;
    } catch (status const &st) {
        std::cerr << "Error (" << static_cast<int>(st) << "): " << error_string(st) << std::endl
                  << "Error log:" << std::endl
                  << ctx.get_error_log() << std::endl;
    }
    return nullptr;
}

template <typename T> void test(queue q, args &a) {
    auto const fill = [](T *ptr, std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            ptr[i] = i % 101;
        }
    };
    auto total_reals = 1024 * 1024 * 1024 / sizeof(T);
    T *A_host = new T[total_reals];
    T *B_host = new T[total_reals];
    T *C_host = new T[total_reals];
    T *C_ref_host = new T[total_reals];
    T *C_ref = malloc_device<T>(total_reals, q);
    T *A = malloc_device<T>(total_reals, q);
    T *B = malloc_device<T>(total_reals, q);
    T *C = malloc_device<T>(total_reals, q);
    fill(A_host, total_reals);
    fill(B_host, total_reals);
    q.copy(A_host, A, total_reals).wait();
    q.copy(B_host, B, total_reals).wait();

    auto const check = [&](std::int64_t M, std::int64_t N, std::size_t howmany) {
        q.copy(C_ref, C_ref_host, total_reals).wait();
        q.copy(C, C_host, total_reals).wait();
        std::size_t num_err = 0;
        for (std::size_t i = 0; i < M * N * howmany; ++i) {
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
        auto na = c.m * c.k;
        auto nb = c.k * c.n;
        auto nc = c.m * c.n;
        auto max_reals = std::max(std::max(na, nb), nc);
        auto howmany = total_reals / max_reals;

        if (a.verify && a.internal_repetitions == 1) {
            q.submit([&](auto &h) {
                bool transa = a.transA == transpose::T;
                bool transb = a.transB == transpose::T;
                h.parallel_for(range{howmany, 32}, [=](id<2> it) {
                    auto batch = it[0];
                    auto m = it[1];
                    auto a = A + batch * na;
                    auto b = B + batch * nb;
                    auto c_ref = C_ref + batch * nc;
                    for (std::int64_t mb = m; mb < c.m; mb += 32) {
                        for (std::int64_t n = 0; n < c.n; ++n) {
                            auto c_acc = 0.0f;
                            for (std::int64_t k = 0; k < c.k; ++k) {
                                c_acc += a[transa ? k + mb * c.k : mb + k * c.m] *
                                         b[transb ? n + k * c.n : k + n * c.k];
                            }
                            c_ref[mb + n * c.m] = c_acc;
                        }
                    }
                });
            });
        }

        T const **AA = malloc_shared<T const *>(howmany, q);
        T const **BB = malloc_shared<T const *>(howmany, q);
        T **CC = malloc_shared<T *>(howmany, q);
        for (std::size_t i = 0; i < howmany; ++i) {
            AA[i] = A + i * na;
            BB[i] = B + i * nb;
            CC[i] = C + i * nc;
        }

        double min_exec_time_ns = 0.0;
        try {
            auto bin = gemm_kernel_with_inner_repetition(
                to_scalar_type_v<T>, a.transA, a.transB, c.m, c.n, c.k,
                {1, a.transA == transpose::T ? c.k : c.m},
                {1, a.transB == transpose::T ? c.n : c.k}, {1, c.m}, a.internal_repetitions, q);
            if (bin) {
                // auto bundle = tensor_kernel_bundle(std::move(bin), q.get_context(),
                // q.get_device()); auto kernel = bundle.get("gemm"); kernel.set_args(AA, BB, CC);
                // kernel.submit(howmany, q).wait();
                auto bundle = create_kernel_bundle(q.get_context(), q.get_device(), bin);
                auto kernel = create_kernel(bundle, "gemm");
                auto exe_range = get_execution_range(kernel, howmany);
                q.submit([&](handler &h) {
                     h.set_args(AA, BB, CC);
                     h.parallel_for(exe_range, kernel);
                 }).wait();
                if (a.internal_repetitions == 1 && a.verify) {
                    check(c.m, c.n, howmany);
                }
                min_exec_time_ns = bench([&]() {
                    q.submit([&](handler &h) {
                         h.set_args(AA, BB, CC);
                         h.parallel_for(exe_range, kernel);
                     }).wait();
                });

                auto gflops =
                    a.internal_repetitions * 2 * c.m * c.n * c.k * howmany / min_exec_time_ns;
                auto roofline_gflops =
                    std::min(512 * 32 * 1.6e9, a.internal_repetitions * 2 * c.m * c.n * c.k /
                                                   (sizeof(T) * (na + nb + nc) / 1.1e12)) /
                    1e9;
                std::cout << type.name() << "," << c.m << "," << c.n << "," << c.k << "," << howmany
                          << "," << min_exec_time_ns / 1e9 << "," << gflops << ","
                          << roofline_gflops << "," << std::round(gflops / roofline_gflops * 100)
                          << "%," << a.internal_repetitions << std::endl;
            }
        } catch (status const &st) {
            std::cerr << "Error: " << error_string(st) << std::endl;
        } catch (std::exception const &e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }

        free(AA, q);
        free(BB, q);
        free(CC, q);
    }

    free(A, q);
    free(B, q);
    free(C, q);
    free(C_ref, q);
    delete[] A_host;
    delete[] B_host;
    delete[] C_host;
    delete[] C_ref_host;
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

    std::cout << "precision,m,n,k,howmany,time,gflops,roofline_gflops,roofline_perc,internal_"
                 "repetitions"
              << std::endl;
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
