// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TINYTC_20240403_HPP
#define TINYTC_20240403_HPP

#include "tinytc/tinytc.h"
#include "tinytc/types.hpp"

#include <array>
#include <complex>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

// For bit_cast, memcpy for C++ < 2020
#if __cplusplus >= 202002L
#include <bit>
#else
#include <cstring>
#endif

namespace tinytc {

////////////////////////////
/////////// Error //////////
////////////////////////////

//! Convert error code to string
inline char const *error_string(status code) {
    return ::tinytc_error_string(static_cast<::tinytc_status_t>(code));
}

//! Throw exception for unsuccessful call to C-API
inline void CHECK_STATUS(tinytc_status_t code) {
    if (code != tinytc_status_success) {
        throw status{std::underlying_type_t<status>(code)};
    }
}

//! Builder exception enhanced with location
class builder_error : public std::exception {
  public:
    //! ctor; taking location and status code
    inline builder_error(status code, location const &loc) : code_(code), loc_(loc) {}
    //! Get status code
    inline auto code() const noexcept { return code_; }
    //! Get location
    inline auto loc() const noexcept -> location const & { return loc_; }
    //! Get explanatory string
    inline char const *what() const noexcept override { return error_string(code_); }

  private:
    status code_;
    location loc_;
};

//! Throw exception for unsuccessful call to C-API
inline void CHECK_STATUS_LOC(tinytc_status_t code, location const &loc) {
    if (code != tinytc_status_success) {
        throw builder_error(status{std::underlying_type_t<status>(code)}, loc);
    }
}

////////////////////////////
////////// FP math /////////
////////////////////////////

/**
 * @brief IEEE754 floating point format parameters
 *
 * @tparam ExponentBits Number of exponent bits
 * @tparam MantissaBits Number of mantissa bits
 */
template <uint32_t ExponentBits, uint32_t MantissaBits> struct ieee754_format {
    constexpr static uint32_t exponent_bits = ExponentBits; ///< Number of exponent bits
    constexpr static uint32_t mantissa_bits = MantissaBits; ///< Number of mantissa bits
    //! Total number of bits
    constexpr static uint32_t num_bits = 1 + exponent_bits + mantissa_bits;
    //! Bias
    constexpr static uint32_t bias = (1 << (exponent_bits - 1)) - 1;
    //! Max exponent when encoded with bias added
    constexpr static uint32_t max_biased_exponent = (1 << exponent_bits) - 1;
    //! Bit mask for sign bit
    constexpr static uint32_t sign_mask = 1 << (num_bits - 1);
    //! Bit mask for exponent bits
    constexpr static uint32_t exponent_mask = max_biased_exponent << mantissa_bits;
    //! Bit mask for exponent mantissa bits
    constexpr static uint32_t mantissa_mask = (1 << mantissa_bits) - 1;
    //! Number of bytes
    constexpr static uint32_t num_bytes = 1 + (num_bits - 1) / 8;
    //! Unsigned integer type large enough to store bit pattern
    using bits_type = std::conditional_t<
        num_bytes == 1, std::uint8_t,
        std::conditional_t<
            num_bytes == 2, std::uint16_t,
            std::conditional_t<num_bytes == 4, std::uint32_t,
                               std::conditional_t<num_bytes == 8, std::uint64_t, void>>>>;
};

//! Floating point format for bf16 (bfloat16)
using bf16_format = ieee754_format<8, 7>;
//! Floating point format for f16 (half)
using f16_format = ieee754_format<5, 10>;
//! Floating point format for f32 (float)
using f32_format = ieee754_format<8, 23>;

/**
 * @brief Truncate high precision floating point number and return low precision floating point
 * number
 *
 * @tparam F16f low precision floating point format
 * @tparam F32f high precision floating point format
 * @param x bit pattern of high precision number
 *
 * @return bit pattern of low precision number
 */
template <typename F16f, typename F32f>
constexpr auto ieee754_truncate(typename F32f::bits_type x) -> F16f::bits_type {
    using UI = F32f::bits_type;
    using UITrunc = F16f::bits_type;
    constexpr UI num_shift_bits = F32f::mantissa_bits - F16f::mantissa_bits;
    auto const round_nearest_even_and_truncate = [](UI mantissa32) {
        constexpr UI midpoint = (1 << num_shift_bits) / 2;
        const UI bias = ((mantissa32 >> num_shift_bits) & 0x1) + (midpoint - 1);
        return (mantissa32 + bias) >> num_shift_bits;
    };

    const UITrunc sign = (x & F32f::sign_mask) >> (F32f::num_bits - F16f::num_bits);
    const UI exponent32 = (x & F32f::exponent_mask) >> F32f::mantissa_bits;
    const UI mantissa32 = x & F32f::mantissa_mask;

    UITrunc exponent16 = 0;
    UITrunc mantissa16 = 0;
    if (exponent32 > F32f::bias + F16f::bias) {
        exponent16 = F16f::max_biased_exponent;
        // Map numbers except NaN to inf
        if (exponent32 < F32f::max_biased_exponent) {
            mantissa16 = 0;
        } else {
            // Need to ceil to make sure that NaN is not truncated to inf
            mantissa16 = 1 + ((mantissa32 - 1) >> num_shift_bits);
        }
    } else if (F32f::bias == F16f::bias || exponent32 > F32f::bias - F16f::bias) {
        // convert bias
        // E_{32} = e + F32f::bias
        // E_{16} = e + F16f::bias
        //        = E_{32} - F32f::bias + F16f::bias
        //        = E_{32} - (F32f::bias - F16f::bias)
        exponent16 = exponent32 - (F32f::bias - F16f::bias);
        mantissa16 = round_nearest_even_and_truncate(mantissa32);
    } else if (exponent32 >= F32f::bias + 1 - F16f::bias - F16f::mantissa_bits) {
        exponent16 = 0;
        mantissa16 = round_nearest_even_and_truncate((mantissa32 | (1 << F32f::mantissa_bits)) >>
                                                     ((F32f::bias + 1 - F16f::bias) - exponent32));
    }

    exponent16 <<= F16f::mantissa_bits;

    // Need to add mantissa as it might overflow during rounding and then we need to increase the
    // exponent by 1
    return (sign | exponent16) + mantissa16;
}

/**
 * @brief Extend low precision floating point number and return high precision floating point
 * number
 *
 * @tparam F32f high precision floating point format
 * @tparam F16f low precision floating point format
 * @param x bit pattern of low precision number
 *
 * @return bit pattern of high precision number
 */
template <typename F32f, typename F16f>
constexpr auto ieee754_extend(typename F16f::bits_type x) -> F32f::bits_type {
    using UIExt = F32f::bits_type;
    const UIExt sign = (x & F16f::sign_mask) << (F32f::num_bits - F16f::num_bits);
    const UIExt exponent16 = (x & F16f::exponent_mask) >> F16f::mantissa_bits;
    const UIExt mantissa16 = x & F16f::mantissa_mask;

    UIExt exponent32 = exponent16;
    UIExt mantissa32 = mantissa16;
    if (F32f::exponent_bits != F16f::exponent_bits) {
        if (exponent16 == F16f::max_biased_exponent) {
            // Inf and NaN
            exponent32 = F32f::max_biased_exponent;
        } else if (exponent16 != 0) {
            // convert bias
            // E_{16} = e + F16f::bias
            // E_{32} = e + F32f::bias
            //        = E_{16} - F16f::bias + F32f::bias
            //        = E_{16} + (F32f::bias - F16f::bias)
            exponent32 += F32f::bias - F16f::bias;
        }

        // Subnormal f16 numbers must be represented as f32 normal numbers
        if (exponent16 == 0 && mantissa16 != 0) {
            UIExt shift_count = 0;
            do {
                mantissa32 <<= 1;
                ++shift_count;
            } while ((mantissa32 & (1 << F16f::mantissa_bits)) != (1 << F16f::mantissa_bits));
            mantissa32 &= F16f::mantissa_mask;
            exponent32 = F32f::bias + 1 - F16f::bias - shift_count;
        }
    }

    // shift mantissa
    mantissa32 <<= F32f::mantissa_bits - F16f::mantissa_bits;

    // shift exponent
    exponent32 <<= F32f::mantissa_bits;

    return sign | exponent32 | mantissa32;
}

/**
 * @brief Low precision float type
 *
 * For all operations, low precision floats are converted single precision, the operation is done in
 * single precision, and then the result is stored in the low precision type
 *
 * @tparam T storage type
 * @tparam F16f low precision floating point format
 */
template <typename T, typename F16f> class lp_float {
  public:
    using lp_format = F16f;

    constexpr lp_float() = default;

    constexpr lp_float(lp_float const &) = default;
    constexpr lp_float(lp_float &&) = default;
    constexpr lp_float &operator=(lp_float const &) = default;
    constexpr lp_float &operator=(lp_float &&) = default;

#if __cplusplus >= 202002L
#define TINYTC_LPFLOAT_CONSTEXPR constexpr
    //! construct from float
    constexpr lp_float(float const &val)
        : data_{ieee754_truncate<F16f, f32_format>(std::bit_cast<f32_format::bits_type>(val))} {}
    //! assign float
    constexpr auto operator=(float const &rhs) -> lp_float & { return *this = lp_float{rhs}; }
    //! implicit conversion to float
    constexpr operator float() const {
        auto bits = ieee754_extend<f32_format, F16f>(data_);
        return std::bit_cast<float>(bits);
    }
#else
#define TINYTC_LPFLOAT_CONSTEXPR
    //! construct from float
    lp_float(float const &val) {
        f32_format::bits_type bits;
        memcpy(&bits, &val, sizeof(f32_format::bits_type));
        data_ = ieee754_truncate<F16f, f32_format>(bits);
    }
    //! assign float
    auto operator=(float const &rhs) -> lp_float & { return *this = lp_float{rhs}; }
    //! implicit conversion to float
    operator float() const {
        auto bits = ieee754_extend<f32_format, F16f>(data_);
        float number;
        memcpy(&number, &bits, sizeof(f32_format::bits_type));
        return number;
    }
#endif

    //! Get bit representation
    TINYTC_LPFLOAT_CONSTEXPR auto bits() const -> T { return data_; }
    //! Construct lp_float from bit representation
    constexpr static auto from_bits(T const &val) -> lp_float {
        auto r = lp_float{};
        r.data_ = val;
        return r;
    }

    //! add
    TINYTC_LPFLOAT_CONSTEXPR auto operator+(lp_float const &rhs) const -> lp_float {
        return operator float() + static_cast<float>(rhs);
    }
    //! add to
    TINYTC_LPFLOAT_CONSTEXPR auto operator+=(lp_float const &rhs) -> lp_float & {
        return *this = *this + rhs;
    }
    //! subtract
    TINYTC_LPFLOAT_CONSTEXPR auto operator-(lp_float const &rhs) const -> lp_float {
        return operator float() - static_cast<float>(rhs);
    }
    //! subtract from
    TINYTC_LPFLOAT_CONSTEXPR auto operator-=(lp_float const &rhs) -> lp_float & {
        return *this = *this - rhs;
    }
    //! multiply
    TINYTC_LPFLOAT_CONSTEXPR auto operator*(lp_float const &rhs) const -> lp_float {
        return operator float() * static_cast<float>(rhs);
    }
    //! multiply with
    TINYTC_LPFLOAT_CONSTEXPR auto operator*=(lp_float const &rhs) -> lp_float & {
        return *this = *this * rhs;
    }
    //! divide
    TINYTC_LPFLOAT_CONSTEXPR auto operator/(lp_float const &rhs) const -> lp_float {
        return operator float() / static_cast<float>(rhs);
    }
    //! divide with
    TINYTC_LPFLOAT_CONSTEXPR auto operator/=(lp_float const &rhs) -> lp_float & {
        return *this = *this / rhs;
    }
    //! unary minus
    TINYTC_LPFLOAT_CONSTEXPR auto operator-() -> lp_float { return -operator float(); }
    //! pre-increase by 1
    TINYTC_LPFLOAT_CONSTEXPR auto operator++() -> lp_float & {
        return *this = operator float() + 1.0f;
    }
    //! post-increase by 1
    TINYTC_LPFLOAT_CONSTEXPR auto operator++(int) -> lp_float {
        lp_float tmp = *this;
        operator++();
        return tmp;
    }
    //! pre-decrease by 1
    TINYTC_LPFLOAT_CONSTEXPR auto operator--() -> lp_float & {
        return *this = operator float() - 1.0f;
    }
    //! post-decrease by 1
    TINYTC_LPFLOAT_CONSTEXPR auto operator--(int) -> lp_float {
        lp_float tmp = *this;
        operator--();
        return tmp;
    }
    //! equal
    TINYTC_LPFLOAT_CONSTEXPR auto operator==(lp_float const &rhs) const -> bool {
        return operator float() == static_cast<float>(rhs);
    }
    //! not equal
    TINYTC_LPFLOAT_CONSTEXPR auto operator!=(lp_float const &rhs) const -> bool {
        return operator float() != static_cast<float>(rhs);
    }
    //! greater than
    TINYTC_LPFLOAT_CONSTEXPR auto operator>(lp_float const &rhs) const -> bool {
        return operator float() > static_cast<float>(rhs);
    }
    //! greater than or equal
    TINYTC_LPFLOAT_CONSTEXPR auto operator>=(lp_float const &rhs) const -> bool {
        return operator float() >= static_cast<float>(rhs);
    }
    //! less than
    TINYTC_LPFLOAT_CONSTEXPR auto operator<(lp_float const &rhs) const -> bool {
        return operator float() < static_cast<float>(rhs);
    }
    //! less than or equal
    TINYTC_LPFLOAT_CONSTEXPR auto operator<=(lp_float const &rhs) const -> bool {
        return operator float() <= static_cast<float>(rhs);
    }

  private:
    T data_;
};

/**
 * @brief bf16 host emulation type
 */
using bfloat16 = lp_float<std::uint16_t, bf16_format>;
/**
 * @brief fp16 host emulation type
 */
using half = lp_float<std::uint16_t, f16_format>;

////////////////////////////
//////// Scalar type ///////
////////////////////////////

//! Convert scalar type to string
inline char const *to_string(scalar_type ty) {
    return ::tinytc_scalar_type_to_string(static_cast<tinytc_scalar_type_t>(ty));
}
//! Size of scalar type in bytes
inline std::size_t size(scalar_type ty) {
    return ::tinytc_scalar_type_size(static_cast<tinytc_scalar_type_t>(ty));
}

/**
 * Returns the scalar type corresponding to C++ type T
 *
 * Specializations exist for bool, (u)int8_t, (u)int16_t, (u)int32_t, (u)int64_t, float, double.
 * The scalar_type is stored in the static constexpr member "value".
 */
template <typename T> struct to_scalar_type;
//! to_scalar_type specialization
template <> struct to_scalar_type<std::int8_t> {
    static constexpr scalar_type value = scalar_type::i8; ///< value
};
//! to_scalar_type specialization
template <> struct to_scalar_type<std::int16_t> {
    static constexpr scalar_type value = scalar_type::i16; ///< value
};
//! to_scalar_type specialization
template <> struct to_scalar_type<std::int32_t> {
    static constexpr scalar_type value = scalar_type::i32; ///< value
};
//! to_scalar_type specialization
template <> struct to_scalar_type<std::int64_t> {
    static constexpr scalar_type value = scalar_type::i64; ///< value
};
//! to_scalar_type specialization
template <> struct to_scalar_type<bfloat16> {
    static constexpr scalar_type value = scalar_type::bf16; ///< value
};
//! to_scalar_type specialization
template <> struct to_scalar_type<half> {
    static constexpr scalar_type value = scalar_type::f16; ///< value
};
//! to_scalar_type specialization
template <> struct to_scalar_type<float> {
    static constexpr scalar_type value = scalar_type::f32; ///< value
};
//! to_scalar_type specialization
template <> struct to_scalar_type<double> {
    static constexpr scalar_type value = scalar_type::f64; ///< value
};
//! to_scalar_type specialization
template <> struct to_scalar_type<std::complex<float>> {
    static constexpr scalar_type value = scalar_type::c32; ///< value
};
//! to_scalar_type specialization
template <> struct to_scalar_type<std::complex<double>> {
    static constexpr scalar_type value = scalar_type::c64; ///< value
};
/**
 * Convenience variable for to_scalar_type.
 *
 * Example: @code scalar_type ty = to_scalar_type_v<float>; @endcode
 */
template <typename T> inline constexpr scalar_type to_scalar_type_v = to_scalar_type<T>::value;

////////////////////////////
// Shared / unique handle //
////////////////////////////

template <typename T> class handle {
  public:
    //! Create empty (invalid) handle
    handle() : obj_{nullptr} {}
    //! Create handle from C handle
    handle(T obj) : obj_(obj) {}

    //! Dereference C handle and get reference to underlying type
    auto operator*() const -> std::remove_pointer_t<T> & { return *obj_; }
    //! Convert handle to C handle
    auto operator->() const -> T { return obj_; }
    //! Returns C handle
    auto get() const -> T { return obj_; }

    //! Check whether handle is non-empty (valid)
    explicit operator bool() const noexcept { return obj_ != nullptr; }

    //! Check equality
    bool operator==(handle<T> const &other) const { return obj_ == other.obj_; }
    //! Check inequality
    bool operator!=(handle<T> const &other) const { return !(*this == other); }

    operator T() const { return obj_; }

  protected:
    T obj_;
};

namespace internal {
//! Wraps retain / release calls for type T
template <typename T> struct shared_handle_traits {};

//! Wraps destroy calls for type T
template <typename T> struct unique_handle_traits {};
} // namespace internal

/**
 * @brief Wraps a C handle in a reference-counted object
 *
 * @tparam T C handle type (handle type = pointer to opaque struct)
 */
template <typename T> class shared_handle {
  public:
    //! Traits shortcut
    using traits = internal::shared_handle_traits<T>;
    //! Typedef for native C handle
    using native_type = T;

    //! Create empty (invalid) handle
    shared_handle() : obj_{nullptr} {}
    //! Create handle from C handle
    explicit shared_handle(T obj, bool needs_retain = false) : obj_(obj) {
        if (needs_retain) {
            CHECK_STATUS(c_retain());
        }
    }
    //! Decrease reference count
    ~shared_handle() { c_release(); }
    //! Copy ctor
    shared_handle(shared_handle const &other) : obj_(other.obj_) { CHECK_STATUS(c_retain()); }
    //! Move ctor
    shared_handle(shared_handle &&other) noexcept : obj_(other.obj_) { other.obj_ = nullptr; }
    //! Copy operator
    shared_handle &operator=(shared_handle const &other) {
        if (obj_ != other.obj_) {
            CHECK_STATUS(c_release());
            obj_ = other.obj_;
            CHECK_STATUS(c_retain());
        }
        return *this;
    }
    //! Move operator
    shared_handle &operator=(shared_handle &&other) {
        if (obj_ != other.obj_) {
            CHECK_STATUS(c_release());
            obj_ = other.obj_;
            other.obj_ = nullptr;
        }
        return *this;
    }

    //! Dereference C handle and get reference to underlying type
    auto operator*() const -> std::remove_pointer_t<T> & { return *obj_; }
    //! Convert handle to C handle
    auto operator->() const -> T { return obj_; }
    //! Returns C handle
    auto get() const -> T { return obj_; }
    //! Returns C handle and releases the ownership of the managed object
    auto release() -> T {
        auto tmp = obj_;
        obj_ = nullptr;
        return tmp;
    }

    //! Check whether handle is non-empty (valid)
    explicit operator bool() const noexcept { return obj_ != nullptr; }

    //! Check equality
    bool operator==(shared_handle<T> const &other) const { return obj_ == other.obj_; }
    //! Check inequality
    bool operator!=(shared_handle<T> const &other) const { return !(*this == other); }

  protected:
    //! Call retain in C-API if C handle is not NULL
    auto c_retain() -> tinytc_status_t {
        if (obj_ != nullptr) {
            return traits::retain(obj_);
        }
        return tinytc_status_success;
    }
    //! Call release in C-API if C handle is not NULL
    auto c_release() -> tinytc_status_t {
        if (obj_ != nullptr) {
            return traits::release(obj_);
        }
        return tinytc_status_success;
    }
    //! The C handle
    T obj_;
};

/**
 * @brief Wraps a C handle in a unique_ptr-alike object
 *
 * @tparam T C handle type (handle type = pointer to opaque struct)
 */
template <typename T> class unique_handle {
  public:
    //! Traits shortcut
    using traits = internal::unique_handle_traits<T>;
    //! Typedef for native C handle
    using native_type = T;

    //! Create empty (invalid) handle
    unique_handle() : obj_{nullptr} {}
    //! Create handle from C handle
    explicit unique_handle(T obj) : obj_(obj) {}
    //! Destroy object
    ~unique_handle() {
        if (obj_) {
            traits::destroy(obj_);
        }
    }
    //! Copy ctor
    unique_handle(unique_handle const &other) = delete;
    //! Move ctor
    unique_handle(unique_handle &&other) noexcept : obj_(other.obj_) { other.obj_ = nullptr; }
    //! Copy operator
    unique_handle &operator=(unique_handle const &other) = delete;
    //! Move operator
    unique_handle &operator=(unique_handle &&other) {
        obj_ = other.obj_;
        other.obj_ = nullptr;
        return *this;
    }

    //! Dereference C handle and get reference to underlying type
    auto operator*() const -> std::remove_pointer_t<T> & { return *obj_; }
    //! Convert handle to C handle
    auto operator->() const -> T { return obj_; }
    //! Returns C handle
    auto get() const -> T { return obj_; }
    //! Returns C handle and releases the ownership of the managed object
    auto release() -> T {
        auto tmp = obj_;
        obj_ = nullptr;
        return tmp;
    }

    //! Check whether handle is non-empty (valid)
    explicit operator bool() const noexcept { return obj_ != nullptr; }

    //! Check equality
    bool operator==(unique_handle<T> const &other) const { return obj_ == other.obj_; }
    //! Check inequality
    bool operator!=(unique_handle<T> const &other) const { return !(*this == other); }

  protected:
    //! The C handle
    T obj_;
};

////////////////////////////
//////// Array view ////////
////////////////////////////

/**
 * @brief Base implementation of array view
 *
 * @tparam T array element type
 */
template <typename T> class array_view_base {
  public:
    using iterator = T *;

    /**
     * @brief Empty array view
     */
    array_view_base() = default;

    /**
     * @brief Single element view
     *
     * @param single the single element
     */
    array_view_base(T &single) : data_{&single}, size_{1} {}

    /**
     * @brief ctor
     *
     * @param data base pointer
     * @param size array size
     */
    array_view_base(T *data, std::size_t size) : data_{data}, size_{size} {}

    /**
     * @brief ctor
     *
     * @param begin begin pointer
     * @param end end pointer (not included)
     */
    array_view_base(T *begin, T *end)
        : data_{begin}, size_{static_cast<std::size_t>(end - begin)} {}

    //! Begin iterator
    auto begin() const -> iterator { return data_; }
    //! End iterator
    auto end() const -> iterator { return data_ + size_; }
    //! Returns true if view is empty
    auto empty() const -> bool { return size_ == 0; }
    //! Returns array size
    auto size() const -> std::size_t { return size_; }
    //! Access first element; must not call when array size is 0
    auto front() const -> T & { return data_[0]; }
    //! Access last element; must not call when array size is 0
    auto back() const -> T & { return data_[size_ - 1]; }
    //! Get data pointer
    auto data() const -> T * { return data_; }
    //! Access operator
    auto operator[](std::size_t n) const -> T & { return data_[n]; }
    //! Convert to vector
    operator std::vector<std::remove_const_t<T>>() const {
        return std::vector<std::remove_const_t<T>>(data_, data_ + size_);
    }
    //! Equals operator
    auto operator==(array_view_base<T> const &other) const -> bool {
        bool eq = true;
        for (std::size_t i = 0; i < size_; ++i) {
            eq = eq && data_[i] == other.data_[i];
        }
        return eq;
    }
    auto operator!=(array_view_base<T> const &other) const -> bool { return !(*this == other); }

  private:
    T *data_ = nullptr;
    std::size_t size_ = 0;
};

/**
 * @brief Stores an immutable view on an array (pointer + size)
 *
 * @tparam T array element type
 */
template <typename T> class array_view : public array_view_base<const T> {
  public:
    using array_view_base<const T>::array_view_base;

    /**
     * @brief Convert vector to array view
     *
     * @param vec standard vector
     */
    array_view(std::vector<T> const &vec)
        : array_view_base<const T>{(!vec.empty() ? vec.data() : nullptr), vec.size()} {}

    /**
     * @brief Convert std::array to array view
     *
     * @tparam N array size
     * @param arr standard array
     */
    template <std::size_t N>
    array_view(std::array<T, N> const &arr) : array_view_base<const T>{arr.data(), arr.size()} {}

    /**
     * @brief Convert initializer list to array view (array_view must be rvalue)
     *
     * @param arr initializer list
     */
    array_view(std::initializer_list<T> const &arr)
        : array_view_base<const T>{(arr.begin() != arr.end() ? arr.begin() : nullptr), arr.size()} {
    }
};

template <typename T> array_view(T const &) -> array_view<T>;
template <typename T> array_view(T const *, std::size_t) -> array_view<T>;
template <typename T> array_view(T const *, T const *) -> array_view<T>;

/**
 * @brief Stores a mutable view on an array (pointer + size)
 *
 * @tparam T array element type
 */
template <typename T> class mutable_array_view : public array_view_base<T> {
  public:
    using array_view_base<T>::array_view_base;

    /**
     * @brief Convert vector to array view
     *
     * @param vec standard vector
     */
    mutable_array_view(std::vector<T> &vec)
        : array_view_base<T>{(!vec.empty() ? vec.data() : nullptr), vec.size()} {}

    /**
     * @brief Convert std::array to array view
     *
     * @tparam N array size
     * @param arr standard array
     */
    template <std::size_t N>
    mutable_array_view(std::array<T, N> &arr) : array_view_base<T>{arr.data(), arr.size()} {}
};

template <typename T> mutable_array_view(T &) -> mutable_array_view<T>;
template <typename T> mutable_array_view(T *, std::size_t) -> mutable_array_view<T>;
template <typename T> mutable_array_view(T *, T *) -> mutable_array_view<T>;

////////////////////////////
///// Compiler context /////
////////////////////////////

namespace internal {
template <> struct shared_handle_traits<tinytc_compiler_context_t> {
    static auto retain(tinytc_compiler_context_t handle) -> tinytc_status_t {
        return tinytc_compiler_context_retain(handle);
    }
    static auto release(tinytc_compiler_context_t handle) -> tinytc_status_t {
        return tinytc_compiler_context_release(handle);
    }
};
} // namespace internal

//! @brief Reference-counting wrapper for tinytc_compiler_context_t
class compiler_context : public shared_handle<tinytc_compiler_context_t> {
  public:
    using shared_handle::shared_handle;

