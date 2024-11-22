// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause
//
#ifndef SCALAR_TYPE_20240411_HPP
#define SCALAR_TYPE_20240411_HPP

#include "tinytc/types.hpp"

#include <cstdint>

namespace tinytc {

using host_index_type = std::int64_t;

enum class component_count { v1 = 1, v2 = 2, v3 = 3, v4 = 4, v8 = 8, v16 = 16 };

bool is_floating_type(scalar_type ty);
bool is_complex_type(scalar_type ty);
bool is_integer_type(scalar_type ty);
scalar_type element_type(scalar_type ty);
scalar_type compatible_type(scalar_type a_ty, scalar_type b_ty);
std::int32_t alignment(scalar_type ty, component_count count = component_count::v1);

} // namespace tinytc

#endif // SCALAR_TYPE_20240411_HPP
