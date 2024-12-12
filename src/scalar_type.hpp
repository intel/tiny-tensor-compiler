// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause
//
#ifndef SCALAR_TYPE_20240411_HPP
#define SCALAR_TYPE_20240411_HPP

#include "tinytc/types.hpp"

#include <cstdint>
#include <optional>

namespace tinytc {

using host_index_type = std::int64_t;

enum class vector_size { v1 = 1, v2 = 2, v3 = 3, v4 = 4, v8 = 8, v16 = 16 };

bool is_floating_type(scalar_type ty);
bool is_complex_type(scalar_type ty);
bool is_integer_type(scalar_type ty);
auto acc_type(scalar_type ty) -> scalar_type;
auto component_count(scalar_type ty) -> vector_size;
auto component_type(scalar_type ty) -> scalar_type;
auto promotable(scalar_type a_ty, scalar_type b_ty) -> bool;
auto promote(scalar_type a_ty, scalar_type b_ty) -> std::optional<scalar_type>;
auto promote_or_throw(scalar_type a_ty, scalar_type b_ty, location const &loc) -> scalar_type;
auto is_cast_allowed(scalar_type from_ty, scalar_type to_ty) -> bool;
auto alignment(scalar_type ty, vector_size count = vector_size::v1) -> std::int32_t;

} // namespace tinytc

#endif // SCALAR_TYPE_20240411_HPP
