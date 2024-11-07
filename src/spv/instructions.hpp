// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

// This file is generated
// Do not edit manually

#ifndef GENERATED_INSTRUCTIONS_2024117_HPP
#define GENERATED_INSTRUCTIONS_2024117_HPP

#include "enums.hpp"
#include "error.hpp"
#include "support/ilist_base.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

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

using DecorationAttr = std::variant<BuiltIn, std::pair<std::string, LinkageType>>;
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

class OpNop : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::Nop; }
    OpNop() : spv_inst{Op::Nop, false} {}

  private:
};
class OpUndef : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::Undef; }
    OpUndef(IdResultType type) : spv_inst{Op::Undef, true}, type_(std::move(type)) {}
    inline auto type() const -> IdResultType const & { return type_; }

  private:
    IdResultType type_;
};
class OpSourceContinued : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::SourceContinued; }
    OpSourceContinued(LiteralString op0)
        : spv_inst{Op::SourceContinued, false}, op0_(std::move(op0)) {}
    inline auto op0() const -> LiteralString const & { return op0_; }

  private:
    LiteralString op0_;
};
class OpSource : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::Source; }
    OpSource(SourceLanguage op0, LiteralInteger op1, std::optional<IdRef> op2 = std::nullopt,
             std::optional<LiteralString> op3 = std::nullopt)
        : spv_inst{Op::Source, false}, op0_(std::move(op0)), op1_(std::move(op1)),
          op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto op0() const -> SourceLanguage const & { return op0_; }
    inline auto op1() const -> LiteralInteger const & { return op1_; }
    inline auto op2() const -> std::optional<IdRef> const & { return op2_; }
    inline auto op3() const -> std::optional<LiteralString> const & { return op3_; }

  private:
    SourceLanguage op0_;
    LiteralInteger op1_;
    std::optional<IdRef> op2_;
    std::optional<LiteralString> op3_;
};
class OpSourceExtension : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::SourceExtension; }
    OpSourceExtension(LiteralString op0)
        : spv_inst{Op::SourceExtension, false}, op0_(std::move(op0)) {}
    inline auto op0() const -> LiteralString const & { return op0_; }

  private:
    LiteralString op0_;
};
class OpName : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::Name; }
    OpName(IdRef op0, LiteralString op1)
        : spv_inst{Op::Name, false}, op0_(std::move(op0)), op1_(std::move(op1)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> LiteralString const & { return op1_; }

  private:
    IdRef op0_;
    LiteralString op1_;
};
class OpMemberName : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::MemberName; }
    OpMemberName(IdRef op0, LiteralInteger op1, LiteralString op2)
        : spv_inst{Op::MemberName, false}, op0_(std::move(op0)), op1_(std::move(op1)),
          op2_(std::move(op2)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> LiteralInteger const & { return op1_; }
    inline auto op2() const -> LiteralString const & { return op2_; }

  private:
    IdRef op0_;
    LiteralInteger op1_;
    LiteralString op2_;
};
class OpString : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::String; }
    OpString(LiteralString op0) : spv_inst{Op::String, true}, op0_(std::move(op0)) {}
    inline auto op0() const -> LiteralString const & { return op0_; }

  private:
    LiteralString op0_;
};
class OpLine : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::Line; }
    OpLine(IdRef op0, LiteralInteger op1, LiteralInteger op2)
        : spv_inst{Op::Line, false}, op0_(std::move(op0)), op1_(std::move(op1)),
          op2_(std::move(op2)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> LiteralInteger const & { return op1_; }
    inline auto op2() const -> LiteralInteger const & { return op2_; }

  private:
    IdRef op0_;
    LiteralInteger op1_;
    LiteralInteger op2_;
};
class OpExtension : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::Extension; }
    OpExtension(LiteralString op0) : spv_inst{Op::Extension, false}, op0_(std::move(op0)) {}
    inline auto op0() const -> LiteralString const & { return op0_; }

  private:
    LiteralString op0_;
};
class OpExtInstImport : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ExtInstImport; }
    OpExtInstImport(LiteralString op0) : spv_inst{Op::ExtInstImport, true}, op0_(std::move(op0)) {}
    inline auto op0() const -> LiteralString const & { return op0_; }

  private:
    LiteralString op0_;
};
class OpExtInst : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ExtInst; }
    OpExtInst(IdResultType type, IdRef op0, LiteralExtInstInteger op1, std::vector<IdRef> op2)
        : spv_inst{Op::ExtInst, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> LiteralExtInstInteger const & { return op1_; }
    inline auto op2() const -> std::vector<IdRef> const & { return op2_; }

  private:
    IdResultType type_;
    IdRef op0_;
    LiteralExtInstInteger op1_;
    std::vector<IdRef> op2_;
};
class OpMemoryModel : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::MemoryModel; }
    OpMemoryModel(AddressingModel op0, MemoryModel op1)
        : spv_inst{Op::MemoryModel, false}, op0_(std::move(op0)), op1_(std::move(op1)) {}
    inline auto op0() const -> AddressingModel const & { return op0_; }
    inline auto op1() const -> MemoryModel const & { return op1_; }

  private:
    AddressingModel op0_;
    MemoryModel op1_;
};
class OpEntryPoint : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::EntryPoint; }
    OpEntryPoint(ExecutionModel op0, IdRef op1, LiteralString op2, std::vector<IdRef> op3)
        : spv_inst{Op::EntryPoint, false}, op0_(std::move(op0)), op1_(std::move(op1)),
          op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto op0() const -> ExecutionModel const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> LiteralString const & { return op2_; }
    inline auto op3() const -> std::vector<IdRef> const & { return op3_; }

  private:
    ExecutionModel op0_;
    IdRef op1_;
    LiteralString op2_;
    std::vector<IdRef> op3_;
};
class OpExecutionMode : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ExecutionMode; }
    OpExecutionMode(IdRef op0, ExecutionMode op1, ExecutionModeAttr op2)
        : spv_inst{Op::ExecutionMode, false}, op0_(std::move(op0)), op1_(std::move(op1)),
          op2_(std::move(op2)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> ExecutionMode const & { return op1_; }
    inline auto op2() const -> ExecutionModeAttr const & { return op2_; }

  private:
    IdRef op0_;
    ExecutionMode op1_;
    ExecutionModeAttr op2_;
};
class OpCapability : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::Capability; }
    OpCapability(Capability op0) : spv_inst{Op::Capability, false}, op0_(std::move(op0)) {}
    inline auto op0() const -> Capability const & { return op0_; }

  private:
    Capability op0_;
};
class OpTypeVoid : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::TypeVoid; }
    OpTypeVoid() : spv_inst{Op::TypeVoid, true} {}

  private:
};
class OpTypeBool : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::TypeBool; }
    OpTypeBool() : spv_inst{Op::TypeBool, true} {}

  private:
};
class OpTypeInt : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::TypeInt; }
    OpTypeInt(LiteralInteger op0, LiteralInteger op1)
        : spv_inst{Op::TypeInt, true}, op0_(std::move(op0)), op1_(std::move(op1)) {}
    inline auto op0() const -> LiteralInteger const & { return op0_; }
    inline auto op1() const -> LiteralInteger const & { return op1_; }

  private:
    LiteralInteger op0_;
    LiteralInteger op1_;
};
class OpTypeFloat : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::TypeFloat; }
    OpTypeFloat(LiteralInteger op0, std::optional<FPEncoding> op1 = std::nullopt)
        : spv_inst{Op::TypeFloat, true}, op0_(std::move(op0)), op1_(std::move(op1)) {}
    inline auto op0() const -> LiteralInteger const & { return op0_; }
    inline auto op1() const -> std::optional<FPEncoding> const & { return op1_; }

  private:
    LiteralInteger op0_;
    std::optional<FPEncoding> op1_;
};
class OpTypeVector : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::TypeVector; }
    OpTypeVector(IdRef op0, LiteralInteger op1)
        : spv_inst{Op::TypeVector, true}, op0_(std::move(op0)), op1_(std::move(op1)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> LiteralInteger const & { return op1_; }

  private:
    IdRef op0_;
    LiteralInteger op1_;
};
class OpTypeMatrix : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::TypeMatrix; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Matrix};
    OpTypeMatrix(IdRef op0, LiteralInteger op1)
        : spv_inst{Op::TypeMatrix, true}, op0_(std::move(op0)), op1_(std::move(op1)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> LiteralInteger const & { return op1_; }

  private:
    IdRef op0_;
    LiteralInteger op1_;
};
class OpTypeImage : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::TypeImage; }
    OpTypeImage(IdRef op0, Dim op1, LiteralInteger op2, LiteralInteger op3, LiteralInteger op4,
                LiteralInteger op5, ImageFormat op6,
                std::optional<AccessQualifier> op7 = std::nullopt)
        : spv_inst{Op::TypeImage, true}, op0_(std::move(op0)), op1_(std::move(op1)),
          op2_(std::move(op2)), op3_(std::move(op3)), op4_(std::move(op4)), op5_(std::move(op5)),
          op6_(std::move(op6)), op7_(std::move(op7)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> Dim const & { return op1_; }
    inline auto op2() const -> LiteralInteger const & { return op2_; }
    inline auto op3() const -> LiteralInteger const & { return op3_; }
    inline auto op4() const -> LiteralInteger const & { return op4_; }
    inline auto op5() const -> LiteralInteger const & { return op5_; }
    inline auto op6() const -> ImageFormat const & { return op6_; }
    inline auto op7() const -> std::optional<AccessQualifier> const & { return op7_; }

  private:
    IdRef op0_;
    Dim op1_;
    LiteralInteger op2_;
    LiteralInteger op3_;
    LiteralInteger op4_;
    LiteralInteger op5_;
    ImageFormat op6_;
    std::optional<AccessQualifier> op7_;
};
class OpTypeSampler : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::TypeSampler; }
    OpTypeSampler() : spv_inst{Op::TypeSampler, true} {}

  private:
};
class OpTypeSampledImage : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::TypeSampledImage; }
    OpTypeSampledImage(IdRef op0) : spv_inst{Op::TypeSampledImage, true}, op0_(std::move(op0)) {}
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdRef op0_;
};
class OpTypeArray : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::TypeArray; }
    OpTypeArray(IdRef op0, IdRef op1)
        : spv_inst{Op::TypeArray, true}, op0_(std::move(op0)), op1_(std::move(op1)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdRef op0_;
    IdRef op1_;
};
class OpTypeRuntimeArray : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::TypeRuntimeArray; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Shader};
    OpTypeRuntimeArray(IdRef op0) : spv_inst{Op::TypeRuntimeArray, true}, op0_(std::move(op0)) {}
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdRef op0_;
};
class OpTypeStruct : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::TypeStruct; }
    OpTypeStruct(std::vector<IdRef> op0) : spv_inst{Op::TypeStruct, true}, op0_(std::move(op0)) {}
    inline auto op0() const -> std::vector<IdRef> const & { return op0_; }

  private:
    std::vector<IdRef> op0_;
};
class OpTypeOpaque : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::TypeOpaque; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Kernel};
    OpTypeOpaque(LiteralString op0) : spv_inst{Op::TypeOpaque, true}, op0_(std::move(op0)) {}
    inline auto op0() const -> LiteralString const & { return op0_; }

  private:
    LiteralString op0_;
};
class OpTypePointer : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::TypePointer; }
    OpTypePointer(StorageClass op0, IdRef op1)
        : spv_inst{Op::TypePointer, true}, op0_(std::move(op0)), op1_(std::move(op1)) {}
    inline auto op0() const -> StorageClass const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    StorageClass op0_;
    IdRef op1_;
};
class OpTypeFunction : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::TypeFunction; }
    OpTypeFunction(IdRef op0, std::vector<IdRef> op1)
        : spv_inst{Op::TypeFunction, true}, op0_(std::move(op0)), op1_(std::move(op1)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> std::vector<IdRef> const & { return op1_; }

  private:
    IdRef op0_;
    std::vector<IdRef> op1_;
};
class OpTypeEvent : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::TypeEvent; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Kernel};
    OpTypeEvent() : spv_inst{Op::TypeEvent, true} {}

  private:
};
class OpTypeDeviceEvent : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::TypeDeviceEvent; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::DeviceEnqueue};
    OpTypeDeviceEvent() : spv_inst{Op::TypeDeviceEvent, true} {}

  private:
};
class OpTypeReserveId : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::TypeReserveId; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Pipes};
    OpTypeReserveId() : spv_inst{Op::TypeReserveId, true} {}

  private:
};
class OpTypeQueue : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::TypeQueue; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::DeviceEnqueue};
    OpTypeQueue() : spv_inst{Op::TypeQueue, true} {}

  private:
};
class OpTypePipe : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::TypePipe; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Pipes};
    OpTypePipe(AccessQualifier op0) : spv_inst{Op::TypePipe, true}, op0_(std::move(op0)) {}
    inline auto op0() const -> AccessQualifier const & { return op0_; }

  private:
    AccessQualifier op0_;
};
class OpTypeForwardPointer : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::TypeForwardPointer; }
    constexpr static std::array<Capability, 2> required_capabilities = {
        Capability::Addresses, Capability::PhysicalStorageBufferAddresses};
    OpTypeForwardPointer(IdRef op0, StorageClass op1)
        : spv_inst{Op::TypeForwardPointer, false}, op0_(std::move(op0)), op1_(std::move(op1)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> StorageClass const & { return op1_; }

  private:
    IdRef op0_;
    StorageClass op1_;
};
class OpConstantTrue : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ConstantTrue; }
    OpConstantTrue(IdResultType type) : spv_inst{Op::ConstantTrue, true}, type_(std::move(type)) {}
    inline auto type() const -> IdResultType const & { return type_; }

  private:
    IdResultType type_;
};
class OpConstantFalse : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ConstantFalse; }
    OpConstantFalse(IdResultType type)
        : spv_inst{Op::ConstantFalse, true}, type_(std::move(type)) {}
    inline auto type() const -> IdResultType const & { return type_; }

  private:
    IdResultType type_;
};
class OpConstant : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::Constant; }
    OpConstant(IdResultType type, LiteralContextDependentNumber op0)
        : spv_inst{Op::Constant, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> LiteralContextDependentNumber const & { return op0_; }

  private:
    IdResultType type_;
    LiteralContextDependentNumber op0_;
};
class OpConstantComposite : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ConstantComposite; }
    OpConstantComposite(IdResultType type, std::vector<IdRef> op0)
        : spv_inst{Op::ConstantComposite, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> std::vector<IdRef> const & { return op0_; }

  private:
    IdResultType type_;
    std::vector<IdRef> op0_;
};
class OpConstantSampler : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ConstantSampler; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::LiteralSampler};
    OpConstantSampler(IdResultType type, SamplerAddressingMode op0, LiteralInteger op1,
                      SamplerFilterMode op2)
        : spv_inst{Op::ConstantSampler, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> SamplerAddressingMode const & { return op0_; }
    inline auto op1() const -> LiteralInteger const & { return op1_; }
    inline auto op2() const -> SamplerFilterMode const & { return op2_; }

  private:
    IdResultType type_;
    SamplerAddressingMode op0_;
    LiteralInteger op1_;
    SamplerFilterMode op2_;
};
class OpConstantNull : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ConstantNull; }
    OpConstantNull(IdResultType type) : spv_inst{Op::ConstantNull, true}, type_(std::move(type)) {}
    inline auto type() const -> IdResultType const & { return type_; }

  private:
    IdResultType type_;
};
class OpFunction : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::Function; }
    OpFunction(IdResultType type, FunctionControl op0, IdRef op1)
        : spv_inst{Op::Function, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> FunctionControl const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    FunctionControl op0_;
    IdRef op1_;
};
class OpFunctionParameter : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::FunctionParameter; }
    OpFunctionParameter(IdResultType type)
        : spv_inst{Op::FunctionParameter, true}, type_(std::move(type)) {}
    inline auto type() const -> IdResultType const & { return type_; }

  private:
    IdResultType type_;
};
class OpFunctionEnd : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::FunctionEnd; }
    OpFunctionEnd() : spv_inst{Op::FunctionEnd, false} {}

  private:
};
class OpFunctionCall : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::FunctionCall; }
    OpFunctionCall(IdResultType type, IdRef op0, std::vector<IdRef> op1)
        : spv_inst{Op::FunctionCall, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> std::vector<IdRef> const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    std::vector<IdRef> op1_;
};
class OpVariable : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::Variable; }
    OpVariable(IdResultType type, StorageClass op0, std::optional<IdRef> op1 = std::nullopt)
        : spv_inst{Op::Variable, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> StorageClass const & { return op0_; }
    inline auto op1() const -> std::optional<IdRef> const & { return op1_; }

  private:
    IdResultType type_;
    StorageClass op0_;
    std::optional<IdRef> op1_;
};
class OpImageTexelPointer : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ImageTexelPointer; }
    OpImageTexelPointer(IdResultType type, IdRef op0, IdRef op1, IdRef op2)
        : spv_inst{Op::ImageTexelPointer, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
};
class OpLoad : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::Load; }
    OpLoad(IdResultType type, IdRef op0, std::optional<MemoryAccess> op1 = std::nullopt,
           std::optional<MemoryAccessAttr> op2 = std::nullopt)
        : spv_inst{Op::Load, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> std::optional<MemoryAccess> const & { return op1_; }
    inline auto op2() const -> std::optional<MemoryAccessAttr> const & { return op2_; }

  private:
    IdResultType type_;
    IdRef op0_;
    std::optional<MemoryAccess> op1_;
    std::optional<MemoryAccessAttr> op2_;
};
class OpStore : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::Store; }
    OpStore(IdRef op0, IdRef op1, std::optional<MemoryAccess> op2 = std::nullopt,
            std::optional<MemoryAccessAttr> op3 = std::nullopt)
        : spv_inst{Op::Store, false}, op0_(std::move(op0)), op1_(std::move(op1)),
          op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> std::optional<MemoryAccess> const & { return op2_; }
    inline auto op3() const -> std::optional<MemoryAccessAttr> const & { return op3_; }

  private:
    IdRef op0_;
    IdRef op1_;
    std::optional<MemoryAccess> op2_;
    std::optional<MemoryAccessAttr> op3_;
};
class OpCopyMemory : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::CopyMemory; }
    OpCopyMemory(IdRef op0, IdRef op1, std::optional<MemoryAccess> op2 = std::nullopt,
                 std::optional<MemoryAccess> op3 = std::nullopt,
                 std::optional<MemoryAccessAttr> op4 = std::nullopt)
        : spv_inst{Op::CopyMemory, false}, op0_(std::move(op0)), op1_(std::move(op1)),
          op2_(std::move(op2)), op3_(std::move(op3)), op4_(std::move(op4)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> std::optional<MemoryAccess> const & { return op2_; }
    inline auto op3() const -> std::optional<MemoryAccess> const & { return op3_; }
    inline auto op4() const -> std::optional<MemoryAccessAttr> const & { return op4_; }

  private:
    IdRef op0_;
    IdRef op1_;
    std::optional<MemoryAccess> op2_;
    std::optional<MemoryAccess> op3_;
    std::optional<MemoryAccessAttr> op4_;
};
class OpCopyMemorySized : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::CopyMemorySized; }
    constexpr static std::array<Capability, 2> required_capabilities = {
        Capability::Addresses, Capability::UntypedPointersKHR};
    OpCopyMemorySized(IdRef op0, IdRef op1, IdRef op2,
                      std::optional<MemoryAccess> op3 = std::nullopt,
                      std::optional<MemoryAccess> op4 = std::nullopt,
                      std::optional<MemoryAccessAttr> op5 = std::nullopt)
        : spv_inst{Op::CopyMemorySized, false}, op0_(std::move(op0)), op1_(std::move(op1)),
          op2_(std::move(op2)), op3_(std::move(op3)), op4_(std::move(op4)), op5_(std::move(op5)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> std::optional<MemoryAccess> const & { return op3_; }
    inline auto op4() const -> std::optional<MemoryAccess> const & { return op4_; }
    inline auto op5() const -> std::optional<MemoryAccessAttr> const & { return op5_; }

  private:
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    std::optional<MemoryAccess> op3_;
    std::optional<MemoryAccess> op4_;
    std::optional<MemoryAccessAttr> op5_;
};
class OpAccessChain : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::AccessChain; }
    OpAccessChain(IdResultType type, IdRef op0, std::vector<IdRef> op1)
        : spv_inst{Op::AccessChain, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> std::vector<IdRef> const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    std::vector<IdRef> op1_;
};
class OpInBoundsAccessChain : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::InBoundsAccessChain; }
    OpInBoundsAccessChain(IdResultType type, IdRef op0, std::vector<IdRef> op1)
        : spv_inst{Op::InBoundsAccessChain, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> std::vector<IdRef> const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    std::vector<IdRef> op1_;
};
class OpPtrAccessChain : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::PtrAccessChain; }
    constexpr static std::array<Capability, 4> required_capabilities = {
        Capability::Addresses, Capability::VariablePointers,
        Capability::VariablePointersStorageBuffer, Capability::PhysicalStorageBufferAddresses};
    OpPtrAccessChain(IdResultType type, IdRef op0, IdRef op1, std::vector<IdRef> op2)
        : spv_inst{Op::PtrAccessChain, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> std::vector<IdRef> const & { return op2_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    std::vector<IdRef> op2_;
};
class OpArrayLength : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ArrayLength; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Shader};
    OpArrayLength(IdResultType type, IdRef op0, LiteralInteger op1)
        : spv_inst{Op::ArrayLength, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> LiteralInteger const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    LiteralInteger op1_;
};
class OpGenericPtrMemSemantics : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::GenericPtrMemSemantics;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Kernel};
    OpGenericPtrMemSemantics(IdResultType type, IdRef op0)
        : spv_inst{Op::GenericPtrMemSemantics, true}, type_(std::move(type)), op0_(std::move(op0)) {
    }
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpInBoundsPtrAccessChain : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::InBoundsPtrAccessChain;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Addresses};
    OpInBoundsPtrAccessChain(IdResultType type, IdRef op0, IdRef op1, std::vector<IdRef> op2)
        : spv_inst{Op::InBoundsPtrAccessChain, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> std::vector<IdRef> const & { return op2_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    std::vector<IdRef> op2_;
};
class OpDecorate : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::Decorate; }
    OpDecorate(IdRef op0, Decoration op1, std::optional<DecorationAttr> op2 = std::nullopt)
        : spv_inst{Op::Decorate, false}, op0_(std::move(op0)), op1_(std::move(op1)),
          op2_(std::move(op2)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> Decoration const & { return op1_; }
    inline auto op2() const -> std::optional<DecorationAttr> const & { return op2_; }

  private:
    IdRef op0_;
    Decoration op1_;
    std::optional<DecorationAttr> op2_;
};
class OpMemberDecorate : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::MemberDecorate; }
    OpMemberDecorate(IdRef op0, LiteralInteger op1, Decoration op2)
        : spv_inst{Op::MemberDecorate, false}, op0_(std::move(op0)), op1_(std::move(op1)),
          op2_(std::move(op2)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> LiteralInteger const & { return op1_; }
    inline auto op2() const -> Decoration const & { return op2_; }

  private:
    IdRef op0_;
    LiteralInteger op1_;
    Decoration op2_;
};
class OpDecorationGroup : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::DecorationGroup; }
    OpDecorationGroup() : spv_inst{Op::DecorationGroup, true} {}

  private:
};
class OpGroupDecorate : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GroupDecorate; }
    OpGroupDecorate(IdRef op0, std::vector<IdRef> op1)
        : spv_inst{Op::GroupDecorate, false}, op0_(std::move(op0)), op1_(std::move(op1)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> std::vector<IdRef> const & { return op1_; }

  private:
    IdRef op0_;
    std::vector<IdRef> op1_;
};
class OpGroupMemberDecorate : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GroupMemberDecorate; }
    OpGroupMemberDecorate(IdRef op0, std::vector<PairIdRefLiteralInteger> op1)
        : spv_inst{Op::GroupMemberDecorate, false}, op0_(std::move(op0)), op1_(std::move(op1)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> std::vector<PairIdRefLiteralInteger> const & { return op1_; }

  private:
    IdRef op0_;
    std::vector<PairIdRefLiteralInteger> op1_;
};
class OpVectorExtractDynamic : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::VectorExtractDynamic; }
    OpVectorExtractDynamic(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::VectorExtractDynamic, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpVectorInsertDynamic : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::VectorInsertDynamic; }
    OpVectorInsertDynamic(IdResultType type, IdRef op0, IdRef op1, IdRef op2)
        : spv_inst{Op::VectorInsertDynamic, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
};
class OpVectorShuffle : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::VectorShuffle; }
    OpVectorShuffle(IdResultType type, IdRef op0, IdRef op1, std::vector<LiteralInteger> op2)
        : spv_inst{Op::VectorShuffle, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> std::vector<LiteralInteger> const & { return op2_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    std::vector<LiteralInteger> op2_;
};
class OpCompositeConstruct : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::CompositeConstruct; }
    OpCompositeConstruct(IdResultType type, std::vector<IdRef> op0)
        : spv_inst{Op::CompositeConstruct, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> std::vector<IdRef> const & { return op0_; }

  private:
    IdResultType type_;
    std::vector<IdRef> op0_;
};
class OpCompositeExtract : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::CompositeExtract; }
    OpCompositeExtract(IdResultType type, IdRef op0, std::vector<LiteralInteger> op1)
        : spv_inst{Op::CompositeExtract, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> std::vector<LiteralInteger> const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    std::vector<LiteralInteger> op1_;
};
class OpCompositeInsert : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::CompositeInsert; }
    OpCompositeInsert(IdResultType type, IdRef op0, IdRef op1, std::vector<LiteralInteger> op2)
        : spv_inst{Op::CompositeInsert, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> std::vector<LiteralInteger> const & { return op2_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    std::vector<LiteralInteger> op2_;
};
class OpCopyObject : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::CopyObject; }
    OpCopyObject(IdResultType type, IdRef op0)
        : spv_inst{Op::CopyObject, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpTranspose : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::Transpose; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Matrix};
    OpTranspose(IdResultType type, IdRef op0)
        : spv_inst{Op::Transpose, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpSampledImage : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::SampledImage; }
    OpSampledImage(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::SampledImage, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpImageSampleImplicitLod : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::ImageSampleImplicitLod;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Shader};
    OpImageSampleImplicitLod(IdResultType type, IdRef op0, IdRef op1,
                             std::optional<ImageOperands> op2 = std::nullopt)
        : spv_inst{Op::ImageSampleImplicitLod, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> std::optional<ImageOperands> const & { return op2_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    std::optional<ImageOperands> op2_;
};
class OpImageSampleExplicitLod : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::ImageSampleExplicitLod;
    }
    OpImageSampleExplicitLod(IdResultType type, IdRef op0, IdRef op1, ImageOperands op2)
        : spv_inst{Op::ImageSampleExplicitLod, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> ImageOperands const & { return op2_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    ImageOperands op2_;
};
class OpImageSampleDrefImplicitLod : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::ImageSampleDrefImplicitLod;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Shader};
    OpImageSampleDrefImplicitLod(IdResultType type, IdRef op0, IdRef op1, IdRef op2,
                                 std::optional<ImageOperands> op3 = std::nullopt)
        : spv_inst{Op::ImageSampleDrefImplicitLod, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> std::optional<ImageOperands> const & { return op3_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    std::optional<ImageOperands> op3_;
};
class OpImageSampleDrefExplicitLod : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::ImageSampleDrefExplicitLod;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Shader};
    OpImageSampleDrefExplicitLod(IdResultType type, IdRef op0, IdRef op1, IdRef op2,
                                 ImageOperands op3)
        : spv_inst{Op::ImageSampleDrefExplicitLod, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> ImageOperands const & { return op3_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    ImageOperands op3_;
};
class OpImageSampleProjImplicitLod : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::ImageSampleProjImplicitLod;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Shader};
    OpImageSampleProjImplicitLod(IdResultType type, IdRef op0, IdRef op1,
                                 std::optional<ImageOperands> op2 = std::nullopt)
        : spv_inst{Op::ImageSampleProjImplicitLod, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> std::optional<ImageOperands> const & { return op2_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    std::optional<ImageOperands> op2_;
};
class OpImageSampleProjExplicitLod : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::ImageSampleProjExplicitLod;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Shader};
    OpImageSampleProjExplicitLod(IdResultType type, IdRef op0, IdRef op1, ImageOperands op2)
        : spv_inst{Op::ImageSampleProjExplicitLod, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> ImageOperands const & { return op2_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    ImageOperands op2_;
};
class OpImageSampleProjDrefImplicitLod : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::ImageSampleProjDrefImplicitLod;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Shader};
    OpImageSampleProjDrefImplicitLod(IdResultType type, IdRef op0, IdRef op1, IdRef op2,
                                     std::optional<ImageOperands> op3 = std::nullopt)
        : spv_inst{Op::ImageSampleProjDrefImplicitLod, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> std::optional<ImageOperands> const & { return op3_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    std::optional<ImageOperands> op3_;
};
class OpImageSampleProjDrefExplicitLod : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::ImageSampleProjDrefExplicitLod;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Shader};
    OpImageSampleProjDrefExplicitLod(IdResultType type, IdRef op0, IdRef op1, IdRef op2,
                                     ImageOperands op3)
        : spv_inst{Op::ImageSampleProjDrefExplicitLod, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> ImageOperands const & { return op3_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    ImageOperands op3_;
};
class OpImageFetch : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ImageFetch; }
    OpImageFetch(IdResultType type, IdRef op0, IdRef op1,
                 std::optional<ImageOperands> op2 = std::nullopt)
        : spv_inst{Op::ImageFetch, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> std::optional<ImageOperands> const & { return op2_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    std::optional<ImageOperands> op2_;
};
class OpImageGather : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ImageGather; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Shader};
    OpImageGather(IdResultType type, IdRef op0, IdRef op1, IdRef op2,
                  std::optional<ImageOperands> op3 = std::nullopt)
        : spv_inst{Op::ImageGather, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> std::optional<ImageOperands> const & { return op3_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    std::optional<ImageOperands> op3_;
};
class OpImageDrefGather : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ImageDrefGather; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Shader};
    OpImageDrefGather(IdResultType type, IdRef op0, IdRef op1, IdRef op2,
                      std::optional<ImageOperands> op3 = std::nullopt)
        : spv_inst{Op::ImageDrefGather, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> std::optional<ImageOperands> const & { return op3_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    std::optional<ImageOperands> op3_;
};
class OpImageRead : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ImageRead; }
    OpImageRead(IdResultType type, IdRef op0, IdRef op1,
                std::optional<ImageOperands> op2 = std::nullopt)
        : spv_inst{Op::ImageRead, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> std::optional<ImageOperands> const & { return op2_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    std::optional<ImageOperands> op2_;
};
class OpImageWrite : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ImageWrite; }
    OpImageWrite(IdRef op0, IdRef op1, IdRef op2, std::optional<ImageOperands> op3 = std::nullopt)
        : spv_inst{Op::ImageWrite, false}, op0_(std::move(op0)), op1_(std::move(op1)),
          op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> std::optional<ImageOperands> const & { return op3_; }

  private:
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    std::optional<ImageOperands> op3_;
};
class OpImage : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::Image; }
    OpImage(IdResultType type, IdRef op0)
        : spv_inst{Op::Image, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpImageQueryFormat : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ImageQueryFormat; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Kernel};
    OpImageQueryFormat(IdResultType type, IdRef op0)
        : spv_inst{Op::ImageQueryFormat, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpImageQueryOrder : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ImageQueryOrder; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Kernel};
    OpImageQueryOrder(IdResultType type, IdRef op0)
        : spv_inst{Op::ImageQueryOrder, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpImageQuerySizeLod : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ImageQuerySizeLod; }
    constexpr static std::array<Capability, 2> required_capabilities = {Capability::Kernel,
                                                                        Capability::ImageQuery};
    OpImageQuerySizeLod(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::ImageQuerySizeLod, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpImageQuerySize : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ImageQuerySize; }
    constexpr static std::array<Capability, 2> required_capabilities = {Capability::Kernel,
                                                                        Capability::ImageQuery};
    OpImageQuerySize(IdResultType type, IdRef op0)
        : spv_inst{Op::ImageQuerySize, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpImageQueryLod : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ImageQueryLod; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::ImageQuery};
    OpImageQueryLod(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::ImageQueryLod, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpImageQueryLevels : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ImageQueryLevels; }
    constexpr static std::array<Capability, 2> required_capabilities = {Capability::Kernel,
                                                                        Capability::ImageQuery};
    OpImageQueryLevels(IdResultType type, IdRef op0)
        : spv_inst{Op::ImageQueryLevels, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpImageQuerySamples : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ImageQuerySamples; }
    constexpr static std::array<Capability, 2> required_capabilities = {Capability::Kernel,
                                                                        Capability::ImageQuery};
    OpImageQuerySamples(IdResultType type, IdRef op0)
        : spv_inst{Op::ImageQuerySamples, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpConvertFToU : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ConvertFToU; }
    OpConvertFToU(IdResultType type, IdRef op0)
        : spv_inst{Op::ConvertFToU, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpConvertFToS : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ConvertFToS; }
    OpConvertFToS(IdResultType type, IdRef op0)
        : spv_inst{Op::ConvertFToS, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpConvertSToF : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ConvertSToF; }
    OpConvertSToF(IdResultType type, IdRef op0)
        : spv_inst{Op::ConvertSToF, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpConvertUToF : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ConvertUToF; }
    OpConvertUToF(IdResultType type, IdRef op0)
        : spv_inst{Op::ConvertUToF, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpUConvert : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::UConvert; }
    OpUConvert(IdResultType type, IdRef op0)
        : spv_inst{Op::UConvert, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpSConvert : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::SConvert; }
    OpSConvert(IdResultType type, IdRef op0)
        : spv_inst{Op::SConvert, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpFConvert : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::FConvert; }
    OpFConvert(IdResultType type, IdRef op0)
        : spv_inst{Op::FConvert, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpQuantizeToF16 : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::QuantizeToF16; }
    OpQuantizeToF16(IdResultType type, IdRef op0)
        : spv_inst{Op::QuantizeToF16, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpConvertPtrToU : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ConvertPtrToU; }
    constexpr static std::array<Capability, 2> required_capabilities = {
        Capability::Addresses, Capability::PhysicalStorageBufferAddresses};
    OpConvertPtrToU(IdResultType type, IdRef op0)
        : spv_inst{Op::ConvertPtrToU, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpSatConvertSToU : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::SatConvertSToU; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Kernel};
    OpSatConvertSToU(IdResultType type, IdRef op0)
        : spv_inst{Op::SatConvertSToU, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpSatConvertUToS : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::SatConvertUToS; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Kernel};
    OpSatConvertUToS(IdResultType type, IdRef op0)
        : spv_inst{Op::SatConvertUToS, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpConvertUToPtr : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ConvertUToPtr; }
    constexpr static std::array<Capability, 2> required_capabilities = {
        Capability::Addresses, Capability::PhysicalStorageBufferAddresses};
    OpConvertUToPtr(IdResultType type, IdRef op0)
        : spv_inst{Op::ConvertUToPtr, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpPtrCastToGeneric : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::PtrCastToGeneric; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Kernel};
    OpPtrCastToGeneric(IdResultType type, IdRef op0)
        : spv_inst{Op::PtrCastToGeneric, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpGenericCastToPtr : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GenericCastToPtr; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Kernel};
    OpGenericCastToPtr(IdResultType type, IdRef op0)
        : spv_inst{Op::GenericCastToPtr, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpGenericCastToPtrExplicit : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::GenericCastToPtrExplicit;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Kernel};
    OpGenericCastToPtrExplicit(IdResultType type, IdRef op0, StorageClass op1)
        : spv_inst{Op::GenericCastToPtrExplicit, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> StorageClass const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    StorageClass op1_;
};
class OpBitcast : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::Bitcast; }
    OpBitcast(IdResultType type, IdRef op0)
        : spv_inst{Op::Bitcast, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpSNegate : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::SNegate; }
    OpSNegate(IdResultType type, IdRef op0)
        : spv_inst{Op::SNegate, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpFNegate : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::FNegate; }
    OpFNegate(IdResultType type, IdRef op0)
        : spv_inst{Op::FNegate, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpIAdd : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::IAdd; }
    OpIAdd(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::IAdd, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpFAdd : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::FAdd; }
    OpFAdd(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::FAdd, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpISub : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ISub; }
    OpISub(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::ISub, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpFSub : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::FSub; }
    OpFSub(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::FSub, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpIMul : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::IMul; }
    OpIMul(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::IMul, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpFMul : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::FMul; }
    OpFMul(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::FMul, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpUDiv : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::UDiv; }
    OpUDiv(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::UDiv, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpSDiv : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::SDiv; }
    OpSDiv(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::SDiv, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpFDiv : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::FDiv; }
    OpFDiv(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::FDiv, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpUMod : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::UMod; }
    OpUMod(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::UMod, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpSRem : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::SRem; }
    OpSRem(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::SRem, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpSMod : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::SMod; }
    OpSMod(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::SMod, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpFRem : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::FRem; }
    OpFRem(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::FRem, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpFMod : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::FMod; }
    OpFMod(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::FMod, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpVectorTimesScalar : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::VectorTimesScalar; }
    OpVectorTimesScalar(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::VectorTimesScalar, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpMatrixTimesScalar : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::MatrixTimesScalar; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Matrix};
    OpMatrixTimesScalar(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::MatrixTimesScalar, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpVectorTimesMatrix : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::VectorTimesMatrix; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Matrix};
    OpVectorTimesMatrix(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::VectorTimesMatrix, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpMatrixTimesVector : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::MatrixTimesVector; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Matrix};
    OpMatrixTimesVector(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::MatrixTimesVector, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpMatrixTimesMatrix : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::MatrixTimesMatrix; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Matrix};
    OpMatrixTimesMatrix(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::MatrixTimesMatrix, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpOuterProduct : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::OuterProduct; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Matrix};
    OpOuterProduct(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::OuterProduct, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpDot : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::Dot; }
    OpDot(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::Dot, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpIAddCarry : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::IAddCarry; }
    OpIAddCarry(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::IAddCarry, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpISubBorrow : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ISubBorrow; }
    OpISubBorrow(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::ISubBorrow, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpUMulExtended : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::UMulExtended; }
    OpUMulExtended(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::UMulExtended, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpSMulExtended : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::SMulExtended; }
    OpSMulExtended(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::SMulExtended, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpAny : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::Any; }
    OpAny(IdResultType type, IdRef op0)
        : spv_inst{Op::Any, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpAll : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::All; }
    OpAll(IdResultType type, IdRef op0)
        : spv_inst{Op::All, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpIsNan : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::IsNan; }
    OpIsNan(IdResultType type, IdRef op0)
        : spv_inst{Op::IsNan, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpIsInf : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::IsInf; }
    OpIsInf(IdResultType type, IdRef op0)
        : spv_inst{Op::IsInf, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpIsFinite : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::IsFinite; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Kernel};
    OpIsFinite(IdResultType type, IdRef op0)
        : spv_inst{Op::IsFinite, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpIsNormal : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::IsNormal; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Kernel};
    OpIsNormal(IdResultType type, IdRef op0)
        : spv_inst{Op::IsNormal, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpSignBitSet : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::SignBitSet; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Kernel};
    OpSignBitSet(IdResultType type, IdRef op0)
        : spv_inst{Op::SignBitSet, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpLessOrGreater : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::LessOrGreater; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Kernel};
    OpLessOrGreater(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::LessOrGreater, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpOrdered : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::Ordered; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Kernel};
    OpOrdered(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::Ordered, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpUnordered : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::Unordered; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Kernel};
    OpUnordered(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::Unordered, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpLogicalEqual : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::LogicalEqual; }
    OpLogicalEqual(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::LogicalEqual, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpLogicalNotEqual : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::LogicalNotEqual; }
    OpLogicalNotEqual(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::LogicalNotEqual, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpLogicalOr : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::LogicalOr; }
    OpLogicalOr(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::LogicalOr, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpLogicalAnd : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::LogicalAnd; }
    OpLogicalAnd(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::LogicalAnd, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpLogicalNot : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::LogicalNot; }
    OpLogicalNot(IdResultType type, IdRef op0)
        : spv_inst{Op::LogicalNot, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpSelect : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::Select; }
    OpSelect(IdResultType type, IdRef op0, IdRef op1, IdRef op2)
        : spv_inst{Op::Select, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
};
class OpIEqual : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::IEqual; }
    OpIEqual(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::IEqual, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpINotEqual : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::INotEqual; }
    OpINotEqual(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::INotEqual, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpUGreaterThan : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::UGreaterThan; }
    OpUGreaterThan(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::UGreaterThan, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpSGreaterThan : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::SGreaterThan; }
    OpSGreaterThan(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::SGreaterThan, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpUGreaterThanEqual : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::UGreaterThanEqual; }
    OpUGreaterThanEqual(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::UGreaterThanEqual, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpSGreaterThanEqual : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::SGreaterThanEqual; }
    OpSGreaterThanEqual(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::SGreaterThanEqual, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpULessThan : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ULessThan; }
    OpULessThan(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::ULessThan, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpSLessThan : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::SLessThan; }
    OpSLessThan(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::SLessThan, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpULessThanEqual : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ULessThanEqual; }
    OpULessThanEqual(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::ULessThanEqual, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpSLessThanEqual : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::SLessThanEqual; }
    OpSLessThanEqual(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::SLessThanEqual, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpFOrdEqual : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::FOrdEqual; }
    OpFOrdEqual(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::FOrdEqual, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpFUnordEqual : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::FUnordEqual; }
    OpFUnordEqual(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::FUnordEqual, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpFOrdNotEqual : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::FOrdNotEqual; }
    OpFOrdNotEqual(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::FOrdNotEqual, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpFUnordNotEqual : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::FUnordNotEqual; }
    OpFUnordNotEqual(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::FUnordNotEqual, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpFOrdLessThan : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::FOrdLessThan; }
    OpFOrdLessThan(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::FOrdLessThan, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpFUnordLessThan : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::FUnordLessThan; }
    OpFUnordLessThan(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::FUnordLessThan, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpFOrdGreaterThan : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::FOrdGreaterThan; }
    OpFOrdGreaterThan(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::FOrdGreaterThan, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpFUnordGreaterThan : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::FUnordGreaterThan; }
    OpFUnordGreaterThan(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::FUnordGreaterThan, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpFOrdLessThanEqual : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::FOrdLessThanEqual; }
    OpFOrdLessThanEqual(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::FOrdLessThanEqual, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpFUnordLessThanEqual : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::FUnordLessThanEqual; }
    OpFUnordLessThanEqual(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::FUnordLessThanEqual, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpFOrdGreaterThanEqual : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::FOrdGreaterThanEqual; }
    OpFOrdGreaterThanEqual(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::FOrdGreaterThanEqual, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpFUnordGreaterThanEqual : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::FUnordGreaterThanEqual;
    }
    OpFUnordGreaterThanEqual(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::FUnordGreaterThanEqual, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpShiftRightLogical : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ShiftRightLogical; }
    OpShiftRightLogical(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::ShiftRightLogical, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpShiftRightArithmetic : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ShiftRightArithmetic; }
    OpShiftRightArithmetic(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::ShiftRightArithmetic, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpShiftLeftLogical : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ShiftLeftLogical; }
    OpShiftLeftLogical(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::ShiftLeftLogical, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpBitwiseOr : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::BitwiseOr; }
    OpBitwiseOr(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::BitwiseOr, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpBitwiseXor : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::BitwiseXor; }
    OpBitwiseXor(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::BitwiseXor, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpBitwiseAnd : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::BitwiseAnd; }
    OpBitwiseAnd(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::BitwiseAnd, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpNot : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::Not; }
    OpNot(IdResultType type, IdRef op0)
        : spv_inst{Op::Not, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpBitFieldInsert : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::BitFieldInsert; }
    constexpr static std::array<Capability, 2> required_capabilities = {
        Capability::Shader, Capability::BitInstructions};
    OpBitFieldInsert(IdResultType type, IdRef op0, IdRef op1, IdRef op2, IdRef op3)
        : spv_inst{Op::BitFieldInsert, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    IdRef op3_;
};
class OpBitFieldSExtract : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::BitFieldSExtract; }
    constexpr static std::array<Capability, 2> required_capabilities = {
        Capability::Shader, Capability::BitInstructions};
    OpBitFieldSExtract(IdResultType type, IdRef op0, IdRef op1, IdRef op2)
        : spv_inst{Op::BitFieldSExtract, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
};
class OpBitFieldUExtract : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::BitFieldUExtract; }
    constexpr static std::array<Capability, 2> required_capabilities = {
        Capability::Shader, Capability::BitInstructions};
    OpBitFieldUExtract(IdResultType type, IdRef op0, IdRef op1, IdRef op2)
        : spv_inst{Op::BitFieldUExtract, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
};
class OpBitReverse : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::BitReverse; }
    constexpr static std::array<Capability, 2> required_capabilities = {
        Capability::Shader, Capability::BitInstructions};
    OpBitReverse(IdResultType type, IdRef op0)
        : spv_inst{Op::BitReverse, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpBitCount : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::BitCount; }
    OpBitCount(IdResultType type, IdRef op0)
        : spv_inst{Op::BitCount, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpDPdx : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::DPdx; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Shader};
    OpDPdx(IdResultType type, IdRef op0)
        : spv_inst{Op::DPdx, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpDPdy : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::DPdy; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Shader};
    OpDPdy(IdResultType type, IdRef op0)
        : spv_inst{Op::DPdy, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpFwidth : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::Fwidth; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Shader};
    OpFwidth(IdResultType type, IdRef op0)
        : spv_inst{Op::Fwidth, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpDPdxFine : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::DPdxFine; }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::DerivativeControl};
    OpDPdxFine(IdResultType type, IdRef op0)
        : spv_inst{Op::DPdxFine, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpDPdyFine : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::DPdyFine; }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::DerivativeControl};
    OpDPdyFine(IdResultType type, IdRef op0)
        : spv_inst{Op::DPdyFine, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpFwidthFine : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::FwidthFine; }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::DerivativeControl};
    OpFwidthFine(IdResultType type, IdRef op0)
        : spv_inst{Op::FwidthFine, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpDPdxCoarse : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::DPdxCoarse; }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::DerivativeControl};
    OpDPdxCoarse(IdResultType type, IdRef op0)
        : spv_inst{Op::DPdxCoarse, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpDPdyCoarse : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::DPdyCoarse; }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::DerivativeControl};
    OpDPdyCoarse(IdResultType type, IdRef op0)
        : spv_inst{Op::DPdyCoarse, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpFwidthCoarse : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::FwidthCoarse; }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::DerivativeControl};
    OpFwidthCoarse(IdResultType type, IdRef op0)
        : spv_inst{Op::FwidthCoarse, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpEmitVertex : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::EmitVertex; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Geometry};
    OpEmitVertex() : spv_inst{Op::EmitVertex, false} {}

  private:
};
class OpEndPrimitive : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::EndPrimitive; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Geometry};
    OpEndPrimitive() : spv_inst{Op::EndPrimitive, false} {}

  private:
};
class OpEmitStreamVertex : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::EmitStreamVertex; }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::GeometryStreams};
    OpEmitStreamVertex(IdRef op0) : spv_inst{Op::EmitStreamVertex, false}, op0_(std::move(op0)) {}
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdRef op0_;
};
class OpEndStreamPrimitive : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::EndStreamPrimitive; }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::GeometryStreams};
    OpEndStreamPrimitive(IdRef op0)
        : spv_inst{Op::EndStreamPrimitive, false}, op0_(std::move(op0)) {}
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdRef op0_;
};
class OpControlBarrier : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ControlBarrier; }
    OpControlBarrier(IdScope op0, IdScope op1, IdMemorySemantics op2)
        : spv_inst{Op::ControlBarrier, false}, op0_(std::move(op0)), op1_(std::move(op1)),
          op2_(std::move(op2)) {}
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> IdScope const & { return op1_; }
    inline auto op2() const -> IdMemorySemantics const & { return op2_; }

  private:
    IdScope op0_;
    IdScope op1_;
    IdMemorySemantics op2_;
};
class OpMemoryBarrier : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::MemoryBarrier; }
    OpMemoryBarrier(IdScope op0, IdMemorySemantics op1)
        : spv_inst{Op::MemoryBarrier, false}, op0_(std::move(op0)), op1_(std::move(op1)) {}
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> IdMemorySemantics const & { return op1_; }

  private:
    IdScope op0_;
    IdMemorySemantics op1_;
};
class OpAtomicLoad : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::AtomicLoad; }
    OpAtomicLoad(IdResultType type, IdRef op0, IdScope op1, IdMemorySemantics op2)
        : spv_inst{Op::AtomicLoad, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdScope const & { return op1_; }
    inline auto op2() const -> IdMemorySemantics const & { return op2_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdScope op1_;
    IdMemorySemantics op2_;
};
class OpAtomicStore : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::AtomicStore; }
    OpAtomicStore(IdRef op0, IdScope op1, IdMemorySemantics op2, IdRef op3)
        : spv_inst{Op::AtomicStore, false}, op0_(std::move(op0)), op1_(std::move(op1)),
          op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdScope const & { return op1_; }
    inline auto op2() const -> IdMemorySemantics const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }

  private:
    IdRef op0_;
    IdScope op1_;
    IdMemorySemantics op2_;
    IdRef op3_;
};
class OpAtomicExchange : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::AtomicExchange; }
    OpAtomicExchange(IdResultType type, IdRef op0, IdScope op1, IdMemorySemantics op2, IdRef op3)
        : spv_inst{Op::AtomicExchange, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdScope const & { return op1_; }
    inline auto op2() const -> IdMemorySemantics const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdScope op1_;
    IdMemorySemantics op2_;
    IdRef op3_;
};
class OpAtomicCompareExchange : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::AtomicCompareExchange;
    }
    OpAtomicCompareExchange(IdResultType type, IdRef op0, IdScope op1, IdMemorySemantics op2,
                            IdMemorySemantics op3, IdRef op4, IdRef op5)
        : spv_inst{Op::AtomicCompareExchange, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)), op4_(std::move(op4)),
          op5_(std::move(op5)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdScope const & { return op1_; }
    inline auto op2() const -> IdMemorySemantics const & { return op2_; }
    inline auto op3() const -> IdMemorySemantics const & { return op3_; }
    inline auto op4() const -> IdRef const & { return op4_; }
    inline auto op5() const -> IdRef const & { return op5_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdScope op1_;
    IdMemorySemantics op2_;
    IdMemorySemantics op3_;
    IdRef op4_;
    IdRef op5_;
};
class OpAtomicCompareExchangeWeak : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::AtomicCompareExchangeWeak;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Kernel};
    OpAtomicCompareExchangeWeak(IdResultType type, IdRef op0, IdScope op1, IdMemorySemantics op2,
                                IdMemorySemantics op3, IdRef op4, IdRef op5)
        : spv_inst{Op::AtomicCompareExchangeWeak, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)),
          op4_(std::move(op4)), op5_(std::move(op5)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdScope const & { return op1_; }
    inline auto op2() const -> IdMemorySemantics const & { return op2_; }
    inline auto op3() const -> IdMemorySemantics const & { return op3_; }
    inline auto op4() const -> IdRef const & { return op4_; }
    inline auto op5() const -> IdRef const & { return op5_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdScope op1_;
    IdMemorySemantics op2_;
    IdMemorySemantics op3_;
    IdRef op4_;
    IdRef op5_;
};
class OpAtomicIIncrement : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::AtomicIIncrement; }
    OpAtomicIIncrement(IdResultType type, IdRef op0, IdScope op1, IdMemorySemantics op2)
        : spv_inst{Op::AtomicIIncrement, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdScope const & { return op1_; }
    inline auto op2() const -> IdMemorySemantics const & { return op2_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdScope op1_;
    IdMemorySemantics op2_;
};
class OpAtomicIDecrement : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::AtomicIDecrement; }
    OpAtomicIDecrement(IdResultType type, IdRef op0, IdScope op1, IdMemorySemantics op2)
        : spv_inst{Op::AtomicIDecrement, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdScope const & { return op1_; }
    inline auto op2() const -> IdMemorySemantics const & { return op2_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdScope op1_;
    IdMemorySemantics op2_;
};
class OpAtomicIAdd : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::AtomicIAdd; }
    OpAtomicIAdd(IdResultType type, IdRef op0, IdScope op1, IdMemorySemantics op2, IdRef op3)
        : spv_inst{Op::AtomicIAdd, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdScope const & { return op1_; }
    inline auto op2() const -> IdMemorySemantics const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdScope op1_;
    IdMemorySemantics op2_;
    IdRef op3_;
};
class OpAtomicISub : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::AtomicISub; }
    OpAtomicISub(IdResultType type, IdRef op0, IdScope op1, IdMemorySemantics op2, IdRef op3)
        : spv_inst{Op::AtomicISub, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdScope const & { return op1_; }
    inline auto op2() const -> IdMemorySemantics const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdScope op1_;
    IdMemorySemantics op2_;
    IdRef op3_;
};
class OpAtomicSMin : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::AtomicSMin; }
    OpAtomicSMin(IdResultType type, IdRef op0, IdScope op1, IdMemorySemantics op2, IdRef op3)
        : spv_inst{Op::AtomicSMin, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdScope const & { return op1_; }
    inline auto op2() const -> IdMemorySemantics const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdScope op1_;
    IdMemorySemantics op2_;
    IdRef op3_;
};
class OpAtomicUMin : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::AtomicUMin; }
    OpAtomicUMin(IdResultType type, IdRef op0, IdScope op1, IdMemorySemantics op2, IdRef op3)
        : spv_inst{Op::AtomicUMin, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdScope const & { return op1_; }
    inline auto op2() const -> IdMemorySemantics const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdScope op1_;
    IdMemorySemantics op2_;
    IdRef op3_;
};
class OpAtomicSMax : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::AtomicSMax; }
    OpAtomicSMax(IdResultType type, IdRef op0, IdScope op1, IdMemorySemantics op2, IdRef op3)
        : spv_inst{Op::AtomicSMax, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdScope const & { return op1_; }
    inline auto op2() const -> IdMemorySemantics const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdScope op1_;
    IdMemorySemantics op2_;
    IdRef op3_;
};
class OpAtomicUMax : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::AtomicUMax; }
    OpAtomicUMax(IdResultType type, IdRef op0, IdScope op1, IdMemorySemantics op2, IdRef op3)
        : spv_inst{Op::AtomicUMax, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdScope const & { return op1_; }
    inline auto op2() const -> IdMemorySemantics const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdScope op1_;
    IdMemorySemantics op2_;
    IdRef op3_;
};
class OpAtomicAnd : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::AtomicAnd; }
    OpAtomicAnd(IdResultType type, IdRef op0, IdScope op1, IdMemorySemantics op2, IdRef op3)
        : spv_inst{Op::AtomicAnd, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdScope const & { return op1_; }
    inline auto op2() const -> IdMemorySemantics const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdScope op1_;
    IdMemorySemantics op2_;
    IdRef op3_;
};
class OpAtomicOr : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::AtomicOr; }
    OpAtomicOr(IdResultType type, IdRef op0, IdScope op1, IdMemorySemantics op2, IdRef op3)
        : spv_inst{Op::AtomicOr, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdScope const & { return op1_; }
    inline auto op2() const -> IdMemorySemantics const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdScope op1_;
    IdMemorySemantics op2_;
    IdRef op3_;
};
class OpAtomicXor : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::AtomicXor; }
    OpAtomicXor(IdResultType type, IdRef op0, IdScope op1, IdMemorySemantics op2, IdRef op3)
        : spv_inst{Op::AtomicXor, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdScope const & { return op1_; }
    inline auto op2() const -> IdMemorySemantics const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdScope op1_;
    IdMemorySemantics op2_;
    IdRef op3_;
};
class OpPhi : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::Phi; }
    OpPhi(IdResultType type, std::vector<PairIdRefIdRef> op0)
        : spv_inst{Op::Phi, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> std::vector<PairIdRefIdRef> const & { return op0_; }

  private:
    IdResultType type_;
    std::vector<PairIdRefIdRef> op0_;
};
class OpLoopMerge : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::LoopMerge; }
    OpLoopMerge(IdRef op0, IdRef op1, LoopControl op2)
        : spv_inst{Op::LoopMerge, false}, op0_(std::move(op0)), op1_(std::move(op1)),
          op2_(std::move(op2)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> LoopControl const & { return op2_; }

  private:
    IdRef op0_;
    IdRef op1_;
    LoopControl op2_;
};
class OpSelectionMerge : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::SelectionMerge; }
    OpSelectionMerge(IdRef op0, SelectionControl op1)
        : spv_inst{Op::SelectionMerge, false}, op0_(std::move(op0)), op1_(std::move(op1)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> SelectionControl const & { return op1_; }

  private:
    IdRef op0_;
    SelectionControl op1_;
};
class OpLabel : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::Label; }
    OpLabel() : spv_inst{Op::Label, true} {}

  private:
};
class OpBranch : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::Branch; }
    OpBranch(IdRef op0) : spv_inst{Op::Branch, false}, op0_(std::move(op0)) {}
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdRef op0_;
};
class OpBranchConditional : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::BranchConditional; }
    OpBranchConditional(IdRef op0, IdRef op1, IdRef op2, std::vector<LiteralInteger> op3)
        : spv_inst{Op::BranchConditional, false}, op0_(std::move(op0)), op1_(std::move(op1)),
          op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> std::vector<LiteralInteger> const & { return op3_; }

  private:
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    std::vector<LiteralInteger> op3_;
};
class OpSwitch : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::Switch; }
    OpSwitch(IdRef op0, IdRef op1, std::vector<PairLiteralIntegerIdRef> op2)
        : spv_inst{Op::Switch, false}, op0_(std::move(op0)), op1_(std::move(op1)),
          op2_(std::move(op2)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> std::vector<PairLiteralIntegerIdRef> const & { return op2_; }

  private:
    IdRef op0_;
    IdRef op1_;
    std::vector<PairLiteralIntegerIdRef> op2_;
};
class OpKill : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::Kill; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Shader};
    OpKill() : spv_inst{Op::Kill, false} {}

  private:
};
class OpReturn : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::Return; }
    OpReturn() : spv_inst{Op::Return, false} {}

  private:
};
class OpReturnValue : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ReturnValue; }
    OpReturnValue(IdRef op0) : spv_inst{Op::ReturnValue, false}, op0_(std::move(op0)) {}
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdRef op0_;
};
class OpUnreachable : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::Unreachable; }
    OpUnreachable() : spv_inst{Op::Unreachable, false} {}

  private:
};
class OpLifetimeStart : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::LifetimeStart; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Kernel};
    OpLifetimeStart(IdRef op0, LiteralInteger op1)
        : spv_inst{Op::LifetimeStart, false}, op0_(std::move(op0)), op1_(std::move(op1)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> LiteralInteger const & { return op1_; }

  private:
    IdRef op0_;
    LiteralInteger op1_;
};
class OpLifetimeStop : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::LifetimeStop; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Kernel};
    OpLifetimeStop(IdRef op0, LiteralInteger op1)
        : spv_inst{Op::LifetimeStop, false}, op0_(std::move(op0)), op1_(std::move(op1)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> LiteralInteger const & { return op1_; }

  private:
    IdRef op0_;
    LiteralInteger op1_;
};
class OpGroupAsyncCopy : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GroupAsyncCopy; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Kernel};
    OpGroupAsyncCopy(IdResultType type, IdScope op0, IdRef op1, IdRef op2, IdRef op3, IdRef op4,
                     IdRef op5)
        : spv_inst{Op::GroupAsyncCopy, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)), op4_(std::move(op4)),
          op5_(std::move(op5)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }
    inline auto op4() const -> IdRef const & { return op4_; }
    inline auto op5() const -> IdRef const & { return op5_; }

  private:
    IdResultType type_;
    IdScope op0_;
    IdRef op1_;
    IdRef op2_;
    IdRef op3_;
    IdRef op4_;
    IdRef op5_;
};
class OpGroupWaitEvents : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GroupWaitEvents; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Kernel};
    OpGroupWaitEvents(IdScope op0, IdRef op1, IdRef op2)
        : spv_inst{Op::GroupWaitEvents, false}, op0_(std::move(op0)), op1_(std::move(op1)),
          op2_(std::move(op2)) {}
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }

  private:
    IdScope op0_;
    IdRef op1_;
    IdRef op2_;
};
class OpGroupAll : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GroupAll; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Groups};
    OpGroupAll(IdResultType type, IdScope op0, IdRef op1)
        : spv_inst{Op::GroupAll, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdScope op0_;
    IdRef op1_;
};
class OpGroupAny : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GroupAny; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Groups};
    OpGroupAny(IdResultType type, IdScope op0, IdRef op1)
        : spv_inst{Op::GroupAny, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdScope op0_;
    IdRef op1_;
};
class OpGroupBroadcast : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GroupBroadcast; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Groups};
    OpGroupBroadcast(IdResultType type, IdScope op0, IdRef op1, IdRef op2)
        : spv_inst{Op::GroupBroadcast, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }

  private:
    IdResultType type_;
    IdScope op0_;
    IdRef op1_;
    IdRef op2_;
};
class OpGroupIAdd : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GroupIAdd; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Groups};
    OpGroupIAdd(IdResultType type, IdScope op0, GroupOperation op1, IdRef op2)
        : spv_inst{Op::GroupIAdd, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> GroupOperation const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }

  private:
    IdResultType type_;
    IdScope op0_;
    GroupOperation op1_;
    IdRef op2_;
};
class OpGroupFAdd : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GroupFAdd; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Groups};
    OpGroupFAdd(IdResultType type, IdScope op0, GroupOperation op1, IdRef op2)
        : spv_inst{Op::GroupFAdd, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> GroupOperation const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }

  private:
    IdResultType type_;
    IdScope op0_;
    GroupOperation op1_;
    IdRef op2_;
};
class OpGroupFMin : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GroupFMin; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Groups};
    OpGroupFMin(IdResultType type, IdScope op0, GroupOperation op1, IdRef op2)
        : spv_inst{Op::GroupFMin, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> GroupOperation const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }

  private:
    IdResultType type_;
    IdScope op0_;
    GroupOperation op1_;
    IdRef op2_;
};
class OpGroupUMin : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GroupUMin; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Groups};
    OpGroupUMin(IdResultType type, IdScope op0, GroupOperation op1, IdRef op2)
        : spv_inst{Op::GroupUMin, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> GroupOperation const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }

  private:
    IdResultType type_;
    IdScope op0_;
    GroupOperation op1_;
    IdRef op2_;
};
class OpGroupSMin : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GroupSMin; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Groups};
    OpGroupSMin(IdResultType type, IdScope op0, GroupOperation op1, IdRef op2)
        : spv_inst{Op::GroupSMin, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> GroupOperation const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }

  private:
    IdResultType type_;
    IdScope op0_;
    GroupOperation op1_;
    IdRef op2_;
};
class OpGroupFMax : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GroupFMax; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Groups};
    OpGroupFMax(IdResultType type, IdScope op0, GroupOperation op1, IdRef op2)
        : spv_inst{Op::GroupFMax, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> GroupOperation const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }

  private:
    IdResultType type_;
    IdScope op0_;
    GroupOperation op1_;
    IdRef op2_;
};
class OpGroupUMax : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GroupUMax; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Groups};
    OpGroupUMax(IdResultType type, IdScope op0, GroupOperation op1, IdRef op2)
        : spv_inst{Op::GroupUMax, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> GroupOperation const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }

  private:
    IdResultType type_;
    IdScope op0_;
    GroupOperation op1_;
    IdRef op2_;
};
class OpGroupSMax : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GroupSMax; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Groups};
    OpGroupSMax(IdResultType type, IdScope op0, GroupOperation op1, IdRef op2)
        : spv_inst{Op::GroupSMax, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> GroupOperation const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }

  private:
    IdResultType type_;
    IdScope op0_;
    GroupOperation op1_;
    IdRef op2_;
};
class OpReadPipe : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ReadPipe; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Pipes};
    OpReadPipe(IdResultType type, IdRef op0, IdRef op1, IdRef op2, IdRef op3)
        : spv_inst{Op::ReadPipe, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    IdRef op3_;
};
class OpWritePipe : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::WritePipe; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Pipes};
    OpWritePipe(IdResultType type, IdRef op0, IdRef op1, IdRef op2, IdRef op3)
        : spv_inst{Op::WritePipe, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    IdRef op3_;
};
class OpReservedReadPipe : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ReservedReadPipe; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Pipes};
    OpReservedReadPipe(IdResultType type, IdRef op0, IdRef op1, IdRef op2, IdRef op3, IdRef op4,
                       IdRef op5)
        : spv_inst{Op::ReservedReadPipe, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)), op4_(std::move(op4)),
          op5_(std::move(op5)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }
    inline auto op4() const -> IdRef const & { return op4_; }
    inline auto op5() const -> IdRef const & { return op5_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    IdRef op3_;
    IdRef op4_;
    IdRef op5_;
};
class OpReservedWritePipe : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ReservedWritePipe; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Pipes};
    OpReservedWritePipe(IdResultType type, IdRef op0, IdRef op1, IdRef op2, IdRef op3, IdRef op4,
                        IdRef op5)
        : spv_inst{Op::ReservedWritePipe, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)), op4_(std::move(op4)),
          op5_(std::move(op5)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }
    inline auto op4() const -> IdRef const & { return op4_; }
    inline auto op5() const -> IdRef const & { return op5_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    IdRef op3_;
    IdRef op4_;
    IdRef op5_;
};
class OpReserveReadPipePackets : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::ReserveReadPipePackets;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Pipes};
    OpReserveReadPipePackets(IdResultType type, IdRef op0, IdRef op1, IdRef op2, IdRef op3)
        : spv_inst{Op::ReserveReadPipePackets, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    IdRef op3_;
};
class OpReserveWritePipePackets : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::ReserveWritePipePackets;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Pipes};
    OpReserveWritePipePackets(IdResultType type, IdRef op0, IdRef op1, IdRef op2, IdRef op3)
        : spv_inst{Op::ReserveWritePipePackets, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    IdRef op3_;
};
class OpCommitReadPipe : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::CommitReadPipe; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Pipes};
    OpCommitReadPipe(IdRef op0, IdRef op1, IdRef op2, IdRef op3)
        : spv_inst{Op::CommitReadPipe, false}, op0_(std::move(op0)), op1_(std::move(op1)),
          op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }

  private:
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    IdRef op3_;
};
class OpCommitWritePipe : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::CommitWritePipe; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Pipes};
    OpCommitWritePipe(IdRef op0, IdRef op1, IdRef op2, IdRef op3)
        : spv_inst{Op::CommitWritePipe, false}, op0_(std::move(op0)), op1_(std::move(op1)),
          op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }

  private:
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    IdRef op3_;
};
class OpIsValidReserveId : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::IsValidReserveId; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Pipes};
    OpIsValidReserveId(IdResultType type, IdRef op0)
        : spv_inst{Op::IsValidReserveId, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpGetNumPipePackets : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GetNumPipePackets; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Pipes};
    OpGetNumPipePackets(IdResultType type, IdRef op0, IdRef op1, IdRef op2)
        : spv_inst{Op::GetNumPipePackets, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
};
class OpGetMaxPipePackets : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GetMaxPipePackets; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Pipes};
    OpGetMaxPipePackets(IdResultType type, IdRef op0, IdRef op1, IdRef op2)
        : spv_inst{Op::GetMaxPipePackets, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
};
class OpGroupReserveReadPipePackets : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::GroupReserveReadPipePackets;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Pipes};
    OpGroupReserveReadPipePackets(IdResultType type, IdScope op0, IdRef op1, IdRef op2, IdRef op3,
                                  IdRef op4)
        : spv_inst{Op::GroupReserveReadPipePackets, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)),
          op4_(std::move(op4)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }
    inline auto op4() const -> IdRef const & { return op4_; }

  private:
    IdResultType type_;
    IdScope op0_;
    IdRef op1_;
    IdRef op2_;
    IdRef op3_;
    IdRef op4_;
};
class OpGroupReserveWritePipePackets : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::GroupReserveWritePipePackets;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Pipes};
    OpGroupReserveWritePipePackets(IdResultType type, IdScope op0, IdRef op1, IdRef op2, IdRef op3,
                                   IdRef op4)
        : spv_inst{Op::GroupReserveWritePipePackets, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)),
          op4_(std::move(op4)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }
    inline auto op4() const -> IdRef const & { return op4_; }

  private:
    IdResultType type_;
    IdScope op0_;
    IdRef op1_;
    IdRef op2_;
    IdRef op3_;
    IdRef op4_;
};
class OpGroupCommitReadPipe : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GroupCommitReadPipe; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Pipes};
    OpGroupCommitReadPipe(IdScope op0, IdRef op1, IdRef op2, IdRef op3, IdRef op4)
        : spv_inst{Op::GroupCommitReadPipe, false}, op0_(std::move(op0)), op1_(std::move(op1)),
          op2_(std::move(op2)), op3_(std::move(op3)), op4_(std::move(op4)) {}
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }
    inline auto op4() const -> IdRef const & { return op4_; }

  private:
    IdScope op0_;
    IdRef op1_;
    IdRef op2_;
    IdRef op3_;
    IdRef op4_;
};
class OpGroupCommitWritePipe : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GroupCommitWritePipe; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Pipes};
    OpGroupCommitWritePipe(IdScope op0, IdRef op1, IdRef op2, IdRef op3, IdRef op4)
        : spv_inst{Op::GroupCommitWritePipe, false}, op0_(std::move(op0)), op1_(std::move(op1)),
          op2_(std::move(op2)), op3_(std::move(op3)), op4_(std::move(op4)) {}
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }
    inline auto op4() const -> IdRef const & { return op4_; }

  private:
    IdScope op0_;
    IdRef op1_;
    IdRef op2_;
    IdRef op3_;
    IdRef op4_;
};
class OpEnqueueMarker : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::EnqueueMarker; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::DeviceEnqueue};
    OpEnqueueMarker(IdResultType type, IdRef op0, IdRef op1, IdRef op2, IdRef op3)
        : spv_inst{Op::EnqueueMarker, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    IdRef op3_;
};
class OpEnqueueKernel : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::EnqueueKernel; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::DeviceEnqueue};
    OpEnqueueKernel(IdResultType type, IdRef op0, IdRef op1, IdRef op2, IdRef op3, IdRef op4,
                    IdRef op5, IdRef op6, IdRef op7, IdRef op8, IdRef op9, std::vector<IdRef> op10)
        : spv_inst{Op::EnqueueKernel, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)), op4_(std::move(op4)),
          op5_(std::move(op5)), op6_(std::move(op6)), op7_(std::move(op7)), op8_(std::move(op8)),
          op9_(std::move(op9)), op10_(std::move(op10)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }
    inline auto op4() const -> IdRef const & { return op4_; }
    inline auto op5() const -> IdRef const & { return op5_; }
    inline auto op6() const -> IdRef const & { return op6_; }
    inline auto op7() const -> IdRef const & { return op7_; }
    inline auto op8() const -> IdRef const & { return op8_; }
    inline auto op9() const -> IdRef const & { return op9_; }
    inline auto op10() const -> std::vector<IdRef> const & { return op10_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    IdRef op3_;
    IdRef op4_;
    IdRef op5_;
    IdRef op6_;
    IdRef op7_;
    IdRef op8_;
    IdRef op9_;
    std::vector<IdRef> op10_;
};
class OpGetKernelNDrangeSubGroupCount : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::GetKernelNDrangeSubGroupCount;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::DeviceEnqueue};
    OpGetKernelNDrangeSubGroupCount(IdResultType type, IdRef op0, IdRef op1, IdRef op2, IdRef op3,
                                    IdRef op4)
        : spv_inst{Op::GetKernelNDrangeSubGroupCount, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)),
          op4_(std::move(op4)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }
    inline auto op4() const -> IdRef const & { return op4_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    IdRef op3_;
    IdRef op4_;
};
class OpGetKernelNDrangeMaxSubGroupSize : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::GetKernelNDrangeMaxSubGroupSize;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::DeviceEnqueue};
    OpGetKernelNDrangeMaxSubGroupSize(IdResultType type, IdRef op0, IdRef op1, IdRef op2, IdRef op3,
                                      IdRef op4)
        : spv_inst{Op::GetKernelNDrangeMaxSubGroupSize, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)),
          op4_(std::move(op4)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }
    inline auto op4() const -> IdRef const & { return op4_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    IdRef op3_;
    IdRef op4_;
};
class OpGetKernelWorkGroupSize : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::GetKernelWorkGroupSize;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::DeviceEnqueue};
    OpGetKernelWorkGroupSize(IdResultType type, IdRef op0, IdRef op1, IdRef op2, IdRef op3)
        : spv_inst{Op::GetKernelWorkGroupSize, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    IdRef op3_;
};
class OpGetKernelPreferredWorkGroupSizeMultiple : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::GetKernelPreferredWorkGroupSizeMultiple;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::DeviceEnqueue};
    OpGetKernelPreferredWorkGroupSizeMultiple(IdResultType type, IdRef op0, IdRef op1, IdRef op2,
                                              IdRef op3)
        : spv_inst{Op::GetKernelPreferredWorkGroupSizeMultiple, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    IdRef op3_;
};
class OpRetainEvent : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::RetainEvent; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::DeviceEnqueue};
    OpRetainEvent(IdRef op0) : spv_inst{Op::RetainEvent, false}, op0_(std::move(op0)) {}
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdRef op0_;
};
class OpReleaseEvent : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ReleaseEvent; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::DeviceEnqueue};
    OpReleaseEvent(IdRef op0) : spv_inst{Op::ReleaseEvent, false}, op0_(std::move(op0)) {}
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdRef op0_;
};
class OpCreateUserEvent : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::CreateUserEvent; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::DeviceEnqueue};
    OpCreateUserEvent(IdResultType type)
        : spv_inst{Op::CreateUserEvent, true}, type_(std::move(type)) {}
    inline auto type() const -> IdResultType const & { return type_; }

  private:
    IdResultType type_;
};
class OpIsValidEvent : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::IsValidEvent; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::DeviceEnqueue};
    OpIsValidEvent(IdResultType type, IdRef op0)
        : spv_inst{Op::IsValidEvent, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpSetUserEventStatus : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::SetUserEventStatus; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::DeviceEnqueue};
    OpSetUserEventStatus(IdRef op0, IdRef op1)
        : spv_inst{Op::SetUserEventStatus, false}, op0_(std::move(op0)), op1_(std::move(op1)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdRef op0_;
    IdRef op1_;
};
class OpCaptureEventProfilingInfo : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::CaptureEventProfilingInfo;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::DeviceEnqueue};
    OpCaptureEventProfilingInfo(IdRef op0, IdRef op1, IdRef op2)
        : spv_inst{Op::CaptureEventProfilingInfo, false}, op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }

  private:
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
};
class OpGetDefaultQueue : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GetDefaultQueue; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::DeviceEnqueue};
    OpGetDefaultQueue(IdResultType type)
        : spv_inst{Op::GetDefaultQueue, true}, type_(std::move(type)) {}
    inline auto type() const -> IdResultType const & { return type_; }

  private:
    IdResultType type_;
};
class OpBuildNDRange : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::BuildNDRange; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::DeviceEnqueue};
    OpBuildNDRange(IdResultType type, IdRef op0, IdRef op1, IdRef op2)
        : spv_inst{Op::BuildNDRange, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
};
class OpImageSparseSampleImplicitLod : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::ImageSparseSampleImplicitLod;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::SparseResidency};
    OpImageSparseSampleImplicitLod(IdResultType type, IdRef op0, IdRef op1,
                                   std::optional<ImageOperands> op2 = std::nullopt)
        : spv_inst{Op::ImageSparseSampleImplicitLod, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> std::optional<ImageOperands> const & { return op2_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    std::optional<ImageOperands> op2_;
};
class OpImageSparseSampleExplicitLod : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::ImageSparseSampleExplicitLod;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::SparseResidency};
    OpImageSparseSampleExplicitLod(IdResultType type, IdRef op0, IdRef op1, ImageOperands op2)
        : spv_inst{Op::ImageSparseSampleExplicitLod, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> ImageOperands const & { return op2_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    ImageOperands op2_;
};
class OpImageSparseSampleDrefImplicitLod : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::ImageSparseSampleDrefImplicitLod;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::SparseResidency};
    OpImageSparseSampleDrefImplicitLod(IdResultType type, IdRef op0, IdRef op1, IdRef op2,
                                       std::optional<ImageOperands> op3 = std::nullopt)
        : spv_inst{Op::ImageSparseSampleDrefImplicitLod, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> std::optional<ImageOperands> const & { return op3_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    std::optional<ImageOperands> op3_;
};
class OpImageSparseSampleDrefExplicitLod : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::ImageSparseSampleDrefExplicitLod;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::SparseResidency};
    OpImageSparseSampleDrefExplicitLod(IdResultType type, IdRef op0, IdRef op1, IdRef op2,
                                       ImageOperands op3)
        : spv_inst{Op::ImageSparseSampleDrefExplicitLod, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> ImageOperands const & { return op3_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    ImageOperands op3_;
};
class OpImageSparseSampleProjImplicitLod : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::ImageSparseSampleProjImplicitLod;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::SparseResidency};
    OpImageSparseSampleProjImplicitLod(IdResultType type, IdRef op0, IdRef op1,
                                       std::optional<ImageOperands> op2 = std::nullopt)
        : spv_inst{Op::ImageSparseSampleProjImplicitLod, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> std::optional<ImageOperands> const & { return op2_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    std::optional<ImageOperands> op2_;
};
class OpImageSparseSampleProjExplicitLod : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::ImageSparseSampleProjExplicitLod;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::SparseResidency};
    OpImageSparseSampleProjExplicitLod(IdResultType type, IdRef op0, IdRef op1, ImageOperands op2)
        : spv_inst{Op::ImageSparseSampleProjExplicitLod, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> ImageOperands const & { return op2_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    ImageOperands op2_;
};
class OpImageSparseSampleProjDrefImplicitLod : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::ImageSparseSampleProjDrefImplicitLod;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::SparseResidency};
    OpImageSparseSampleProjDrefImplicitLod(IdResultType type, IdRef op0, IdRef op1, IdRef op2,
                                           std::optional<ImageOperands> op3 = std::nullopt)
        : spv_inst{Op::ImageSparseSampleProjDrefImplicitLod, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> std::optional<ImageOperands> const & { return op3_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    std::optional<ImageOperands> op3_;
};
class OpImageSparseSampleProjDrefExplicitLod : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::ImageSparseSampleProjDrefExplicitLod;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::SparseResidency};
    OpImageSparseSampleProjDrefExplicitLod(IdResultType type, IdRef op0, IdRef op1, IdRef op2,
                                           ImageOperands op3)
        : spv_inst{Op::ImageSparseSampleProjDrefExplicitLod, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> ImageOperands const & { return op3_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    ImageOperands op3_;
};
class OpImageSparseFetch : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ImageSparseFetch; }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::SparseResidency};
    OpImageSparseFetch(IdResultType type, IdRef op0, IdRef op1,
                       std::optional<ImageOperands> op2 = std::nullopt)
        : spv_inst{Op::ImageSparseFetch, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> std::optional<ImageOperands> const & { return op2_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    std::optional<ImageOperands> op2_;
};
class OpImageSparseGather : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ImageSparseGather; }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::SparseResidency};
    OpImageSparseGather(IdResultType type, IdRef op0, IdRef op1, IdRef op2,
                        std::optional<ImageOperands> op3 = std::nullopt)
        : spv_inst{Op::ImageSparseGather, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> std::optional<ImageOperands> const & { return op3_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    std::optional<ImageOperands> op3_;
};
class OpImageSparseDrefGather : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::ImageSparseDrefGather;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::SparseResidency};
    OpImageSparseDrefGather(IdResultType type, IdRef op0, IdRef op1, IdRef op2,
                            std::optional<ImageOperands> op3 = std::nullopt)
        : spv_inst{Op::ImageSparseDrefGather, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> std::optional<ImageOperands> const & { return op3_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    std::optional<ImageOperands> op3_;
};
class OpImageSparseTexelsResident : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::ImageSparseTexelsResident;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::SparseResidency};
    OpImageSparseTexelsResident(IdResultType type, IdRef op0)
        : spv_inst{Op::ImageSparseTexelsResident, true}, type_(std::move(type)),
          op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpNoLine : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::NoLine; }
    OpNoLine() : spv_inst{Op::NoLine, false} {}

  private:
};
class OpAtomicFlagTestAndSet : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::AtomicFlagTestAndSet; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Kernel};
    OpAtomicFlagTestAndSet(IdResultType type, IdRef op0, IdScope op1, IdMemorySemantics op2)
        : spv_inst{Op::AtomicFlagTestAndSet, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdScope const & { return op1_; }
    inline auto op2() const -> IdMemorySemantics const & { return op2_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdScope op1_;
    IdMemorySemantics op2_;
};
class OpAtomicFlagClear : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::AtomicFlagClear; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Kernel};
    OpAtomicFlagClear(IdRef op0, IdScope op1, IdMemorySemantics op2)
        : spv_inst{Op::AtomicFlagClear, false}, op0_(std::move(op0)), op1_(std::move(op1)),
          op2_(std::move(op2)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdScope const & { return op1_; }
    inline auto op2() const -> IdMemorySemantics const & { return op2_; }

  private:
    IdRef op0_;
    IdScope op1_;
    IdMemorySemantics op2_;
};
class OpImageSparseRead : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ImageSparseRead; }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::SparseResidency};
    OpImageSparseRead(IdResultType type, IdRef op0, IdRef op1,
                      std::optional<ImageOperands> op2 = std::nullopt)
        : spv_inst{Op::ImageSparseRead, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> std::optional<ImageOperands> const & { return op2_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    std::optional<ImageOperands> op2_;
};
class OpSizeOf : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::SizeOf; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::Addresses};
    OpSizeOf(IdResultType type, IdRef op0)
        : spv_inst{Op::SizeOf, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpTypePipeStorage : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::TypePipeStorage; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::PipeStorage};
    OpTypePipeStorage() : spv_inst{Op::TypePipeStorage, true} {}

  private:
};
class OpConstantPipeStorage : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ConstantPipeStorage; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::PipeStorage};
    OpConstantPipeStorage(IdResultType type, LiteralInteger op0, LiteralInteger op1,
                          LiteralInteger op2)
        : spv_inst{Op::ConstantPipeStorage, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> LiteralInteger const & { return op0_; }
    inline auto op1() const -> LiteralInteger const & { return op1_; }
    inline auto op2() const -> LiteralInteger const & { return op2_; }

  private:
    IdResultType type_;
    LiteralInteger op0_;
    LiteralInteger op1_;
    LiteralInteger op2_;
};
class OpCreatePipeFromPipeStorage : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::CreatePipeFromPipeStorage;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::PipeStorage};
    OpCreatePipeFromPipeStorage(IdResultType type, IdRef op0)
        : spv_inst{Op::CreatePipeFromPipeStorage, true}, type_(std::move(type)),
          op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpGetKernelLocalSizeForSubgroupCount : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::GetKernelLocalSizeForSubgroupCount;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::SubgroupDispatch};
    OpGetKernelLocalSizeForSubgroupCount(IdResultType type, IdRef op0, IdRef op1, IdRef op2,
                                         IdRef op3, IdRef op4)
        : spv_inst{Op::GetKernelLocalSizeForSubgroupCount, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)),
          op4_(std::move(op4)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }
    inline auto op4() const -> IdRef const & { return op4_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    IdRef op3_;
    IdRef op4_;
};
class OpGetKernelMaxNumSubgroups : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::GetKernelMaxNumSubgroups;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::SubgroupDispatch};
    OpGetKernelMaxNumSubgroups(IdResultType type, IdRef op0, IdRef op1, IdRef op2, IdRef op3)
        : spv_inst{Op::GetKernelMaxNumSubgroups, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    IdRef op3_;
};
class OpTypeNamedBarrier : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::TypeNamedBarrier; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::NamedBarrier};
    OpTypeNamedBarrier() : spv_inst{Op::TypeNamedBarrier, true} {}

  private:
};
class OpNamedBarrierInitialize : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::NamedBarrierInitialize;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::NamedBarrier};
    OpNamedBarrierInitialize(IdResultType type, IdRef op0)
        : spv_inst{Op::NamedBarrierInitialize, true}, type_(std::move(type)), op0_(std::move(op0)) {
    }
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpMemoryNamedBarrier : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::MemoryNamedBarrier; }
    constexpr static std::array<Capability, 1> required_capabilities = {Capability::NamedBarrier};
    OpMemoryNamedBarrier(IdRef op0, IdScope op1, IdMemorySemantics op2)
        : spv_inst{Op::MemoryNamedBarrier, false}, op0_(std::move(op0)), op1_(std::move(op1)),
          op2_(std::move(op2)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdScope const & { return op1_; }
    inline auto op2() const -> IdMemorySemantics const & { return op2_; }

  private:
    IdRef op0_;
    IdScope op1_;
    IdMemorySemantics op2_;
};
class OpModuleProcessed : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ModuleProcessed; }
    OpModuleProcessed(LiteralString op0)
        : spv_inst{Op::ModuleProcessed, false}, op0_(std::move(op0)) {}
    inline auto op0() const -> LiteralString const & { return op0_; }

  private:
    LiteralString op0_;
};
class OpExecutionModeId : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::ExecutionModeId; }
    OpExecutionModeId(IdRef op0, ExecutionMode op1)
        : spv_inst{Op::ExecutionModeId, false}, op0_(std::move(op0)), op1_(std::move(op1)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> ExecutionMode const & { return op1_; }

  private:
    IdRef op0_;
    ExecutionMode op1_;
};
class OpDecorateId : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::DecorateId; }
    OpDecorateId(IdRef op0, Decoration op1)
        : spv_inst{Op::DecorateId, false}, op0_(std::move(op0)), op1_(std::move(op1)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> Decoration const & { return op1_; }

  private:
    IdRef op0_;
    Decoration op1_;
};
class OpGroupNonUniformElect : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GroupNonUniformElect; }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::GroupNonUniform};
    OpGroupNonUniformElect(IdResultType type, IdScope op0)
        : spv_inst{Op::GroupNonUniformElect, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }

  private:
    IdResultType type_;
    IdScope op0_;
};
class OpGroupNonUniformAll : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GroupNonUniformAll; }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::GroupNonUniformVote};
    OpGroupNonUniformAll(IdResultType type, IdScope op0, IdRef op1)
        : spv_inst{Op::GroupNonUniformAll, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdScope op0_;
    IdRef op1_;
};
class OpGroupNonUniformAny : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GroupNonUniformAny; }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::GroupNonUniformVote};
    OpGroupNonUniformAny(IdResultType type, IdScope op0, IdRef op1)
        : spv_inst{Op::GroupNonUniformAny, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdScope op0_;
    IdRef op1_;
};
class OpGroupNonUniformAllEqual : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::GroupNonUniformAllEqual;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::GroupNonUniformVote};
    OpGroupNonUniformAllEqual(IdResultType type, IdScope op0, IdRef op1)
        : spv_inst{Op::GroupNonUniformAllEqual, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdScope op0_;
    IdRef op1_;
};
class OpGroupNonUniformBroadcast : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::GroupNonUniformBroadcast;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::GroupNonUniformBallot};
    OpGroupNonUniformBroadcast(IdResultType type, IdScope op0, IdRef op1, IdRef op2)
        : spv_inst{Op::GroupNonUniformBroadcast, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }

  private:
    IdResultType type_;
    IdScope op0_;
    IdRef op1_;
    IdRef op2_;
};
class OpGroupNonUniformBroadcastFirst : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::GroupNonUniformBroadcastFirst;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::GroupNonUniformBallot};
    OpGroupNonUniformBroadcastFirst(IdResultType type, IdScope op0, IdRef op1)
        : spv_inst{Op::GroupNonUniformBroadcastFirst, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdScope op0_;
    IdRef op1_;
};
class OpGroupNonUniformBallot : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::GroupNonUniformBallot;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::GroupNonUniformBallot};
    OpGroupNonUniformBallot(IdResultType type, IdScope op0, IdRef op1)
        : spv_inst{Op::GroupNonUniformBallot, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdScope op0_;
    IdRef op1_;
};
class OpGroupNonUniformInverseBallot : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::GroupNonUniformInverseBallot;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::GroupNonUniformBallot};
    OpGroupNonUniformInverseBallot(IdResultType type, IdScope op0, IdRef op1)
        : spv_inst{Op::GroupNonUniformInverseBallot, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdScope op0_;
    IdRef op1_;
};
class OpGroupNonUniformBallotBitExtract : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::GroupNonUniformBallotBitExtract;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::GroupNonUniformBallot};
    OpGroupNonUniformBallotBitExtract(IdResultType type, IdScope op0, IdRef op1, IdRef op2)
        : spv_inst{Op::GroupNonUniformBallotBitExtract, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }

  private:
    IdResultType type_;
    IdScope op0_;
    IdRef op1_;
    IdRef op2_;
};
class OpGroupNonUniformBallotBitCount : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::GroupNonUniformBallotBitCount;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::GroupNonUniformBallot};
    OpGroupNonUniformBallotBitCount(IdResultType type, IdScope op0, GroupOperation op1, IdRef op2)
        : spv_inst{Op::GroupNonUniformBallotBitCount, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> GroupOperation const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }

  private:
    IdResultType type_;
    IdScope op0_;
    GroupOperation op1_;
    IdRef op2_;
};
class OpGroupNonUniformBallotFindLSB : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::GroupNonUniformBallotFindLSB;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::GroupNonUniformBallot};
    OpGroupNonUniformBallotFindLSB(IdResultType type, IdScope op0, IdRef op1)
        : spv_inst{Op::GroupNonUniformBallotFindLSB, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdScope op0_;
    IdRef op1_;
};
class OpGroupNonUniformBallotFindMSB : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::GroupNonUniformBallotFindMSB;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::GroupNonUniformBallot};
    OpGroupNonUniformBallotFindMSB(IdResultType type, IdScope op0, IdRef op1)
        : spv_inst{Op::GroupNonUniformBallotFindMSB, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdScope op0_;
    IdRef op1_;
};
class OpGroupNonUniformShuffle : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::GroupNonUniformShuffle;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::GroupNonUniformShuffle};
    OpGroupNonUniformShuffle(IdResultType type, IdScope op0, IdRef op1, IdRef op2)
        : spv_inst{Op::GroupNonUniformShuffle, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }

  private:
    IdResultType type_;
    IdScope op0_;
    IdRef op1_;
    IdRef op2_;
};
class OpGroupNonUniformShuffleXor : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::GroupNonUniformShuffleXor;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::GroupNonUniformShuffle};
    OpGroupNonUniformShuffleXor(IdResultType type, IdScope op0, IdRef op1, IdRef op2)
        : spv_inst{Op::GroupNonUniformShuffleXor, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }

  private:
    IdResultType type_;
    IdScope op0_;
    IdRef op1_;
    IdRef op2_;
};
class OpGroupNonUniformShuffleUp : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::GroupNonUniformShuffleUp;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::GroupNonUniformShuffleRelative};
    OpGroupNonUniformShuffleUp(IdResultType type, IdScope op0, IdRef op1, IdRef op2)
        : spv_inst{Op::GroupNonUniformShuffleUp, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }

  private:
    IdResultType type_;
    IdScope op0_;
    IdRef op1_;
    IdRef op2_;
};
class OpGroupNonUniformShuffleDown : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::GroupNonUniformShuffleDown;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::GroupNonUniformShuffleRelative};
    OpGroupNonUniformShuffleDown(IdResultType type, IdScope op0, IdRef op1, IdRef op2)
        : spv_inst{Op::GroupNonUniformShuffleDown, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }

  private:
    IdResultType type_;
    IdScope op0_;
    IdRef op1_;
    IdRef op2_;
};
class OpGroupNonUniformIAdd : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GroupNonUniformIAdd; }
    constexpr static std::array<Capability, 3> required_capabilities = {
        Capability::GroupNonUniformArithmetic, Capability::GroupNonUniformClustered,
        Capability::GroupNonUniformPartitionedNV};
    OpGroupNonUniformIAdd(IdResultType type, IdScope op0, GroupOperation op1, IdRef op2,
                          std::optional<IdRef> op3 = std::nullopt)
        : spv_inst{Op::GroupNonUniformIAdd, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> GroupOperation const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> std::optional<IdRef> const & { return op3_; }

  private:
    IdResultType type_;
    IdScope op0_;
    GroupOperation op1_;
    IdRef op2_;
    std::optional<IdRef> op3_;
};
class OpGroupNonUniformFAdd : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GroupNonUniformFAdd; }
    constexpr static std::array<Capability, 3> required_capabilities = {
        Capability::GroupNonUniformArithmetic, Capability::GroupNonUniformClustered,
        Capability::GroupNonUniformPartitionedNV};
    OpGroupNonUniformFAdd(IdResultType type, IdScope op0, GroupOperation op1, IdRef op2,
                          std::optional<IdRef> op3 = std::nullopt)
        : spv_inst{Op::GroupNonUniformFAdd, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> GroupOperation const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> std::optional<IdRef> const & { return op3_; }

  private:
    IdResultType type_;
    IdScope op0_;
    GroupOperation op1_;
    IdRef op2_;
    std::optional<IdRef> op3_;
};
class OpGroupNonUniformIMul : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GroupNonUniformIMul; }
    constexpr static std::array<Capability, 3> required_capabilities = {
        Capability::GroupNonUniformArithmetic, Capability::GroupNonUniformClustered,
        Capability::GroupNonUniformPartitionedNV};
    OpGroupNonUniformIMul(IdResultType type, IdScope op0, GroupOperation op1, IdRef op2,
                          std::optional<IdRef> op3 = std::nullopt)
        : spv_inst{Op::GroupNonUniformIMul, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> GroupOperation const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> std::optional<IdRef> const & { return op3_; }

  private:
    IdResultType type_;
    IdScope op0_;
    GroupOperation op1_;
    IdRef op2_;
    std::optional<IdRef> op3_;
};
class OpGroupNonUniformFMul : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GroupNonUniformFMul; }
    constexpr static std::array<Capability, 3> required_capabilities = {
        Capability::GroupNonUniformArithmetic, Capability::GroupNonUniformClustered,
        Capability::GroupNonUniformPartitionedNV};
    OpGroupNonUniformFMul(IdResultType type, IdScope op0, GroupOperation op1, IdRef op2,
                          std::optional<IdRef> op3 = std::nullopt)
        : spv_inst{Op::GroupNonUniformFMul, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> GroupOperation const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> std::optional<IdRef> const & { return op3_; }

  private:
    IdResultType type_;
    IdScope op0_;
    GroupOperation op1_;
    IdRef op2_;
    std::optional<IdRef> op3_;
};
class OpGroupNonUniformSMin : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GroupNonUniformSMin; }
    constexpr static std::array<Capability, 3> required_capabilities = {
        Capability::GroupNonUniformArithmetic, Capability::GroupNonUniformClustered,
        Capability::GroupNonUniformPartitionedNV};
    OpGroupNonUniformSMin(IdResultType type, IdScope op0, GroupOperation op1, IdRef op2,
                          std::optional<IdRef> op3 = std::nullopt)
        : spv_inst{Op::GroupNonUniformSMin, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> GroupOperation const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> std::optional<IdRef> const & { return op3_; }

  private:
    IdResultType type_;
    IdScope op0_;
    GroupOperation op1_;
    IdRef op2_;
    std::optional<IdRef> op3_;
};
class OpGroupNonUniformUMin : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GroupNonUniformUMin; }
    constexpr static std::array<Capability, 3> required_capabilities = {
        Capability::GroupNonUniformArithmetic, Capability::GroupNonUniformClustered,
        Capability::GroupNonUniformPartitionedNV};
    OpGroupNonUniformUMin(IdResultType type, IdScope op0, GroupOperation op1, IdRef op2,
                          std::optional<IdRef> op3 = std::nullopt)
        : spv_inst{Op::GroupNonUniformUMin, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> GroupOperation const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> std::optional<IdRef> const & { return op3_; }

  private:
    IdResultType type_;
    IdScope op0_;
    GroupOperation op1_;
    IdRef op2_;
    std::optional<IdRef> op3_;
};
class OpGroupNonUniformFMin : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GroupNonUniformFMin; }
    constexpr static std::array<Capability, 3> required_capabilities = {
        Capability::GroupNonUniformArithmetic, Capability::GroupNonUniformClustered,
        Capability::GroupNonUniformPartitionedNV};
    OpGroupNonUniformFMin(IdResultType type, IdScope op0, GroupOperation op1, IdRef op2,
                          std::optional<IdRef> op3 = std::nullopt)
        : spv_inst{Op::GroupNonUniformFMin, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> GroupOperation const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> std::optional<IdRef> const & { return op3_; }

  private:
    IdResultType type_;
    IdScope op0_;
    GroupOperation op1_;
    IdRef op2_;
    std::optional<IdRef> op3_;
};
class OpGroupNonUniformSMax : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GroupNonUniformSMax; }
    constexpr static std::array<Capability, 3> required_capabilities = {
        Capability::GroupNonUniformArithmetic, Capability::GroupNonUniformClustered,
        Capability::GroupNonUniformPartitionedNV};
    OpGroupNonUniformSMax(IdResultType type, IdScope op0, GroupOperation op1, IdRef op2,
                          std::optional<IdRef> op3 = std::nullopt)
        : spv_inst{Op::GroupNonUniformSMax, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> GroupOperation const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> std::optional<IdRef> const & { return op3_; }

  private:
    IdResultType type_;
    IdScope op0_;
    GroupOperation op1_;
    IdRef op2_;
    std::optional<IdRef> op3_;
};
class OpGroupNonUniformUMax : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GroupNonUniformUMax; }
    constexpr static std::array<Capability, 3> required_capabilities = {
        Capability::GroupNonUniformArithmetic, Capability::GroupNonUniformClustered,
        Capability::GroupNonUniformPartitionedNV};
    OpGroupNonUniformUMax(IdResultType type, IdScope op0, GroupOperation op1, IdRef op2,
                          std::optional<IdRef> op3 = std::nullopt)
        : spv_inst{Op::GroupNonUniformUMax, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> GroupOperation const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> std::optional<IdRef> const & { return op3_; }

  private:
    IdResultType type_;
    IdScope op0_;
    GroupOperation op1_;
    IdRef op2_;
    std::optional<IdRef> op3_;
};
class OpGroupNonUniformFMax : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::GroupNonUniformFMax; }
    constexpr static std::array<Capability, 3> required_capabilities = {
        Capability::GroupNonUniformArithmetic, Capability::GroupNonUniformClustered,
        Capability::GroupNonUniformPartitionedNV};
    OpGroupNonUniformFMax(IdResultType type, IdScope op0, GroupOperation op1, IdRef op2,
                          std::optional<IdRef> op3 = std::nullopt)
        : spv_inst{Op::GroupNonUniformFMax, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> GroupOperation const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> std::optional<IdRef> const & { return op3_; }

  private:
    IdResultType type_;
    IdScope op0_;
    GroupOperation op1_;
    IdRef op2_;
    std::optional<IdRef> op3_;
};
class OpGroupNonUniformBitwiseAnd : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::GroupNonUniformBitwiseAnd;
    }
    constexpr static std::array<Capability, 3> required_capabilities = {
        Capability::GroupNonUniformArithmetic, Capability::GroupNonUniformClustered,
        Capability::GroupNonUniformPartitionedNV};
    OpGroupNonUniformBitwiseAnd(IdResultType type, IdScope op0, GroupOperation op1, IdRef op2,
                                std::optional<IdRef> op3 = std::nullopt)
        : spv_inst{Op::GroupNonUniformBitwiseAnd, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> GroupOperation const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> std::optional<IdRef> const & { return op3_; }

  private:
    IdResultType type_;
    IdScope op0_;
    GroupOperation op1_;
    IdRef op2_;
    std::optional<IdRef> op3_;
};
class OpGroupNonUniformBitwiseOr : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::GroupNonUniformBitwiseOr;
    }
    constexpr static std::array<Capability, 3> required_capabilities = {
        Capability::GroupNonUniformArithmetic, Capability::GroupNonUniformClustered,
        Capability::GroupNonUniformPartitionedNV};
    OpGroupNonUniformBitwiseOr(IdResultType type, IdScope op0, GroupOperation op1, IdRef op2,
                               std::optional<IdRef> op3 = std::nullopt)
        : spv_inst{Op::GroupNonUniformBitwiseOr, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> GroupOperation const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> std::optional<IdRef> const & { return op3_; }

  private:
    IdResultType type_;
    IdScope op0_;
    GroupOperation op1_;
    IdRef op2_;
    std::optional<IdRef> op3_;
};
class OpGroupNonUniformBitwiseXor : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::GroupNonUniformBitwiseXor;
    }
    constexpr static std::array<Capability, 3> required_capabilities = {
        Capability::GroupNonUniformArithmetic, Capability::GroupNonUniformClustered,
        Capability::GroupNonUniformPartitionedNV};
    OpGroupNonUniformBitwiseXor(IdResultType type, IdScope op0, GroupOperation op1, IdRef op2,
                                std::optional<IdRef> op3 = std::nullopt)
        : spv_inst{Op::GroupNonUniformBitwiseXor, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> GroupOperation const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> std::optional<IdRef> const & { return op3_; }

  private:
    IdResultType type_;
    IdScope op0_;
    GroupOperation op1_;
    IdRef op2_;
    std::optional<IdRef> op3_;
};
class OpGroupNonUniformLogicalAnd : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::GroupNonUniformLogicalAnd;
    }
    constexpr static std::array<Capability, 3> required_capabilities = {
        Capability::GroupNonUniformArithmetic, Capability::GroupNonUniformClustered,
        Capability::GroupNonUniformPartitionedNV};
    OpGroupNonUniformLogicalAnd(IdResultType type, IdScope op0, GroupOperation op1, IdRef op2,
                                std::optional<IdRef> op3 = std::nullopt)
        : spv_inst{Op::GroupNonUniformLogicalAnd, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> GroupOperation const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> std::optional<IdRef> const & { return op3_; }

  private:
    IdResultType type_;
    IdScope op0_;
    GroupOperation op1_;
    IdRef op2_;
    std::optional<IdRef> op3_;
};
class OpGroupNonUniformLogicalOr : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::GroupNonUniformLogicalOr;
    }
    constexpr static std::array<Capability, 3> required_capabilities = {
        Capability::GroupNonUniformArithmetic, Capability::GroupNonUniformClustered,
        Capability::GroupNonUniformPartitionedNV};
    OpGroupNonUniformLogicalOr(IdResultType type, IdScope op0, GroupOperation op1, IdRef op2,
                               std::optional<IdRef> op3 = std::nullopt)
        : spv_inst{Op::GroupNonUniformLogicalOr, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> GroupOperation const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> std::optional<IdRef> const & { return op3_; }

  private:
    IdResultType type_;
    IdScope op0_;
    GroupOperation op1_;
    IdRef op2_;
    std::optional<IdRef> op3_;
};
class OpGroupNonUniformLogicalXor : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::GroupNonUniformLogicalXor;
    }
    constexpr static std::array<Capability, 3> required_capabilities = {
        Capability::GroupNonUniformArithmetic, Capability::GroupNonUniformClustered,
        Capability::GroupNonUniformPartitionedNV};
    OpGroupNonUniformLogicalXor(IdResultType type, IdScope op0, GroupOperation op1, IdRef op2,
                                std::optional<IdRef> op3 = std::nullopt)
        : spv_inst{Op::GroupNonUniformLogicalXor, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> GroupOperation const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> std::optional<IdRef> const & { return op3_; }

  private:
    IdResultType type_;
    IdScope op0_;
    GroupOperation op1_;
    IdRef op2_;
    std::optional<IdRef> op3_;
};
class OpGroupNonUniformQuadBroadcast : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::GroupNonUniformQuadBroadcast;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::GroupNonUniformQuad};
    OpGroupNonUniformQuadBroadcast(IdResultType type, IdScope op0, IdRef op1, IdRef op2)
        : spv_inst{Op::GroupNonUniformQuadBroadcast, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }

  private:
    IdResultType type_;
    IdScope op0_;
    IdRef op1_;
    IdRef op2_;
};
class OpGroupNonUniformQuadSwap : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::GroupNonUniformQuadSwap;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::GroupNonUniformQuad};
    OpGroupNonUniformQuadSwap(IdResultType type, IdScope op0, IdRef op1, IdRef op2)
        : spv_inst{Op::GroupNonUniformQuadSwap, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdScope const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }

  private:
    IdResultType type_;
    IdScope op0_;
    IdRef op1_;
    IdRef op2_;
};
class OpCopyLogical : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::CopyLogical; }
    OpCopyLogical(IdResultType type, IdRef op0)
        : spv_inst{Op::CopyLogical, true}, type_(std::move(type)), op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};