    /**
     * @brief Add compiler to context
     *
     * @param name File name
     * @param text Source text
     *
     * @return Source id (should be set in position.source_id)
     */
    inline auto add_source(char const *name, char const *text) -> std::int32_t {
        std::int32_t source_id;
        CHECK_STATUS(tinytc_compiler_context_add_source(obj_, name, text, &source_id));
        return source_id;
    }
    /**
     * @brief Set error reporter
     *
     * Error reporting function that is called whenever an error occurs in the parser or the
     * builder.
     *
     * @param reporter error reporting callback
     * @param user_data pointer to user data that is passed to the callback
     *
     * @return tinytc_status_success on success and error otherwise
     */
    inline void set_error_reporter(error_reporter_t reporter, void *user_data) {
        CHECK_STATUS(tinytc_compiler_context_set_error_reporter(obj_, reporter, user_data));
    }

    /**
     * @brief Sets an optimization flag
     *
     * The state can be 0 (disabled), 1 (enabled), or -1 (use default according to optimization
     * level).
     *
     * @param flag optimization flag
     * @param state flag state
     */
    inline void set_optimization_flag(optflag flag, std::int32_t state) {
        CHECK_STATUS(tinytc_compiler_context_set_optimization_flag(
            obj_, static_cast<tinytc_optflag_t>(flag), state));
    }
    /**
     * @brief Set optimization level
     *
     * @param level optimization level
     */
    inline void set_optimization_level(std::int32_t level) {
        CHECK_STATUS(tinytc_compiler_context_set_optimization_level(obj_, level));
    }
    /**
     * @brief Enhance error message with compiler context; useful when builder is used
     *
     * @param loc Source location
     * @param what Error description
     */
    inline void report_error(location const &loc, char const *what) {
        CHECK_STATUS(tinytc_compiler_context_report_error(obj_, &loc, what));
    }
};

/**
 * @brief Create compiler context
 *
 * @return Compiler context
 */
inline auto make_compiler_context() -> compiler_context {
    tinytc_compiler_context_t ctx;
    CHECK_STATUS(tinytc_compiler_context_create(&ctx));
    return compiler_context{ctx};
}

////////////////////////////
///////// Attribute ////////
////////////////////////////

//! Alias for tinytc_attr_t
using attr = tinytc_attr_t;
//! Alias for tinytc_named_attr_t
using named_attr = tinytc_named_attr_t;

/**
 * @brief Get array attribute
 *
 * @param ctx compiler context
 * @param array attribute array
 *
 * @return Attribute
 */
inline attr get_array_attr(compiler_context const &ctx, array_view<attr> array) {
    attr a;
    CHECK_STATUS(tinytc_array_attr_get(&a, ctx.get(), array.size(), array.data()));
    return a;
}

/**
 * @brief Get boolean attribute
 *
 * @param ctx compiler context
 * @param value boolean value
 *
 * @return Attribute
 */
inline attr get_boolean_attr(compiler_context const &ctx, bool value) {
    attr a;
    CHECK_STATUS(tinytc_boolean_attr_get(&a, ctx.get(), value));
    return a;
}

/**
 * @brief Get dictionary attribute
 *
 * Each name must only appear once.
 *
 * @param ctx compiler context
 * @param items named items array
 *
 * @return Attribute
 */
inline attr get_dictionary_attr(compiler_context const &ctx, mutable_array_view<named_attr> items) {
    attr a;
    CHECK_STATUS(tinytc_dictionary_attr_get(&a, ctx.get(), items.size(), items.data()));
    return a;
}

/**
 * @brief Get dictionary attribute
 *
 * The list of items must be sorted by name and each name must only appear once.
 *
 * @param ctx compiler context
 * @param items named items array
 *
 * @return Attribute
 */
inline attr get_dictionary_attr_with_sorted(compiler_context const &ctx,
                                            array_view<named_attr> items) {
    attr a;
    CHECK_STATUS(tinytc_dictionary_attr_get_with_sorted(&a, ctx.get(), items.size(), items.data()));
    return a;
}

/**
 * @brief Sort list of items
 *
 * Each name must only appear once.
 *
 * @param items named items array
 */
inline void sort_items(mutable_array_view<named_attr> items) {
    CHECK_STATUS(tinytc_dictionary_attr_sort(items.size(), items.data()));
}

/**
 * @brief Get integer attribute
 *
 * @param ctx compiler context
 * @param value integer value
 *
 * @return Attribute
 */
inline attr get_integer_attr(compiler_context const &ctx, std::int64_t value) {
    attr a;
    CHECK_STATUS(tinytc_integer_attr_get(&a, ctx.get(), value));
    return a;
}

/**
 * @brief Get string attribute
 *
 * @param ctx compiler context
 * @param str string
 *
 * @return Attribute
 */
inline attr get_string_attr(compiler_context const &ctx, std::string_view str) {
    attr a;
    CHECK_STATUS(tinytc_string_attr_get(&a, ctx.get(), str.size(), str.data()));
    return a;
}

////////////////////////////
///////// Data type ////////
////////////////////////////

//! Check if mode i is dynamic ('?')
inline bool is_dynamic_value(std::int64_t i) { return i == dynamic; }

//! Alias for tinytc_data_type_t
using data_type = tinytc_data_type_t;

/**
 * @brief Get the boolean data type
 *
 * Cf. \ref tinytc_boolean_type_get
 *
 * @param ctx Compiler context
 *
 * @return Data type
 */
inline data_type get_boolean(compiler_context const &ctx) {
    tinytc_data_type_t bt;
    CHECK_STATUS(tinytc_boolean_type_get(&bt, ctx.get()));
    return bt;
}

/**
 * @brief Get a scalar data type
 *
 * Cf. \ref tinytc_scalar_type_get
 *
 * @param ctx Compiler context
 * @param scalar_ty Scalar type
 *
 * @return Data type
 */
inline data_type get_scalar(compiler_context const &ctx, scalar_type scalar_ty) {
    tinytc_data_type_t st;
    CHECK_STATUS(
        tinytc_scalar_type_get(&st, ctx.get(), static_cast<tinytc_scalar_type_t>(scalar_ty)));
    return st;
}

/**
 * @brief Get a memref data type
 *
 * Cf. \ref tinytc_memref_type_get
 *
 * @param scalar_ty Element type
 * @param shape Tensor shape
 * @param stride Tensor stride
 * @param addrspace Address space
 * @param loc Source code location
 *
 * @return Data type
 */
inline data_type get_memref(data_type scalar_ty, array_view<std::int64_t> shape,
                            array_view<std::int64_t> stride = {},
                            address_space addrspace = address_space::global,
                            location const &loc = {}) {
    tinytc_data_type_t mt;
    CHECK_STATUS_LOC(tinytc_memref_type_get(&mt, scalar_ty, shape.size(), shape.data(),
                                            stride.size(), stride.data(),
                                            static_cast<tinytc_address_space_t>(addrspace), &loc),
                     loc);
    return mt;
}

/**
 * @brief Get a group data type
 *
 * @param memref_ty Memref data type
 * @param size Size parameter
 * @param offset Offset parameter
 * @param loc Source code location
 *
 * @return Data type
 */
inline data_type get_group(data_type memref_ty, std::int64_t size, std::int64_t offset = 0,
                           location const &loc = {}) {
    tinytc_data_type_t gt;
    CHECK_STATUS_LOC(tinytc_group_type_get(&gt, memref_ty, size, offset, &loc), loc);
    return gt;
}

/**
 * @brief Get a coopmatrix data type
 *
 * @param scalar_ty Component type
 * @param rows Number of rows
 * @param cols Number of cols
 * @param use Matrix use
 * @param loc Source code location
 *
 * @return Data type
 */
inline data_type get_coopmatrix(data_type scalar_ty, std::int64_t rows, std::int64_t cols,
                                matrix_use use, location const &loc = {}) {
    tinytc_data_type_t ct;
    CHECK_STATUS_LOC(tinytc_coopmatrix_type_get(&ct, scalar_ty, rows, cols,
                                                static_cast<::tinytc_matrix_use_t>(use), &loc),
                     loc);
    return ct;
}

/**
 * @brief Get a void data type
 *
 * @param ctx Context
 *
 * @return Data type
 */
inline data_type get_void(compiler_context const &ctx) {
    tinytc_data_type_t vt;
    CHECK_STATUS(tinytc_void_type_get(&vt, ctx.get()));
    return vt;
}

////////////////////////////
/////////// Value //////////
////////////////////////////

//! @brief OO-wrapper for tinytc_value_t
class value : public handle<tinytc_value_t> {
  public:
    using handle::handle;

