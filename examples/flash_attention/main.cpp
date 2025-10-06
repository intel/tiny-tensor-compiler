// Copyright (C) 2025 Intel Corporation
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
    examples::test_type ty = examples::test_type::f16;
    std::int64_t headdim = 64, seqlen = 2048, nheads = 32, batch = 1;
};

auto flash_attention_code(examples::test_type dtype, std::int64_t headdim, std::int64_t block_size)
    -> std::string {
    extern const std::uint8_t _binary_flash_attention_template_start,
        _binary_flash_attention_template_end;
    std::uint8_t const *start = &_binary_flash_attention_template_start;
    std::uint8_t const *end = &_binary_flash_attention_template_end;
    const std::size_t size = end - start;

    auto code = std::ostringstream{};
    code << "$dtype = " << to_string(dtype) << "\n";
    code << "$headdim = " << headdim << "\n";
    code << "$block_size = " << block_size << "\n";
    code.write((char const *)start, size);
    return std::move(code).str();
}

template <typename T> void test(queue q, args &a) {
    const std::size_t num_elements = a.headdim * a.seqlen * a.nheads * a.batch;
    T *Q = malloc_device<T>(num_elements, q);
    T *K = malloc_device<T>(num_elements, q);
    T *V = malloc_device<T>(num_elements, q);
    T *O = malloc_device<T>(num_elements, q);

    try {
        const std::int64_t block_size = 512 / (a.headdim / 64);
        char const *kernel_name = a.headdim == 64 ? "flash_attention_64" : "flash_attention";
        if (a.headdim % 64 != 0 || a.headdim > 1024 || a.headdim <= 0) {
            throw std::runtime_error(
                "Headdim must be multiple of 64 and smaller or equal than 1024.");
        }

        auto info = create_core_info(q.get_device());
        set_core_features(info.get(), tinytc_core_feature_flag_large_register_file);

        auto ctx = create_compiler_context();
        set_error_reporter(ctx.get(), [](char const *what, const tinytc_location_t *, void *) {
            std::cerr << what << std::endl;
        });

        auto prg = parse_string(flash_attention_code(a.ty, a.headdim, block_size), ctx.get());
        if (a.dump) {
            dump(prg.get());
        }
        auto bundle = create_kernel_bundle(q.get_context(), q.get_device(), prg.get(),
                                           tinytc_core_feature_flag_large_register_file);
        auto kernel = create_kernel(bundle, kernel_name);

        auto num_groups =
            sycl::range<3u>{static_cast<std::size_t>(a.batch), static_cast<std::size_t>(a.nheads),
                            static_cast<std::size_t>(a.seqlen) / block_size};
        auto exe_range = get_execution_range(kernel, num_groups);
        const std::int64_t stride0 = 1;
        const std::int64_t stride1 = stride0 * a.headdim;
        const std::int64_t stride2 = stride1 * a.seqlen;
        const std::int64_t stride3 = stride2 * a.nheads;
        const float scale_factor = 1.0 / std::sqrt(static_cast<double>(a.headdim));
        auto run = [&] {
            q.submit([&](handler &h) {
                 h.set_args(Q, a.seqlen, a.nheads, a.batch, stride2, stride3, //
                            K, a.seqlen, a.nheads, a.batch, stride2, stride3, //
                            V, a.seqlen, a.nheads, a.batch, stride2, stride3, //
                            O, a.seqlen, a.nheads, a.batch, stride2, stride3, //
                            scale_factor);
                 h.parallel_for(exe_range, kernel);
             }).wait();
        };

        run();
        double min_exec_time_ns = examples::bench([&]() { run(); }, 100);

        auto bw = sizeof(T) * 4 * (a.headdim * a.seqlen * a.nheads * a.batch) / min_exec_time_ns;
        auto gflops = 4 * a.nheads * a.seqlen * a.seqlen * a.headdim * a.batch / min_exec_time_ns;
        std::cout << to_string(a.ty) << "," << a.headdim << "," << a.seqlen << "," << a.nheads
                  << "," << a.batch << "," << min_exec_time_ns / 1e6 << "," << bw << "," << gflops
                  << std::endl;
    } catch (status const &st) {
        std::cerr << "Error (" << static_cast<int>(st) << "): " << tinytc::to_string(st)
                  << std::endl;
    } catch (std::exception const &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    free(Q, q);
    free(K, q);
    free(V, q);
    free(O, q);
}

int main(int argc, char **argv) {
    auto a = args{};
    bool help = false;

    auto parser = cmd::arg_parser{};
    try {
        parser.set_short_opt('o', &a.dump, "Dump IR to stdout");
        parser.set_short_opt('f', &a.ty, "Data type (bf16, f16, f32)")
            .converter(examples::convert_data_type);
        parser.set_short_opt('d', &a.headdim, "Head dimension");
        parser.set_short_opt('t', &a.seqlen, "Sequence length");
        parser.set_short_opt('n', &a.nheads, "Number of heads");
        parser.set_short_opt('b', &a.batch, "Batch size");
        parser.set_short_opt('h', &help, "Show help");

        parser.parse(argc, argv);
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    if (help) {
        parser.print_help(std::cout, "flash_attention", "");
        return !help ? -1 : 0;
    }

    auto q = queue{};

    std::cout << "precision,headdim,seqlen,nheads,batch,time_ms,bandwidth,gflops" << std::endl;
    try {
        dispatch(a.ty, [&]<typename T>() { test<T>(q, a); });
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}
