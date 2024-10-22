// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SMM_20240314_HPP
#define SMM_20240314_HPP

#include "tensor3.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <doctest/doctest.h>

#include <complex>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#define DOCTEST_TENSOR4_TEST(MM, NN, KK, HH)                                                       \
    do {                                                                                           \
        for (auto hh : HH) {                                                                       \
            for (auto kk : KK) {                                                                   \
                for (auto nn : NN) {                                                               \
                    for (auto mm : MM) {                                                           \
                        DOCTEST_SUBCASE((std::to_string(mm) + "x" + std::to_string(nn) + "x" +     \
                                         std::to_string(kk) + "*" + std::to_string(hh))            \
                                            .c_str()) {                                            \
                            howmany = hh;                                                          \
                            K = kk;                                                                \
                            N = nn;                                                                \
                            M = mm;                                                                \
                        }                                                                          \
                    }                                                                              \
                }                                                                                  \
            }                                                                                      \
        }                                                                                          \
    } while (false)

#define TEST_PRECISIONS float, double

template <typename T> struct is_complex : public std::false_type {};
template <typename T> struct is_complex<std::complex<T>> : public std::true_type {};
template <typename T> inline constexpr bool is_complex_v = is_complex<T>::value;

template <typename T>
void small_gemm_batched_ref(tinytc::transpose transA, tinytc::transpose transB, T alpha,
                            tensor3<T> const &A, tensor3<T> const &B, T beta, tensor3<T> &C) {
    bool compatible = A.shape(2) == B.shape(2) && B.shape(2) == C.shape(2);
    auto Arows = A.shape(0);
    auto Acols = A.shape(1);
    if (transA == tinytc::transpose::T) {
        std::swap(Arows, Acols);
    }
    auto Brows = B.shape(0);
    auto Bcols = B.shape(1);
    if (transB == tinytc::transpose::T) {
        std::swap(Brows, Bcols);
    }
    compatible = compatible && Arows == C.shape(0) && Bcols == C.shape(1) && Acols == Brows;
    if (!compatible) {
        throw std::runtime_error("incompatible matmul");
    }
    for (std::uint32_t j = 0; j < C.shape(2); ++j) {
        for (std::uint32_t n = 0; n < C.shape(1); ++n) {
            for (std::uint32_t m = 0; m < C.shape(0); ++m) {
                T c_acc = T(0.0);
                for (std::uint32_t k = 0; k < Acols; ++k) {
                    auto const a = transA == tinytc::transpose::T ? A(k, m, j) : A(m, k, j);
                    auto const b = transB == tinytc::transpose::T ? B(n, k, j) : B(k, n, j);
                    c_acc += a * b;
                }
                C(m, n, j) = alpha * c_acc + beta * C(m, n, j);
            }
        }
    }
}

template <typename T>
concept test_runtime_gpu =
    requires(T rt, std::size_t bytes, typename T::mem_t buf, typename T::const_mem_t const_buf,
             void *dst, void const *src, int value, tinytc::recipe const &rec) {
        typename T::device_t;
        typename T::context_t;
        typename T::command_list_t;
        typename T::recipe_handler_t;
        typename T::mem_t;
        typename T::const_mem_t;
        { rt.create_buffer(bytes) } -> std::same_as<typename T::mem_t>;
        rt.free_buffer(buf);
        rt.fill_buffer(buf, value, bytes);
        rt.memcpy_h2d(buf, src, bytes);
        rt.memcpy_d2h(dst, const_buf, bytes);
        { rt.get_core_info() } -> std::same_as<tinytc::core_info>;
        { rt.get_device() } -> std::same_as<typename T::device_t>;
        { rt.get_context() } -> std::same_as<typename T::context_t>;
        { rt.get_command_list() } -> std::same_as<typename T::command_list_t>;
        { rt.get_recipe_handler(rec) } -> std::same_as<typename T::recipe_handler_t>;
        { rt.supports_fp64() } -> std::same_as<bool>;
        rt.synchronize();
    };

template <typename T, test_runtime_gpu R>
void check_small_gemm_batched(tinytc::transpose transA, tinytc::transpose transB, std::uint32_t M,
                              std::uint32_t N, std::uint32_t K, std::uint32_t ldA,
                              std::uint32_t strideA, std::uint32_t ldB, std::uint32_t strideB,
                              std::uint32_t ldC, std::uint32_t strideC, T alpha, T beta,
                              std::uint32_t howmany) {
    auto const selA = [&](std::uint32_t N1, std::uint32_t N2) {
        return transA == tinytc::transpose::T ? N2 : N1;
    };
    auto const selB = [&](std::uint32_t N1, std::uint32_t N2) {
        return transB == tinytc::transpose::T ? N2 : N1;
    };

    auto gpu_rt = std::make_shared<R>();
    if constexpr (std::is_same_v<T, double> || std::is_same_v<T, std::complex<double>>) {
        if (!gpu_rt->supports_fp64()) {
            WARN_MESSAGE(false, "Double precision tests need double precision device support");
            return;
        }
    }

    auto const fill = [](tensor3<T> &x) {
        T *data = x.data();
        std::size_t n = x.size();
        for (std::size_t i = 0; i < n; ++i) {
            constexpr std::size_t prime = 101;
            if constexpr (is_complex_v<T>) {
                data[i] = T{static_cast<T::value_type>((2 * i) % prime),
                            static_cast<T::value_type>((2 * i + 1) % prime)};
            } else {
                data[i] = static_cast<T>(i % prime);
            }
        }
    };

    auto A_ref = tensor3<T>({selA(M, K), selA(K, M), howmany}, {1, ldA, strideA});
    auto B_ref = tensor3<T>({selB(K, N), selB(N, K), howmany}, {1, ldB, strideB});
    auto C_ref = tensor3<T>({M, N, howmany}, {1, ldC, strideC});
    fill(A_ref);
    fill(B_ref);
    C_ref.set_zero();

    small_gemm_batched_ref<T>(transA, transB, alpha, A_ref, B_ref, beta, C_ref);

    auto A = gpu_rt->create_buffer(A_ref.size() * sizeof(T));
    auto B = gpu_rt->create_buffer(B_ref.size() * sizeof(T));
    auto C = gpu_rt->create_buffer(C_ref.size() * sizeof(T));
    gpu_rt->memcpy_h2d(A, A_ref.data(), A_ref.size() * sizeof(T));
    gpu_rt->memcpy_h2d(B, B_ref.data(), B_ref.size() * sizeof(T));
    gpu_rt->fill_buffer(C, 0, C_ref.size() * sizeof(T));

    auto info = gpu_rt->get_core_info();

    auto g = gpu_rt->get_recipe_handler(
        tinytc::make_small_gemm_batched(info, tinytc::to_scalar_type_v<T>, transA, transB, M, N, K,
                                        ldA, strideA, ldB, strideB, ldC, strideC));
    tinytc::small_gemm_batched::set_args(g, howmany, alpha, A, B, beta, C);
    g.submit(gpu_rt->get_command_list());
    gpu_rt->synchronize();

    auto C_host = tensor3<T>({M, N, howmany}, {1, ldC, strideC});
    gpu_rt->memcpy_d2h(C_host.data(), C, C_host.size() * sizeof(T));

    compare<T>(C_host, C_ref);

    gpu_rt->free_buffer(A);
    gpu_rt->free_buffer(B);
    gpu_rt->free_buffer(C);
}

#endif // SMM_20240314_HPP