    /**
     * @brief Get name
     *
     * @return Name as C-string
     */
    inline auto get_name() -> char const * {
        char const *name;
        CHECK_STATUS(tinytc_value_get_name(obj_, &name));
        return name;
    }

    /**
     * @brief Set value name
     *
     * @param name Name
     */
    inline void set_name(std::string_view name) {
        CHECK_STATUS(tinytc_value_set_name_n(obj_, name.size(), name.data()));
    }

    /**
     * @brief Get type
     *
     * @return Data type
     */
    inline auto get_type() -> data_type {
        tinytc_data_type_t ty;
        CHECK_STATUS(tinytc_value_get_type(obj_, &ty));
        return ty;
    }
};
static_assert(std::is_standard_layout_v<value> && sizeof(value) == sizeof(tinytc_value_t));

////////////////////////////
/////////// Inst ///////////
////////////////////////////

/**
 * @brief Convert address space to string
 *
 * @param as Address space
 *
 * @return C-string
 */
inline char const *to_string(address_space as) {
    return ::tinytc_address_space_to_string(static_cast<::tinytc_address_space_t>(as));
}

/**
 * @brief Convert arithmetic operation type to string
 *
 * @param op Arithmetic operation type
 *
 * @return C-string
 */
inline char const *to_string(arithmetic op) {
    return ::tinytc_arithmetic_to_string(static_cast<::tinytc_arithmetic_t>(op));
}

/**
 * @brief Convert arithmetic operation type to string (unary)
 *
 * @param op Arithmetic operation type
 *
 * @return C-string
 */
inline char const *to_string(arithmetic_unary op) {
    return ::tinytc_arithmetic_unary_to_string(static_cast<::tinytc_arithmetic_unary_t>(op));
}

/**
 * @brief Convert builtin type to string
 *
 * @param b Builtin type
 *
 * @return C-string
 */
inline char const *to_string(builtin b) {
    return ::tinytc_builtin_to_string(static_cast<::tinytc_builtin_t>(b));
}

/**
 * @brief Convert checked flag string
 *
 * @param flag Flag
 *
 * @return C-string
 */
inline char const *to_string(checked_flag flag) {
    return ::tinytc_checked_flag_to_string(static_cast<::tinytc_checked_flag_t>(flag));
}

/**
 * @brief Convert cmp condition to string
 *
 * @param cond Condition
 *
 * @return C-string
 */
inline char const *to_string(cmp_condition cond) {
    return ::tinytc_cmp_condition_to_string(static_cast<::tinytc_cmp_condition_t>(cond));
}

/**
 * @brief Convert subgroup operation to string
 *
 * @param op Operation type
 *
 * @return C-string
 */
inline char const *to_string(group_operation op) {
    return ::tinytc_group_operation_to_string(static_cast<::tinytc_group_operation_t>(op));
}

/**
 * @brief Convert math operation type to string (unary)
 *
 * @param op Math operation type
 *
 * @return C-string
 */
inline char const *to_string(math_unary op) {
    return ::tinytc_math_unary_to_string(static_cast<::tinytc_math_unary_t>(op));
}

/**
 * @brief Convert matrix use to string
 *
 * @param u Matrix use
 *
 * @return C-string
 */
inline char const *to_string(matrix_use u) {
    return ::tinytc_matrix_use_to_string(static_cast<::tinytc_matrix_use_t>(u));
}

/**
 * @brief Convert store flag to string
 *
 * @param flag Store flag
 *
 * @return C-string
 */
inline char const *to_string(store_flag flag) {
    return ::tinytc_store_flag_to_string(static_cast<tinytc_store_flag_t>(flag));
}

/**
 * @brief Convert transpose to string
 *
 * @param t Transpose
 *
 * @return C-string
 */
inline char const *to_string(transpose t) {
    return ::tinytc_transpose_to_string(static_cast<tinytc_transpose_t>(t));
}

namespace internal {
template <> struct unique_handle_traits<tinytc_inst_t> {
    static void destroy(tinytc_inst_t handle) { return tinytc_inst_destroy(handle); }
};
} // namespace internal

class region;

//! @brief Reference-counting wrapper for tinytc_inst_t
class inst : public unique_handle<tinytc_inst_t> {
  public:
    using unique_handle::unique_handle;

