// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/tinytc.hpp"

#include <bit>

using namespace tinytc;

extern "C" {

uint16_t tinytc_f32_to_f16_as_ui16(float x) {
    return ieee754_truncate<f16_format, f32_format>(std::bit_cast<uint32_t>(x));
}

float tinytc_f16_as_ui16_to_f32(uint16_t x) {
    const auto y = ieee754_extend<f32_format, f16_format>(x);
    return std::bit_cast<float>(y);
}

uint16_t tinytc_f32_to_bf16_as_ui16(float x) {
    return ieee754_truncate<bf16_format, f32_format>(std::bit_cast<uint32_t>(x));
}

float tinytc_bf16_as_ui16_to_f32(uint16_t x) {
    const auto y = ieee754_extend<f32_format, bf16_format>(x);
    return std::bit_cast<float>(y);
}
}
