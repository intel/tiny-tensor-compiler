// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef UTIL_20230411_HPP
#define UTIL_20230411_HPP

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <sycl/sycl.hpp>

constexpr inline std::size_t binomial(std::size_t n, std::size_t k) {
    return k > n                  ? 0
           : k == 0 || k == n     ? 1
           : k == 1 || k == n - 1 ? n
                                  : binomial(n - 1, k - 1) * n / k;
}

constexpr inline std::int64_t num_basis(std::int64_t N, std::int64_t dim) {
    return binomial(N + dim, dim);
}

template <typename T> std::int64_t aligned(std::int64_t n, std::size_t alignment) {
    auto aligned_reals = std::max(std::size_t(1), alignment / sizeof(T));
    return aligned_reals * (1 + (n - 1) / aligned_reals);
}

template <typename T>
bool is_approx_equal(matrix_batch<T> const &opt, matrix_batch<T> const &ref, sycl::queue &q) {
    if (opt.size() != ref.size()) {
        return false;
    }
    T *err = sycl::malloc_device<T>(1, q);
    q.fill(err, T(0), 1).wait();
    auto opt_ptr = opt.get();
    auto ref_ptr = ref.get();
    q.parallel_for({opt.size()}, sycl::reduction(err, sycl::plus<>()),
                   [=](sycl::id<1> it, auto &err) {
                       auto const i = it[0];
                       T local_err = ref_ptr[i] - opt_ptr[i];
                       local_err = local_err * local_err;
                       err.combine(local_err);
                   })
        .wait();
    T err_host;
    q.copy(err, &err_host, 1).wait();
    sycl::free(err, q);
    err_host = std::sqrt(err_host);
    constexpr auto threshold = std::numeric_limits<T>::epsilon();
    bool ok = err_host < threshold;
    if (!ok) {
        std::cout << "Error: L2 error of " << std::sqrt(err_host) << " exceeds threshold of "
                  << threshold << std::endl;
    }
    return ok;
}

#endif // UTIL_20230411_HPP
