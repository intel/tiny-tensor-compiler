// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef VAR_20230309_HPP
#define VAR_20230309_HPP

#include "tinytc/export.h"
#include "tinytc/ir/scalar_type.hpp"

#include <clir/handle.hpp>

#include <cstdint>
#include <memory>
#include <string>

namespace tinytc {

class value_node;
class data_type;

/**
 * Reference-counted value handle
 *
 * A value may be immediate or may reference a run-time value
 * (that is, result of an instruction or function argument, everything that starts with "%").
 * The type is always stored along the value.
 */
class TINYTC_EXPORT value : public clir::handle<value_node> {
  public:
    using clir::handle<value_node>::handle;
    //! Create value with data type ty
    value(data_type ty, std::string prefix = "");
    //! Create empty (invalid) value
    value(std::string prefix = "");
    //! Create immediate value from float
    value(float imm, scalar_type ty = scalar_type::f32);
    //! Create immediate value from double
    value(double imm, scalar_type ty = scalar_type::f64);
    //! Create immediate value from int8_t
    value(std::int8_t imm, scalar_type ty = scalar_type::i8);
    //! Create immediate value from int16_t
    value(std::int16_t imm, scalar_type ty = scalar_type::i16);
    //! Create immediate value from int32_t
    value(std::int32_t imm, scalar_type ty = scalar_type::i32);
    //! Create immediate value from int64_t
    value(std::int64_t imm, scalar_type ty = scalar_type::i64);
    //! Create immediate value from uint32_t (index type)
    value(std::uint32_t imm, scalar_type ty = scalar_type::index);
};

} // namespace tinytc

#endif // VAR_20230309_HPP
