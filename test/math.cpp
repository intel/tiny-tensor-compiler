// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/core.h"

#include <doctest/doctest.h>

#include <bit>
#include <cstdint>

TEST_CASE("f16 -> f32") {
    // Regular numbers
    CHECK(tinytc_f16_as_ui16_to_f32(0x0000) == 0.0f);
    CHECK(tinytc_f16_as_ui16_to_f32(0x3c00) == std::bit_cast<float>(0x3f800000)); // 1.0f
    CHECK(tinytc_f16_as_ui16_to_f32(0x5148) == std::bit_cast<float>(0x42290000)); // 42.25f
    CHECK(tinytc_f16_as_ui16_to_f32(0xd148) == std::bit_cast<float>(0xc2290000)); // -42.25f
    CHECK(tinytc_f16_as_ui16_to_f32(0xfbff) == std::bit_cast<float>(0xc77fe000)); // -65504.0f

    // Subnormals
    CHECK(tinytc_f16_as_ui16_to_f32(0x0001) == std::bit_cast<float>(0x33800000)); // 2^-24
    CHECK(tinytc_f16_as_ui16_to_f32(0x03ff) ==
          std::bit_cast<float>(0x387fc000)); // 1.111111111 * 2^-15
    CHECK(tinytc_f16_as_ui16_to_f32(0x0021) ==
          std::bit_cast<float>(0x36040000)); // 1.966953277587890625e-6f);

    // Inf and NaN
    CHECK(tinytc_f16_as_ui16_to_f32(0x7c00) == std::bit_cast<float>(0x7f800000));         // inf
    CHECK(tinytc_f16_as_ui16_to_f32(0xfc00) == std::bit_cast<float>(0xff800000));         // -inf
    CHECK(std::bit_cast<std::uint32_t>(tinytc_f16_as_ui16_to_f32(0x7c01)) == 0x7f802000); // nan
    CHECK(std::bit_cast<std::uint32_t>(tinytc_f16_as_ui16_to_f32(0xfc01)) == 0xff802000); // -nan
}

TEST_CASE("f32 -> f16") {
    // Lossless conversion
    CHECK(tinytc_f32_to_f16_as_ui16(0.0f) == 0x0000);
    CHECK(tinytc_f32_to_f16_as_ui16(std::bit_cast<float>(0x3f800000)) == 0x3c00); // 1.0f
    CHECK(tinytc_f32_to_f16_as_ui16(std::bit_cast<float>(0x42290000)) == 0x5148); // 42.25f
    CHECK(tinytc_f32_to_f16_as_ui16(std::bit_cast<float>(0xc2290000)) == 0xd148); // -42.25f
    CHECK(tinytc_f32_to_f16_as_ui16(std::bit_cast<float>(0xc77fe000)) == 0xfbff); // -65504.0f

    // Big number -> inf
    CHECK(tinytc_f32_to_f16_as_ui16(std::bit_cast<float>(0x7c010840)) == 0x7c00); // inf
    CHECK(tinytc_f32_to_f16_as_ui16(std::bit_cast<float>(0xfc010840)) == 0xfc00); // -inf

    // Round to nearest even
    CHECK(tinytc_f32_to_f16_as_ui16(std::bit_cast<float>(0x41fa0000)) == 0x4fd0); // round down
    CHECK(tinytc_f32_to_f16_as_ui16(std::bit_cast<float>(0x41fa1fff)) == 0x4fd1); // round up
    CHECK(tinytc_f32_to_f16_as_ui16(std::bit_cast<float>(0x41fa0fff)) == 0x4fd0); // round down
    CHECK(tinytc_f32_to_f16_as_ui16(std::bit_cast<float>(0x41fa1001)) == 0x4fd1); // round up
    CHECK(tinytc_f32_to_f16_as_ui16(std::bit_cast<float>(0x41fa1000)) == 0x4fd0); // tie
    CHECK(tinytc_f32_to_f16_as_ui16(std::bit_cast<float>(0x41fa3000)) == 0x4fd2); // tie
    CHECK(tinytc_f32_to_f16_as_ui16(std::bit_cast<float>(0x46ffffff)) ==
          0x7800); // 32767.998 -> 2^15
    CHECK(tinytc_f32_to_f16_as_ui16(std::bit_cast<float>(0x477fffff)) ==
          0x7c00); // 65535.996 -> inf

    // Subnormals
    CHECK(tinytc_f32_to_f16_as_ui16(std::bit_cast<float>(0x33800000)) == 0x0001); // 2^-24
    CHECK(tinytc_f32_to_f16_as_ui16(std::bit_cast<float>(0x387fc000)) ==
          0x03ff); // 1.111111111 * 2^-15
    CHECK(tinytc_f32_to_f16_as_ui16(std::bit_cast<float>(0x36040000)) ==
          0x0021); // 1.966953277587890625e-6f);
    CHECK(tinytc_f32_to_f16_as_ui16(std::bit_cast<float>(0x3607ffff)) ==
          0x0022); // 1.966953277587890625e-6f);

    // Inf and NaN
    CHECK(tinytc_f32_to_f16_as_ui16(std::bit_cast<float>(0x7f800000)) == 0x7c00); // inf
    CHECK(tinytc_f32_to_f16_as_ui16(std::bit_cast<float>(0xff800000)) == 0xfc00); // -inf
    CHECK(tinytc_f32_to_f16_as_ui16(std::bit_cast<float>(0x7f802000)) == 0x7c01); // nan
    CHECK(tinytc_f32_to_f16_as_ui16(std::bit_cast<float>(0xff802000)) == 0xfc01); // -nan
}

