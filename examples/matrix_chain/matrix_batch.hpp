// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MATRIX_BATCH_20230411_HPP
#define MATRIX_BATCH_20230411_HPP

#include "device_array.hpp"

#include <sycl/sycl.hpp>
#include <tinytc/tinytc.hpp>

#include <cstdint>
#include <utility>

template <typename T> class matrix_batch {
  public:
    matrix_batch(std::int64_t nrows, std::int64_t ncols, std::int64_t ld, std::int64_t howmany,
                 sycl::queue q)
        : nrows_(nrows), ncols_(ncols), ld_(ld), howmany_(howmany),
          data_(ld_ * ncols_ * howmany, std::move(q)) {}
    inline T *get() { return data_.get(); }
    inline T const *get() const { return data_.get(); }
    inline std::int64_t nrows() const { return nrows_; }
    inline std::int64_t ncols() const { return ncols_; }
    inline std::int64_t ld() const { return ld_; }
    inline std::int64_t howmany() const { return howmany_; }
    inline std::int64_t stride() const { return ld_ * ncols_; }
    inline std::size_t size() const { return data_.size(); }
    inline void fill(T const &v) { data_.fill(v); }
    inline void random() { data_.random(); }

    inline tinytc::data_type type(bool include_batch_dim = true) {
        constexpr auto real_t = tinytc::to_scalar_type_v<T>;
        if (include_batch_dim && howmany() > 1) {
            return tinytc::create_memref(real_t, {nrows(), ncols(), tinytc::dynamic},
                                         {1, ld(), stride()});
        }
        return tinytc::create_memref(real_t, {nrows(), ncols()}, {1, ld()});
    }

  private:
    std::int64_t nrows_, ncols_, ld_, howmany_;
    device_array<T> data_;
};

#endif // MATRIX_BATCH_20230411_HPP
