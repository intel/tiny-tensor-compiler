// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "args.hpp"

#include <sycl/sycl.hpp>
#include <tinytc/tinytc-sycl.hpp>
#include <tinytc/tinytc.hpp>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
#include <memory>
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

auto gemm_kernel_with_inner_repetition(ir::gemm_configuration const &cfg, int repetitions, queue q)
    -> std::shared_ptr<binary> {
    auto kernel = [&](ir::function_builder &fb) {
        auto A =
            fb.argument(ir::group_type(ir::memref_type(
                            cfg.ty.A, {cfg.M, cfg.K},
                            std::vector<std::int64_t>(cfg.A_stride.begin(), cfg.A_stride.end()))),
                        "A");
        auto B =
            fb.argument(ir::group_type(ir::memref_type(
                            cfg.ty.B, {cfg.K, cfg.N},
                            std::vector<std::int64_t>(cfg.B_stride.begin(), cfg.B_stride.end()))),
                        "B");
        auto C =
            fb.argument(ir::group_type(ir::memref_type(
                            cfg.ty.B, {cfg.M, cfg.N},
                            std::vector<std::int64_t>(cfg.C_stride.begin(), cfg.C_stride.end()))),
                        "C");
        fb.body([&](ir::region_builder &bb) {
            auto gid = bb.create_group_id();
            auto a = bb.create_load(A, {gid});
            auto b = bb.create_load(B, {gid});
            auto c = bb.create_load(C, {std::move(gid)});
            bb.create_for(ir::scalar_type::index, ir::value(0, ir::scalar_type::index),
                          ir::value(repetitions, ir::scalar_type::index),
                          [&](ir::region_builder &bb) {
                              bb.create_gemm(cfg.transA, cfg.transB, ir::value(1.0, cfg.ty.alpha),
                                             a, b, ir::value(0.0, cfg.ty.beta), c);
                          });
        });
    };
    auto const err = [](ir::location const &loc, std::string const &what) {
        std::cerr << loc << ": " << what << std::endl;
    };

    auto pb = ir::program_builder{};
    try {
        pb.create("gemm", kernel);
    } catch (ir::compilation_error const &e) {
        err(e.loc(), e.what());
        return nullptr;
    }

    auto info = get_core_info(q.get_device());
    info->set_core_feature(core_feature_flag::large_register_file);

    return optimize_and_make_binary(pb.get_product(), bundle_format::native, std::move(info),
                                    std::move(err));
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
                bool transa = a.transA == ir::transpose::T;
                bool transb = a.transB == ir::transpose::T;
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
        ir::gemm_configuration cfg = {ir::to_scalar_type_v<T>,
                                      a.transA,
                                      a.transB,
                                      c.m,
                                      c.n,
                                      c.k,
                                      {1, a.transA == ir::transpose::T ? c.k : c.m},
                                      {1, a.transB == ir::transpose::T ? c.n : c.k},
                                      {1, c.m},
                                      std::nullopt,
                                      std::nullopt};

        double min_exec_time_ns = 0.0;
        auto bin = gemm_kernel_with_inner_repetition(cfg, a.internal_repetitions, q);
        if (bin) {
            auto bundle = tensor_kernel_bundle(std::move(bin), q.get_context(), q.get_device());
            auto kernel = bundle.get("gemm");
            kernel.set_args(AA, BB, CC);
            kernel.submit(howmany, q).wait();
            if (a.internal_repetitions == 1 && a.verify) {
                check(c.m, c.n, howmany);
            }
            min_exec_time_ns = bench([&]() { kernel.submit(howmany, q).wait(); });

            auto gflops = a.internal_repetitions * 2 * c.m * c.n * c.k * howmany / min_exec_time_ns;
            auto roofline_gflops =
                std::min(512 * 32 * 1.6e9, a.internal_repetitions * 2 * c.m * c.n * c.k /
                                               (sizeof(T) * (na + nb + nc) / 1.1e12)) /
                1e9;
            std::cout << type.name() << "," << c.m << "," << c.n << "," << c.k << "," << howmany
                      << "," << min_exec_time_ns / 1e9 << "," << gflops << "," << roofline_gflops
                      << "," << std::round(gflops / roofline_gflops * 100) << "%,"
                      << a.internal_repetitions << std::endl;
        } else {
            std::cerr << "Kernel compilation failed" << std::endl;
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

    std::cout
        << "precision,m,n,k,howmany,time,gflops,roofline_gflops,roofline_perc,internal_repetitions"
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
