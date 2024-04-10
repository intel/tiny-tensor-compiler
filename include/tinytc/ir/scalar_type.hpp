// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SCALAR_TYPE_20230330_HPP
#define SCALAR_TYPE_20230330_HPP

#include "tinytc/types.h"
#include "tinytc/export.h"

#include "clir/builtin_type.hpp"
#include "clir/data_type.hpp"

#include <cstddef>
#include <cstdint>

namespace tinytc {

//! Scalar types
enum class scalar_type {
    bool_ = tinytc_bool,  ///< boolean
    index = tinytc_index, ///< Unsigned integer type for indices
    i8 = tinytc_i8,       ///< Signed 8 bit integer
    i16 = tinytc_i16,     ///< Signed 16 bit integer
    i32 = tinytc_i32,     ///< Signed 32 bit integer
    i64 = tinytc_i64,     ///< Signed 64 bit integer
    u8 = tinytc_u8,       ///< Unsigned 8 bit integer
    u16 = tinytc_u16,     ///< Unsigned 16 bit integer
    u32 = tinytc_u32,     ///< Unsigned 32 bit integer
    u64 = tinytc_u64,     ///< Unsigned 64 bit integer
    f32 = tinytc_f32,     ///< Single precision floating point (32 bit)
    f64 = tinytc_f64      ///< Double precision floating point (64 bit)
};

//! Convert scalar type to string
TINYTC_EXPORT char const *to_string(scalar_type ty);
//! Size of scalar type in bytes
TINYTC_EXPORT std::size_t size(scalar_type ty);
//! Returns true if ty is a floating point type
TINYTC_EXPORT bool is_floating_type(scalar_type ty);

/**
 * Returns the scalar type corresponding to C++ type T
 *
 * Specializations exist for bool, (u)int8_t, (u)int16_t, (u)int32_t, (u)int64_t, float, double.
 * The scalar_type is stored in the static constexpr member "value".
 */
template <typename T> struct to_scalar_type;
//! to_scalar_type specialization
template <> struct to_scalar_type<bool> {
    static constexpr scalar_type value = scalar_type::bool_; ///< value
};
//! to_scalar_type specialization
template <> struct to_scalar_type<int8_t> {
    static constexpr scalar_type value = scalar_type::i8; ///< value
};
//! to_scalar_type specialization
template <> struct to_scalar_type<int16_t> {
    static constexpr scalar_type value = scalar_type::i16; ///< value
};
//! to_scalar_type specialization
template <> struct to_scalar_type<int32_t> {
    static constexpr scalar_type value = scalar_type::i32; ///< value
};
//! to_scalar_type specialization
template <> struct to_scalar_type<int64_t> {
    static constexpr scalar_type value = scalar_type::i64; ///< value
};
//! to_scalar_type specialization
template <> struct to_scalar_type<uint8_t> {
    static constexpr scalar_type value = scalar_type::u8; ///< value
};
//! to_scalar_type specialization
template <> struct to_scalar_type<uint16_t> {
    static constexpr scalar_type value = scalar_type::u16; ///< value
};
//! to_scalar_type specialization
template <> struct to_scalar_type<uint32_t> {
    static constexpr scalar_type value = scalar_type::u32; ///< value
};
//! to_scalar_type specialization
template <> struct to_scalar_type<uint64_t> {
    static constexpr scalar_type value = scalar_type::u64; ///< value
};
//! to_scalar_type specialization
template <> struct to_scalar_type<float> {
    static constexpr scalar_type value = scalar_type::f32; ///< value
};
//! to_scalar_type specialization
template <> struct to_scalar_type<double> {
    static constexpr scalar_type value = scalar_type::f64; ///< value
};
/**
 * Convenience variable for to_scalar_type.
 *
 * Example: @code scalar_type ty = to_scalar_type_v<float>; @endcode
 */
template <typename T> inline constexpr scalar_type to_scalar_type_v = to_scalar_type<T>::value;

namespace internal {
TINYTC_EXPORT clir::builtin_type to_clir_builtin_ty(scalar_type ty);
TINYTC_EXPORT clir::builtin_type to_clir_atomic_builtin_ty(scalar_type ty);
TINYTC_EXPORT clir::data_type to_clir_ty(scalar_type ty,
                                         clir::address_space as = clir::address_space::generic_t,
                                         clir::type_qualifier q = clir::type_qualifier::none);
TINYTC_EXPORT clir::data_type
to_clir_atomic_ty(scalar_type ty, clir::address_space as = clir::address_space::generic_t,
                  clir::type_qualifier q = clir::type_qualifier::none);
} // namespace internal

} // namespace tinytc

#endif // SCALAR_TYPE_20230330_HPP
