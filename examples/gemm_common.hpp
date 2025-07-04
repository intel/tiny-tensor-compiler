// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef GEMM_COMMON_20241014_HPP
#define GEMM_COMMON_20241014_HPP

#include "argparser.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <cmath>
#include <complex>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <sycl/sycl.hpp>
#include <type_traits>

namespace tinytc::examples {

struct test_case {
    std::int64_t m;
    std::int64_t n;
    std::int64_t k;
};

enum class test_type { bf16, f16, f32, f64, c32, c64 };
auto to_string(test_type ty) {
    switch (ty) {
    case test_type::bf16:
        return "bf16";
    case test_type::f16:
        return "f16";
    case test_type::f32:
        return "f32";
    case test_type::f64:
        return "f64";
    case test_type::c32:
        return "c32";
    case test_type::c64:
        return "c64";
    }
    return "unknown";
}

inline auto convert_data_type(char const *str, test_type &val) -> cmd::parser_status {
    if (std::strcmp(str, "bf16") == 0) {
        val = test_type::bf16;
    } else if (std::strcmp(str, "f16") == 0) {
        val = test_type::f16;
    } else if (std::strcmp(str, "f32") == 0) {
        val = test_type::f32;
    } else if (std::strcmp(str, "f64") == 0) {
        val = test_type::f64;
    } else if (std::strcmp(str, "c32") == 0) {
        val = test_type::c32;
    } else if (std::strcmp(str, "c64") == 0) {
        val = test_type::c64;
    } else {
        return cmd::parser_status::invalid_argument;
    }
    return cmd::parser_status::success;
}
template <typename F> auto dispatch(test_type ty, F &&f) {
    switch (ty) {
    case test_type::bf16:
        f.template operator()<tinytc::bfloat16>();
        break;
    case test_type::f16:
        f.template operator()<tinytc::half>();
        break;
    case test_type::f32:
        f.template operator()<float>();
        break;
    case test_type::f64:
        f.template operator()<double>();
        break;
    case test_type::c32:
        f.template operator()<std::complex<float>>();
        break;
    case test_type::c64:
        f.template operator()<std::complex<double>>();
        break;
    default:
        throw std::runtime_error("Unknown test type");
    }
}

inline auto convert_test_case(char const *str, test_case &tc) -> cmd::parser_status {
    auto const parse = [](std::int64_t *v, char const *str, char **end, char sep) {
        *v = strtol(str, end, 10);
        if (*v == 0 || **end != sep) {
            throw cmd::parser_status::invalid_argument;
        }
        if (errno == ERANGE) {
            throw cmd::parser_status::argument_out_of_range;
        }
    };
    char *end = nullptr;
    try {
        parse(&tc.m, str, &end, 'x');
        parse(&tc.n, end + 1, &end, 'x');
        parse(&tc.k, end + 1, &end, 0);
    } catch (cmd::parser_status st) {
        return st;
    }
    return cmd::parser_status::success;
}
inline auto validate_test_case(test_case const &tc) -> bool {
    return tc.m > 0 && tc.n > 0 && tc.k > 0;
}

template <typename T> auto fabs(T x) {
    if constexpr (std::is_same_v<T, sycl::half>) {
        return sycl::fabs(x);
    } else {
        return std::abs(x);
    }
}

template <typename T> auto compute_error(T x, T x_ref) {
    auto err = examples::fabs(x - x_ref);
    const auto scale = examples::fabs(x_ref);
    return scale > std::numeric_limits<decltype(scale)>::epsilon() ? err / scale : err;
}

// Increment values in bf16 epsilons
constexpr double test_gemm_smallest_eps = 0.0078125;

template <typename T, matrix_use Use>
void test_gemm_matrix(T *data, std::size_t M, std::size_t N, bool transposed = false) {
    for (std::size_t j = 0; j < N; ++j) {
        for (std::size_t i = 0; i < M; ++i) {
            const auto idx = transposed ? j + i * N : i + j * M;
            if constexpr (Use == matrix_use::a) {
                data[idx] = (1.0 + i * test_gemm_smallest_eps) * (j + 1) / N;
            } else if constexpr (Use == matrix_use::b) {
                data[idx] = 1.0 / ((i + 1) * (1 + j * test_gemm_smallest_eps));
            } else {
                data[idx] = 0;
            }
        }
    }
}

template <typename T> struct is_complex : public std::false_type {};
template <std::floating_point F> struct is_complex<std::complex<F>> : public std::true_type {};
template <typename T> inline constexpr bool is_complex_v = is_complex<T>::value;

template <typename T> struct is_lp_float : public std::false_type {};
template <typename T, typename F16f>
struct is_lp_float<lp_float<T, F16f>> : public std::true_type {};
template <typename T> inline constexpr bool is_lp_float_v = is_lp_float<T>::value;

template <typename T>
auto test_gemm_rel_error(T *data, std::size_t i, std::size_t j, std::size_t M) -> double {
    const double ref = (1 + i * test_gemm_smallest_eps) / (1 + j * test_gemm_smallest_eps);
    if constexpr (is_complex_v<T>) {
        return std::abs(static_cast<std::complex<double>>(data[i + j * M]) -
                        std::complex<double>{ref}) /
               ref;
    } else {
        return std::abs(static_cast<double>(data[i + j * M]) - ref) / ref;
    }
}

template <typename T> auto test_gemm_error_bound(std::size_t K) {
    const auto gamma = [](std::size_t K, double u) { return K * u / (1.0 - K * u); };
    if constexpr (is_lp_float_v<T>) {
        const double u = std::pow(2.0, -static_cast<double>(T::lp_format::mantissa_bits));
        const double u_f32 = std::numeric_limits<float>::epsilon();
        // Accumulation is done in single precision
        return 2.0 * u + u * u + gamma(K, u_f32) * (1 + u) * (1 + u);
    } else if constexpr (is_complex_v<T>) {
        return gamma(K, std::numeric_limits<typename T::value_type>::epsilon());
    } else {
        return gamma(K, std::numeric_limits<T>::epsilon());
    }
}

} // namespace tinytc::examples

#endif // GEMM_COMMON_20241014_HPP
