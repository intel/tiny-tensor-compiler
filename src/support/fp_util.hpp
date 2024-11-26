// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef FP_UTIL_20241126_HPP
#define FP_UTIL_20241126_HPP

#include "tinytc/tinytc.hpp"

#include <complex>
#include <type_traits>

namespace tinytc {

template <class T, template <typename...> class U>
struct is_instance_of : public std::false_type {};
template <template <typename...> class U, typename... Vs>
struct is_instance_of<U<Vs...>, U> : public std::true_type {};
template <class T, template <typename...> class U>
inline constexpr bool is_instance_of_v = is_instance_of<T, U>::value;

template <typename T> struct is_complex : public std::false_type {};
template <typename F>
requires(std::is_floating_point_v<F>)
struct is_complex<std::complex<F>> : public std::true_type {};
template <typename T> inline constexpr bool is_complex_v = is_complex<T>::value;

template <typename T>
struct is_floating_point_or_lp_float
    : public std::integral_constant<bool, std::is_floating_point_v<T> ||
                                              is_instance_of_v<T, lp_float>> {};
template <typename T>
inline constexpr bool is_floating_point_or_lp_float_v = is_floating_point_or_lp_float<T>::value;

} // namespace tinytc

#endif // FP_UTIL_20241126_HPP
