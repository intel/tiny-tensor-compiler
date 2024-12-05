// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MATRIX_EXT_INFO_20241204_HPP
#define MATRIX_EXT_INFO_20241204_HPP

#include "tinytc/tinytc.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <utility>

namespace tinytc {

class coopmatrix_data_type;
enum class matrix_use;
enum class scalar_type;

struct gemm_mnk {
    std::int64_t M, N, K;
};

class matrix_ext_type {
  public:
    matrix_ext_type(scalar_type a, scalar_type b, std::initializer_list<scalar_type> acc,
                    std::initializer_list<gemm_mnk> mnk);

    inline auto a() const -> scalar_type { return a_; }
    inline auto b() const -> scalar_type { return b_; }
    inline auto acc() const -> array_view<scalar_type> {
        return array_view(acc_.data(), acc_size_);
    }
    inline auto mnk() const -> array_view<gemm_mnk> { return array_view(mnk_.data(), mnk_size_); }

    auto have_acc(scalar_type acc) const -> bool;
    auto have_type(scalar_type sty, std::int64_t rows, std::int64_t cols,
                   matrix_use use) const -> bool;

  private:
    scalar_type a_, b_;
    std::array<scalar_type, 2u> acc_;
    std::array<gemm_mnk, 4u> mnk_;
    std::size_t acc_size_, mnk_size_;
};

struct matrix_ext_block_io_info {
    std::int32_t address_alignment;
    std::int32_t base_address_alignment;
    std::int32_t min_stride;
    std::int32_t stride_alignment;
};

class matrix_ext_info {
  public:
    matrix_ext_info() = default;
    inline matrix_ext_info(std::int32_t required_subgroup_size, matrix_ext_block_io_info block_io,
                           array_view<matrix_ext_type> types)
        : required_sgs_{required_subgroup_size}, block_io_{block_io}, types_(std::move(types)) {}

    auto have_gemm(scalar_type a, scalar_type b, scalar_type c, scalar_type d, std::int64_t M,
                   std::int64_t N, std::int64_t K) const -> bool;
    auto have_precision(scalar_type a, scalar_type b, scalar_type acc) const -> bool;
    auto have_type(scalar_type sty, std::int64_t rows, std::int64_t cols,
                   matrix_use use) const -> bool;
    auto have_type(const coopmatrix_data_type *ty) const -> bool;

    inline auto required_subgroup_size() const -> std::int32_t { return required_sgs_; }
    inline auto block_io() const -> matrix_ext_block_io_info const & { return block_io_; }

  private:
    std::int32_t required_sgs_;
    matrix_ext_block_io_info block_io_;
    array_view<matrix_ext_type> types_;
};

extern const std::array<matrix_ext_type, 3u> pvc_matrix_ext_types;

} // namespace tinytc

#endif // MATRIX_EXT_INFO_20241204_HPP
