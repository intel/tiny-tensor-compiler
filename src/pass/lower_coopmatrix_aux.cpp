// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "pass/lower_coopmatrix_aux.hpp"
#include "tinytc/types.hpp"

#include <array>
#include <cstdint>
#include <utility>

namespace tinytc {

auto normalize_checked_flag(checked_flag checked, matrix_use use) -> checked_flag {
    if (use == matrix_use::b) {
        switch (checked) {
        case checked_flag::none:
            return checked_flag::none;
        case checked_flag::cols:
            return checked_flag::rows;
        case checked_flag::rows:
            return checked_flag::cols;
        case checked_flag::both:
            return checked_flag::both;
        }
    }
    return checked;
}
auto normalize_shape(std::array<std::int64_t, 2u> shape,
                     matrix_use use) -> std::array<std::int64_t, 2u> {
    if (use == matrix_use::b) {
        std::swap(shape[0], shape[1]);
    }
    return shape;
}
auto normalize_transpose(transpose trans, matrix_use use) -> transpose {
    if (use == matrix_use::b) {
        switch (trans) {
        case transpose::T:
            return transpose::N;
        case transpose::N:
            return transpose::T;
        }
    }
    return trans;
}

} // namespace tinytc
