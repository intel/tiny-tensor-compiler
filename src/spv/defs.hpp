// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

// This file is generated
// Do not edit manually

#ifndef GENERATED_DEFS_2024118_HPP
#define GENERATED_DEFS_2024118_HPP

#include "enums.hpp"
#include "support/ilist_base.hpp"

#include <cstdint>
#include <string>
#include <utility>
#include <variant>

namespace tinytc::spv {

class spv_inst : public ilist_node<spv_inst> {
  public:
    inline spv_inst(Op opcode, bool has_result_id)
        : opcode_{opcode}, has_result_id_{has_result_id} {}
    virtual ~spv_inst() = default;

    spv_inst(spv_inst const &other) = delete;
    spv_inst(spv_inst &&other) = delete;
    spv_inst &operator=(spv_inst const &other) = delete;
    spv_inst &operator=(spv_inst &&other) = delete;

    inline auto opcode() const -> Op { return opcode_; }
    inline auto has_result_id() const -> bool { return has_result_id_; }

  private:
    Op opcode_;
    bool has_result_id_;
};

using DecorationAttr = std::variant<BuiltIn, std::int32_t, std::pair<std::string, LinkageType>>;
using ExecutionModeAttr = std::variant<std::int32_t, std::array<std::int32_t, 3u>>;
using LiteralContextDependentNumber =
    std::variant<std::int8_t, std::int16_t, std::int32_t, std::int64_t, float, double>;
using LiteralString = std::string;
using LiteralInteger = std::int32_t;
using LiteralExtInstInteger = std::int32_t;
using IdResultType = spv_inst *;
using IdRef = spv_inst *;
using IdScope = spv_inst *;
using IdMemorySemantics = spv_inst *;
using MemoryAccessAttr = std::int32_t;
using PairIdRefIdRef = std::pair<spv_inst *, spv_inst *>;
using PairLiteralIntegerIdRef =
    std::pair<std::variant<std::int8_t, std::int16_t, std::int32_t, std::int64_t>, spv_inst *>;
using PairIdRefLiteralInteger = std::pair<spv_inst *, std::int32_t>;

} // namespace tinytc::spv

#endif // GENERATED_DEFS_2024118_HPP
