// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef NUMBER_20250702_HPP
#define NUMBER_20250702_HPP

#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <cstddef>
#include <cstdint>

namespace tinytc {

enum class vector_size { v1 = 1, v2 = 2, v3 = 3, v4 = 4, v8 = 8, v16 = 16 };

auto acc_type(tinytc_type_t ty) -> tinytc_type_t;
auto component_count(tinytc_type_t ty) -> vector_size;
auto component_type(tinytc_type_t ty) -> tinytc_type_t;
auto promotable(tinytc_type_t a_ty, tinytc_type_t b_ty) -> bool;
auto promote(tinytc_type_t a_ty, tinytc_type_t b_ty) -> tinytc_type_t;
auto promote_or_throw(tinytc_type_t a_ty, tinytc_type_t b_ty, location const &loc) -> tinytc_type_t;
auto is_cast_allowed(tinytc_type_t from_ty, tinytc_type_t to_ty) -> bool;
auto alignment(tinytc_type_t ty, vector_size count = vector_size::v1) -> std::int32_t;
auto size(tinytc_type_t ty) -> std::size_t;
auto bit_width(tinytc_type_t ty) -> std::size_t;

} // namespace tinytc

#endif // NUMBER_20250702_HPP
