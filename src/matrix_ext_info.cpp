// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "matrix_ext_info.hpp"
#include "node/data_type_node.hpp"
#include "tinytc/types.hpp"

#include <algorithm>
#include <optional>

namespace tinytc {

matrix_ext_type::matrix_ext_type(scalar_type a, scalar_type b, std::vector<scalar_type> acc,
                                 std::vector<gemm_mnk> mnk)
    : a_{a}, b_{b}, acc_(std::move(acc)), mnk_(std::move(mnk)) {}

auto matrix_ext_type::have_acc(scalar_type acc) const -> bool {
    return std::find(acc_.begin(), acc_.end(), acc) != acc_.end();
}
auto matrix_ext_type::have_type(scalar_type sty, std::int64_t rows, std::int64_t cols,
                                matrix_use use) const -> bool {
    auto find_shape = [&rows, &cols](array_view<gemm_mnk> mnks, std::int64_t gemm_mnk::*shape0,
                                     std::int64_t gemm_mnk::*shape1) {
        for (auto const &mnk : mnks) {
            if (mnk.*shape0 == rows && mnk.*shape1 == cols) {
                return true;
            }
        }
        return false;
    };
    switch (use) {
    case matrix_use::a:
        return a() == sty && find_shape(mnk(), &gemm_mnk::M, &gemm_mnk::K);
    case matrix_use::b:
        return b() == sty && find_shape(mnk(), &gemm_mnk::K, &gemm_mnk::N);
    case matrix_use::acc:
        return have_acc(sty) && find_shape(mnk(), &gemm_mnk::M, &gemm_mnk::N);
    }
}

template <typename Get>
auto block_sizes(std::vector<gemm_mnk> const &mnks, Get get) -> std::vector<std::int32_t> {
    auto bs = std::vector<std::int32_t>{};
    bs.reserve(mnks.size());
    for (auto &mnk : mnks) {
        auto val = get(mnk);
        if (val) {
            bs.push_back(*val);
        }
    }
    std::sort(bs.begin(), bs.end());
    auto end = std::unique(bs.begin(), bs.end());
    bs.erase(end, bs.end());
    return bs;
}

auto matrix_ext_type::M_block_sizes() const -> std::vector<std::int32_t> {
    return block_sizes(mnk_,
                       [](gemm_mnk const &mnk) -> std::optional<std::int32_t> { return mnk.M; });
}
auto matrix_ext_type::N_block_sizes(std::int32_t M) const -> std::vector<std::int32_t> {
    return block_sizes(mnk_, [&M](gemm_mnk const &mnk) -> std::optional<std::int32_t> {
        return mnk.M == M ? std::make_optional(mnk.N) : std::nullopt;
    });
}
auto matrix_ext_type::K_block_sizes(std::int32_t M, std::int32_t N) const
    -> std::vector<std::int32_t> {
    return block_sizes(mnk_, [&M, N](gemm_mnk const &mnk) -> std::optional<std::int32_t> {
        return mnk.M == M && mnk.N == N ? std::make_optional(mnk.K) : std::nullopt;
    });
}

auto matrix_ext_info::get_precision(scalar_type a, scalar_type b, scalar_type acc) const
    -> matrix_ext_type const * {
    for (auto const &type : types_) {
        if (type.a() == a && type.b() == b && type.have_acc(acc)) {
            return &type;
        }
    }
    return nullptr;
}

auto matrix_ext_info::have_gemm(scalar_type a, scalar_type b, scalar_type c, scalar_type d,
                                std::int64_t M, std::int64_t N, std::int64_t K) const -> bool {
    for (auto const &type : types_) {
        if (type.have_type(a, M, K, matrix_use::a) && type.have_type(b, K, N, matrix_use::b) &&
            type.have_type(c, M, N, matrix_use::acc) && type.have_type(d, M, N, matrix_use::acc)) {
            return true;
        }
    }
    return false;
}

auto matrix_ext_info::have_precision(scalar_type a, scalar_type b, scalar_type acc) const -> bool {
    return get_precision(a, b, acc) != nullptr;
}

auto matrix_ext_info::have_type(scalar_type sty, std::int64_t rows, std::int64_t cols,
                                matrix_use use) const -> bool {
    for (auto const &type : types_) {
        if (type.have_type(sty, rows, cols, use)) {
            return true;
        }
    }
    return false;
}

auto matrix_ext_info::have_type(const coopmatrix_data_type *ty) const -> bool {
    return have_type(ty->component_ty(), ty->rows(), ty->cols(), ty->use());
}

const std::array<matrix_ext_type, 3u> pvc_matrix_ext_types = {
    {{scalar_type::i8,
      scalar_type::i8,
      {scalar_type::i32},
      {{16, 1, 32}, {16, 2, 32}, {16, 4, 32}, {16, 8, 32}}},
     {scalar_type::f16,
      scalar_type::f16,
      {scalar_type::f16, scalar_type::f32},
      {{16, 1, 16}, {16, 2, 16}, {16, 4, 16}, {16, 8, 16}}},
     {scalar_type::bf16,
      scalar_type::bf16,
      {scalar_type::bf16, scalar_type::f32},
      {{16, 1, 16}, {16, 2, 16}, {16, 4, 16}, {16, 8, 16}}}}};

const std::array<matrix_ext_type, 3u> pvc_matrix_ext_types_diy = {
    {{scalar_type::i8,
      scalar_type::i8,
      {scalar_type::i32},
      {{16, 8, 32},
       {32, 8, 32},
       {64, 8, 32},
       {16, 16, 32},
       {32, 16, 32},
       {64, 16, 32},
       {16, 32, 32},
       {32, 32, 32},
       {64, 32, 32},
       {16, 8, 64},
       {32, 8, 64},
       {64, 8, 64},
       {16, 16, 64},
       {32, 16, 64},
       {64, 16, 64},
       {16, 32, 64},
       {32, 32, 64},
       {64, 32, 64}}},
     {scalar_type::f16,
      scalar_type::f16,
      {scalar_type::f16, scalar_type::f32},
      {{16, 8, 16},
       {32, 8, 16},
       {16, 16, 16},
       {32, 16, 16},
       {16, 32, 16},
       {32, 32, 16},
       {16, 8, 32},
       {32, 8, 32},
       {16, 16, 32},
       {32, 16, 32},
       {16, 32, 32},
       {32, 32, 32}}},
     {scalar_type::bf16,
      scalar_type::bf16,
      {scalar_type::bf16, scalar_type::f32},
      {{16, 8, 16},
       {32, 8, 16},
       {16, 16, 16},
       {32, 16, 16},
       {16, 32, 16},
       {32, 32, 16},
       {16, 8, 32},
       {32, 8, 32},
       {16, 16, 32},
       {32, 16, 32},
       {16, 32, 32},
       {32, 32, 32}}}}};

} // namespace tinytc
