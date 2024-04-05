// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DEVICE_ARRAY_20230411_HPP
#define DEVICE_ARRAY_20230411_HPP

#include <cstdint>
#include <sycl/sycl.hpp>
#include <utility>

template <typename T> class device_array {
  public:
    device_array(std::size_t size, sycl::queue q) : size_(size), q_(std::move(q)) { alloc(); }
    ~device_array() { free(); }
    device_array(device_array const &other) : size_(other.size_), q_(other.q_) {
        alloc();
        q_.copy(other.data_, data_, size_).wait();
    }
    device_array(device_array &&other)
        : size_(std::exchange(other.size_, 0)), q_(std::move(other.q_)),
          data_(std::exchange(other.data_, nullptr)) {}
    device_array &operator=(device_array const &other) {
        if (other.size_ != size_) {
            realloc(other.size_);
        }
        q_.copy(other.data_, data_, size_).wait();
    }
    device_array &operator=(device_array &&other) {
        free();
        size_ = std::exchange(other.size_, 0);
        q_ = std::move(other.q_);
        data_ = std::exchange(other.data_, nullptr);
    }
    inline T *get() { return data_; }
    inline T const *get() const { return data_; }
    inline std::int64_t size() const { return size_; }
    void fill(T const &value) { q_.fill(data_, value, size_).wait(); }
    void random() {
        auto ptr = data_;
        q_.parallel_for({size_}, [=](sycl::id<1> it) {
              auto const i = it[0];
              ptr[i] = T(i % 101);
          }).wait();
    }

  private:
    void alloc() { data_ = sycl::aligned_alloc_device<T>(4096, size_, q_); }
    void free() {
        if (data_ != nullptr) {
            sycl::free(data_, q_);
        }
    }
    void realloc(std::size_t size) {
        free();
        size_ = size;
        alloc();
    }
    std::size_t size_;
    sycl::queue q_;
    T *data_;
};

#endif // DEVICE_ARRAY_20230411_HPP
