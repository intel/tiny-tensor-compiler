// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef COOPMATRIX_INFO_20241204_HPP
#define COOPMATRIX_INFO_20241204_HPP

#include "tinytc/tinytc.hpp"
#include "tinytc/types.hpp"

#include <array>
#include <cstdint>
#include <utility>

namespace tinytc {

struct coopmatrix_mnk {
    std::int64_t M, N, K;
};

class accelerated_coopmatrix_type {
  public:
    constexpr accelerated_coopmatrix_type(scalar_type A, scalar_type B,
                                          std::initializer_list<scalar_type> C, scalar_type D,
                                          std::initializer_list<coopmatrix_mnk> mnk)
        : A_{A}, B_{B}, D_{D} {
        std::copy(C.begin(), C.end(), C_.begin());
        C_size_ = C.size();
        std::copy(mnk.begin(), mnk.end(), mnk_.begin());
        mnk_size_ = mnk.size();
    }

    inline auto A() const -> scalar_type { return A_; }
    inline auto B() const -> scalar_type { return B_; }
    inline auto C() const -> array_view<scalar_type> { return array_view(C_.data(), C_size_); }
    inline auto D() const -> scalar_type { return D_; }
    inline auto mnk() const -> array_view<coopmatrix_mnk> {
        return array_view(mnk_.data(), mnk_size_);
    }

  private:
    scalar_type A_, B_, D_;
    std::array<scalar_type, 2u> C_;
    std::array<coopmatrix_mnk, 4u> mnk_;
    std::size_t C_size_, mnk_size_;
};

class accelerated_coopmatrix_info {
  public:
    accelerated_coopmatrix_info() = default;
    inline accelerated_coopmatrix_info(std::int32_t required_subgroup_size,
                                       array_view<accelerated_coopmatrix_type> types)
        : required_sgs_{required_subgroup_size}, types_(std::move(types)) {}

    auto have_precision(scalar_type A, scalar_type B, scalar_type D) const -> bool;

    inline auto required_subgroup_size() const -> std::int32_t { return required_sgs_; }

  private:
    std::int32_t required_sgs_;
    array_view<accelerated_coopmatrix_type> types_;
};

extern const std::array<accelerated_coopmatrix_type, 5u> pvc_accelerated_coopmatrix_types;

} // namespace tinytc

#endif // COOPMATRIX_INFO_20241204_HPP
