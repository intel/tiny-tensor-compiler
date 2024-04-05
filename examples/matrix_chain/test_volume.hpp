// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TEST_VOLUME_20230411_HPP
#define TEST_VOLUME_20230411_HPP

#include "matrix_batch.hpp"
#include "test.hpp"

#include <tinytc/tinytc-sycl.hpp>
#include <tinytc/tinytc.hpp>

#include <sycl/sycl.hpp>

#include <cstdint>
#include <memory>
#include <vector>

template <typename T> class test_volume : public test {
  public:
    test_volume(std::int64_t N, std::int64_t P, std::int64_t howmany, std::size_t alignment,
                ::sycl::queue q);
    ~test_volume() = default;
    test_volume(test_volume<T> const &other) = delete;
    test_volume(test_volume<T> &&other) = default;
    test_volume<T> &operator=(test_volume<T> const &other) = delete;
    test_volume<T> &operator=(test_volume<T> &&other) = default;

    std::vector<::sycl::event> reference() override;
    std::vector<::sycl::event> optimized() override;
    bool check() override;
    std::int64_t flop() override;
    std::int64_t flop_aligned() override;
    std::int64_t bytes() override;

  private:
    constexpr static std::size_t dim = 3;
    auto get_core_info() -> std::shared_ptr<tinytc::core_info>;
    auto make_optimized_kernel() -> tinytc::tensor_kernel_bundle<tinytc::sycl_runtime>;

    std::int64_t B3_, B2_, P_, howmany_, B3_aligned_, B2_aligned_;
    ::sycl::queue q_;
    std::shared_ptr<tinytc::core_info> dev_info_;
    matrix_batch<T> Q_ref_, Q_opt_, I_, tmp_;
    std::vector<matrix_batch<T>> A_, K_;
    tinytc::tensor_kernel_bundle<tinytc::sycl_runtime> opt_bundle_;
    tinytc::tensor_kernel<tinytc::sycl_runtime> opt_kernel_;
    std::vector<tinytc::recipe::small_gemm_batched<T, tinytc::sycl_runtime>> g_;
};

extern template class test_volume<float>;
extern template class test_volume<double>;

#endif // TEST_VOLUME_20230411_HPP
