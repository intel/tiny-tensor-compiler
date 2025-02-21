// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "../gemm_common.hpp"

#include <argparser.hpp>
#include <sycl/sycl.hpp>
#include <tinytc/tinytc.hpp>
#include <tinytc/tinytc_sycl.hpp>

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <complex>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <limits>
#include <memory>
#include <source_location>
#include <sstream>
#include <stdexcept>

using namespace sycl;
using namespace tinytc;

struct args {
    std::int32_t alignment = 0;
    bool atomic = false;
    bool dump = false;
    std::int32_t internal_repetitions = 1;
    bool trans_a = false;
    bool trans_b = false;
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

auto gemm_kernel_with_inner_repetition(scalar_type ty, transpose tA, transpose tB, bool atomic,
                                       std::int64_t M, std::int64_t N, std::int64_t K,
                                       std::array<std::int64_t, 2> A_stride,
                                       std::array<std::int64_t, 2> B_stride, bool update,
                                       std::array<std::int64_t, 2> C_stride, std::int32_t alignment,
                                       std::int32_t repetitions, bool dump, queue q) -> binary {
    auto ctx = make_compiler_context();
    ctx.set_error_reporter(
        [](char const *what, const tinytc_location_t *, void *) { std::cerr << what << std::endl; },
        nullptr);
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
    auto const make_memref = [](data_type element_ty, transpose t, int64_t A, std::int64_t B,
                                std::array<std::int64_t, 2u> const &stride, location const &loc) {
        auto s = std::array<std::int64_t, 2u>{A, B};
        if (t == transpose::T) {
            std::swap(s[0], s[1]);
        }
        return get_memref(element_ty, s, stride, address_space::global, loc);
    };

    auto kernel = [&](compiler_context const &ctx) {
        auto index_ty = get_scalar(ctx, scalar_type::index);
        auto element_ty = get_scalar(ctx, ty);
        auto A_ty = make_memref(element_ty, tA, M, K, A_stride, my_loc());
        auto B_ty = make_memref(element_ty, tB, K, N, B_stride, my_loc());
        auto C_ty = make_memref(element_ty, transpose::N, M, N, C_stride, my_loc());
        auto f =
            make_func("gemm",
                      {get_group(A_ty, dynamic, 0, my_loc()), get_group(B_ty, dynamic, 0, my_loc()),
                       get_group(C_ty, dynamic, 0, my_loc())},
                      get_void(ctx), my_loc());
        if (alignment > 0) {
            auto align_attr = get_dictionary_attr_with_sorted(
                ctx, named_attr{get_string_attr(ctx, "align"), get_integer_attr(ctx, alignment)});
            f.set_parameter_attr(0, align_attr);
            f.set_parameter_attr(1, align_attr);
            f.set_parameter_attr(2, align_attr);
        }
        auto fn_body = f.get_body();
        auto params = std::array<value, 3u>{};
        fn_body.get_parameters(params);

        auto bb = region_builder{fn_body};
        auto gid = bb.add(make_builtin(builtin::group_id, index_ty, my_loc()));
        auto from = bb.add(make_constant_zero(index_ty, my_loc()));
        auto to = bb.add(make_constant(repetitions, index_ty, my_loc()));
        auto calpha = bb.add(make_constant_one(element_ty, my_loc()));
        auto cbeta = bb.add(update ? make_constant_one(element_ty, my_loc())
                                   : make_constant_zero(element_ty, my_loc()));
        auto a = bb.add(make_load(params[0], {gid}, A_ty, my_loc()));
        auto b = bb.add(make_load(params[1], {gid}, B_ty, my_loc()));
        auto c = bb.add(make_load(params[2], {gid}, C_ty, my_loc()));
        bb.for_loop(
            index_ty, from, to,
            [&](region_builder &bb, value const &) {
                bb.add(make_gemm(tA, tB, atomic, calpha, a, b, cbeta, c, my_loc()));
            },
            nullptr, my_loc());

        return f;
    };

    try {
        auto p = make_prog(ctx, my_loc());
        p.add_function(kernel(ctx));
        if (dump) {
            p.dump();
        }

        auto info = make_core_info(q.get_device());
        info.set_core_features(tinytc_core_feature_flag_large_register_file);
        return compile_to_spirv_and_assemble(std::move(p), info);
    } catch (builder_error const &e) {
        ctx.report_error(e.loc(), e.what());
        std::cerr << "Error  (" << static_cast<int>(e.code()) << "): " << error_string(e.code())
                  << std::endl;
    } catch (status const &st) {
        std::cerr << "Error (" << static_cast<int>(st) << "): " << error_string(st) << std::endl;
    }
    return binary{nullptr};
}

template <typename T> void test(queue q, args &a) {
    auto total_reals = 1024 * 1024 * 1024 / sizeof(T);
    T *A_host = new T[total_reals];
    T *B_host = new T[total_reals];
    T *C_host = new T[total_reals];
    auto const alloc_device = [&a, &q](std::size_t num_bytes) {
        if (a.alignment == 0) {
            return malloc_device<T>(num_bytes, q);
        } else {
            return aligned_alloc_device<T>(a.alignment, num_bytes, q);
        }
    };
    T *A = alloc_device(total_reals);
    T *B = alloc_device(total_reals);
    T *C = alloc_device(total_reals);

    auto const check = [&](std::int64_t M, std::int64_t N, std::int64_t K, std::int64_t howmany) {
        q.copy(C, C_host, total_reals).wait();
        std::size_t num_err = 0;
        const auto error_bound = examples::test_gemm_error_bound<T>(K);
        for (std::int64_t b = 0; b < howmany; ++b) {
            auto C_host_b = C_host + b * M * N;
            for (std::int64_t j = 0; j < N; ++j) {
                for (std::int64_t i = 0; i < M; ++i) {
                    const auto relerr = examples::test_gemm_rel_error<T>(C_host_b, i, j, M);
                    if (relerr > error_bound) {
                        if (num_err < 10) {
                            std::cout
                                << "C_{" << i << "," << j << "," << b << "}=" << C_host_b[i + j * M]
                                << ", relative_error=" << relerr << ", error_bound=" << error_bound
                                << std::endl;
                        }
                        ++num_err;
                    }
                }
            }
        }
        if (num_err > 10) {
            std::cout << "and " << num_err - 10 << " further errors." << std::endl;
        }
    };

    for (auto &c : a.tc) {
        auto na = c.m * c.k;
        auto nb = c.k * c.n;
        auto nc = c.m * c.n;
        auto max_reals = std::max(std::max(na, nb), nc);
        auto howmany = total_reals / max_reals;

        for (std::size_t i = 0; i < howmany; ++i) {
            examples::test_gemm_matrix<T, matrix_use::a>(A_host + i * na, c.m, c.k, a.trans_a);
            examples::test_gemm_matrix<T, matrix_use::b>(B_host + i * nb, c.k, c.n, a.trans_b);
        }
        q.copy(A_host, A, total_reals).wait();
        q.copy(B_host, B, total_reals).wait();
        q.memset(C, 0, total_reals * sizeof(T)).wait();

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
            auto src = gemm_kernel_with_inner_repetition(
                a.ty, a.trans_a ? transpose::T : transpose::N,
                a.trans_b ? transpose::T : transpose::N, a.atomic, c.m, c.n, c.k,
                {1, a.trans_a ? c.k : c.m}, {1, a.trans_b ? c.n : c.k}, a.update, {1, c.m},
                a.alignment, a.internal_repetitions, a.dump, q);
            if (src) {
                auto bundle = make_kernel_bundle(q.get_context(), q.get_device(), src);
                auto kernel = make_kernel(bundle, "gemm");
                auto exe_range = get_execution_range(kernel, howmany);
                q.submit([&](handler &h) {
                     h.set_args(AA, howmany, BB, howmany, CC, howmany);
                     h.parallel_for(exe_range, kernel);
                 }).wait();
                if (a.internal_repetitions == 1 && a.verify) {
                    check(c.m, c.n, c.k, howmany);
                }
                min_exec_time_ns = bench([&]() {
                    q.submit([&](handler &h) {
                         h.set_args(AA, howmany, BB, howmany, CC, howmany);
                         h.parallel_for(exe_range, kernel);
                     }).wait();
                });

                const auto ops_per_mnk = [&] {
                    switch (a.ty) {
                    case scalar_type::c32:
                    case scalar_type::c64:
                        return 8;
                    default:
                        return 2;
                    }
                }();

                auto gflops = a.internal_repetitions * ops_per_mnk * c.m * c.n * c.k * howmany /
                              min_exec_time_ns;
                auto roofline_gflops =
                    std::min(512 * 32 * 1.6e9, a.internal_repetitions * 2 * c.m * c.n * c.k /
                                                   (sizeof(T) * (na + nb + nc) / 1.1e12)) /
                    1e9;
                std::cout << to_string(a.ty) << "," << c.m << "," << c.n << "," << c.k << ","
                          << howmany << "," << min_exec_time_ns / 1e9 << "," << gflops << ","
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
    delete[] A_host;
    delete[] B_host;
    delete[] C_host;
};

int main(int argc, char **argv) {
    auto a = args{};
    bool help = false;

    auto parser = cmd::arg_parser{};
    try {
        parser.set_short_opt('a', &a.atomic, "Update C atomically");
        parser.set_short_opt('d', &a.dump, "Dump IR to stdout");
        parser.set_short_opt('f', &a.ty, "Data type (f32, f64, c32, c64)")
            .converter(examples::convert_data_type);
        parser
            .set_short_opt('i', &a.internal_repetitions,
                           "Number of GEMM repetitions inside kernel (default: 1)")
            .validator([](std::int32_t rep) { return 0 <= rep; });
        parser.set_short_opt('h', &help, "Show help");
        parser.set_short_opt('u', &a.update,
                             "Add A*B to C (beta=1) instead of overwriting C (beta=0)");
        parser.set_short_opt('v', &a.verify, "Verify optimized implementation");
        parser.set_long_opt("help", &help, "Show help");
        parser.set_long_opt("alignment", &a.alignment, "Memory alignment");
        parser.set_long_opt("transpose-a", &a.trans_a, "Transpose A matrix");
        parser.set_long_opt("transpose-b", &a.trans_b, "Transpose B matrix");
        parser.add_positional_arg("test-case", &a.tc, "MxNxK triplet (e.g. 64x64x64)")
            .converter(examples::convert_test_case)
            .validator(examples::validate_test_case);

        parser.parse(argc, argv);
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    if (help || a.tc.empty()) {
        parser.print_help(std::cout, "tinytc-bench", "");
        return !help ? -1 : 0;
    }

    auto q = queue{};

    std::cout << "precision,m,n,k,howmany,time,gflops,roofline_gflops,roofline_perc,internal_"
                 "repetitions"
              << std::endl;
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
