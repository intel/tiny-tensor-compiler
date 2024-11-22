// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include <bit>
#include <cstdint>

template <uint32_t ExponentBits, uint32_t MantissaBits> struct fp_info {
    constexpr static uint32_t exponent_bits = ExponentBits;
    constexpr static uint32_t mantissa_bits = MantissaBits;
    constexpr static uint32_t num_bits = 1 + exponent_bits + mantissa_bits;
    constexpr static uint32_t bias = (1 << (exponent_bits - 1)) - 1;
    constexpr static uint32_t max_biased_exponent = (1 << exponent_bits) - 1;
    constexpr static uint32_t sign_mask = 1 << (num_bits - 1);
    constexpr static uint32_t exponent_mask = max_biased_exponent << mantissa_bits;
    constexpr static uint32_t mantissa_mask = (1 << mantissa_bits) - 1;
};

using f16i = fp_info<5, 10>;
using f32i = fp_info<8, 23>;

extern "C" {

uint16_t tinytc_f32_to_f16(float x) {
    const uint32_t y = std::bit_cast<uint32_t>(x);
    const uint16_t sign = (y & f32i::sign_mask) >> (f32i::num_bits - f16i::num_bits);
    uint32_t exponent = (y & f32i::exponent_mask) >> f32i::mantissa_bits;
    uint32_t mantissa = y & f32i::mantissa_mask;

    if (exponent > f32i::bias + f16i::bias) {
        // Large numbers are mapped to inf
        exponent = f16i::max_biased_exponent;
        mantissa = 0;
    } else if (exponent > f32i::bias - f16i::bias) {
        // Normal numbers

        // convert bias
        // E_{32} = e + f32i::bias
        // E_{16} = e + f16i::bias
        //        = E_{32} - f32i::bias + f16i::bias
        //        = E_{32} - (f32i::bias - f16i::bias)
        exponent -= f32i::bias - f16i::bias;

        constexpr uint32_t num_shift_bits = f32i::mantissa_bits - f16i::mantissa_bits;
        constexpr uint32_t midpoint = (1 << num_shift_bits) / 2;
        constexpr uint32_t low_bit_mask = (1 << num_shift_bits) - 1;
        const uint32_t truncated_bits = mantissa & low_bit_mask;

        // shift mantissa and round correctly
        mantissa >>= num_shift_bits;
        if (truncated_bits > midpoint) {
            mantissa += 1;
        } else if (truncated_bits == midpoint) {
            // when there is a tie round to nearest even
            mantissa += mantissa & 1;
        }
        // We had an overflow during rounding
        if ((mantissa & (1 << f16i::mantissa_bits)) != 0) {
            ++exponent;
            if (exponent > f16i::max_biased_exponent) {
                // Overflow to infinity
                exponent = f16i::max_biased_exponent;
                mantissa = 0;
            } else {
                mantissa &= f16i::mantissa_mask;
            }
        }
    } else {
        // @todo
    }

    exponent <<= f16i::exponent_bits;

    return sign | static_cast<uint16_t>(exponent) | static_cast<uint16_t>(mantissa);
}

float tinytc_f16_to_f32(uint16_t x) {
    const uint32_t sign = (x & f16i::sign_mask) << (f32i::num_bits - f16i::num_bits);
    uint32_t exponent = (x & f16i::exponent_mask) >> f16i::mantissa_bits;
    uint32_t mantissa = x & f16i::mantissa_mask;

    if (exponent == f16i::max_biased_exponent) {
        // Inf and NaN
        exponent = (f32i::max_biased_exponent << f32i::mantissa_bits);
    } else if (exponent != 0) {
        // convert bias
        // E_{16} = e + f16i::bias
        // E_{32} = e + f32i::bias
        //        = E_{16} - f16i::bias + f32i::bias
        //        = E_{16} + (f32i::bias - f16i::bias)
        exponent += f32i::bias - f16i::bias;

        // shift exponent
        exponent <<= f32i::mantissa_bits;
    }

    // Subnormal f16 numbers must be represented as f32 normal numbers
    if (exponent == 0 && mantissa != 0) {
        uint8_t shift_count = 1;
        do {
            mantissa <<= 1;
            ++shift_count;
        } while ((mantissa & (1 << f16i::mantissa_bits)) != (1 << f16i::mantissa_bits));
        mantissa &= f16i::mantissa_mask;
        exponent = (f32i::bias - f16i::bias + 1) - shift_count;
    }

    // shift mantissa
    mantissa <<= f32i::mantissa_bits - f16i::mantissa_bits;

    const uint32_t y = sign | exponent | mantissa;
    return std::bit_cast<float>(y);
}
}