    /**
     * @brief Get result values
     *
     * May be called with empty view (vals = {}) to get the number of results.
     *
     * @param vals view on buffer that stores results
     *
     * @return Minimum of view size and actual number of result values
     */
    inline auto get_values(mutable_array_view<value> vals) const -> std::uint32_t {
        std::uint32_t result_list_size = vals.size();
        tinytc_value_t *vs = reinterpret_cast<tinytc_value_t *>(vals.data());
        CHECK_STATUS(tinytc_inst_get_values(obj_, &result_list_size, vs));
        return result_list_size;
    }

    /**
     * @brief Get child regions
     *
     * May be called with empty view (vals = {}) to get the number of child regions.
     *
     * @param regs view on buffer that stores results
     *
     * @return Minimum of view size and actual number of child regions
     */
    inline auto get_regions(mutable_array_view<region> regs) const -> std::uint32_t {
        std::uint32_t result_list_size = regs.size();
        tinytc_region_t *rl = reinterpret_cast<tinytc_region_t *>(regs.data());
        CHECK_STATUS(tinytc_inst_get_regions(obj_, &result_list_size, rl));
        return result_list_size;
    }

    /**
     * @brief Set attribute
     *
     * @param a attribute
     */
    inline void set_attr(attr a) { CHECK_STATUS(tinytc_inst_set_attr(obj_, a)); }
};

////////////////////////////
////////// Region //////////
////////////////////////////

//! @brief OO-wrapper for tinytc_region_t
class region : public handle<tinytc_region_t> {
  public:
    using handle::handle;

    /**
     * @brief Append instruction to region
     *
     * @param instruction instruction object
     */
    inline void append(inst instruction) {
        CHECK_STATUS(tinytc_region_append(obj_, instruction.release()));
    }

    /**
     * @brief Get iterator pointing to the begin of the region
     *
     * @return iterator
     */
    inline auto begin() -> tinytc_inst_iterator_t {
        tinytc_inst_iterator_t it;
        CHECK_STATUS(tinytc_region_begin(obj_, &it));
        return it;
    }

    /**
     * @brief Get iterator pointing to past the end of the region
     *
     * @return iterator
     */
    inline auto end() -> tinytc_inst_iterator_t {
        tinytc_inst_iterator_t it;
        CHECK_STATUS(tinytc_region_end(obj_, &it));
        return it;
    }

    /**
     * @brief Erase instruction at iterator
     *
     * @param iterator Iterator
     *
     * @return Iterator pointing to the instruction after the one erased
     */
    inline auto erase(tinytc_inst_iterator_t iterator) -> tinytc_inst_iterator_t {
        auto it = iterator;
        CHECK_STATUS(tinytc_region_erase(obj_, &it));
        return it;
    }

    /**
     * @brief Insert instruction into region before the iterator
     *
     * @param iterator Iterator
     * @param instruction instruction object
     *
     * @return Iterator pointing to the newly inserted instruction
     */
    inline auto insert(tinytc_inst_iterator_t iterator, inst instruction)
        -> tinytc_inst_iterator_t {
        auto it = iterator;
        CHECK_STATUS(tinytc_region_insert(obj_, &it, instruction.release()));
        return it;
    }

    /**
     *
     * @brief Get region parameters
     *
     * May be called with empty view (vals = {}) to get the number of parameters.
     *
     * @param params view on buffer that stores parameters
     *
     * @return Minimum of view size and actual number of parameters
     */
    inline auto get_parameters(mutable_array_view<value> params) -> std::uint32_t {
        std::uint32_t result_list_size = params.size();
        tinytc_value_t *ps = reinterpret_cast<tinytc_value_t *>(params.data());
        CHECK_STATUS(tinytc_region_get_parameters(obj_, &result_list_size, ps));
        return result_list_size;
    }
};
static_assert(std::is_standard_layout_v<region> && sizeof(region) == sizeof(tinytc_region_t));

/**
 * @brief Move iterator to next instruction
 *
 * @param iterator
 */
inline void next(tinytc_inst_iterator_t &iterator) { CHECK_STATUS(tinytc_next_inst(&iterator)); }
/**
 * @brief Move iterator to previous instruction
 *
 * @param iterator
 */
inline void prev(tinytc_inst_iterator_t &iterator) { CHECK_STATUS(tinytc_prev_inst(&iterator)); }

////////////////////////////
/////// Instructions ///////
////////////////////////////

/**
 * @brief Make arithmetic instruction (binary)
 *
 * @param op Arithmetic operation type
 * @param a First operand
 * @param b Second operand
 * @param ty Result type
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_arith(arithmetic op, value a, value b, data_type ty, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(
        tinytc_arith_inst_create(&instr, static_cast<tinytc_arithmetic_t>(op), a, b, ty, &loc),
        loc);
    return inst(instr);
}

/**
 * @brief Make arithmetic instruction (unary)
 *
 * @param op Arithmetic operation type
 * @param a Operand
 * @param ty Result type
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_arith(arithmetic_unary op, value a, data_type ty, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_arith_unary_inst_create(
                         &instr, static_cast<tinytc_arithmetic_unary_t>(op), a, ty, &loc),
                     loc);
    return inst(instr);
}

/**
 * @brief Make barrier instruction
 *
 * @param fence_flags address space(s) of memory fence; set to 0 for no fence
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_barrier(tinytc_address_spaces_t fence_flags, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_barrier_inst_create(&instr, fence_flags, &loc), loc);
    return inst(instr);
}

/**
 * @brief Make builtin instruction
 *
 * @param btype Builtin type
 * @param ty Result type
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_builtin(builtin btype, data_type ty, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(
        tinytc_builtin_inst_create(&instr, static_cast<tinytc_builtin_t>(btype), ty, &loc), loc);
    return inst(instr);
}

/**
 * @brief Make cast instruction
 *
 * @param a Operand
 * @param to_ty Target type
 * @param loc Source code lcoation
 *
 * @return Instruction
 */
inline inst make_cast(value a, data_type to_ty, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_cast_inst_create(&instr, a, to_ty, &loc), loc);
    return inst(instr);
}

