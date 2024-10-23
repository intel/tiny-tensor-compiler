// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef LINALG_RUNNER_20241023_HPP
#define LINALG_RUNNER_20241023_HPP

#include "linalg_ops.hpp"
#include "linalg_types.hpp"
#include "runtime_concept.hpp"
#include "tinytc/tinytc.hpp"

#include <cstdint>
#include <doctest/doctest.h>
#include <type_traits>

namespace tinytc::test {

template <typename T> struct is_complex : public std::false_type {};
template <typename T> struct is_complex<std::complex<T>> : public std::true_type {};
template <typename T> inline constexpr bool is_complex_v = is_complex<T>::value;
template <typename T>
constexpr bool requires_dp_v = std::is_same_v<T, double> || std::is_same_v<T, std::complex<double>>;

template <test_runtime_gpu R, op_blas_a3 T>
void test_blas_a3(T op, typename T::alpha_type alpha, typename T::beta_type beta) {
    auto gpu_rt = std::make_shared<R>();
    if constexpr (requires_dp_v<typename T::alpha_type> || requires_dp_v<typename T::A_type> ||
                  requires_dp_v<typename T::B_type> || requires_dp_v<typename T::beta_type> ||
                  requires_dp_v<typename T::C_type>) {
        if (!gpu_rt->supports_fp64()) {
            WARN_MESSAGE(false, "Double precision tests need double precision device support");
            return;
        }
    }

    auto const make_test_data = []<typename ScalarT>(std::size_t size) {
        auto data = std::vector<ScalarT>(size);
        for (std::size_t i = 0; i < data.size(); ++i) {
            constexpr std::size_t prime = 101;
            if constexpr (is_complex_v<ScalarT>) {
                data[i] = ScalarT{static_cast<ScalarT::value_type>((2 * i) % prime),
                                  static_cast<ScalarT::value_type>((2 * i + 1) % prime)};
            } else {
                data[i] = static_cast<ScalarT>(i % prime);
            }
        }
        return data;
    };
    auto const compare_data = []<typename ScalarT>(std::vector<ScalarT> const &A,
                                                   std::vector<ScalarT> const &B) {
        REQUIRE(A.size() == B.size());
        for (std::size_t i = 0; i < A.size(); ++i) {
            constexpr auto eps =
                10.0 * std::numeric_limits<decltype(std::abs(ScalarT{}))>::epsilon();
            REQUIRE(std::abs(A[i] - B[i]) == doctest::Approx(0.0).epsilon(eps));
        }
    };

    auto A_ref = make_test_data.template operator()<typename T::A_type>(op.lA().size());
    auto B_ref = make_test_data.template operator()<typename T::B_type>(op.lB().size());
    auto C_ref = std::vector<typename T::C_type>(op.lC().size());

    op.reference_impl(alpha, A_ref.data(), B_ref.data(), beta, C_ref.data());

    auto A = gpu_rt->create_buffer(A_ref.size() * sizeof(typename T::A_type));
    auto B = gpu_rt->create_buffer(B_ref.size() * sizeof(typename T::B_type));
    auto C = gpu_rt->create_buffer(C_ref.size() * sizeof(typename T::C_type));
    gpu_rt->memcpy_h2d(A, A_ref.data(), A_ref.size() * sizeof(typename T::A_type));
    gpu_rt->memcpy_h2d(B, B_ref.data(), B_ref.size() * sizeof(typename T::B_type));
    gpu_rt->fill_buffer(C, 0, C_ref.size() * sizeof(typename T::C_type));

    auto bundle = gpu_rt->get_kernel_bundle(op.make_prog());
    auto kernel = gpu_rt->get_kernel(bundle, T::kernel_name);

    auto const set_dope_vector = [&](tensor_layout const &layout, std::uint32_t &arg_index) {
        for (std::size_t i = 0; i < layout.shape().size(); ++i) {
            if (layout.static_shape(i) == dynamic) {
                std::int64_t s = layout.shape(i);
                gpu_rt->set_arg(kernel, arg_index++, sizeof(s), &s);
            }
        }
        for (std::size_t i = 0; i < layout.stride().size(); ++i) {
            if (layout.static_stride(i) == dynamic) {
                std::int64_t s = layout.stride(i);
                gpu_rt->set_arg(kernel, arg_index++, sizeof(s), &s);
            }
        }
    };

    std::uint32_t i = 0;
    gpu_rt->set_arg(kernel, i++, sizeof(typename T::alpha_type), &alpha);
    gpu_rt->set_mem_arg(kernel, i++, A, auto_mem_type_v<typename R::mem_t>);
    set_dope_vector(op.lA(), i);
    gpu_rt->set_mem_arg(kernel, i++, B, auto_mem_type_v<typename R::mem_t>);
    set_dope_vector(op.lB(), i);
    gpu_rt->set_arg(kernel, i++, sizeof(typename T::beta_type), &beta);
    gpu_rt->set_mem_arg(kernel, i++, C, auto_mem_type_v<typename R::mem_t>);
    set_dope_vector(op.lC(), i);
    gpu_rt->submit(kernel);
    gpu_rt->synchronize();

    auto C_host = std::vector<typename T::C_type>(C_ref.size());
    gpu_rt->memcpy_d2h(C_host.data(), C, C_host.size() * sizeof(typename T::C_type));

    compare_data(C_host, C_ref);

    gpu_rt->free_buffer(A);
    gpu_rt->free_buffer(B);
    gpu_rt->free_buffer(C);
}

} // namespace tinytc::test

#endif // LINALG_RUNNER_20241023_HPP
