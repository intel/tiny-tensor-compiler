// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MATRIX_EXT_INFO_20241204_HPP
#define MATRIX_EXT_INFO_20241204_HPP

#include "tinytc/core.hpp"
#include "tinytc/types.hpp"

#include <array>
#include <cstdint>
#include <utility>
#include <vector>

namespace tinytc {

enum class TK;

struct gemm_mnk {
    std::int64_t M, N, K;
};

class matrix_ext_type {
  public:
    matrix_ext_type(TK a, TK b, std::vector<TK> acc, std::vector<gemm_mnk> mnk);

    inline auto a() const -> TK { return a_; }
    inline auto b() const -> TK { return b_; }
    inline auto acc() const -> array_view<TK> { return acc_; }
    inline auto mnk() const -> array_view<gemm_mnk> { return mnk_; }

    auto M_block_sizes() const -> std::vector<std::int32_t>;
    auto N_block_sizes(std::int32_t M) const -> std::vector<std::int32_t>;
    auto K_block_sizes(std::int32_t M, std::int32_t N) const -> std::vector<std::int32_t>;

    auto have_acc(TK acc) const -> bool;
    auto have_type(TK sty, std::int64_t rows, std::int64_t cols, matrix_use use) const -> bool;

  private:
    TK a_, b_;
    std::vector<TK> acc_;
    std::vector<gemm_mnk> mnk_;
};

struct matrix_ext_block_io_info {
    std::int32_t base_address_alignment;
    std::int32_t min_stride;
    std::int32_t max_stride;
    std::int32_t pos0_alignment;
    std::int32_t stride_alignment;
    std::int32_t width_alignment;
};

class matrix_ext_info {
  public:
    matrix_ext_info() = default;
    inline matrix_ext_info(std::int32_t required_subgroup_size, matrix_ext_block_io_info block_io,
                           array_view<matrix_ext_type> mat_types)
        : required_sgs_{required_subgroup_size}, block_io_{block_io},
          mat_types_(std::move(mat_types)) {}

    auto get_precision(TK a, TK b, TK acc) const -> matrix_ext_type const *;
    auto have_gemm(TK a, TK b, TK c, TK d, std::int64_t M, std::int64_t N, std::int64_t K) const
        -> bool;
    auto have_precision(TK a, TK b, TK acc) const -> bool;
    auto have_type(TK sty, std::int64_t rows, std::int64_t cols, matrix_use use) const -> bool;
    auto have_type(const coopmatrix_type *ty) const -> bool;

    inline auto required_subgroup_size() const -> std::int32_t { return required_sgs_; }
    inline auto block_io() const -> matrix_ext_block_io_info const & { return block_io_; }

    inline auto have_dpas() const { return mat_types_.size() > 0; }

  private:
    std::int32_t required_sgs_;
    matrix_ext_block_io_info block_io_;
    array_view<matrix_ext_type> mat_types_;
};

extern const std::array<matrix_ext_type, 3u> pvc_matrix_ext_types;

} // namespace tinytc

#endif // MATRIX_EXT_INFO_20241204_HPP
