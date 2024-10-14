// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MATRIX_BATCH_20230411_HPP
#define MATRIX_BATCH_20230411_HPP

#include "device_array.hpp"

#include <sycl/sycl.hpp>
#include <tinytc/tinytc.hpp>

#include <array>
#include <cstdint>
#include <utility>

template <typename T> class matrix_batch {
  public:
    matrix_batch(std::int64_t nrows, std::int64_t ncols, std::int64_t ld, std::int64_t howmany,
                 sycl::queue q)
        : shape_{nrows, ncols}, ld_{ld}, howmany_{howmany},
          data_(stride() * howmany_, std::move(q)) {}
    inline T *get() { return data_.get(); }
    inline T const *get() const { return data_.get(); }
    inline auto shape() const -> tinytc::array_view<std::int64_t> { return shape_; }
    inline std::int64_t nrows() const { return shape_[0]; }
    inline std::int64_t ncols() const { return shape_[1]; }
    inline std::int64_t howmany() const { return howmany_; }
    inline std::int64_t ld() const { return ld_; }
    inline std::int64_t stride() const { return ld_ * ncols(); }
    inline std::size_t size() const { return data_.size(); }
    inline void fill(T const &v) { data_.fill(v); }
    inline void random() { data_.random(); }

    inline auto type(tinytc::data_type element_ty) -> tinytc::data_type {
        if (howmany_ == 1) {
            return tinytc::get_memref(element_ty, {nrows(), ncols()}, {1, ld()});
        }
        return tinytc::get_memref(element_ty, {nrows(), ncols(), tinytc::dynamic},
                                  {1, ld(), stride()});
    }
    inline auto local_type(tinytc::data_type element_ty) -> tinytc::data_type {
        return tinytc::get_memref(element_ty, {nrows(), ncols()}, {1, ld()},
                                  tinytc::address_space::local);
    }

  private:
    std::array<std::int64_t, 2u> shape_;
    std::int64_t ld_, howmany_;
    device_array<T> data_;
};

#endif // MATRIX_BATCH_20230411_HPP
