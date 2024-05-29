// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause
//
#ifndef SCALAR_TYPE_20240411_HPP
#define SCALAR_TYPE_20240411_HPP

#include "tinytc/types.hpp"

#include <clir/builtin_type.hpp>
#include <clir/data_type.hpp>

namespace tinytc {

bool is_floating_type(scalar_type ty);
bool is_complex_type(scalar_type ty);
scalar_type element_type(scalar_type ty);
clir::data_type to_clir_ty(scalar_type ty, clir::address_space as = clir::address_space::generic_t,
                           clir::type_qualifier q = clir::type_qualifier::none);
clir::data_type to_clir_ty(scalar_type ty, short size,
                           clir::address_space as = clir::address_space::generic_t,
                           clir::type_qualifier q = clir::type_qualifier::none);
clir::data_type to_clir_atomic_ty(scalar_type ty,
                                  clir::address_space as = clir::address_space::generic_t,
                                  clir::type_qualifier q = clir::type_qualifier::none);

} // namespace tinytc

#endif // SCALAR_TYPE_20240411_HPP