/**
 * @brief Make compare instruction
 *
 * @param cond Condition type
 * @param a First operand
 * @param b Second operand
 * @param ty Result type
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_cmp(cmp_condition cond, value a, value b, data_type ty, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(
        tinytc_cmp_inst_create(&instr, static_cast<tinytc_cmp_condition_t>(cond), a, b, ty, &loc),
        loc);
    return inst(instr);
}

/**
 * @brief Make boolean constant
 *
 * @param value Constant
 * @param ty Data type
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_constant(bool value, data_type ty, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_constant_inst_create_boolean(&instr, value, ty, &loc), loc);
    return inst(instr);
}

/**
 * @brief Make complex constant
 *
 * @param value Complex constant
 * @param ty Data type
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_constant(std::complex<double> value, data_type ty, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(
        tinytc_constant_inst_create_complex(&instr, value.real(), value.imag(), ty, &loc), loc);
    return inst(instr);
}

/**
 * @brief Make floating constant
 *
 * @param value Constant
 * @param ty Data type
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_constant(double value, data_type ty, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_constant_inst_create_float(&instr, value, ty, &loc), loc);
    return inst(instr);
}

/**
 * @brief Make integer constant
 *
 * @param value Constant
 * @param ty Data type
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_constant(std::int32_t value, data_type ty, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_constant_inst_create_int(&instr, value, ty, &loc), loc);
    return inst(instr);
}

/**
 * @brief Make integer constant
 *
 * @param value Constant
 * @param ty Data type
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_constant(std::int64_t value, data_type ty, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_constant_inst_create_int(&instr, value, ty, &loc), loc);
    return inst(instr);
}

/**
 * @brief Make multiplicative identity constant ("1") for the given data type
 *
 * @param ty Scalar data type
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_constant_one(data_type ty, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_constant_inst_create_one(&instr, ty, &loc), loc);
    return inst(instr);
}

/**
 * @brief Make additive identity constant ("0") for the given data type
 *
 * @param ty Scalar data type
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_constant_zero(data_type ty, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_constant_inst_create_zero(&instr, ty, &loc), loc);
    return inst(instr);
}

/**
 * @brief Create cooperative matrix load instruction
 *
 * @param trans transpose operation applied on load
 * @param flag out-of-bounds checks type
 * @param op %op
 * @param p0 %p0
 * @param p1 %p1
 * @param to_ty result type
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_cooperative_matrix_load(transpose trans, checked_flag flag, value op, value p0,
                                         value p1, data_type to_ty, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_cooperative_matrix_load_inst_create(
                         &instr, static_cast<tinytc_transpose_t>(trans),
                         static_cast<tinytc_checked_flag_t>(flag), op, p0, p1, to_ty, &loc),
                     loc);
    return inst(instr);
}

/**
 * @brief Create cooperative matrix mul add instruction
 *
 * @param a %a
 * @param b %b
 * @param c %c
 * @param to_ty result type
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_cooperative_matrix_mul_add(value a, value b, value c, data_type to_ty,
                                            location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_cooperative_matrix_mul_add_inst_create(&instr, a, b, c, to_ty, &loc),
                     loc);
    return inst(instr);
}

/**
 * @brief Create cooperative matrix prefetch instruction
 *
 * @param cache_level Cache-level; "0" is closest to the core
 * @param op %op
 * @param p0 %p0
 * @param p1 %p1
 * @param rows Number of rows
 * @param cols Number of cols
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_cooperative_matrix_prefetch(std::int32_t cache_level, value op, value p0, value p1,
                                             std::int32_t rows, std::int32_t cols,
                                             location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_cooperative_matrix_prefetch_inst_create(&instr, cache_level, op, p0, p1,
                                                                    rows, cols, &loc),
                     loc);
    return inst(instr);
}

/**
 * @brief Create cooperative matrix scale instruction
 *
 * @param a %a
 * @param b %b
 * @param ty Result type
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_cooperative_matrix_scale(value a, value b, data_type ty,
                                          location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_cooperative_matrix_scale_inst_create(&instr, a, b, ty, &loc), loc);
    return inst(instr);
}

/**
 * @brief Create cooperative matrix store instruction
 *
 * @param cflag out-of-bounds checks type
 * @param sflag store flag
 * @param val %val
 * @param op %op
 * @param p0 %p0
 * @param p1 %p1
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_cooperative_matrix_store(checked_flag cflag, store_flag sflag, value val, value op,
                                          value p0, value p1, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_cooperative_matrix_store_inst_create(
                         &instr, static_cast<tinytc_checked_flag_t>(cflag),
                         static_cast<tinytc_store_flag_t>(sflag), val, op, p0, p1, &loc),
                     loc);
    return inst(instr);
}

/**
 * @brief Make alloca instruction
 *
 * @param ty Memref type of allocated variable
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_alloca(data_type ty, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_alloca_inst_create(&instr, ty, &loc), loc);
    return inst(instr);
}

/**
 * @brief Make AXPBY instruction
 *
 * @param tA Operation applied on A
 * @param atomic true for atomic updates of B
 * @param alpha @f$\alpha@f$
 * @param A A
 * @param beta @f$\beta@f$
 * @param B B
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_axpby(transpose tA, bool atomic, value alpha, value A, value beta, value B,
                       location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_axpby_inst_create(&instr, static_cast<tinytc_transpose_t>(tA), atomic,
                                              alpha, A, beta, B, &loc),
                     loc);
    return inst(instr);
}

/**
 * @brief Make cumsum instruction
 *
 * @param atomic true for atomic updates of B
 * @param alpha @f$\alpha@f$
 * @param A A
 * @param mode n (summation mode)
 * @param beta @f$\beta@f$
 * @param B B
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_cumsum(bool atomic, value alpha, value A, std::int64_t mode, value beta, value B,
                        location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_cumsum_inst_create(&instr, atomic, alpha, A, mode, beta, B, &loc), loc);
    return inst(instr);
}

/**
 * @brief Make expand instruction
 *
 * @param a Operand
 * @param expanded_mode Expanded mode
 * @param static_expand_shape Static expand shape
 * @param expand_shape Dynamic expand shape
 * @param ty Result type
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_expand(value a, std::int64_t expanded_mode,
                        array_view<std::int64_t> static_expand_shape,
                        array_view<value> expand_shape, data_type ty, location const &loc = {}) {
    tinytc_inst_t instr;
    auto static_len = static_expand_shape.size();
    if (static_len > std::numeric_limits<std::uint32_t>::max()) {
        throw std::out_of_range("static expand shape too large");
    }
    auto len = expand_shape.size();
    if (len > std::numeric_limits<std::uint32_t>::max()) {
        throw std::out_of_range("expand shape too large");
    }
    const tinytc_value_t *es = reinterpret_cast<const tinytc_value_t *>(expand_shape.data());
    CHECK_STATUS_LOC(tinytc_expand_inst_create(&instr, a, expanded_mode, static_len,
                                               static_expand_shape.data(), len, es, ty, &loc),
                     loc);
    return inst(instr);
}

/**
 * @brief Make fuse instruciton
 *
 * @param a Operand
 * @param from First mode to fuse
 * @param to Last mode to fuse
 * @param ty Result type
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_fuse(value a, std::int64_t from, std::int64_t to, data_type ty,
                      location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_fuse_inst_create(&instr, a, from, to, ty, &loc), loc);
    return inst(instr);
}

/**
 * @brief Make load instruction
 *
 * @param a Operand
 * @param index_list Vector of indices
 * @param ty Result type
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_load(value a, array_view<value> index_list, tinytc_data_type_t ty,
                      location const &loc = {}) {
    tinytc_inst_t instr;
    auto len = index_list.size();
    if (len > std::numeric_limits<std::uint32_t>::max()) {
        throw std::out_of_range("index list too long");
    }
    const tinytc_value_t *il = reinterpret_cast<const tinytc_value_t *>(index_list.data());
    CHECK_STATUS_LOC(tinytc_load_inst_create(&instr, a, len, il, ty, &loc), loc);
    return inst(instr);
}

/**
 * @brief Make GEMM instruction
 *
 * @param tA Operation applied on A
 * @param tB Operation applied on B
 * @param atomic true for atomic updates of C
 * @param alpha @f$\alpha@f$
 * @param A A
 * @param B B
 * @param beta @f$\beta@f$
 * @param C C
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_gemm(transpose tA, transpose tB, bool atomic, value alpha, value A, value B,
                      value beta, value C, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_gemm_inst_create(&instr, static_cast<tinytc_transpose_t>(tA),
                                             static_cast<tinytc_transpose_t>(tB), atomic, alpha, A,
                                             B, beta, C, &loc),
                     loc);
    return inst(instr);
}

/**
 * @brief Make GEMV instruction
 *
 * @param tA Operation applied on A
 * @param atomic true for atomic updates of C
 * @param alpha @f$\alpha@f$
 * @param A A
 * @param B B
 * @param beta @f$\beta@f$
 * @param C C
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_gemv(transpose tA, bool atomic, value alpha, value A, value B, value beta, value C,
                      location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_gemv_inst_create(&instr, static_cast<tinytc_transpose_t>(tA), atomic,
                                             alpha, A, B, beta, C, &loc),
                     loc);
    return inst(instr);
}

/**
 * @brief Make GER instruction
 *
 * @param atomic true for atomic updates of C
 * @param alpha @f$\alpha@f$
 * @param A A
 * @param B B
 * @param beta @f$\beta@f$
 * @param C C
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_ger(bool atomic, value alpha, value A, value B, value beta, value C,
                     location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_ger_inst_create(&instr, atomic, alpha, A, B, beta, C, &loc), loc);
    return inst(instr);
}

/**
 * @brief Make hadamard instruction
 *
 * @param atomic true for atomic updates of C
 * @param alpha @f$\alpha@f$
 * @param A A
 * @param B B
 * @param beta @f$\beta@f$
 * @param C C
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_hadamard(bool atomic, value alpha, value A, value B, value beta, value C,
                          location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_hadamard_inst_create(&instr, atomic, alpha, A, B, beta, C, &loc), loc);
    return inst(instr);
}

/**
 * @brief Make math instruction (unary)
 *
 * @param op Math operation type
 * @param a Operand
 * @param ty Result type
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_math(math_unary op, value a, data_type ty, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(
        tinytc_math_unary_inst_create(&instr, static_cast<tinytc_math_unary_t>(op), a, ty, &loc),
        loc);
    return inst(instr);
}

/**
 * @brief Make parallel region
 *
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_parallel(location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_parallel_inst_create(&instr, &loc), loc);
    return inst(instr);
}

/**
 * @brief Make size instruction
 *
 * @param a Operand
 * @param mode Mode
 * @param ty Result type
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_size(value a, std::int64_t mode, data_type ty, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_size_inst_create(&instr, a, mode, ty, &loc), loc);
    return inst(instr);
}

/**
 * @brief Make subgroup add instruction
 *
 * @param op Group operation
 * @param a Operand
 * @param ty Result type
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_subgroup_add(group_operation op, value a, data_type ty, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_subgroup_add_inst_create(
                         &instr, static_cast<tinytc_group_operation_t>(op), a, ty, &loc),
                     loc);
    return inst(instr);
}

/**
 * @brief Make subgroup broadcast instruction
 *
 * @param a Operand
 * @param idx Subgroup local index
 * @param ty Result type
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_subgroup_broadcast(value a, value idx, data_type ty, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_subgroup_broadcast_inst_create(&instr, a, idx, ty, &loc), loc);
    return inst(instr);
}

/**
 * @brief Make subgroup max instruction
 *
 * @param op Group operation
 * @param a Operand
 * @param ty Result type
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_subgroup_max(group_operation op, value a, data_type ty, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_subgroup_max_inst_create(
                         &instr, static_cast<tinytc_group_operation_t>(op), a, ty, &loc),
                     loc);
    return inst(instr);
}

/**
 *
 * @brief Make subgroup min instruction
 *
 * @param op Group operation
 * @param a Operand
 * @param ty Result type
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_subgroup_min(group_operation op, value a, data_type ty, location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_subgroup_min_inst_create(
                         &instr, static_cast<tinytc_group_operation_t>(op), a, ty, &loc),
                     loc);
    return inst(instr);
}

/**
 * @brief Make subview instruction
 *
 * @param a Operand
 * @param static_offset_list Static offsets
 * @param static_size_list Static sizes
 * @param offset_list Vector of offsets; need to add dynamic offsets here if static_offset_list
 * contains "dynamic"
 * @param size_list Vector of sizes; need to add dynamic sizes here if static_size_list contains
 * "dynamic"
 * @param ty Return type
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_subview(value a, array_view<std::int64_t> static_offset_list,
                         array_view<std::int64_t> static_size_list, array_view<value> offset_list,
                         array_view<value> size_list, data_type ty, location const &loc = {}) {
    tinytc_inst_t instr;
    if (static_offset_list.size() != static_size_list.size()) {
        throw std::invalid_argument(
            "static offset list must have the same length as the static size list");
    }
    auto static_len = static_offset_list.size();
    if (static_len > std::numeric_limits<std::uint32_t>::max()) {
        throw std::out_of_range("static slice list too long");
    }
    auto offset_len = offset_list.size();
    if (offset_len > std::numeric_limits<std::uint32_t>::max()) {
        throw std::out_of_range("dynamic offset list too long");
    }
    auto size_len = size_list.size();
    if (size_len > std::numeric_limits<std::uint32_t>::max()) {
        throw std::out_of_range("dynamic size list too long");
    }
    const tinytc_value_t *ol = reinterpret_cast<const tinytc_value_t *>(offset_list.data());
    const tinytc_value_t *sl = reinterpret_cast<const tinytc_value_t *>(size_list.data());
    CHECK_STATUS_LOC(tinytc_subview_inst_create(&instr, a, static_len, static_offset_list.data(),
                                                static_size_list.data(), offset_len, ol, size_len,
                                                sl, ty, &loc),
                     loc);
    return inst(instr);
}

/**
 * @brief Make store instruction
 *
 * @param flag store flag
 * @param val Value that is stored
 * @param a Target memref
 * @param index_list Vector of indices
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_store(store_flag flag, value val, value a, array_view<value> index_list,
                       location const &loc = {}) {
    tinytc_inst_t instr;
    auto len = index_list.size();
    if (len > std::numeric_limits<std::uint32_t>::max()) {
        throw std::out_of_range("index list too long");
    }
    const tinytc_value_t *il = reinterpret_cast<const tinytc_value_t *>(index_list.data());
    CHECK_STATUS_LOC(tinytc_store_inst_create(&instr, static_cast<tinytc_store_flag_t>(flag), val,
                                              a, len, il, &loc),
                     loc);
    return inst(instr);
}

/**
 * @brief Make sum instruction
 *
 * @param tA Operation applied on A
 * @param atomic true for atomic updates of B
 * @param alpha @f$\alpha@f$
 * @param A A
 * @param beta @f$\beta@f$
 * @param B B
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_sum(transpose tA, bool atomic, value alpha, value A, value beta, value B,
                     location const &loc = {}) {
    tinytc_inst_t instr;
    CHECK_STATUS_LOC(tinytc_sum_inst_create(&instr, static_cast<tinytc_transpose_t>(tA), atomic,
                                            alpha, A, beta, B, &loc),
                     loc);
    return inst(instr);
}

/**
 * @brief Make for loop instruction
 *
 * @param loop_var_type Type of loop variable
 * @param from Loop variable start
 * @param to Loop variable bound
 * @param step Loop variable step; can be {}
 * @param initial_value_list Array of initial values; can be {}
 * @param return_type_list Array of return types; can be {}
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_for(data_type loop_var_type, value from, value to, value step,
                     array_view<value> initial_value_list, array_view<data_type> return_type_list,
                     location const &loc = {}) {
    tinytc_inst_t instr;
    auto len = initial_value_list.size();
    if (len > std::numeric_limits<std::uint32_t>::max()) {
        throw std::out_of_range("initial value list too long");
    }
    if (len != return_type_list.size()) {
        throw std::invalid_argument(
            "initial value list must have the same length as the return type list");
    }
    const tinytc_value_t *il = reinterpret_cast<const tinytc_value_t *>(initial_value_list.data());
    CHECK_STATUS_LOC(tinytc_for_inst_create(&instr, loop_var_type, from, to, step, len, il,
                                            return_type_list.data(), &loc),
                     loc);
    return inst(instr);
}

/**
 * @brief Make foreach loop instruction
 *
 * @param loop_var_type Type of loop variable
 * @param from_list List of loop variable start
 * @param to_list List of loop variable bound
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_foreach(data_type loop_var_type, array_view<value> from_list,
                         array_view<value> to_list, location const &loc = {}) {

    tinytc_inst_t instr;
    if (from_list.size() != to_list.size()) {
        throw std::invalid_argument("from list must have the same length as the to list");
    }
    const auto from_len = from_list.size();
    if (from_len > std::numeric_limits<std::uint32_t>::max()) {
        throw std::out_of_range("from list too long");
    }
    const auto to_len = to_list.size();
    if (to_len > std::numeric_limits<std::uint32_t>::max()) {
        throw std::out_of_range("to list too long");
    }
    const tinytc_value_t *fl = reinterpret_cast<const tinytc_value_t *>(from_list.data());
    const tinytc_value_t *tl = reinterpret_cast<const tinytc_value_t *>(to_list.data());
    CHECK_STATUS_LOC(tinytc_foreach_inst_create(&instr, loop_var_type, from_len, fl, tl, &loc),
                     loc);
    return inst(instr);
}

/**
 * @brief Make if condition instruction
 *
 * @param condition Condition value (of type bool)
 * @param return_type_list Types of returned values
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_if(value condition, array_view<data_type> return_type_list = {},
                    location const &loc = {}) {
    tinytc_inst_t instr;
    auto len = return_type_list.size();
    if (len > std::numeric_limits<std::uint32_t>::max()) {
        throw std::out_of_range("return type list too long");
    }
    CHECK_STATUS_LOC(tinytc_if_inst_create(&instr, condition, len, return_type_list.data(), &loc),
                     loc);
    return inst(instr);
}

/**
 * @brief Make yield instruction
 *
 * @param yield_list Yielded values
 * @param loc Source code location
 *
 * @return Instruction
 */
