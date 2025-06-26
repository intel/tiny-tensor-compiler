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
    inline char const *what() const noexcept override { return to_string(code_); }

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

/**
 * @brief Add compiler to context
 *
 * @param ctx compiler context
 * @param name File name
 * @param text Source text
 *
 * @return Source id (should be set in position.source_id)
 */
inline auto add_source(compiler_context &ctx, char const *name, char const *text) -> std::int32_t {
    std::int32_t source_id;
    CHECK_STATUS(tinytc_compiler_context_add_source(ctx.get(), name, text, &source_id));
    return source_id;
}

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

/**
 * @brief Set error reporter
 *
 * Error reporting function that is called whenever an error occurs in the parser or the
 * builder.
 *
 * @param ctx compiler context
 * @param reporter error reporting callback
 * @param user_data pointer to user data that is passed to the callback
 */
inline void set_error_reporter(compiler_context &ctx, error_reporter_t reporter,
                               void *user_data = nullptr) {
    CHECK_STATUS(tinytc_compiler_context_set_error_reporter(ctx.get(), reporter, user_data));
}

/**
 * @brief Sets an optimization flag
 *
 * The state can be 0 (disabled), 1 (enabled), or -1 (use default according to optimization
 * level).
 *
 * @param ctx compiler context
 * @param flag optimization flag
 * @param state flag state
 */
inline void set_optimization_flag(compiler_context &ctx, optflag flag, std::int32_t state) {
    CHECK_STATUS(tinytc_compiler_context_set_optimization_flag(
        ctx.get(), static_cast<tinytc_optflag_t>(flag), state));
}
/**
 * @brief Set optimization level
 *
 * @param ctx compiler context
 * @param level optimization level
 */
inline void set_optimization_level(compiler_context &ctx, std::int32_t level) {
    CHECK_STATUS(tinytc_compiler_context_set_optimization_level(ctx.get(), level));
}
/**
 * @brief Enhance error message with compiler context; useful when builder is used
 *
 * @param ctx compiler context
 * @param loc Source location
 * @param what Error description
 */
inline void report_error(compiler_context const &ctx, location const &loc, char const *what) {
    CHECK_STATUS(tinytc_compiler_context_report_error(ctx.get(), &loc, what));
}

////////////////////////////
/////////// Prog ///////////
////////////////////////////

/**
 * @brief Dump program to stderr
 *
 * @param p program
 */
inline void dump(prog const &p) { CHECK_STATUS(tinytc_prog_dump(p.get())); }
/**
 * @brief Get context
 *
 * @param p program
 *
 * @return Compiler context
 */
inline auto get_compiler_context(prog const &p) -> compiler_context {
    tinytc_compiler_context_t ctx;
    CHECK_STATUS(tinytc_prog_get_compiler_context(p.get(), &ctx));
    return compiler_context{ctx, true};
}
/**
 * @brief Dump program to file
 *
 * @param p program
 * @param filename Path to file
 */
inline void print_to_file(prog const &p, char const *filename) {
    CHECK_STATUS(tinytc_prog_print_to_file(p.get(), filename));
}
/**
 * @brief Dump program to string
 *
 * @param p program
 *
 * @return C-string (unique handle)
 */
inline auto print_to_string(prog const &p) -> unique_handle<char *> {
    char *str;
    CHECK_STATUS(tinytc_prog_print_to_string(p.get(), &str));
    return unique_handle<char *>{str};
}

////////////////////////////
/////// SPIR-V Module //////
////////////////////////////

/**
 * @brief Dump module to stderr
 *
 * @param mod SPIR-V module
 */
inline void dump(spv_mod const &mod) { CHECK_STATUS(tinytc_spv_mod_dump(mod.get())); }
/**
 * @brief Dump module to file
 *
 * @param mod SPIR-V module
 * @param filename Path to file
 */
inline void print_to_file(spv_mod const &mod, char const *filename) {
    CHECK_STATUS(tinytc_spv_mod_print_to_file(mod.get(), filename));
}
/**
 * @brief Dump module to string
 *
 * @param mod SPIR-V module
 *
 * @return C-string (unique handle)
 */
inline auto print_to_string(spv_mod const &mod) -> unique_handle<char *> {
    char *str;
    CHECK_STATUS(tinytc_spv_mod_print_to_string(mod.get(), &str));
    return unique_handle<char *>{str};
}

////////////////////////////
//////// Device info ///////
////////////////////////////

/**
 * @brief Get subgroup sizes
 *
 * @param info Core info
 *
 * @return Subgroup sizes
 */
inline auto get_subgroup_sizes(core_info const &info) -> array_view<std::int32_t> {
    std::size_t sgs_size = 0;
    std::int32_t const *sgs = nullptr;
    CHECK_STATUS(tinytc_core_info_get_subgroup_sizes(info.get(), &sgs_size, &sgs));
    return array_view(sgs, sgs_size);
}

/**
 * @brief Get register space per subgroup in bytes
 *
 * @param info Core info
 *
 * @return Register space
 */
