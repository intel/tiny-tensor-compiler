// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef GEMM_COMMON_20241014_HPP
#define GEMM_COMMON_20241014_HPP

#include "argparser.hpp"
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

inline auto convert_data_type(char const *str, scalar_type &val) -> cmd::parser_status {
    if (std::strcmp(str, "bf16") == 0) {
        val = scalar_type::bf16;
    } else if (std::strcmp(str, "f16") == 0) {
        val = scalar_type::f16;
    } else if (std::strcmp(str, "f32") == 0) {
        val = scalar_type::f32;
    } else if (std::strcmp(str, "f64") == 0) {
        val = scalar_type::f64;
    } else if (std::strcmp(str, "c32") == 0) {
        val = scalar_type::c32;
    } else if (std::strcmp(str, "c64") == 0) {
        val = scalar_type::c64;
    } else {
        return cmd::parser_status::invalid_argument;
    }
    return cmd::parser_status::success;
};
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
};

template <typename T> inline auto fabs(T x) {
    if constexpr (std::is_same_v<T, sycl::half>) {
        return sycl::fabs(x);
    } else {
        return std::abs(x);
    }
}

template <typename T> inline auto compute_error(T x, T x_ref) {
    auto err = examples::fabs(x - x_ref);
    const auto scale = examples::fabs(x_ref);
    return scale > std::numeric_limits<decltype(scale)>::epsilon() ? err / scale : err;
}

} // namespace tinytc::examples

#endif // GEMM_COMMON_20241014_HPP