inline inst make_yield(array_view<value> yield_list, location const &loc = {}) {
    tinytc_inst_t instr;
    auto len = yield_list.size();
    if (len > std::numeric_limits<std::uint32_t>::max()) {
        throw std::out_of_range("yield list too long");
    }
    const tinytc_value_t *yl = reinterpret_cast<const tinytc_value_t *>(yield_list.data());
    CHECK_STATUS_LOC(tinytc_yield_inst_create(&instr, len, yl, &loc), loc);
    return inst(instr);
}

////////////////////////////
/////////// Func ///////////
////////////////////////////

namespace internal {
template <> struct unique_handle_traits<tinytc_func_t> {
    static void destroy(tinytc_func_t handle) { return tinytc_func_destroy(handle); }
};
} // namespace internal

//! @brief Reference-counting wrapper for tinytc_func_t
class func : public unique_handle<tinytc_func_t> {
  public:
    using unique_handle::unique_handle;

    auto get_body() -> region {
        tinytc_region_t body;
        CHECK_STATUS(tinytc_func_get_body(obj_, &body));
        return region{body};
    }

    void set_attr(attr a) { CHECK_STATUS(tinytc_func_set_attr(obj_, a)); }

    void set_parameter_attr(std::int32_t arg_no, attr a) {
        CHECK_STATUS(tinytc_func_set_parameter_attr(obj_, arg_no, a));
    }
};

/**
 * @brief Make function
 *
 * @param name Function name
 * @param param_type_list List of parameter types
 * @param ty Result type (must be void for host-callable function)
 * @param loc Source code location
 *
 * @return Function
 */
inline func make_func(std::string_view name, array_view<data_type> param_type_list, data_type ty,
                      location const &loc = {}) {
    tinytc_func_t fun;
    auto len = param_type_list.size();
    if (len > std::numeric_limits<std::uint32_t>::max()) {
        throw std::out_of_range("param list too long");
    }
    CHECK_STATUS_LOC(
        tinytc_func_create(&fun, name.size(), name.data(), len, param_type_list.data(), ty, &loc),
        loc);
    return func(fun);
}

////////////////////////////
/////////// Prog ///////////
////////////////////////////

namespace internal {
template <> struct shared_handle_traits<tinytc_prog_t> {
    static auto retain(tinytc_prog_t handle) -> tinytc_status_t {
        return tinytc_prog_retain(handle);
    }
    static auto release(tinytc_prog_t handle) -> tinytc_status_t {
        return tinytc_prog_release(handle);
    }
};
template <> struct unique_handle_traits<char *> {
    static void destroy(char *obj) { tinytc_string_destroy(obj); }
};
} // namespace internal

//! @brief Reference-counting wrapper for tinytc_prog_t
class prog : public shared_handle<tinytc_prog_t> {
  public:
    using shared_handle::shared_handle;

    /**
     * @brief Append function to program
     *
     * @param fun function
     */
    inline void add_function(func fun) {
        CHECK_STATUS(tinytc_prog_add_function(obj_, fun.release()));
    }

    /**
     * @brief Dump program to stderr
     */
    void dump() const { CHECK_STATUS(tinytc_prog_dump(obj_)); }
    /**
     * @brief Get context
     *
     * @return Compiler context
     */
    auto get_compiler_context() const -> compiler_context {
        tinytc_compiler_context_t ctx;
        CHECK_STATUS(tinytc_prog_get_compiler_context(obj_, &ctx));
        return compiler_context{ctx, true};
    }
    /**
     * @brief Dump program to file
     *
     * @param filename Path to file
     */
    void print_to_file(char const *filename) const {
        CHECK_STATUS(tinytc_prog_print_to_file(obj_, filename));
    }
    /**
     * @brief Dump program to string
     *
     * @return C-string (unique handle)
     */
    auto print_to_string() const -> unique_handle<char *> {
        char *str;
        CHECK_STATUS(tinytc_prog_print_to_string(obj_, &str));
        return unique_handle<char *>{str};
    }
};

/**
 * @brief Make program
 *
 * @param ctx Compiler context
 * @param loc Source code location
 *
 * @return Program
 */
inline prog make_prog(compiler_context const &ctx, location const &loc = {}) {
    tinytc_prog_t prg;
    CHECK_STATUS_LOC(tinytc_prog_create(&prg, ctx.get(), &loc), loc);
    return prog{prg};
}

////////////////////////////
/////// SPIR-V Module //////
////////////////////////////

namespace internal {
template <> struct shared_handle_traits<tinytc_spv_mod_t> {
    static auto retain(tinytc_spv_mod_t handle) -> tinytc_status_t {
        return tinytc_spv_mod_retain(handle);
    }
    static auto release(tinytc_spv_mod_t handle) -> tinytc_status_t {
        return tinytc_spv_mod_release(handle);
    }
};
} // namespace internal

//! @brief Reference-counting wrapper for tinytc_spv_mod_t
class spv_mod : public shared_handle<tinytc_spv_mod_t> {
  public:
    using shared_handle::shared_handle;

    /**
     * @brief Dump module to stderr
     */
    void dump() const { CHECK_STATUS(tinytc_spv_mod_dump(obj_)); }
    /**
     * @brief Dump module to file
     *
     * @param filename Path to file
     */
    void print_to_file(char const *filename) const {
        CHECK_STATUS(tinytc_spv_mod_print_to_file(obj_, filename));
    }
    /**
     * @brief Dump module to string
     *
     * @return C-string (unique handle)
     */
    auto print_to_string() const -> unique_handle<char *> {
        char *str;
        CHECK_STATUS(tinytc_spv_mod_print_to_string(obj_, &str));
        return unique_handle<char *>{str};
    }
};

////////////////////////////
////////// Builder /////////
////////////////////////////

//! Builder for regions
class region_builder {
  public:
    /**
     * @brief ctor
     *
     * @param reg region object
     */
    region_builder(region reg) : reg_{reg}, ip_{reg_.end()} {}
    /**
     * @brief ctor
     *
     * @param reg region object
     * @param ip insertion point
     */
    region_builder(region reg, tinytc_inst_iterator_t ip) : reg_{reg}, ip_{ip} {}

    /**
     * @brief Get insertion point
     *
     * @return Iterator
     */
    inline auto get_insertion_point() const -> tinytc_inst_iterator_t { return ip_; }

    /**
     * @brief Add instruction
     *
     * @param i Instruction
     *
     * @return Value returned by instruction; may be empty
     */
    [[maybe_unused]] inline auto add(inst i) -> value {
        auto result = value{};
        i.get_values(result);
        reg_.insert(ip_, std::move(i));
        return result;
    }

    /**
     * @brief Add instruction that returns multiple values
     *
     * @param i Instruction
     *
     * @return Values returned by instruction
     */
    [[maybe_unused]] inline auto add_multivalued(inst i) -> std::vector<value> {
        auto num_results = i.get_values({});
        auto results = std::vector<value>(static_cast<std::size_t>(num_results));
        results.resize(i.get_values(results));
        reg_.insert(ip_, std::move(i));
        return results;
    }

    /**
     * @brief Build for-loop with functor f(region_builder&, value) -> void
     *
     * The loop trip count is passed as second argument to the functor.
     *
     * @tparam F Functor type
     * @param from Loop variable start
     * @param to Loop variable bound
     * @param loop_var_ty Type of loop variable
     * @param f Functor
     * @param attributes For attributes
     * @param loc Source code location
     */
    template <typename F>
    void for_loop(data_type loop_var_ty, value from, value to, F &&f, attr attributes = nullptr,
                  location const &loc = {}) {
        for_loop<F>(std::move(loop_var_ty), std::move(from), std::move(to), nullptr,
                    std::forward<F>(f), attributes, loc);
    }
    /**
     * @brief Build for-loop with functor f(region_builder&, value) -> void
     *
     * The loop trip count is passed as second argument to the functor.
     *
     * @tparam F Functor type
     * @param loop_var_ty Type of loop variable
     * @param from Loop variable start
     * @param to Loop variable bound
     * @param step Loop variable step
     * @param f Functor
     * @param attributes For attributes
     * @param loc Source code location
     */
    template <typename F>
    void for_loop(data_type loop_var_ty, value from, value to, value step, F &&f,
                  attr attributes = nullptr, location const &loc = {}) {
        auto fi = ::tinytc::make_for(loop_var_ty, from, to, step, {}, {}, loc);
        fi.set_attr(attributes);
        auto reg = region{};
        fi.get_regions(reg);
        auto loop_var = value{};
        reg.get_parameters(loop_var);
        if (!reg || !loop_var) {
            throw status::internal_compiler_error;
        }
        reg_.insert(ip_, std::move(fi));
        auto bb = region_builder{reg};
        f(bb, loop_var);
    }
    /**
     * @brief Build for-loop with functor f(region_builder&, array_view<value>) -> void
     *
     * The loop trip count is the first value in the array_view.
     * The following values are the loop-carried values.
     *
     * @tparam F Functor type
     * @param loop_var_ty Type of loop variable
     * @param from Loop variable start
     * @param to Loop variable bound
     * @param step Loop variable step
     * @param initial_value_list Array of initial values; can be {}
     * @param return_type_list Array of return types; can be {}
     * @param f Functor
     * @param attributes For attributes
     * @param loc Source code location
     */
    template <typename F>
    auto for_loop(data_type loop_var_ty, value from, value to, value step,
                  array_view<value> initial_value_list, array_view<data_type> return_type_list,
                  F &&f, attr attributes = nullptr, location const &loc = {})
        -> std::vector<value> {
        auto fi = ::tinytc::make_for(loop_var_ty, from, to, step, initial_value_list,
                                     return_type_list, loc);
        fi.set_attr(attributes);
        auto reg = region{};
        fi.get_regions(reg);
        auto num_params = reg.get_parameters({});
        auto params = std::vector<value>(num_params);
        reg.get_parameters(params);
        if (!reg || num_params != 1 + initial_value_list.size()) {
            throw status::internal_compiler_error;
        }
        auto results = add_multivalued(std::move(fi));
        auto bb = region_builder{reg};
        f(bb, array_view<value>(params));
        return results;
    }
    /**
     * @brief Build foreach-loop with functor f(region_builder&, array_view<value>) -> void
     *
     * @tparam F Functor type
     * @param loop_var_ty Type of loop variable
     * @param from Loop variable start list
     * @param to Loop variable bound list
     * @param f functor
     * @param loc Source code location
     */
    template <typename F>
    void foreach_loop(data_type loop_var_ty, array_view<value> from, array_view<value> to, F &&f,
                      location const &loc = {}) {
        auto fi = ::tinytc::make_foreach(loop_var_ty, std::move(from), std::move(to), loc);
        auto reg = region{};
        fi.get_regions(reg);
        auto num_params = reg.get_parameters({});
        auto params = std::vector<value>(num_params);
        reg.get_parameters(params);
        if (!reg || num_params != from.size() || num_params != to.size()) {
            throw status::internal_compiler_error;
        }
        reg_.insert(ip_, std::move(fi));
        auto bb = region_builder{reg};
        f(bb, array_view<value>(params));
    }