inline auto get_register_space(core_info const &info) -> std::int32_t {
    std::int32_t space;
    CHECK_STATUS(tinytc_core_info_get_register_space(info.get(), &space));
    return space;
}

/**
 * @brief Set core features
 *
 * @param info Core info
 *
 * @param flags set core features; must be 0 or a combination of tinytc_core_feature_flag_t
 */
inline void set_core_features(core_info &info, tinytc_core_feature_flags_t flags) {
    CHECK_STATUS(tinytc_core_info_set_core_features(info.get(), flags));
}

/**
 * @brief Get core features
 *
 * @param info Core info
 *
 * @return Core features
 */
inline auto get_core_features(core_info const &info) -> tinytc_core_feature_flags_t {
    tinytc_core_feature_flags_t flags;
    CHECK_STATUS(tinytc_core_info_get_core_features(info.get(), &flags));
    return flags;
}

/**
 * @brief Set SPIR-V feature
 *
 * @param info Core info
 * @param feature SPIR-V feature
 * @param available true if feature is available and false otherwise
 */
inline void set_spirv_feature(core_info &info, spirv_feature feature, bool available) {
    CHECK_STATUS(tinytc_core_info_set_spirv_feature(
        info.get(), static_cast<tinytc_spirv_feature_t>(feature), available));
}

/**
 * @brief Get SPIR-V feature
 *
 * @param info Core info
 * @param feature SPIR-V feature
 *
 * @return true if feature is available and false otherwise
 */
inline auto have_spirv_feature(core_info const &info, spirv_feature feature) -> bool {
    tinytc_bool_t available;
    CHECK_STATUS(tinytc_core_info_have_spirv_feature(
        info.get(), static_cast<tinytc_spirv_feature_t>(feature), &available));
    return available;
}

/**
 * @brief Get default alignment
 *
 * @param info Core info
 *
 * @return alignment in bytes
 */
inline auto get_default_alignment(core_info const &info) -> std::int32_t {
    std::int32_t alignment;
    CHECK_STATUS(tinytc_core_info_get_default_alignment(info.get(), &alignment));
    return alignment;
}

/**
 * @brief Set default alignment
 *
 * @param info Core info
 *
 * @param alignment alignment in bytes
 */
inline void set_default_alignment(core_info &info, std::int32_t alignment) {
    CHECK_STATUS(tinytc_core_info_set_default_alignment(info.get(), alignment));
}

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
 * @return Program
 */
inline auto parse_string(std::string const &src, compiler_context const &ctx = {}) -> prog {
    tinytc_prog_t prg;
    CHECK_STATUS(tinytc_parse_string(&prg, src.size(), src.c_str(), ctx.get()));
    return prog(prg);
}

////////////////////////////
///////// Compiler /////////
////////////////////////////

/**
 * @brief Get compiler context
 *
 * @param bin Binary
 *
 * @return Compiler context
 */
inline auto get_compiler_context(binary const &bin) -> compiler_context {
    tinytc_compiler_context_t ctx;
    CHECK_STATUS(tinytc_binary_get_compiler_context(bin.get(), &ctx));
    return compiler_context{ctx, true};
}
/**
 * @brief Get core features
 *
 * @param bin Binary
 *
 * @return Core features
 */
inline auto get_core_features(binary const &bin) -> tinytc_core_feature_flags_t {
    tinytc_core_feature_flags_t cf;
    CHECK_STATUS(tinytc_binary_get_core_features(bin.get(), &cf));
    return cf;
}

//! Container for raw data
struct raw_binary {
    bundle_format format;     ///< Bundle format
    std::size_t data_size;    ///< Size of binary data in bytes
    std::uint8_t const *data; ///< Pointer to binary data
};

/**
 * @brief Get raw data
 *
 * @param bin Binary
 *
 * @return Raw data
 */
inline auto get_raw(binary const &bin) -> raw_binary {
    raw_binary r;
    tinytc_bundle_format_t f;
    CHECK_STATUS(tinytc_binary_get_raw(bin.get(), &f, &r.data_size, &r.data));
    r.format = bundle_format{std::underlying_type_t<bundle_format>(f)};
    return r;
}

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
inline void list_function_passes(std::size_t &names_size, char const *const *&names) {
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

/**
 * @brief Get program
 *
 * @param rec Recipe
 *
 * @return Program
 */
inline auto get_prog(recipe const &rec) -> prog {
    tinytc_prog_t prg;
    CHECK_STATUS(tinytc_recipe_get_prog(rec.get(), &prg));
    return prog{prg};
}

/**
 * @brief Get binary
 *
 * @param rec Recipe
 *
 * @return Binary
 */
inline auto get_binary(recipe const &rec) -> binary {
    tinytc_binary_t bin;
    CHECK_STATUS(tinytc_recipe_get_binary(rec.get(), &bin));
    return binary{bin};
}

/**
 * @brief Get recipe
 *
 * @param handler Recipe handler
 *
 * @return Recipe
 */
inline auto get_recipe(recipe_handler const &handler) -> recipe {
    tinytc_recipe_t rec;
    CHECK_STATUS(tinytc_recipe_handler_get_recipe(handler.get(), &rec));
    return recipe{rec};
}

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
