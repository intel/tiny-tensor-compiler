// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/ir/value.hpp"
#include "ir/node/value_node.hpp"
#include "tinytc/ir/data_type.hpp"

#include <memory>
#include <utility>

namespace tinytc {

value::value(data_type ty, std::string prefix)
    : value(std::make_shared<val>(std::move(ty), std::move(prefix))) {}

value::value(std::string prefix) : value(std::make_shared<val>(data_type{}, std::move(prefix))) {}

value::value(float imm, scalar_type ty) : value(std::make_shared<float_imm>(imm, ty)) {}
value::value(double imm, scalar_type ty) : value(std::make_shared<float_imm>(imm, ty)) {}
value::value(std::int8_t imm, scalar_type ty) : value(std::make_shared<int_imm>(imm, ty)) {}
value::value(std::int16_t imm, scalar_type ty) : value(std::make_shared<int_imm>(imm, ty)) {}
value::value(std::int32_t imm, scalar_type ty) : value(std::make_shared<int_imm>(imm, ty)) {}
value::value(std::int64_t imm, scalar_type ty) : value(std::make_shared<int_imm>(imm, ty)) {}
value::value(std::uint32_t imm, scalar_type ty) : value(std::make_shared<int_imm>(imm, ty)) {}

} // namespace tinytc