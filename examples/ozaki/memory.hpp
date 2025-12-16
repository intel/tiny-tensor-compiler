// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MEMORY_20251216_HPP
#define MEMORY_20251216_HPP

#include <sycl/sycl.hpp>

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

template <typename T> class usm_delete {
  public:
    explicit usm_delete(sycl::queue q) : q_(std::move(q)) {}

    void operator()(T *ptr) { sycl::free(ptr, q_); }

  private:
    sycl::queue q_;
};

template <typename T> using sycl_unique_ptr = std::unique_ptr<T, usm_delete<T>>;

template <typename T> auto malloc_device_unique(std::size_t size, sycl::queue q) {
    T *ptr = nullptr;
    if constexpr (std::is_same_v<T, void>) {
        ptr = sycl::malloc_device(size, q);
    } else {
        ptr = sycl::malloc_device<T>(size, q);
    }
    return sycl_unique_ptr(ptr, usm_delete<T>(std::move(q)));
}

#endif // MEMORY_20251216_HPP
