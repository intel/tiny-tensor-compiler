// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef LINALG_BLAS_A2_20241025_HPP
#define LINALG_BLAS_A2_20241025_HPP

#include "linalg_types.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <cstdint>
#include <functional>
#include <stdexcept>
#include <utility>

namespace tinytc::test {

auto make_blas_a2_prog(char const *name, tensor_layout const &layoutA, tensor_layout const &layoutB,
                       scalar_type alpha_ty, scalar_type A_ty, scalar_type beta_ty,
                       scalar_type B_ty,
                       std::function<void(region_builder &, array_view<value>)> make_op,
                       std::int32_t work_group_size = 0) -> prog;

template <typename AlphaT, typename AT, typename BetaT, typename BT> class axpby {
  public:
    using alpha_type = AlphaT;
    using A_type = AT;
    using beta_type = BetaT;
    using B_type = BT;
    static constexpr char const *kernel_name = "axpby";

    axpby(transpose tA, tensor_layout layoutA, tensor_layout layoutB)
        : tA_(tA), lA_{std::move(layoutA)}, lB_{std::move(layoutB)} {}

    auto lA() const -> tensor_layout const & { return lA_; }
    auto lB() const -> tensor_layout const & { return lB_; }

    auto make_prog() const -> prog {
        return make_blas_a2_prog(
            kernel_name, lA_, lB_, to_scalar_type_v<AlphaT>, to_scalar_type_v<AT>,
            to_scalar_type_v<BetaT>, to_scalar_type_v<BT>,
            [&](region_builder &bb, array_view<value> params) {
                bb.add(make_axpby(tA_, false, params[0], params[1], params[2], params[3]));
            });
    }
    void reference_impl(AlphaT alpha, AT const *A, BetaT beta, BT *B) {
        if (lA_.dim() == 0 && lB_.dim() == 0) {
            *B = alpha * (*A) + beta * (*B);
        } else if (lA_.dim() == 1 && lB_.dim() == 1) {
            const auto M = lB_.shape(0);
            if (M != lA_.shape(0)) {
                throw std::runtime_error("incompatible axpby");
            }
            for (std::int64_t m = 0; m < M; ++m) {
                auto &b = B[lB_.linear_index({m})];
                b = alpha * A[lA_.linear_index({m})] + beta * b;
            }
        } else if (lA_.dim() == 2 && lB_.dim() == 2) {
            const int A_mmode = tA_ == transpose::T ? 1 : 0;
            const auto M = lB_.shape(0);
            const auto N = lB_.shape(1);
            if (M != lA_.shape(A_mmode) || N != lA_.shape(1 - A_mmode)) {
                throw std::runtime_error("incompatible axpby");
            }
            for (std::int64_t n = 0; n < N; ++n) {
                for (std::int64_t m = 0; m < M; ++m) {
                    auto &b = B[lB_.linear_index({m, n})];
                    b = alpha * A[lA_.linear_index(make_index_2d(tA_, m, n))] + beta * b;
                }
            }
        } else {
            throw std::runtime_error("invald axpby dimension combination");
        }
    }

  private:
    transpose tA_;
    tensor_layout lA_, lB_;
};

template <typename AlphaT, typename AT, typename BetaT, typename BT> class sum {
  public:
    using alpha_type = AlphaT;
    using A_type = AT;
    using beta_type = BetaT;
    using B_type = BT;
    static constexpr char const *kernel_name = "sum";

    sum(transpose tA, tensor_layout layoutA, tensor_layout layoutB,
        std::int32_t work_group_size = 0)
        : tA_(tA), lA_{std::move(layoutA)}, lB_{std::move(layoutB)},
          work_group_size_{work_group_size} {}

    auto lA() const -> tensor_layout const & { return lA_; }
    auto lB() const -> tensor_layout const & { return lB_; }

    auto make_prog() const -> prog {
        return make_blas_a2_prog(
            kernel_name, lA_, lB_, to_scalar_type_v<AlphaT>, to_scalar_type_v<AT>,
            to_scalar_type_v<BetaT>, to_scalar_type_v<BT>,
            [&](region_builder &bb, array_view<value> params) {
                bb.add(make_sum(tA_, false, params[0], params[1], params[2], params[3]));
            },
            work_group_size_);
    }
    void reference_impl(AlphaT alpha, AT const *A, BetaT beta, BT *B) {
        if (lA_.dim() == 1 && lB_.dim() == 0) {
            const auto M = lA_.shape(0);
            AT a_acc = AT{0};
            for (std::int64_t m = 0; m < M; ++m) {
                a_acc += A[lA_.linear_index({m})];
            }
            *B = alpha * a_acc + beta * (*B);
        } else if (lA_.dim() == 2 && lB_.dim() == 1) {
            const int A_nmode = tA_ == transpose::T ? 0 : 1;
            const auto M = lB_.shape(0);
            const auto N = lA_.shape(A_nmode);
            if (M != lA_.shape(1 - A_nmode)) {
                throw std::runtime_error("incompatible sum");
            }
            for (std::int64_t m = 0; m < M; ++m) {
                auto &b = B[lB_.linear_index({m})];
                AT a_acc = AT{0};
                for (std::int64_t n = 0; n < N; ++n) {
                    a_acc += A[lA_.linear_index(make_index_2d(tA_, m, n))];
                }
                b = alpha * a_acc + beta * b;
            }
        } else {
            throw std::runtime_error("invald sum dimension combination");
        }
    }

  private:
    transpose tA_;
    tensor_layout lA_, lB_;
    std::int32_t work_group_size_;
};

} // namespace tinytc::test

#endif // LINALG_BLAS_A2_20241025_HPP
