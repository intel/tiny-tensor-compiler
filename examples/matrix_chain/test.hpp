// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TEST_20230411_HPP
#define TEST_20230411_HPP

#include <sycl/sycl.hpp>

#include <cstdint>
#include <vector>

class test {
  public:
    inline virtual ~test() {}
    virtual std::vector<::sycl::event> reference() = 0;
    virtual std::vector<::sycl::event> optimized() = 0;
    virtual bool check() = 0;
    virtual std::int64_t flop() = 0;
    virtual std::int64_t flop_aligned() = 0;
    virtual std::int64_t bytes() = 0;
};

#endif // TEST_20230411_HPP
