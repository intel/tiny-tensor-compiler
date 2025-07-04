// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef LINALG_BLAS_A3_20241025_HPP
#define LINALG_BLAS_A3_20241025_HPP

#include "linalg_types.hpp"
#include "tinytc/builder.hpp"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <utility>

namespace tinytc::test {

auto gemm_mnk(transpose tA, transpose tB, tensor_layout const &A, tensor_layout const &B,
              tensor_layout const &C) -> std::array<std::int64_t, 3u>;
auto gemv_mk(transpose tA, tensor_layout const &A, tensor_layout const &B, tensor_layout const &C)
    -> std::array<std::int64_t, 2u>;
auto ger_mn(tensor_layout const &A, tensor_layout const &B, tensor_layout const &C)
    -> std::array<std::int64_t, 2u>;
auto hadamard_m(tensor_layout const &A, tensor_layout const &B, tensor_layout const &C)
    -> std::int64_t;
auto hadamard_mn(tensor_layout const &A, tensor_layout const &B, tensor_layout const &C)
    -> std::array<std::int64_t, 2u>;

auto make_blas_a3_prog(char const *name, tensor_layout const &layoutA, tensor_layout const &layoutB,
                       tensor_layout const &layoutC, tinytc_type_t alpha_ty, tinytc_type_t A_ty,
                       tinytc_type_t B_ty, tinytc_type_t beta_ty, tinytc_type_t C_ty,
                       std::function<void(region_builder &, array_view<tinytc_value_t>)> make_op)
    -> shared_handle<tinytc_prog_t>;

template <typename AlphaT, typename AT, typename BT, typename BetaT, typename CT>
auto make_blas_a3_prog(char const *name, tensor_layout const &layoutA, tensor_layout const &layoutB,
                       tensor_layout const &layoutC,
                       std::function<void(region_builder &, array_view<tinytc_value_t>)> make_op)
    -> shared_handle<tinytc_prog_t> {
    auto ctx = make_compiler_context();
    return make_blas_a3_prog(name, layoutA, layoutB, layoutC, to_type<AlphaT>(ctx.get()),
                             to_type<AT>(ctx.get()), to_type<BT>(ctx.get()),
                             to_type<BetaT>(ctx.get()), to_type<CT>(ctx.get()), std::move(make_op));
}

template <typename AlphaT, typename AT, typename BT, typename BetaT, typename CT> class gemm {
  public:
    using alpha_type = AlphaT;
    using A_type = AT;
    using B_type = BT;
    using beta_type = BetaT;
    using C_type = CT;
    static constexpr char const *kernel_name = "gemm";

    gemm(transpose tA, transpose tB, tensor_layout layoutA, tensor_layout layoutB,
         tensor_layout layoutC)
        : tA_(tA), tB_(tB), lA_{std::move(layoutA)}, lB_{std::move(layoutB)},
          lC_{std::move(layoutC)} {}

    auto lA() const -> tensor_layout const & { return lA_; }
    auto lB() const -> tensor_layout const & { return lB_; }
    auto lC() const -> tensor_layout const & { return lC_; }

    auto make_prog() const -> shared_handle<tinytc_prog_t> {
        return make_blas_a3_prog<AlphaT, AT, BT, BetaT, CT>(
            kernel_name, lA_, lB_, lC_, [&](region_builder &bb, array_view<tinytc_value_t> params) {
                bb.create<gemm_inst>(false, tA_, tB_, params[0], params[1], params[2], params[3],
                                     params[4]);
            });
    }
    void reference_impl(AlphaT alpha, AT const *A, BT const *B, BetaT beta, CT *C) {
        const auto [M, N, K] = gemm_mnk(tA_, tB_, lA_, lB_, lC_);
        for (std::int64_t n = 0; n < N; ++n) {
            for (std::int64_t m = 0; m < M; ++m) {
                CT c_acc = CT{0};
                for (std::int64_t k = 0; k < K; ++k) {
                    c_acc = c_acc + A[lA_.linear_index(make_index_2d(tA_, m, k))] *
                                        B[lB_.linear_index(make_index_2d(tB_, k, n))];
                }
                auto &c = C[lC_.linear_index({m, n})];
                c = alpha * c_acc + beta * c;
            }
        }
    }

  private:
    transpose tA_, tB_;
    tensor_layout lA_, lB_, lC_;
};

template <typename AlphaT, typename AT, typename BT, typename BetaT, typename CT> class gemv {
  public:
    using alpha_type = AlphaT;
    using A_type = AT;
    using B_type = BT;
    using beta_type = BetaT;
    using C_type = CT;
    static constexpr char const *kernel_name = "gemv";

    gemv(transpose tA, tensor_layout layoutA, tensor_layout layoutB, tensor_layout layoutC)
        : tA_(tA), lA_{std::move(layoutA)}, lB_{std::move(layoutB)}, lC_{std::move(layoutC)} {}

    auto lA() const -> tensor_layout const & { return lA_; }
    auto lB() const -> tensor_layout const & { return lB_; }
    auto lC() const -> tensor_layout const & { return lC_; }

    auto make_prog() const -> shared_handle<tinytc_prog_t> {
        return make_blas_a3_prog<AlphaT, AT, BT, BetaT, CT>(
            kernel_name, lA_, lB_, lC_, [&](region_builder &bb, array_view<tinytc_value_t> params) {
                bb.create<gemv_inst>(false, tA_, params[0], params[1], params[2], params[3],
                                     params[4]);
            });
    }
    void reference_impl(AlphaT alpha, AT const *A, BT const *B, BetaT beta, CT *C) {
        const auto [M, K] = gemv_mk(tA_, lA_, lB_, lC_);
        for (std::int64_t m = 0; m < M; ++m) {
            CT c_acc = CT{0};
            for (std::int64_t k = 0; k < K; ++k) {
                c_acc = c_acc +
                        A[lA_.linear_index(make_index_2d(tA_, m, k))] * B[lB_.linear_index({k})];
            }
            auto &c = C[lC_.linear_index({m})];
            c = alpha * c_acc + beta * c;
        }
    }

  private:
    transpose tA_;
    tensor_layout lA_, lB_, lC_;
};

template <typename AlphaT, typename AT, typename BT, typename BetaT, typename CT> class ger {
  public:
    using alpha_type = AlphaT;
    using A_type = AT;
    using B_type = BT;
    using beta_type = BetaT;
    using C_type = CT;
    static constexpr char const *kernel_name = "ger";

    ger(tensor_layout layoutA, tensor_layout layoutB, tensor_layout layoutC)
        : lA_{std::move(layoutA)}, lB_{std::move(layoutB)}, lC_{std::move(layoutC)} {}

    auto lA() const -> tensor_layout const & { return lA_; }
    auto lB() const -> tensor_layout const & { return lB_; }
    auto lC() const -> tensor_layout const & { return lC_; }

    auto make_prog() const -> shared_handle<tinytc_prog_t> {
        return make_blas_a3_prog<AlphaT, AT, BT, BetaT, CT>(
            kernel_name, lA_, lB_, lC_, [&](region_builder &bb, array_view<tinytc_value_t> params) {
                bb.create<ger_inst>(false, params[0], params[1], params[2], params[3], params[4]);
            });
    }
    void reference_impl(AlphaT alpha, AT const *A, BT const *B, BetaT beta, CT *C) {
        const auto [M, N] = ger_mn(lA_, lB_, lC_);
        for (std::int64_t n = 0; n < N; ++n) {
            for (std::int64_t m = 0; m < M; ++m) {
                auto ab = A[lA_.linear_index({m})] * B[lB_.linear_index({n})];
                auto &c = C[lC_.linear_index({m, n})];
                c = alpha * ab + beta * c;
            }
        }
    }

  private:
    tensor_layout lA_, lB_, lC_;
};

template <typename AlphaT, typename AT, typename BT, typename BetaT, typename CT> class hadamard {
  public:
    using alpha_type = AlphaT;
    using A_type = AT;
    using B_type = BT;
    using beta_type = BetaT;
    using C_type = CT;
    static constexpr char const *kernel_name = "hadamard";

    hadamard(tensor_layout layoutA, tensor_layout layoutB, tensor_layout layoutC)
        : lA_{std::move(layoutA)}, lB_{std::move(layoutB)}, lC_{std::move(layoutC)} {}

    auto lA() const -> tensor_layout const & { return lA_; }
    auto lB() const -> tensor_layout const & { return lB_; }
    auto lC() const -> tensor_layout const & { return lC_; }

    auto make_prog() const -> shared_handle<tinytc_prog_t> {
        return make_blas_a3_prog<AlphaT, AT, BT, BetaT, CT>(
            kernel_name, lA_, lB_, lC_, [&](region_builder &bb, array_view<tinytc_value_t> params) {
                bb.create<hadamard_inst>(false, params[0], params[1], params[2], params[3],
                                         params[4]);
            });
    }
    void reference_impl(AlphaT alpha, AT const *A, BT const *B, BetaT beta, CT *C) {
        if (lC().dim() == 2) {
            const auto [M, N] = hadamard_mn(lA_, lB_, lC_);
            for (std::int64_t n = 0; n < N; ++n) {
                for (std::int64_t m = 0; m < M; ++m) {
                    auto ab = A[lA_.linear_index({m, n})] * B[lB_.linear_index({m, n})];
                    auto &c = C[lC_.linear_index({m, n})];
                    c = alpha * ab + beta * c;
                }
            }
        } else {
            const auto M = hadamard_m(lA_, lB_, lC_);
            for (std::int64_t m = 0; m < M; ++m) {
                auto ab = A[lA_.linear_index({m})] * B[lB_.linear_index({m})];
                auto &c = C[lC_.linear_index({m})];
                c = alpha * ab + beta * c;
            }
        }
    }

  private:
    tensor_layout lA_, lB_, lC_;
};

} // namespace tinytc::test

#endif // LINALG_BLAS_A3_20241025_HPP