    /**
     * @brief Build if with functor then(region_builder&) -> void
     *
     * Note: If the if instruction returns values then we must have a "yield" instruction in
     * both the "then" and the "else" branch. So to return values use the "ifelse" function.
     *
     * @tparam F Functor type
     * @param condition Condition value
     * @param then Then region functor
     * @param loc Source code location
     *
     * @return Returned values
     */
    template <typename F> void if_condition(value condition, F &&then, location const &loc = {}) {
        auto ii = ::tinytc::make_if(std::move(condition), {}, loc);
        auto reg = region{};
        ii.get_regions(reg);
        if (!reg) {
            throw status::internal_compiler_error;
        }
        reg_.insert(ip_, std::move(ii));
        auto bb = region_builder{reg};
        then(bb);
    }
    /**
     * @brief Build if/else with functors then(region_builder&) -> void and
     * otherwise(region_builder&) -> void
     *
     * @tparam F "if" functor type
     * @tparam G "else" functor type
     * @param condition If condition
     * @param then "if" functor
     * @param otherwise "else" functor
     * @param return_type_list List of types of returned values
     * @param loc Source code location
     *
     * @return Returned values
     */
    template <typename F, typename G>
    auto ifelse(value condition, F &&then, G &&otherwise,
                array_view<data_type> return_type_list = {}, location const &loc = {})
        -> std::vector<value> {
        auto ii = ::tinytc::make_if(std::move(condition), return_type_list, loc);
        std::array<region, 2u> regs = {};
        ii.get_regions(regs);
        if (!regs[0] || !regs[1]) {
            throw status::internal_compiler_error;
        }
        auto results = add_multivalued(std::move(ii));
        auto bb0 = region_builder{regs[0]};
        then(bb0);
        auto bb1 = region_builder{regs[1]};
        otherwise(bb1);
        return results;
    }

    /**
     * @brief Get region
     *
     * @return Region
     */
    inline auto get_region() -> region { return reg_; }

  private:
    region reg_;
    tinytc_inst_iterator_t ip_;
};

////////////////////////////
//////// Device info ///////
////////////////////////////

namespace internal {
template <> struct shared_handle_traits<tinytc_core_info_t> {
    static auto retain(tinytc_core_info_t handle) -> tinytc_status_t {
        return tinytc_core_info_retain(handle);
    }
    static auto release(tinytc_core_info_t handle) -> tinytc_status_t {
        return tinytc_core_info_release(handle);
    }
};
} // namespace internal

//! @brief Reference-counting wrapper for tinytc_core_info_t
class core_info : public shared_handle<tinytc_core_info_t> {
  public:
    using shared_handle::shared_handle;

    /**
     * @brief Get subgroup sizes
     *
     * @return Subgroup sizes
     */
    auto get_subgroup_sizes() const -> array_view<std::int32_t> {
        std::uint32_t sgs_size = 0;
        std::int32_t const *sgs = nullptr;
        CHECK_STATUS(tinytc_core_info_get_subgroup_sizes(obj_, &sgs_size, &sgs));
        return array_view(sgs, static_cast<std::size_t>(sgs_size));
    }

    /**
     * @brief Get register space per subgroup in bytes
     *
     * @return Register space
     */
    auto get_register_space() const -> std::int32_t {
        std::int32_t space;
        CHECK_STATUS(tinytc_core_info_get_register_space(obj_, &space));
        return space;
    }

    /**
     * @brief Set core features
     *
     * @param flags set core features; must be 0 or a combination of tinytc_core_feature_flag_t
     */
    void set_core_features(tinytc_core_feature_flags_t flags) {
        CHECK_STATUS(tinytc_core_info_set_core_features(obj_, flags));
    }

    /**
     * @brief Get core features
     *
     * @return Core features
     */
    auto get_core_features() const -> tinytc_core_feature_flags_t {
        tinytc_core_feature_flags_t flags;
        CHECK_STATUS(tinytc_core_info_get_core_features(obj_, &flags));
        return flags;
    }

    /**
     * @brief Set SPIR-V feature
     *
     * @param feature SPIR-V feature
     * @param available true if feature is available and false otherwise
     */
    void set_spirv_feature(spirv_feature feature, bool available) {
        CHECK_STATUS(tinytc_core_info_set_spirv_feature(
            obj_, static_cast<tinytc_spirv_feature_t>(feature), available));
    }

    /**
     * @brief Get SPIR-V feature
     *
     * @param feature SPIR-V feature
     *
     * @return true if feature is available and false otherwise
     */
    auto have_spirv_feature(spirv_feature feature) const -> bool {
        tinytc_bool_t available;
        CHECK_STATUS(tinytc_core_info_have_spirv_feature(
            obj_, static_cast<tinytc_spirv_feature_t>(feature), &available));
        return available;
    }

    /**
     * @brief Get default alignment
     *
     * @return alignment in bytes
     */
    auto get_default_alignment() const -> std::int32_t {
        std::int32_t alignment;
        CHECK_STATUS(tinytc_core_info_get_default_alignment(obj_, &alignment));
        return alignment;
    }

    /**
     * @brief Set default alignment
     *
     * @param alignment alignment in bytes
     */
    void set_default_alignment(std::int32_t alignment) {
        CHECK_STATUS(tinytc_core_info_set_default_alignment(obj_, alignment));
    }
};

/**
 * @brief Create core info for generic GPUs manually
 *
 * @param register_space Size of register file per subgroup in bytes
 * @param max_work_group_size Maximum size of local work group
 * @param sgs Subgrouip sizes
 *
 * @return Core info
 */
inline auto make_core_info_generic(std::int32_t register_space, std::int32_t max_work_group_size,
                                   array_view<std::int32_t> sgs) -> core_info {
    tinytc_core_info_t info;
    CHECK_STATUS(tinytc_core_info_generic_create(&info, register_space, max_work_group_size,
                                                 sgs.size(), sgs.data()));
    return core_info{info};
}

/**
 * @brief Get core info for Intel GPUs from lookup table
 *
 * @param arch IP version
 *
 * @return Core info
 */
inline auto make_core_info_intel_from_arch(intel_gpu_architecture arch) -> core_info {
    tinytc_core_info_t info;
    CHECK_STATUS(tinytc_core_info_intel_create_from_arch(
        &info, static_cast<tinytc_intel_gpu_architecture_t>(arch)));
    return core_info{info};
}

/**
 * @brief Get core info for Intel GPUs from lookup table
 *
 * @param name architecture name
 *
 * @return Core info
 */
inline auto make_core_info_intel_from_name(char const *name) -> core_info {
    tinytc_core_info_t info;
    CHECK_STATUS(tinytc_core_info_intel_create_from_name(&info, name));
    return core_info{info};
}

/**
 * @brief Create core info for Intel GPUs manually
 *
 * @param ip_version IP version
 * @param num_eus_per_subslice Number of EUs (XVEs) per subslice (XeCore)
 * @param num_threads_per_eu Number of hardware threads per EU (XVE)
 * @param sgs Subgrouip sizes
 *
 * @return Core info
 */
inline auto make_core_info_intel(std::uint32_t ip_version, std::int32_t num_eus_per_subslice,
                                 std::int32_t num_threads_per_eu, array_view<std::int32_t> sgs)
    -> core_info {
    tinytc_core_info_t info;
    CHECK_STATUS(tinytc_core_info_intel_create(&info, ip_version, num_eus_per_subslice,
                                               num_threads_per_eu, sgs.size(), sgs.data()));
    return core_info{info};
}

//! Convert SPIR-V feature to string
inline char const *to_string(spirv_feature f) {
    return ::tinytc_spirv_feature_to_string(static_cast<tinytc_spirv_feature_t>(f));
}

////////////////////////////
////////// Parser //////////
////////////////////////////

/**
 * @brief Parse source text from file
 *
 * @param filename Filename
 * @param ctx Compiler context
 *
 * @return Program
 */
inline auto parse_file(char const *filename, compiler_context const &ctx = {}) -> prog {
    tinytc_prog_t prg;
    CHECK_STATUS(tinytc_parse_file(&prg, filename, ctx.get()));
    return prog(prg);
}

/**
 * @brief Parse source text from stdin
 *
 * @param ctx Compiler context
 *
 * @return Program
 */
inline auto parse_stdin(compiler_context const &ctx = {}) -> prog {
    tinytc_prog_t prg;
    CHECK_STATUS(tinytc_parse_stdin(&prg, ctx.get()));
    return prog(prg);
}
/**
 * @brief Parse source text from string
 *
 * @param src Source text
 * @param ctx Compiler context
 *
 * @return Porgram
 */
inline auto parse_string(std::string const &src, compiler_context const &ctx = {}) -> prog {
    tinytc_prog_t prg;
    CHECK_STATUS(tinytc_parse_string(&prg, src.size(), src.c_str(), ctx.get()));
    return prog(prg);
}

////////////////////////////
///////// Compiler /////////
////////////////////////////

namespace internal {
template <> struct shared_handle_traits<tinytc_binary_t> {
    static auto retain(tinytc_binary_t handle) -> tinytc_status_t {
        return tinytc_binary_retain(handle);
    }
    static auto release(tinytc_binary_t handle) -> tinytc_status_t {
        return tinytc_binary_release(handle);
    }
};
} // namespace internal

//! @brief Reference-counting wrapper for tinytc_binary_t
class binary : public shared_handle<tinytc_binary_t> {
  public:
    using shared_handle::shared_handle;

    //! Container for raw data
    struct raw {
        bundle_format format;     ///< Bundle format
        std::size_t data_size;    ///< Size of binary data in bytes
        std::uint8_t const *data; ///< Pointer to binary data
    };

    /**
     * @brief Get raw data
     *
     * @return Raw data
     */
    inline auto get_raw() const -> raw {
        raw r;
        tinytc_bundle_format_t f;
        CHECK_STATUS(tinytc_binary_get_raw(obj_, &f, &r.data_size, &r.data));
        r.format = bundle_format{std::underlying_type_t<bundle_format>(f)};
        return r;
    }
    /**
     * @brief Get compiler context
     *
     * @return Compiler context
     */
    inline auto get_compiler_context() const -> compiler_context {
        tinytc_compiler_context_t ctx;
        CHECK_STATUS(tinytc_binary_get_compiler_context(obj_, &ctx));
        return compiler_context{ctx, true};
    }
    /**
     * @brief Get core features
     *
     * @return Core features
     */
    inline auto get_core_features() const -> tinytc_core_feature_flags_t {
        tinytc_core_feature_flags_t cf;
        CHECK_STATUS(tinytc_binary_get_core_features(obj_, &cf));
        return cf;
    }
};

/**
 * @brief Make binary
 *
 * @param ctx Compiler context
 * @param format Bundle format (SPIR-V or Native)
 * @param data_size Size of data in bytes
 * @param data Binary data; data is copied
 * @param core_features requested core features; must be 0 (default) or a combination of
 * tinytc_core_feature_flag_t
 *
 * @return Binary
 */
inline auto make_binary(compiler_context const &ctx, bundle_format format, std::size_t data_size,
                        std::uint8_t const *data, tinytc_core_feature_flags_t core_features)
    -> binary {
    tinytc_binary_t bin;
    CHECK_STATUS(tinytc_binary_create(&bin, ctx.get(), static_cast<tinytc_bundle_format_t>(format),
                                      data_size, data, core_features));
    return binary{bin};
}

/**
 * @brief Run a function pass on every function of a program
 *
 * @param pass_name name of function pass; cf. list_function_passes
 * @param prg tensor program; modified as compiler pass is run
 * @param info core info object; might be nullptr if core info is not required for pass
 */
inline void run_function_pass(char const *pass_name, prog prg, core_info info = {}) {
    CHECK_STATUS(tinytc_run_function_pass(pass_name, prg.get(), info.get()));
}

/**
 * @brief Get function pass names
 *
 * @param names_size Number of function pass names
 * @param names Array of function pass names
 */
inline void list_function_passes(std::uint32_t &names_size, char const *const *&names) {
    CHECK_STATUS(tinytc_list_function_passes(&names_size, &names));
}

/**
 * @brief Convert tensor language to SPIR-V
 *
 * @param prg Program
 * @param info Core info
 *
 * @return SPIR-V module
 */
