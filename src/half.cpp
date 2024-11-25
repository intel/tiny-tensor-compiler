// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include <bit>
#include <cstdint>

namespace tinytc {

template <uint32_t ExponentBits, uint32_t MantissaBits> struct ieee754_info {
    constexpr static uint32_t exponent_bits = ExponentBits;
    constexpr static uint32_t mantissa_bits = MantissaBits;
    constexpr static uint32_t num_bits = 1 + exponent_bits + mantissa_bits;
    constexpr static uint32_t bias = (1 << (exponent_bits - 1)) - 1;
    constexpr static uint32_t max_biased_exponent = (1 << exponent_bits) - 1;
    constexpr static uint32_t sign_mask = 1 << (num_bits - 1);
    constexpr static uint32_t exponent_mask = max_biased_exponent << mantissa_bits;
    constexpr static uint32_t mantissa_mask = (1 << mantissa_bits) - 1;
};

using bf16i = ieee754_info<8, 7>;
using f16i = ieee754_info<5, 10>;
using f32i = ieee754_info<8, 23>;

template <typename F16i, typename F32i, typename UITrunc, typename UI>
auto ieee754_truncate(UI x) -> UITrunc {
    constexpr UI num_shift_bits = F32i::mantissa_bits - F16i::mantissa_bits;
    auto const round_nearest_even_and_truncate = [](UI mantissa32) {
        constexpr UI midpoint = (1 << num_shift_bits) / 2;
        const UI bias = ((mantissa32 >> num_shift_bits) & 0x1) + (midpoint - 1);
        return (mantissa32 + bias) >> num_shift_bits;
    };

    const UITrunc sign = (x & F32i::sign_mask) >> (F32i::num_bits - F16i::num_bits);
    const UI exponent32 = (x & F32i::exponent_mask) >> F32i::mantissa_bits;
    const UI mantissa32 = x & F32i::mantissa_mask;

    UITrunc exponent16 = 0;
    UITrunc mantissa16 = 0;
    if (exponent32 > F32i::bias + F16i::bias) {
        exponent16 = F16i::max_biased_exponent;
        // Map numbers except NaN to inf
        if (exponent32 < F32i::max_biased_exponent) {
            mantissa16 = 0;
        } else {
            // Need to ceil to make sure that NaN is not truncated to inf
            mantissa16 = 1 + ((mantissa32 - 1) >> num_shift_bits);
        }
    } else if (F32i::bias == F16i::bias || exponent32 > F32i::bias - F16i::bias) {
        // convert bias
        // E_{32} = e + F32i::bias
        // E_{16} = e + F16i::bias
        //        = E_{32} - F32i::bias + F16i::bias
        //        = E_{32} - (F32i::bias - F16i::bias)
        exponent16 = exponent32 - (F32i::bias - F16i::bias);
        mantissa16 = round_nearest_even_and_truncate(mantissa32);
    } else if (exponent32 >= F32i::bias + 1 - F16i::bias - F16i::mantissa_bits) {
        exponent16 = 0;
        mantissa16 = round_nearest_even_and_truncate((mantissa32 | (1 << F32i::mantissa_bits)) >>
                                                     ((F32i::bias + 1 - F16i::bias) - exponent32));
    }

    exponent16 <<= F16i::mantissa_bits;

    // Need to add mantissa as it might overflow during rounding and then we need to increase the
    // exponent by 1
    return (sign | exponent16) + mantissa16;
}

template <typename F32i, typename F16i, typename UIExt, typename UI>
auto ieee754_extend(UI x) -> UIExt {
    const UIExt sign = (x & F16i::sign_mask) << (F32i::num_bits - F16i::num_bits);
    const UIExt exponent16 = (x & F16i::exponent_mask) >> F16i::mantissa_bits;
    const UIExt mantissa16 = x & F16i::mantissa_mask;

    UIExt exponent32 = exponent16;
    UIExt mantissa32 = mantissa16;
    if (F32i::exponent_bits != F16i::exponent_bits) {
        if (exponent16 == F16i::max_biased_exponent) {
            // Inf and NaN
            exponent32 = F32i::max_biased_exponent;
        } else if (exponent16 != 0) {
            // convert bias
            // E_{16} = e + F16i::bias
            // E_{32} = e + F32i::bias
            //        = E_{16} - F16i::bias + F32i::bias
            //        = E_{16} + (F32i::bias - F16i::bias)
            exponent32 += F32i::bias - F16i::bias;
        }

        // Subnormal f16 numbers must be represented as f32 normal numbers
        if (exponent16 == 0 && mantissa16 != 0) {
            UIExt shift_count = 0;
            do {
                mantissa32 <<= 1;
                ++shift_count;
            } while ((mantissa32 & (1 << F16i::mantissa_bits)) != (1 << F16i::mantissa_bits));
            mantissa32 &= F16i::mantissa_mask;
            exponent32 = F32i::bias + 1 - F16i::bias - shift_count;
        }
    }

    // shift mantissa
    mantissa32 <<= F32i::mantissa_bits - F16i::mantissa_bits;

    // shift exponent
    exponent32 <<= F32i::mantissa_bits;

    return sign | exponent32 | mantissa32;
}

} // namespace tinytc

using namespace tinytc;

extern "C" {

uint16_t tinytc_f32_to_f16_as_ui16(float x) {
    return ieee754_truncate<f16i, f32i, uint16_t>(std::bit_cast<uint32_t>(x));
}

float tinytc_f16_as_ui16_to_f32(uint16_t x) {
    const auto y = ieee754_extend<f32i, f16i, uint32_t>(x);
    return std::bit_cast<float>(y);
}

uint16_t tinytc_f32_to_bf16_as_ui16(float x) {
    return ieee754_truncate<bf16i, f32i, uint16_t>(std::bit_cast<uint32_t>(x));
}

float tinytc_bf16_as_ui16_to_f32(uint16_t x) {
    const auto y = ieee754_extend<f32i, bf16i, uint32_t>(x);
    return std::bit_cast<float>(y);
}
}
