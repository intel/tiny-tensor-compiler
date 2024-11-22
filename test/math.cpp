// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/tinytc.h"

#include <doctest/doctest.h>

#include <cstdint>

TEST_CASE("fp32 -> fp16 -> fp32 round trip") {
    CHECK(tinytc_f32_to_f16(1.0f) == 1.0f);
    CHECK(tinytc_f16_to_f32(tinytc_f32_to_f16(1.0f)) == 1.0f);
    CHECK(tinytc_f16_to_f32(tinytc_f32_to_f16(5.0f)) == 5.0f);
}
