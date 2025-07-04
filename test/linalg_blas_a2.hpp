// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef LINALG_BLAS_A2_20241025_HPP
#define LINALG_BLAS_A2_20241025_HPP

#include "linalg_types.hpp"
#include "tinytc/builder.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <cstdint>
#include <functional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace tinytc::test {

auto make_blas_a2_prog(char const *name, tensor_layout const &layoutA, tensor_layout const &layoutB,
                       tinytc_type_t alpha_ty, tinytc_type_t A_ty, tinytc_type_t beta_ty,
                       tinytc_type_t B_ty,
                       std::function<void(region_builder &, array_view<tinytc_value_t>)> make_op,
                       std::int32_t work_group_size = 0) -> shared_handle<tinytc_prog_t>;

template <typename AlphaT, typename AT, typename BetaT, typename BT>
auto make_blas_a2_prog(char const *name, tensor_layout const &layoutA, tensor_layout const &layoutB,
                       std::function<void(region_builder &, array_view<tinytc_value_t>)> make_op,
                       std::int32_t work_group_size = 0) -> shared_handle<tinytc_prog_t> {
    auto ctx = make_compiler_context();
    return make_blas_a2_prog(name, layoutA, layoutB, to_type<AlphaT>(ctx.get()),
                             to_type<AT>(ctx.get()), to_type<BetaT>(ctx.get()),
                             to_type<BT>(ctx.get()), std::move(make_op), work_group_size);
}

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

    auto make_prog() const -> shared_handle<tinytc_prog_t> {
        return make_blas_a2_prog<AlphaT, AT, BetaT, BT>(
            kernel_name, lA_, lB_, [&](region_builder &bb, array_view<tinytc_value_t> params) {
                bb.create<axpby_inst>(false, tA_, params[0], params[1], params[2], params[3]);
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

template <typename AlphaT, typename AT, typename BetaT, typename BT> class cumsum {
  public:
    using alpha_type = AlphaT;
    using A_type = AT;
    using beta_type = BetaT;
    using B_type = BT;
    static constexpr char const *kernel_name = "cumsum";

    cumsum(tensor_layout layoutA, std::int64_t mode, tensor_layout layoutB,
           std::int32_t work_group_size = 0)
        : lA_{std::move(layoutA)}, mode_{mode}, lB_{std::move(layoutB)},
          work_group_size_{work_group_size} {}

    auto lA() const -> tensor_layout const & { return lA_; }
    auto lB() const -> tensor_layout const & { return lB_; }

    auto make_prog() const -> shared_handle<tinytc_prog_t> {
        return make_blas_a2_prog<AlphaT, AT, BetaT, BT>(
            kernel_name, lA_, lB_,
            [&](region_builder &bb, array_view<tinytc_value_t> params) {
                bb.create<cumsum_inst>(false, mode_, params[0], params[1], params[2], params[3]);
            },
            work_group_size_);
    }
    void reference_impl(AlphaT alpha, AT const *A, BetaT beta, BT *B) {
        if (lA_.dim() != lB_.dim() || lB_.dim() == 0) {
            throw std::runtime_error("unsupported cumsum dimension combination");
        }
        for (std::int64_t i = 0; i < lB_.dim(); ++i) {
            if (lA_.shape(i) != lB_.shape(i)) {
                throw std::runtime_error("incompatible cumsum");
            }
        }

        auto J = lB_.shape(mode_);
        auto const inner_loop = [&](std::vector<std::int64_t> &index) {
            AT prefix = AT{0};
            for (std::int64_t j = 0; j < J; ++j) {
                index[mode_] = j;
                prefix += A[lA_.linear_index(index)];
                auto &b = B[lB_.linear_index(index)];
                b = alpha * prefix + beta * b;
            }
        };

        auto index = std::vector<std::int64_t>(lB_.dim(), 0);
        if (lB_.dim() == 1) {
            inner_loop(index);
        } else {
            auto reduced_shape = std::vector<std::int64_t>{lB_.shape()};
            reduced_shape.erase(reduced_shape.begin() + mode_);
            nd_foreach(reduced_shape, [&](array_view<std::int64_t> reduced_index) {
                for (std::int64_t i = 0; i < mode_; ++i) {
                    index[i] = reduced_index[i];
                }
                for (std::int64_t i = mode_ + 1; i < lB_.dim(); ++i) {
                    index[i] = reduced_index[i - 1];
                }
                inner_loop(index);
            });
        }
    }

  private:
    tensor_layout lA_;
    std::int64_t mode_;
    tensor_layout lB_;
    std::int32_t work_group_size_;
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

    auto make_prog() const -> shared_handle<tinytc_prog_t> {
        return make_blas_a2_prog<AlphaT, AT, BetaT, BT>(
            kernel_name, lA_, lB_,
            [&](region_builder &bb, array_view<tinytc_value_t> params) {
                bb.create<sum_inst>(false, tA_, params[0], params[1], params[2], params[3]);
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
            throw std::runtime_error("unsupported sum dimension combination");
        }
    }

  private:
    transpose tA_;
    tensor_layout lA_, lB_;
    std::int32_t work_group_size_;
};

} // namespace tinytc::test

#endif // LINALG_BLAS_A2_20241025_HPP