TEST_CASE("bf16 -> f32") {
    // Regular numbers
    CHECK(tinytc_bf16_as_ui16_to_f32(0x0000) == 0.0f);
    CHECK(tinytc_bf16_as_ui16_to_f32(0x3f80) == std::bit_cast<float>(0x3f800000)); // 1.0f
    CHECK(tinytc_bf16_as_ui16_to_f32(0x4229) == std::bit_cast<float>(0x42290000)); // 42.25f
    CHECK(tinytc_bf16_as_ui16_to_f32(0xc229) == std::bit_cast<float>(0xc2290000)); // -42.25f
    CHECK(tinytc_bf16_as_ui16_to_f32(0xc77f) == std::bit_cast<float>(0xc77f0000)); // -65280.0f

    // Subnormals
    CHECK(tinytc_bf16_as_ui16_to_f32(0x0001) == std::bit_cast<float>(0x00010000)); // 2^-133
    CHECK(tinytc_bf16_as_ui16_to_f32(0x03ff) == std::bit_cast<float>(0x03ff0000));
    CHECK(tinytc_bf16_as_ui16_to_f32(0x0021) == std::bit_cast<float>(0x00210000));

    // Inf and NaN
    CHECK(tinytc_bf16_as_ui16_to_f32(0x7f80) == std::bit_cast<float>(0x7f800000));         // inf
    CHECK(tinytc_bf16_as_ui16_to_f32(0xff80) == std::bit_cast<float>(0xff800000));         // -inf
    CHECK(std::bit_cast<std::uint32_t>(tinytc_bf16_as_ui16_to_f32(0x7f81)) == 0x7f810000); // nan
    CHECK(std::bit_cast<std::uint32_t>(tinytc_bf16_as_ui16_to_f32(0xff81)) == 0xff810000); // -nan
}

TEST_CASE("f32 -> bf16") {
    // Lossless conversion
    CHECK(tinytc_f32_to_bf16_as_ui16(0.0f) == 0x0000);
    CHECK(tinytc_f32_to_bf16_as_ui16(std::bit_cast<float>(0x3f800000)) == 0x3f80); // 1.0f
    CHECK(tinytc_f32_to_bf16_as_ui16(std::bit_cast<float>(0x42290000)) == 0x4229); // 42.25f
    CHECK(tinytc_f32_to_bf16_as_ui16(std::bit_cast<float>(0xc2290000)) == 0xc229); // -42.25f
    CHECK(tinytc_f32_to_bf16_as_ui16(std::bit_cast<float>(0xc77f0000)) == 0xc77f); // -65280.0f

    // Round to nearest even
    CHECK(tinytc_f32_to_bf16_as_ui16(std::bit_cast<float>(0x41fa0000)) == 0x41fa); // round down
    CHECK(tinytc_f32_to_bf16_as_ui16(std::bit_cast<float>(0x41faffff)) == 0x41fb); // round up
    CHECK(tinytc_f32_to_bf16_as_ui16(std::bit_cast<float>(0x41fa0fff)) == 0x41fa); // round down
    CHECK(tinytc_f32_to_bf16_as_ui16(std::bit_cast<float>(0x41fa8001)) == 0x41fb); // round up
    CHECK(tinytc_f32_to_bf16_as_ui16(std::bit_cast<float>(0x41fa8000)) == 0x41fa); // tie
    CHECK(tinytc_f32_to_bf16_as_ui16(std::bit_cast<float>(0x41fb8000)) == 0x41fc); // tie
    CHECK(tinytc_f32_to_bf16_as_ui16(std::bit_cast<float>(0x46ffffff)) ==
          0x4700); // 32767.998 -> 2^15
    CHECK(tinytc_f32_to_bf16_as_ui16(std::bit_cast<float>(0x7f7fffff)) ==
          0x7f80); // 65535.996 -> inf

    // Subnormals
    CHECK(tinytc_f32_to_bf16_as_ui16(std::bit_cast<float>(0x00010000)) == 0x0001); // 2^-24

    // Inf and NaN
    CHECK(tinytc_f32_to_bf16_as_ui16(std::bit_cast<float>(0x7f800000)) == 0x7f80); // inf
    CHECK(tinytc_f32_to_bf16_as_ui16(std::bit_cast<float>(0xff800000)) == 0xff80); // -inf
    CHECK(tinytc_f32_to_bf16_as_ui16(std::bit_cast<float>(0x7f802000)) == 0x7f81); // nan
    CHECK(tinytc_f32_to_bf16_as_ui16(std::bit_cast<float>(0xff802000)) == 0xff81); // -nan
}
