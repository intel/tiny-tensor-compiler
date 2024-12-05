// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "matrix_ext_info.hpp"
#include "node/data_type_node.hpp"
#include "tinytc/types.hpp"

#include <algorithm>

namespace tinytc {

matrix_ext_type::matrix_ext_type(scalar_type a, scalar_type b,
                                 std::initializer_list<scalar_type> acc,
                                 std::initializer_list<gemm_mnk> mnk)
    : a_{a}, b_{b} {
    std::copy(acc.begin(), acc.end(), acc_.begin());
    acc_size_ = acc.size();
    std::copy(mnk.begin(), mnk.end(), mnk_.begin());
    mnk_size_ = mnk.size();
}

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
    for (auto const &type : types_) {
        if (type.a() == a && type.b() == b && type.have_acc(acc)) {
            return true;
        }
    }
    return false;
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

} // namespace tinytc