inline auto compile_to_spirv(prog prg, core_info const &info) -> spv_mod {
    tinytc_spv_mod_t mod;
    CHECK_STATUS(tinytc_prog_compile_to_spirv(&mod, prg.get(), info.get()));
    return spv_mod{mod};
}

/**
 * @brief Compile program to SPIR-V and assemble
 *
 * @param prg Program
 * @param info Core info
 *
 * @return Binary
 */
inline auto compile_to_spirv_and_assemble(prog prg, core_info const &info) -> binary {
    tinytc_binary_t bin;
    CHECK_STATUS(tinytc_prog_compile_to_spirv_and_assemble(&bin, prg.get(), info.get()));
    return binary{bin};
}

/**
 * @brief Assemble SPIR-V module
 *
 * @param mod [in] SPIR-V module
 *
 * @return Binary
 */
inline auto spirv_assemble(spv_mod const &mod) -> binary {
    tinytc_binary_t bin;
    CHECK_STATUS(tinytc_spirv_assemble(&bin, mod.get()));
    return binary{bin};
}

////////////////////////////
////////// Recipe //////////
////////////////////////////

/**
 * @brief Guess memory type of memory object
 *
 * @tparam T memory object type
 */
template <typename T, typename Enable = void> struct auto_mem_type;

/**
 * @brief Check whether T maps to a scalar data type
 *
 * @tparam T type
 */
template <typename T>
constexpr bool is_supported_scalar_type = std::is_same_v<T, std::int8_t> ||         // i8
                                          std::is_same_v<T, std::int16_t> ||        // i16
                                          std::is_same_v<T, std::int32_t> ||        // i32
                                          std::is_same_v<T, std::int64_t> ||        // i64
                                          std::is_same_v<T, float> ||               // f32
                                          std::is_same_v<T, double> ||              // f64
                                          std::is_same_v<T, std::complex<float>> || // c32
                                          std::is_same_v<T, std::complex<double>>;  // c64

/**
 * @brief True if T is either pointer to a support scalar type or a pointer to a pointer to a
 * supported scalar type; void* is fine, too
 *
 * @tparam T type
 */
template <typename T>
constexpr bool is_usm_pointer_type =
    std::is_same_v<T, void *> ||
    (std::is_pointer_v<T> &&
     (is_supported_scalar_type<std::remove_pointer_t<T>> ||
      is_supported_scalar_type<std::remove_pointer_t<std::remove_pointer_t<T>>>));

/**
 * @brief Specialize auto_mem_type for pointer to non-class types
 *
 * All pointers to scalars are assumed to be Unified Shared Memory pointers.
 * (Automatic guessing for Shared Virtual Memory pointers not implemented.)
 *
 * @tparam T memory object type
 */
template <typename T> struct auto_mem_type<T, std::enable_if_t<is_usm_pointer_type<T>>> {
    constexpr static mem_type value = mem_type::usm_pointer; ///< Pointer maps to USM pointer type
};

/**
 * @brief Convenience wrapper for auto_mem_type
 *
 * @tparam T memory object type
 */
template <typename T> inline constexpr auto auto_mem_type_v = auto_mem_type<T>::value;

//! Type-safe wrapper for memory objects
struct mem {
    /**
     * @brief ctor
     *
     * @tparam T pointer type or buffer type
     * @param value USM / SVM pointer or cl_mem (cl_mem implicitly converts to void*)
     * @param type memory object type
     */
    template <typename T>
    inline mem(T const value, mem_type type = auto_mem_type_v<T>) : value{value}, type{type} {}

    const void *value; ///< USM / SVM pointer or cl_mem (passed by value)
    mem_type type;     ///< Memory object type
};

namespace internal {
template <> struct shared_handle_traits<tinytc_recipe_t> {
    static auto retain(tinytc_recipe_t handle) -> tinytc_status_t {
        return tinytc_recipe_retain(handle);
    }
    static auto release(tinytc_recipe_t handle) -> tinytc_status_t {
        return tinytc_recipe_release(handle);
    }
};
template <> struct shared_handle_traits<tinytc_recipe_handler_t> {
    static auto retain(tinytc_recipe_handler_t handle) -> tinytc_status_t {
        return tinytc_recipe_handler_retain(handle);
    }
    static auto release(tinytc_recipe_handler_t handle) -> tinytc_status_t {
        return tinytc_recipe_handler_release(handle);
    }
};
} // namespace internal

//! @brief Reference-counting wrapper for tinytc_recipe_t
class recipe : public shared_handle<tinytc_recipe_t> {
  public:
    using shared_handle::shared_handle;

    /**
     * @brief Get program
     *
     * @return Program
     */
    auto get_prog() const -> prog {
        tinytc_prog_t prg;
        CHECK_STATUS(tinytc_recipe_get_prog(obj_, &prg));
        return prog{prg};
    }

    /**
     * @brief Get binary
     *
     * @return Binary
     */
    auto get_binary() const -> binary {
        tinytc_binary_t bin;
        CHECK_STATUS(tinytc_recipe_get_binary(obj_, &bin));
        return binary{bin};
    }
};

//! @brief Reference-counting wrapper for tinytc_recipe_handler_t
class recipe_handler : public shared_handle<tinytc_recipe_handler_t> {
  public:
    using shared_handle::shared_handle;

    /**
     * @brief Get recipe
     *
     * @return Recipe
     */
    auto get_recipe() const -> recipe {
        tinytc_recipe_t rec;
        CHECK_STATUS(tinytc_recipe_handler_get_recipe(obj_, &rec));
        return recipe{rec};
    }
};

//! @brief Reference-counting wrapper for tinytc_recipe_t
class small_gemm_batched : public recipe {
  public:
    using recipe::recipe;

    /**
     * @brief Set kernel arguments
     *
     * @tparam T Scalar type; must match scalar_type passed to constructor
     * @param handler Recipe handler
     * @param howmany Batch size
     * @param alpha @f$\alpha@f$
     * @param A Memory object used for A-matrix
     * @param B Memory object used for B-matrix
     * @param beta @f$\beta@f$
     * @param C Memory object used for C-matrix
     */
    template <typename T>
    static void set_args(recipe_handler &handler, std::int64_t howmany, T alpha, mem A, mem B,
                         T beta, mem C) {
        CHECK_STATUS(tinytc_recipe_small_gemm_batched_set_args(
            handler.get(), howmany, sizeof(alpha), &alpha, static_cast<tinytc_mem_type_t>(A.type),
            A.value, static_cast<tinytc_mem_type_t>(B.type), B.value, sizeof(beta), &beta,
            static_cast<tinytc_mem_type_t>(C.type), C.value));
    }
};

/**
 * @brief Make small GEMM batched recipe
 *
 * Cf. @ref tinytc_recipe_small_gemm_batched_create
 *
 * @param info Core info
 * @param ty Scalar type of @f$\alpha@f$, A, B, @f$\beta@f$, C
 * @param tA Operation applied on A
 * @param tB Operation applied on B
 * @param M Number of rows of A and C
 * @param N Number of columns of B and C
 * @param K Number of columns of A, number of rows of B
 * @param ldA Leading dimension of an A matrix
 * @param strideA Stride of A-matrices
 * @param ldB Leading dimension of an B matrix
 * @param strideB Stride of B-matrices
 * @param ldC Leading dimension of an C matrix
 * @param strideC Stride of C-matrices
 * @param ctx Compiler context
 *
 * @return Small GEMM batched recipe
 */
inline auto make_small_gemm_batched(core_info const &info, scalar_type ty, transpose tA,
                                    transpose tB, std::int64_t M, std::int64_t N, std::int64_t K,
                                    std::int64_t ldA, std::int64_t strideA, std::int64_t ldB,
                                    std::int64_t strideB, std::int64_t ldC, std::int64_t strideC,
                                    compiler_context const &ctx = {}) -> small_gemm_batched {
    tinytc_recipe_t rec;
    CHECK_STATUS(tinytc_recipe_small_gemm_batched_create(
        &rec, info.get(), static_cast<tinytc_scalar_type_t>(ty),
        static_cast<tinytc_transpose_t>(tA), static_cast<tinytc_transpose_t>(tB), M, N, K, ldA,
        strideA, ldB, strideB, ldC, strideC, ctx.get()));
    return small_gemm_batched{rec};
}

//! @brief Reference-counting wrapper for tinytc_recipe_t
class tall_and_skinny : public recipe {
  public:
    using recipe::recipe;

    /**
     * @brief Set kernel arguments
     *
     * @tparam T Scalar type; must match scalar_type passed to constructor
     * @param handler Recipe handler
     * @param M Number of rows of A and C
     * @param alpha @f$\alpha@f$
     * @param A Memory object used for A-matrix
     * @param ldA Leading dimension of A
     * @param B Memory object used for B-matrix
     * @param ldB Leading dimension of B
     * @param beta @f$\beta@f$
     * @param C Memory object used for C-matrix
     * @param ldC Leading dimension of C
     */
    template <typename T>
    static void set_args(recipe_handler &handler, std::int64_t M, T alpha, mem A, std::int64_t ldA,
                         mem B, std::int64_t ldB, T beta, mem C, std::int64_t ldC) {
        CHECK_STATUS(tinytc_recipe_tall_and_skinny_set_args(
            handler.get(), M, sizeof(alpha), &alpha, static_cast<tinytc_mem_type_t>(A.type),
            A.value, ldA, static_cast<tinytc_mem_type_t>(B.type), B.value, ldB, sizeof(beta), &beta,
            static_cast<tinytc_mem_type_t>(C.type), C.value, ldC));
    }
};

/**
 * @brief Make tall and skinny recipe
 *
 * Cf. @ref tinytc_recipe_tall_and_skinny_create
 *
 * @param info Core info
 * @param ty Scalar type of @f$\alpha@f$, A, B, @f$\beta@f$, C
 * @param N Number of columns of B and C
 * @param K Number of columns of A, number of rows of B
 * @param M_block_size Chunk size for M-mode
 * @param ctx Compiler context
 *
 * @return Tall and skinny recipe
 */
inline auto make_tall_and_skinny(core_info const &info, scalar_type ty, std::int64_t N,
                                 std::int64_t K, std::int32_t M_block_size = 0,
                                 compiler_context const &ctx = {}) -> tall_and_skinny {
    tinytc_recipe_t rec;
    CHECK_STATUS(tinytc_recipe_tall_and_skinny_create(
        &rec, info.get(), static_cast<tinytc_scalar_type_t>(ty), N, K, M_block_size, ctx.get()));
    return tall_and_skinny{rec};
}

/**
 * @brief Make tall and skinny recipe with additional specialization constants
 *
 * Cf. @ref tinytc_recipe_tall_and_skinny_create_specialized
 *
 * @param info Core info
 * @param ty Scalar type of @f$\alpha@f$, A, B, @f$\beta@f$, C
 * @param M Number of rows of A and C; can be dynamic
 * @param N Number of columns of B and C
 * @param K Number of columns of A, number of rows of B
 * @param ldA Leading dimension of A; can be dynamic
 * @param ldB Leading dimension of B; can be dynamic
 * @param ldC Leading dimension of C; can be dynamic
 * @param alignA [in] Memory alignment of A; can be 0
 * @param alignB [in] Memory alignment of B; can be 0
 * @param alignC [in] Memory alignment of C; can be 0
 * @param M_block_size Chunk size for M-mode
 * @param ctx Compiler context
 *
 * @return Tall and skinny recipe
 */
inline auto make_tall_and_skinny_specialized(core_info const &info, scalar_type ty, std::int64_t M,
                                             std::int64_t N, std::int64_t K, std::int64_t ldA,
                                             std::int64_t ldB, std::int64_t ldC,
                                             std::int32_t alignA, std::int32_t alignB,
                                             std::int32_t alignC, std::int32_t M_block_size = 0,
                                             compiler_context const &ctx = {}) -> tall_and_skinny {
    tinytc_recipe_t rec;
    CHECK_STATUS(tinytc_recipe_tall_and_skinny_create_specialized(
        &rec, info.get(), static_cast<tinytc_scalar_type_t>(ty), M, N, K, ldA, ldB, ldC, alignA,
        alignB, alignC, M_block_size, ctx.get()));
    return tall_and_skinny{rec};
}

} // namespace tinytc

namespace std {
template <typename... T> struct hash<tinytc::lp_float<T...>> {
    size_t operator()(tinytc::lp_float<T...> const &val) const noexcept {
        using h = hash<typename tinytc::lp_float<T...>::lp_format::bits_type>;
        return h{}(val.bits());
    }
};

} // namespace std

#endif // TINYTC_20240403_HPP