class OpPtrEqual : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::PtrEqual; }
    OpPtrEqual(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::PtrEqual, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpPtrNotEqual : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::PtrNotEqual; }
    OpPtrNotEqual(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::PtrNotEqual, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpPtrDiff : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) { return s.opcode() == Op::PtrDiff; }
    constexpr static std::array<Capability, 3> required_capabilities = {
        Capability::Addresses, Capability::VariablePointers,
        Capability::VariablePointersStorageBuffer};
    OpPtrDiff(IdResultType type, IdRef op0, IdRef op1)
        : spv_inst{Op::PtrDiff, true}, type_(std::move(type)), op0_(std::move(op0)),
          op1_(std::move(op1)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
};
class OpTypeCooperativeMatrixKHR : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::TypeCooperativeMatrixKHR;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::CooperativeMatrixKHR};
    OpTypeCooperativeMatrixKHR(IdRef op0, IdScope op1, IdRef op2, IdRef op3, IdRef op4)
        : spv_inst{Op::TypeCooperativeMatrixKHR, true}, op0_(std::move(op0)), op1_(std::move(op1)),
          op2_(std::move(op2)), op3_(std::move(op3)), op4_(std::move(op4)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdScope const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> IdRef const & { return op3_; }
    inline auto op4() const -> IdRef const & { return op4_; }

  private:
    IdRef op0_;
    IdScope op1_;
    IdRef op2_;
    IdRef op3_;
    IdRef op4_;
};
class OpCooperativeMatrixLoadKHR : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::CooperativeMatrixLoadKHR;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::CooperativeMatrixKHR};
    OpCooperativeMatrixLoadKHR(IdResultType type, IdRef op0, IdRef op1,
                               std::optional<IdRef> op2 = std::nullopt,
                               std::optional<MemoryAccess> op3 = std::nullopt,
                               std::optional<MemoryAccessAttr> op4 = std::nullopt)
        : spv_inst{Op::CooperativeMatrixLoadKHR, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)),
          op4_(std::move(op4)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> std::optional<IdRef> const & { return op2_; }
    inline auto op3() const -> std::optional<MemoryAccess> const & { return op3_; }
    inline auto op4() const -> std::optional<MemoryAccessAttr> const & { return op4_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    std::optional<IdRef> op2_;
    std::optional<MemoryAccess> op3_;
    std::optional<MemoryAccessAttr> op4_;
};
class OpCooperativeMatrixStoreKHR : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::CooperativeMatrixStoreKHR;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::CooperativeMatrixKHR};
    OpCooperativeMatrixStoreKHR(IdRef op0, IdRef op1, IdRef op2,
                                std::optional<IdRef> op3 = std::nullopt,
                                std::optional<MemoryAccess> op4 = std::nullopt,
                                std::optional<MemoryAccessAttr> op5 = std::nullopt)
        : spv_inst{Op::CooperativeMatrixStoreKHR, false}, op0_(std::move(op0)),
          op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)), op4_(std::move(op4)),
          op5_(std::move(op5)) {}
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> std::optional<IdRef> const & { return op3_; }
    inline auto op4() const -> std::optional<MemoryAccess> const & { return op4_; }
    inline auto op5() const -> std::optional<MemoryAccessAttr> const & { return op5_; }

  private:
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    std::optional<IdRef> op3_;
    std::optional<MemoryAccess> op4_;
    std::optional<MemoryAccessAttr> op5_;
};
class OpCooperativeMatrixMulAddKHR : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::CooperativeMatrixMulAddKHR;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::CooperativeMatrixKHR};
    OpCooperativeMatrixMulAddKHR(IdResultType type, IdRef op0, IdRef op1, IdRef op2,
                                 std::optional<CooperativeMatrixOperands> op3 = std::nullopt)
        : spv_inst{Op::CooperativeMatrixMulAddKHR, true}, type_(std::move(type)),
          op0_(std::move(op0)), op1_(std::move(op1)), op2_(std::move(op2)), op3_(std::move(op3)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }
    inline auto op1() const -> IdRef const & { return op1_; }
    inline auto op2() const -> IdRef const & { return op2_; }
    inline auto op3() const -> std::optional<CooperativeMatrixOperands> const & { return op3_; }

  private:
    IdResultType type_;
    IdRef op0_;
    IdRef op1_;
    IdRef op2_;
    std::optional<CooperativeMatrixOperands> op3_;
};
class OpCooperativeMatrixLengthKHR : public spv_inst {
  public:
    inline static bool classof(spv_inst const &s) {
        return s.opcode() == Op::CooperativeMatrixLengthKHR;
    }
    constexpr static std::array<Capability, 1> required_capabilities = {
        Capability::CooperativeMatrixKHR};
    OpCooperativeMatrixLengthKHR(IdResultType type, IdRef op0)
        : spv_inst{Op::CooperativeMatrixLengthKHR, true}, type_(std::move(type)),
          op0_(std::move(op0)) {}
    inline auto type() const -> IdResultType const & { return type_; }
    inline auto op0() const -> IdRef const & { return op0_; }

  private:
    IdResultType type_;
    IdRef op0_;
};

} // namespace tinytc::spv

#endif // GENERATED_INSTRUCTIONS_2024117_HPP
