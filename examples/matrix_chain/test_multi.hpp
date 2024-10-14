// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TEST_MULTI_20230313_HPP
#define TEST_MULTI_20230313_HPP

#include "test.hpp"

#include <sycl/sycl.hpp>

#include <cstdint>
#include <memory>
#include <vector>

enum class test_case { volume, ader };

template <typename T> class test_multi {
  public:
    test_multi(std::int64_t N, std::int64_t P, std::int64_t howmany, std::size_t alignment,
               test_case tc, std::vector<::sycl::queue> const &q, bool dump = false);

    void reference();
    void optimized();
    void print_header();
    void print_performance(double time_ns);
    bool check();

  private:
    std::vector<std::unique_ptr<test>> instances_;
};

extern template class test_multi<float>;
extern template class test_multi<double>;

#endif // TEST_MULTI_20230313_HPP
