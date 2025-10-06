// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TEST_ADER_20230411_HPP
#define TEST_ADER_20230411_HPP

#include "matrix_batch.hpp"
#include "test.hpp"
#include "util.hpp"

#include <tinytc/tinytc.hpp>
#include <tinytc/tinytc_sycl.hpp>

#include <sycl/sycl.hpp>

#include <algorithm>
#include <cstdint>
#include <vector>

template <typename T> class test_ader : public test {
  public:
    test_ader(std::int64_t N, std::int64_t P, std::int64_t howmany, std::size_t alignment,
              sycl::queue q, bool dump = false);
    ~test_ader() = default;
    test_ader(test_ader<T> const &other) = delete;
    test_ader(test_ader<T> &&other) = default;
    test_ader<T> &operator=(test_ader<T> const &other) = delete;
    test_ader<T> &operator=(test_ader<T> &&other) = default;

    std::vector<::sycl::event> reference() override;
    std::vector<::sycl::event> optimized() override;
    bool check() override;
    std::int64_t flop() override;
    std::int64_t flop_aligned() override;
    std::int64_t bytes() override;

  private:
    constexpr static std::size_t dim = 3;
    inline std::int64_t Bd() { return num_basis(N_, dim); }
    inline std::int64_t Bd(std::int64_t N) { return num_basis(N, dim); }
    inline std::int64_t Bd_aligned() { return aligned<T>(Bd(N_), alignment_); }
    inline std::int64_t Bd_aligned(std::int64_t N) { return aligned<T>(Bd(N), alignment_); }
    std::vector<matrix_batch<T>> make_dQ();
    auto make_optimized_kernel(bool dump) -> sycl::kernel_bundle<sycl::bundle_state::executable>;
    sycl::event taylor_sum(matrix_batch<T> &I, matrix_batch<T> &dQ, T factor,
                           std::vector<sycl::event> const &dep_events = {});

    std::int64_t N_, P_, howmany_, alignment_;
    sycl::queue q_;
    tinytc::shared_handle<tinytc_core_info_t> dev_info_;
    matrix_batch<T> I_ref_, I_opt_, tmp_;
    std::vector<matrix_batch<T>> A_, K_, dQ_;
    std::vector<tinytc::shared_handle<tinytc_recipe_handler_t>> g_;
    sycl::kernel_bundle<sycl::bundle_state::executable> opt_bundle_;
    sycl::kernel opt_kernel_;
};

extern template class test_ader<float>;
extern template class test_ader<double>;

#endif // TEST_ADER_20230411_HPP
