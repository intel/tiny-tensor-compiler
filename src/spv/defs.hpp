// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

// This file is generated
// Do not edit manually

#ifndef GENERATED_DEFS_20241111_HPP
#define GENERATED_DEFS_20241111_HPP

#include "enums.hpp"
#include "support/ilist_base.hpp"

#include <cstdint>
#include <limits>
#include <string>
#include <utility>
#include <variant>

namespace tinytc::spv {

class spv_inst : public ilist_node<spv_inst> {
  public:
    inline spv_inst(Op opcode, bool has_result_id)
        : opcode_{opcode}, id_{has_result_id ? 0 : std::numeric_limits<std::uint32_t>::max()} {}
    virtual ~spv_inst() = default;

    spv_inst(spv_inst const &other) = delete;
    spv_inst(spv_inst &&other) = delete;
    spv_inst &operator=(spv_inst const &other) = delete;
    spv_inst &operator=(spv_inst &&other) = delete;

    inline auto opcode() const -> Op { return opcode_; }
    // SPIR-V requires 0 < id < Bound, therefore, we can reserve 0 for encoding "produces result; id
    // not yet assigned" and uint32_max for encoding "does not produce result"
    inline auto has_result_id() const -> bool {
        return id_ != std::numeric_limits<std::uint32_t>::max();
    }
    inline auto id() const -> std::uint32_t { return id_; }
    inline void id(std::uint32_t id) { id_ = id; }

  private:
    Op opcode_;
    std::uint32_t id_;
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

#endif // GENERATED_DEFS_20241111_HPP
