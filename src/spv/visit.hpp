// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

// This file is generated
// Do not edit manually

#ifndef GENERATED_VISIT_20241111_HPP
#define GENERATED_VISIT_20241111_HPP

#include "defs.hpp"
#include "enums.hpp"
#include "instructions.hpp"

namespace tinytc::spv {

template <class... Ts> struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
template <typename Visitor> auto visit(Visitor &&visitor, spv_inst &inst) {
    switch (inst.opcode()) {
    case Op::Nop:
        return visitor(static_cast<OpNop &>(inst));
    case Op::Undef:
        return visitor(static_cast<OpUndef &>(inst));
    case Op::SourceContinued:
        return visitor(static_cast<OpSourceContinued &>(inst));
    case Op::Source:
        return visitor(static_cast<OpSource &>(inst));
    case Op::SourceExtension:
        return visitor(static_cast<OpSourceExtension &>(inst));
    case Op::Name:
        return visitor(static_cast<OpName &>(inst));
    case Op::MemberName:
        return visitor(static_cast<OpMemberName &>(inst));
    case Op::String:
        return visitor(static_cast<OpString &>(inst));
    case Op::Line:
        return visitor(static_cast<OpLine &>(inst));
    case Op::Extension:
        return visitor(static_cast<OpExtension &>(inst));
    case Op::ExtInstImport:
        return visitor(static_cast<OpExtInstImport &>(inst));
    case Op::ExtInst:
        return visitor(static_cast<OpExtInst &>(inst));
    case Op::MemoryModel:
        return visitor(static_cast<OpMemoryModel &>(inst));
    case Op::EntryPoint:
        return visitor(static_cast<OpEntryPoint &>(inst));
    case Op::ExecutionMode:
        return visitor(static_cast<OpExecutionMode &>(inst));
    case Op::Capability:
        return visitor(static_cast<OpCapability &>(inst));
    case Op::TypeVoid:
        return visitor(static_cast<OpTypeVoid &>(inst));
    case Op::TypeBool:
        return visitor(static_cast<OpTypeBool &>(inst));
    case Op::TypeInt:
        return visitor(static_cast<OpTypeInt &>(inst));
    case Op::TypeFloat:
        return visitor(static_cast<OpTypeFloat &>(inst));
    case Op::TypeVector:
        return visitor(static_cast<OpTypeVector &>(inst));
    case Op::TypeMatrix:
        return visitor(static_cast<OpTypeMatrix &>(inst));
    case Op::TypeImage:
        return visitor(static_cast<OpTypeImage &>(inst));
    case Op::TypeSampler:
        return visitor(static_cast<OpTypeSampler &>(inst));
    case Op::TypeSampledImage:
        return visitor(static_cast<OpTypeSampledImage &>(inst));
    case Op::TypeArray:
        return visitor(static_cast<OpTypeArray &>(inst));
    case Op::TypeRuntimeArray:
        return visitor(static_cast<OpTypeRuntimeArray &>(inst));
    case Op::TypeStruct:
        return visitor(static_cast<OpTypeStruct &>(inst));
    case Op::TypeOpaque:
        return visitor(static_cast<OpTypeOpaque &>(inst));
    case Op::TypePointer:
        return visitor(static_cast<OpTypePointer &>(inst));
    case Op::TypeFunction:
        return visitor(static_cast<OpTypeFunction &>(inst));
    case Op::TypeEvent:
        return visitor(static_cast<OpTypeEvent &>(inst));
    case Op::TypeDeviceEvent:
        return visitor(static_cast<OpTypeDeviceEvent &>(inst));
    case Op::TypeReserveId:
        return visitor(static_cast<OpTypeReserveId &>(inst));
    case Op::TypeQueue:
        return visitor(static_cast<OpTypeQueue &>(inst));
    case Op::TypePipe:
        return visitor(static_cast<OpTypePipe &>(inst));
    case Op::TypeForwardPointer:
        return visitor(static_cast<OpTypeForwardPointer &>(inst));
    case Op::ConstantTrue:
        return visitor(static_cast<OpConstantTrue &>(inst));
    case Op::ConstantFalse:
        return visitor(static_cast<OpConstantFalse &>(inst));
    case Op::Constant:
        return visitor(static_cast<OpConstant &>(inst));
    case Op::ConstantComposite:
        return visitor(static_cast<OpConstantComposite &>(inst));
    case Op::ConstantSampler:
        return visitor(static_cast<OpConstantSampler &>(inst));
    case Op::ConstantNull:
        return visitor(static_cast<OpConstantNull &>(inst));
    case Op::Function:
        return visitor(static_cast<OpFunction &>(inst));
    case Op::FunctionParameter:
        return visitor(static_cast<OpFunctionParameter &>(inst));
    case Op::FunctionEnd:
        return visitor(static_cast<OpFunctionEnd &>(inst));
    case Op::FunctionCall:
        return visitor(static_cast<OpFunctionCall &>(inst));
    case Op::Variable:
        return visitor(static_cast<OpVariable &>(inst));
    case Op::ImageTexelPointer:
        return visitor(static_cast<OpImageTexelPointer &>(inst));
    case Op::Load:
        return visitor(static_cast<OpLoad &>(inst));
    case Op::Store:
        return visitor(static_cast<OpStore &>(inst));
    case Op::CopyMemory:
        return visitor(static_cast<OpCopyMemory &>(inst));
    case Op::CopyMemorySized:
        return visitor(static_cast<OpCopyMemorySized &>(inst));
    case Op::AccessChain:
        return visitor(static_cast<OpAccessChain &>(inst));
    case Op::InBoundsAccessChain:
        return visitor(static_cast<OpInBoundsAccessChain &>(inst));
    case Op::PtrAccessChain:
        return visitor(static_cast<OpPtrAccessChain &>(inst));
    case Op::ArrayLength:
        return visitor(static_cast<OpArrayLength &>(inst));
    case Op::GenericPtrMemSemantics:
        return visitor(static_cast<OpGenericPtrMemSemantics &>(inst));
    case Op::InBoundsPtrAccessChain:
        return visitor(static_cast<OpInBoundsPtrAccessChain &>(inst));
    case Op::Decorate:
        return visitor(static_cast<OpDecorate &>(inst));
    case Op::MemberDecorate:
        return visitor(static_cast<OpMemberDecorate &>(inst));
    case Op::DecorationGroup:
        return visitor(static_cast<OpDecorationGroup &>(inst));
    case Op::GroupDecorate:
        return visitor(static_cast<OpGroupDecorate &>(inst));
    case Op::GroupMemberDecorate:
        return visitor(static_cast<OpGroupMemberDecorate &>(inst));
    case Op::VectorExtractDynamic:
        return visitor(static_cast<OpVectorExtractDynamic &>(inst));
    case Op::VectorInsertDynamic:
        return visitor(static_cast<OpVectorInsertDynamic &>(inst));
    case Op::VectorShuffle:
        return visitor(static_cast<OpVectorShuffle &>(inst));
    case Op::CompositeConstruct:
        return visitor(static_cast<OpCompositeConstruct &>(inst));
    case Op::CompositeExtract:
        return visitor(static_cast<OpCompositeExtract &>(inst));
    case Op::CompositeInsert:
        return visitor(static_cast<OpCompositeInsert &>(inst));
    case Op::CopyObject:
        return visitor(static_cast<OpCopyObject &>(inst));
    case Op::Transpose:
        return visitor(static_cast<OpTranspose &>(inst));
    case Op::SampledImage:
        return visitor(static_cast<OpSampledImage &>(inst));
    case Op::ImageSampleImplicitLod:
        return visitor(static_cast<OpImageSampleImplicitLod &>(inst));
    case Op::ImageSampleExplicitLod:
        return visitor(static_cast<OpImageSampleExplicitLod &>(inst));
    case Op::ImageSampleDrefImplicitLod:
        return visitor(static_cast<OpImageSampleDrefImplicitLod &>(inst));
    case Op::ImageSampleDrefExplicitLod:
        return visitor(static_cast<OpImageSampleDrefExplicitLod &>(inst));
    case Op::ImageSampleProjImplicitLod:
        return visitor(static_cast<OpImageSampleProjImplicitLod &>(inst));
    case Op::ImageSampleProjExplicitLod:
        return visitor(static_cast<OpImageSampleProjExplicitLod &>(inst));
    case Op::ImageSampleProjDrefImplicitLod:
        return visitor(static_cast<OpImageSampleProjDrefImplicitLod &>(inst));
    case Op::ImageSampleProjDrefExplicitLod:
        return visitor(static_cast<OpImageSampleProjDrefExplicitLod &>(inst));
    case Op::ImageFetch:
        return visitor(static_cast<OpImageFetch &>(inst));
    case Op::ImageGather:
        return visitor(static_cast<OpImageGather &>(inst));
    case Op::ImageDrefGather:
        return visitor(static_cast<OpImageDrefGather &>(inst));
    case Op::ImageRead:
        return visitor(static_cast<OpImageRead &>(inst));
    case Op::ImageWrite:
        return visitor(static_cast<OpImageWrite &>(inst));
    case Op::Image:
        return visitor(static_cast<OpImage &>(inst));
    case Op::ImageQueryFormat:
        return visitor(static_cast<OpImageQueryFormat &>(inst));
    case Op::ImageQueryOrder:
        return visitor(static_cast<OpImageQueryOrder &>(inst));
    case Op::ImageQuerySizeLod:
        return visitor(static_cast<OpImageQuerySizeLod &>(inst));
    case Op::ImageQuerySize:
        return visitor(static_cast<OpImageQuerySize &>(inst));
    case Op::ImageQueryLod:
        return visitor(static_cast<OpImageQueryLod &>(inst));
    case Op::ImageQueryLevels:
        return visitor(static_cast<OpImageQueryLevels &>(inst));
    case Op::ImageQuerySamples:
        return visitor(static_cast<OpImageQuerySamples &>(inst));
    case Op::ConvertFToU:
        return visitor(static_cast<OpConvertFToU &>(inst));
    case Op::ConvertFToS:
        return visitor(static_cast<OpConvertFToS &>(inst));
    case Op::ConvertSToF:
        return visitor(static_cast<OpConvertSToF &>(inst));
    case Op::ConvertUToF:
        return visitor(static_cast<OpConvertUToF &>(inst));
    case Op::UConvert:
        return visitor(static_cast<OpUConvert &>(inst));
    case Op::SConvert:
        return visitor(static_cast<OpSConvert &>(inst));
    case Op::FConvert:
        return visitor(static_cast<OpFConvert &>(inst));
    case Op::QuantizeToF16:
        return visitor(static_cast<OpQuantizeToF16 &>(inst));
    case Op::ConvertPtrToU:
        return visitor(static_cast<OpConvertPtrToU &>(inst));
    case Op::SatConvertSToU:
        return visitor(static_cast<OpSatConvertSToU &>(inst));
    case Op::SatConvertUToS:
        return visitor(static_cast<OpSatConvertUToS &>(inst));
    case Op::ConvertUToPtr:
        return visitor(static_cast<OpConvertUToPtr &>(inst));
    case Op::PtrCastToGeneric:
        return visitor(static_cast<OpPtrCastToGeneric &>(inst));
    case Op::GenericCastToPtr:
        return visitor(static_cast<OpGenericCastToPtr &>(inst));
    case Op::GenericCastToPtrExplicit:
        return visitor(static_cast<OpGenericCastToPtrExplicit &>(inst));
    case Op::Bitcast:
        return visitor(static_cast<OpBitcast &>(inst));
    case Op::SNegate:
        return visitor(static_cast<OpSNegate &>(inst));
    case Op::FNegate:
        return visitor(static_cast<OpFNegate &>(inst));
    case Op::IAdd:
        return visitor(static_cast<OpIAdd &>(inst));
    case Op::FAdd:
        return visitor(static_cast<OpFAdd &>(inst));
    case Op::ISub:
        return visitor(static_cast<OpISub &>(inst));
    case Op::FSub:
        return visitor(static_cast<OpFSub &>(inst));
    case Op::IMul:
        return visitor(static_cast<OpIMul &>(inst));
    case Op::FMul:
        return visitor(static_cast<OpFMul &>(inst));
    case Op::UDiv:
        return visitor(static_cast<OpUDiv &>(inst));
    case Op::SDiv:
        return visitor(static_cast<OpSDiv &>(inst));
    case Op::FDiv:
        return visitor(static_cast<OpFDiv &>(inst));
    case Op::UMod:
        return visitor(static_cast<OpUMod &>(inst));
    case Op::SRem:
        return visitor(static_cast<OpSRem &>(inst));
    case Op::SMod:
        return visitor(static_cast<OpSMod &>(inst));
    case Op::FRem:
        return visitor(static_cast<OpFRem &>(inst));
    case Op::FMod:
        return visitor(static_cast<OpFMod &>(inst));
    case Op::VectorTimesScalar:
        return visitor(static_cast<OpVectorTimesScalar &>(inst));
    case Op::MatrixTimesScalar:
        return visitor(static_cast<OpMatrixTimesScalar &>(inst));
    case Op::VectorTimesMatrix:
        return visitor(static_cast<OpVectorTimesMatrix &>(inst));
    case Op::MatrixTimesVector:
        return visitor(static_cast<OpMatrixTimesVector &>(inst));
    case Op::MatrixTimesMatrix:
        return visitor(static_cast<OpMatrixTimesMatrix &>(inst));
    case Op::OuterProduct:
        return visitor(static_cast<OpOuterProduct &>(inst));
    case Op::Dot:
        return visitor(static_cast<OpDot &>(inst));
    case Op::IAddCarry:
        return visitor(static_cast<OpIAddCarry &>(inst));
    case Op::ISubBorrow:
        return visitor(static_cast<OpISubBorrow &>(inst));
    case Op::UMulExtended:
        return visitor(static_cast<OpUMulExtended &>(inst));
    case Op::SMulExtended:
        return visitor(static_cast<OpSMulExtended &>(inst));
    case Op::Any:
        return visitor(static_cast<OpAny &>(inst));
    case Op::All:
        return visitor(static_cast<OpAll &>(inst));
    case Op::IsNan:
        return visitor(static_cast<OpIsNan &>(inst));
    case Op::IsInf:
        return visitor(static_cast<OpIsInf &>(inst));
    case Op::IsFinite:
        return visitor(static_cast<OpIsFinite &>(inst));
    case Op::IsNormal:
        return visitor(static_cast<OpIsNormal &>(inst));
    case Op::SignBitSet:
        return visitor(static_cast<OpSignBitSet &>(inst));
    case Op::LessOrGreater:
        return visitor(static_cast<OpLessOrGreater &>(inst));
    case Op::Ordered:
        return visitor(static_cast<OpOrdered &>(inst));
    case Op::Unordered:
        return visitor(static_cast<OpUnordered &>(inst));
    case Op::LogicalEqual:
        return visitor(static_cast<OpLogicalEqual &>(inst));
    case Op::LogicalNotEqual:
        return visitor(static_cast<OpLogicalNotEqual &>(inst));
    case Op::LogicalOr:
        return visitor(static_cast<OpLogicalOr &>(inst));
    case Op::LogicalAnd:
        return visitor(static_cast<OpLogicalAnd &>(inst));
    case Op::LogicalNot:
        return visitor(static_cast<OpLogicalNot &>(inst));
    case Op::Select:
        return visitor(static_cast<OpSelect &>(inst));
    case Op::IEqual:
        return visitor(static_cast<OpIEqual &>(inst));
    case Op::INotEqual:
        return visitor(static_cast<OpINotEqual &>(inst));
    case Op::UGreaterThan:
        return visitor(static_cast<OpUGreaterThan &>(inst));
    case Op::SGreaterThan:
        return visitor(static_cast<OpSGreaterThan &>(inst));
    case Op::UGreaterThanEqual:
        return visitor(static_cast<OpUGreaterThanEqual &>(inst));
    case Op::SGreaterThanEqual:
        return visitor(static_cast<OpSGreaterThanEqual &>(inst));
    case Op::ULessThan:
        return visitor(static_cast<OpULessThan &>(inst));
    case Op::SLessThan:
        return visitor(static_cast<OpSLessThan &>(inst));
    case Op::ULessThanEqual:
        return visitor(static_cast<OpULessThanEqual &>(inst));
    case Op::SLessThanEqual:
        return visitor(static_cast<OpSLessThanEqual &>(inst));
    case Op::FOrdEqual:
        return visitor(static_cast<OpFOrdEqual &>(inst));
    case Op::FUnordEqual:
        return visitor(static_cast<OpFUnordEqual &>(inst));
    case Op::FOrdNotEqual:
        return visitor(static_cast<OpFOrdNotEqual &>(inst));
    case Op::FUnordNotEqual:
        return visitor(static_cast<OpFUnordNotEqual &>(inst));
    case Op::FOrdLessThan:
        return visitor(static_cast<OpFOrdLessThan &>(inst));
    case Op::FUnordLessThan:
        return visitor(static_cast<OpFUnordLessThan &>(inst));
    case Op::FOrdGreaterThan:
        return visitor(static_cast<OpFOrdGreaterThan &>(inst));
    case Op::FUnordGreaterThan:
        return visitor(static_cast<OpFUnordGreaterThan &>(inst));
    case Op::FOrdLessThanEqual:
        return visitor(static_cast<OpFOrdLessThanEqual &>(inst));
    case Op::FUnordLessThanEqual:
        return visitor(static_cast<OpFUnordLessThanEqual &>(inst));
    case Op::FOrdGreaterThanEqual:
        return visitor(static_cast<OpFOrdGreaterThanEqual &>(inst));
    case Op::FUnordGreaterThanEqual:
        return visitor(static_cast<OpFUnordGreaterThanEqual &>(inst));
    case Op::ShiftRightLogical:
        return visitor(static_cast<OpShiftRightLogical &>(inst));
    case Op::ShiftRightArithmetic:
        return visitor(static_cast<OpShiftRightArithmetic &>(inst));
    case Op::ShiftLeftLogical:
        return visitor(static_cast<OpShiftLeftLogical &>(inst));
    case Op::BitwiseOr:
        return visitor(static_cast<OpBitwiseOr &>(inst));
    case Op::BitwiseXor:
        return visitor(static_cast<OpBitwiseXor &>(inst));
    case Op::BitwiseAnd:
        return visitor(static_cast<OpBitwiseAnd &>(inst));
    case Op::Not:
        return visitor(static_cast<OpNot &>(inst));
    case Op::BitFieldInsert:
        return visitor(static_cast<OpBitFieldInsert &>(inst));
    case Op::BitFieldSExtract:
        return visitor(static_cast<OpBitFieldSExtract &>(inst));
    case Op::BitFieldUExtract:
        return visitor(static_cast<OpBitFieldUExtract &>(inst));
    case Op::BitReverse:
        return visitor(static_cast<OpBitReverse &>(inst));
    case Op::BitCount:
        return visitor(static_cast<OpBitCount &>(inst));
    case Op::DPdx:
        return visitor(static_cast<OpDPdx &>(inst));
    case Op::DPdy:
        return visitor(static_cast<OpDPdy &>(inst));
    case Op::Fwidth:
        return visitor(static_cast<OpFwidth &>(inst));
    case Op::DPdxFine:
        return visitor(static_cast<OpDPdxFine &>(inst));
    case Op::DPdyFine:
        return visitor(static_cast<OpDPdyFine &>(inst));
    case Op::FwidthFine:
        return visitor(static_cast<OpFwidthFine &>(inst));
    case Op::DPdxCoarse:
        return visitor(static_cast<OpDPdxCoarse &>(inst));
    case Op::DPdyCoarse:
        return visitor(static_cast<OpDPdyCoarse &>(inst));
    case Op::FwidthCoarse:
        return visitor(static_cast<OpFwidthCoarse &>(inst));
    case Op::EmitVertex:
        return visitor(static_cast<OpEmitVertex &>(inst));
    case Op::EndPrimitive:
        return visitor(static_cast<OpEndPrimitive &>(inst));
    case Op::EmitStreamVertex:
        return visitor(static_cast<OpEmitStreamVertex &>(inst));
    case Op::EndStreamPrimitive:
        return visitor(static_cast<OpEndStreamPrimitive &>(inst));
    case Op::ControlBarrier:
        return visitor(static_cast<OpControlBarrier &>(inst));
    case Op::MemoryBarrier:
        return visitor(static_cast<OpMemoryBarrier &>(inst));
    case Op::AtomicLoad:
        return visitor(static_cast<OpAtomicLoad &>(inst));
    case Op::AtomicStore:
        return visitor(static_cast<OpAtomicStore &>(inst));
    case Op::AtomicExchange:
        return visitor(static_cast<OpAtomicExchange &>(inst));
    case Op::AtomicCompareExchange:
        return visitor(static_cast<OpAtomicCompareExchange &>(inst));
    case Op::AtomicCompareExchangeWeak:
        return visitor(static_cast<OpAtomicCompareExchangeWeak &>(inst));
    case Op::AtomicIIncrement:
        return visitor(static_cast<OpAtomicIIncrement &>(inst));
    case Op::AtomicIDecrement:
        return visitor(static_cast<OpAtomicIDecrement &>(inst));
    case Op::AtomicIAdd:
        return visitor(static_cast<OpAtomicIAdd &>(inst));
    case Op::AtomicISub:
        return visitor(static_cast<OpAtomicISub &>(inst));
    case Op::AtomicSMin:
        return visitor(static_cast<OpAtomicSMin &>(inst));
    case Op::AtomicUMin:
        return visitor(static_cast<OpAtomicUMin &>(inst));
    case Op::AtomicSMax:
        return visitor(static_cast<OpAtomicSMax &>(inst));
    case Op::AtomicUMax:
        return visitor(static_cast<OpAtomicUMax &>(inst));
    case Op::AtomicAnd:
        return visitor(static_cast<OpAtomicAnd &>(inst));
    case Op::AtomicOr:
        return visitor(static_cast<OpAtomicOr &>(inst));
    case Op::AtomicXor:
        return visitor(static_cast<OpAtomicXor &>(inst));
    case Op::Phi:
        return visitor(static_cast<OpPhi &>(inst));
    case Op::LoopMerge:
        return visitor(static_cast<OpLoopMerge &>(inst));
    case Op::SelectionMerge:
        return visitor(static_cast<OpSelectionMerge &>(inst));
    case Op::Label:
        return visitor(static_cast<OpLabel &>(inst));
    case Op::Branch:
        return visitor(static_cast<OpBranch &>(inst));
    case Op::BranchConditional:
        return visitor(static_cast<OpBranchConditional &>(inst));
    case Op::Switch:
        return visitor(static_cast<OpSwitch &>(inst));
    case Op::Kill:
        return visitor(static_cast<OpKill &>(inst));
    case Op::Return:
        return visitor(static_cast<OpReturn &>(inst));
    case Op::ReturnValue:
        return visitor(static_cast<OpReturnValue &>(inst));
    case Op::Unreachable:
        return visitor(static_cast<OpUnreachable &>(inst));
    case Op::LifetimeStart:
        return visitor(static_cast<OpLifetimeStart &>(inst));
    case Op::LifetimeStop:
        return visitor(static_cast<OpLifetimeStop &>(inst));
    case Op::GroupAsyncCopy:
        return visitor(static_cast<OpGroupAsyncCopy &>(inst));
    case Op::GroupWaitEvents:
        return visitor(static_cast<OpGroupWaitEvents &>(inst));
    case Op::GroupAll:
        return visitor(static_cast<OpGroupAll &>(inst));
    case Op::GroupAny:
        return visitor(static_cast<OpGroupAny &>(inst));
    case Op::GroupBroadcast:
        return visitor(static_cast<OpGroupBroadcast &>(inst));
    case Op::GroupIAdd:
        return visitor(static_cast<OpGroupIAdd &>(inst));
    case Op::GroupFAdd:
        return visitor(static_cast<OpGroupFAdd &>(inst));
    case Op::GroupFMin:
        return visitor(static_cast<OpGroupFMin &>(inst));
    case Op::GroupUMin:
        return visitor(static_cast<OpGroupUMin &>(inst));
    case Op::GroupSMin:
        return visitor(static_cast<OpGroupSMin &>(inst));
    case Op::GroupFMax:
        return visitor(static_cast<OpGroupFMax &>(inst));
    case Op::GroupUMax:
        return visitor(static_cast<OpGroupUMax &>(inst));
    case Op::GroupSMax:
        return visitor(static_cast<OpGroupSMax &>(inst));
    case Op::ReadPipe:
        return visitor(static_cast<OpReadPipe &>(inst));
    case Op::WritePipe:
        return visitor(static_cast<OpWritePipe &>(inst));
    case Op::ReservedReadPipe:
        return visitor(static_cast<OpReservedReadPipe &>(inst));
    case Op::ReservedWritePipe:
        return visitor(static_cast<OpReservedWritePipe &>(inst));
    case Op::ReserveReadPipePackets:
        return visitor(static_cast<OpReserveReadPipePackets &>(inst));
    case Op::ReserveWritePipePackets:
        return visitor(static_cast<OpReserveWritePipePackets &>(inst));
    case Op::CommitReadPipe:
        return visitor(static_cast<OpCommitReadPipe &>(inst));
    case Op::CommitWritePipe:
        return visitor(static_cast<OpCommitWritePipe &>(inst));
    case Op::IsValidReserveId:
        return visitor(static_cast<OpIsValidReserveId &>(inst));
    case Op::GetNumPipePackets:
        return visitor(static_cast<OpGetNumPipePackets &>(inst));
    case Op::GetMaxPipePackets:
        return visitor(static_cast<OpGetMaxPipePackets &>(inst));
    case Op::GroupReserveReadPipePackets:
        return visitor(static_cast<OpGroupReserveReadPipePackets &>(inst));
    case Op::GroupReserveWritePipePackets:
        return visitor(static_cast<OpGroupReserveWritePipePackets &>(inst));
    case Op::GroupCommitReadPipe:
        return visitor(static_cast<OpGroupCommitReadPipe &>(inst));
    case Op::GroupCommitWritePipe:
        return visitor(static_cast<OpGroupCommitWritePipe &>(inst));
    case Op::EnqueueMarker:
        return visitor(static_cast<OpEnqueueMarker &>(inst));
    case Op::EnqueueKernel:
        return visitor(static_cast<OpEnqueueKernel &>(inst));
    case Op::GetKernelNDrangeSubGroupCount:
        return visitor(static_cast<OpGetKernelNDrangeSubGroupCount &>(inst));
    case Op::GetKernelNDrangeMaxSubGroupSize:
        return visitor(static_cast<OpGetKernelNDrangeMaxSubGroupSize &>(inst));
    case Op::GetKernelWorkGroupSize:
        return visitor(static_cast<OpGetKernelWorkGroupSize &>(inst));
    case Op::GetKernelPreferredWorkGroupSizeMultiple:
        return visitor(static_cast<OpGetKernelPreferredWorkGroupSizeMultiple &>(inst));
    case Op::RetainEvent:
        return visitor(static_cast<OpRetainEvent &>(inst));
    case Op::ReleaseEvent:
        return visitor(static_cast<OpReleaseEvent &>(inst));
    case Op::CreateUserEvent:
        return visitor(static_cast<OpCreateUserEvent &>(inst));
    case Op::IsValidEvent:
        return visitor(static_cast<OpIsValidEvent &>(inst));
    case Op::SetUserEventStatus:
        return visitor(static_cast<OpSetUserEventStatus &>(inst));
    case Op::CaptureEventProfilingInfo:
        return visitor(static_cast<OpCaptureEventProfilingInfo &>(inst));
    case Op::GetDefaultQueue:
        return visitor(static_cast<OpGetDefaultQueue &>(inst));
    case Op::BuildNDRange:
        return visitor(static_cast<OpBuildNDRange &>(inst));
    case Op::ImageSparseSampleImplicitLod:
        return visitor(static_cast<OpImageSparseSampleImplicitLod &>(inst));
    case Op::ImageSparseSampleExplicitLod:
        return visitor(static_cast<OpImageSparseSampleExplicitLod &>(inst));
    case Op::ImageSparseSampleDrefImplicitLod:
        return visitor(static_cast<OpImageSparseSampleDrefImplicitLod &>(inst));
    case Op::ImageSparseSampleDrefExplicitLod:
        return visitor(static_cast<OpImageSparseSampleDrefExplicitLod &>(inst));
    case Op::ImageSparseSampleProjImplicitLod:
        return visitor(static_cast<OpImageSparseSampleProjImplicitLod &>(inst));
    case Op::ImageSparseSampleProjExplicitLod:
        return visitor(static_cast<OpImageSparseSampleProjExplicitLod &>(inst));
    case Op::ImageSparseSampleProjDrefImplicitLod:
        return visitor(static_cast<OpImageSparseSampleProjDrefImplicitLod &>(inst));
    case Op::ImageSparseSampleProjDrefExplicitLod:
        return visitor(static_cast<OpImageSparseSampleProjDrefExplicitLod &>(inst));
    case Op::ImageSparseFetch:
        return visitor(static_cast<OpImageSparseFetch &>(inst));
    case Op::ImageSparseGather:
        return visitor(static_cast<OpImageSparseGather &>(inst));
    case Op::ImageSparseDrefGather:
        return visitor(static_cast<OpImageSparseDrefGather &>(inst));
    case Op::ImageSparseTexelsResident:
        return visitor(static_cast<OpImageSparseTexelsResident &>(inst));
    case Op::NoLine:
        return visitor(static_cast<OpNoLine &>(inst));
    case Op::AtomicFlagTestAndSet:
        return visitor(static_cast<OpAtomicFlagTestAndSet &>(inst));
    case Op::AtomicFlagClear:
        return visitor(static_cast<OpAtomicFlagClear &>(inst));
    case Op::ImageSparseRead:
        return visitor(static_cast<OpImageSparseRead &>(inst));
    case Op::SizeOf:
        return visitor(static_cast<OpSizeOf &>(inst));
    case Op::TypePipeStorage:
        return visitor(static_cast<OpTypePipeStorage &>(inst));
    case Op::ConstantPipeStorage:
        return visitor(static_cast<OpConstantPipeStorage &>(inst));
    case Op::CreatePipeFromPipeStorage:
        return visitor(static_cast<OpCreatePipeFromPipeStorage &>(inst));
    case Op::GetKernelLocalSizeForSubgroupCount:
        return visitor(static_cast<OpGetKernelLocalSizeForSubgroupCount &>(inst));
    case Op::GetKernelMaxNumSubgroups:
        return visitor(static_cast<OpGetKernelMaxNumSubgroups &>(inst));
    case Op::TypeNamedBarrier:
        return visitor(static_cast<OpTypeNamedBarrier &>(inst));
    case Op::NamedBarrierInitialize:
        return visitor(static_cast<OpNamedBarrierInitialize &>(inst));
    case Op::MemoryNamedBarrier:
        return visitor(static_cast<OpMemoryNamedBarrier &>(inst));
    case Op::ModuleProcessed:
        return visitor(static_cast<OpModuleProcessed &>(inst));
    case Op::ExecutionModeId:
        return visitor(static_cast<OpExecutionModeId &>(inst));
    case Op::DecorateId:
        return visitor(static_cast<OpDecorateId &>(inst));
    case Op::GroupNonUniformElect:
        return visitor(static_cast<OpGroupNonUniformElect &>(inst));
    case Op::GroupNonUniformAll:
        return visitor(static_cast<OpGroupNonUniformAll &>(inst));
    case Op::GroupNonUniformAny:
        return visitor(static_cast<OpGroupNonUniformAny &>(inst));
    case Op::GroupNonUniformAllEqual:
        return visitor(static_cast<OpGroupNonUniformAllEqual &>(inst));
    case Op::GroupNonUniformBroadcast:
        return visitor(static_cast<OpGroupNonUniformBroadcast &>(inst));
    case Op::GroupNonUniformBroadcastFirst:
        return visitor(static_cast<OpGroupNonUniformBroadcastFirst &>(inst));
    case Op::GroupNonUniformBallot:
        return visitor(static_cast<OpGroupNonUniformBallot &>(inst));
    case Op::GroupNonUniformInverseBallot:
        return visitor(static_cast<OpGroupNonUniformInverseBallot &>(inst));
    case Op::GroupNonUniformBallotBitExtract:
        return visitor(static_cast<OpGroupNonUniformBallotBitExtract &>(inst));
    case Op::GroupNonUniformBallotBitCount:
        return visitor(static_cast<OpGroupNonUniformBallotBitCount &>(inst));
    case Op::GroupNonUniformBallotFindLSB:
        return visitor(static_cast<OpGroupNonUniformBallotFindLSB &>(inst));
    case Op::GroupNonUniformBallotFindMSB:
        return visitor(static_cast<OpGroupNonUniformBallotFindMSB &>(inst));
    case Op::GroupNonUniformShuffle:
        return visitor(static_cast<OpGroupNonUniformShuffle &>(inst));
    case Op::GroupNonUniformShuffleXor:
        return visitor(static_cast<OpGroupNonUniformShuffleXor &>(inst));
    case Op::GroupNonUniformShuffleUp:
        return visitor(static_cast<OpGroupNonUniformShuffleUp &>(inst));
    case Op::GroupNonUniformShuffleDown:
        return visitor(static_cast<OpGroupNonUniformShuffleDown &>(inst));
    case Op::GroupNonUniformIAdd:
        return visitor(static_cast<OpGroupNonUniformIAdd &>(inst));
    case Op::GroupNonUniformFAdd:
        return visitor(static_cast<OpGroupNonUniformFAdd &>(inst));
    case Op::GroupNonUniformIMul:
        return visitor(static_cast<OpGroupNonUniformIMul &>(inst));
    case Op::GroupNonUniformFMul:
        return visitor(static_cast<OpGroupNonUniformFMul &>(inst));
    case Op::GroupNonUniformSMin:
        return visitor(static_cast<OpGroupNonUniformSMin &>(inst));
    case Op::GroupNonUniformUMin:
        return visitor(static_cast<OpGroupNonUniformUMin &>(inst));
    case Op::GroupNonUniformFMin:
        return visitor(static_cast<OpGroupNonUniformFMin &>(inst));
    case Op::GroupNonUniformSMax:
        return visitor(static_cast<OpGroupNonUniformSMax &>(inst));
    case Op::GroupNonUniformUMax:
        return visitor(static_cast<OpGroupNonUniformUMax &>(inst));
    case Op::GroupNonUniformFMax:
        return visitor(static_cast<OpGroupNonUniformFMax &>(inst));
    case Op::GroupNonUniformBitwiseAnd:
        return visitor(static_cast<OpGroupNonUniformBitwiseAnd &>(inst));
    case Op::GroupNonUniformBitwiseOr:
        return visitor(static_cast<OpGroupNonUniformBitwiseOr &>(inst));
    case Op::GroupNonUniformBitwiseXor:
        return visitor(static_cast<OpGroupNonUniformBitwiseXor &>(inst));
    case Op::GroupNonUniformLogicalAnd:
        return visitor(static_cast<OpGroupNonUniformLogicalAnd &>(inst));
    case Op::GroupNonUniformLogicalOr:
        return visitor(static_cast<OpGroupNonUniformLogicalOr &>(inst));
    case Op::GroupNonUniformLogicalXor:
        return visitor(static_cast<OpGroupNonUniformLogicalXor &>(inst));
    case Op::GroupNonUniformQuadBroadcast:
        return visitor(static_cast<OpGroupNonUniformQuadBroadcast &>(inst));
    case Op::GroupNonUniformQuadSwap:
        return visitor(static_cast<OpGroupNonUniformQuadSwap &>(inst));
    case Op::CopyLogical:
        return visitor(static_cast<OpCopyLogical &>(inst));
    case Op::PtrEqual:
        return visitor(static_cast<OpPtrEqual &>(inst));
    case Op::PtrNotEqual:
        return visitor(static_cast<OpPtrNotEqual &>(inst));
    case Op::PtrDiff:
        return visitor(static_cast<OpPtrDiff &>(inst));
    case Op::TypeCooperativeMatrixKHR:
        return visitor(static_cast<OpTypeCooperativeMatrixKHR &>(inst));
    case Op::CooperativeMatrixLoadKHR:
        return visitor(static_cast<OpCooperativeMatrixLoadKHR &>(inst));
    case Op::CooperativeMatrixStoreKHR:
        return visitor(static_cast<OpCooperativeMatrixStoreKHR &>(inst));
    case Op::CooperativeMatrixMulAddKHR:
        return visitor(static_cast<OpCooperativeMatrixMulAddKHR &>(inst));
    case Op::CooperativeMatrixLengthKHR:
        return visitor(static_cast<OpCooperativeMatrixLengthKHR &>(inst));
    case Op::AtomicFMinEXT:
        return visitor(static_cast<OpAtomicFMinEXT &>(inst));
    case Op::AtomicFMaxEXT:
        return visitor(static_cast<OpAtomicFMaxEXT &>(inst));
    case Op::AtomicFAddEXT:
        return visitor(static_cast<OpAtomicFAddEXT &>(inst));
    }
    throw internal_compiler_error();
}

template <typename Visitor> auto visit(Visitor &&visitor, spv_inst const &inst) {
    switch (inst.opcode()) {
    case Op::Nop:
        return visitor(static_cast<OpNop const &>(inst));
    case Op::Undef:
        return visitor(static_cast<OpUndef const &>(inst));
    case Op::SourceContinued:
        return visitor(static_cast<OpSourceContinued const &>(inst));
    case Op::Source:
        return visitor(static_cast<OpSource const &>(inst));
    case Op::SourceExtension:
        return visitor(static_cast<OpSourceExtension const &>(inst));
    case Op::Name:
        return visitor(static_cast<OpName const &>(inst));
    case Op::MemberName:
        return visitor(static_cast<OpMemberName const &>(inst));
    case Op::String:
        return visitor(static_cast<OpString const &>(inst));
    case Op::Line:
        return visitor(static_cast<OpLine const &>(inst));
    case Op::Extension:
        return visitor(static_cast<OpExtension const &>(inst));
    case Op::ExtInstImport:
        return visitor(static_cast<OpExtInstImport const &>(inst));
    case Op::ExtInst:
        return visitor(static_cast<OpExtInst const &>(inst));
    case Op::MemoryModel:
        return visitor(static_cast<OpMemoryModel const &>(inst));
    case Op::EntryPoint:
        return visitor(static_cast<OpEntryPoint const &>(inst));
    case Op::ExecutionMode:
        return visitor(static_cast<OpExecutionMode const &>(inst));
    case Op::Capability:
        return visitor(static_cast<OpCapability const &>(inst));
    case Op::TypeVoid:
        return visitor(static_cast<OpTypeVoid const &>(inst));
    case Op::TypeBool:
        return visitor(static_cast<OpTypeBool const &>(inst));
    case Op::TypeInt:
        return visitor(static_cast<OpTypeInt const &>(inst));
    case Op::TypeFloat:
        return visitor(static_cast<OpTypeFloat const &>(inst));
    case Op::TypeVector:
        return visitor(static_cast<OpTypeVector const &>(inst));
    case Op::TypeMatrix:
        return visitor(static_cast<OpTypeMatrix const &>(inst));
    case Op::TypeImage:
        return visitor(static_cast<OpTypeImage const &>(inst));
    case Op::TypeSampler:
        return visitor(static_cast<OpTypeSampler const &>(inst));
    case Op::TypeSampledImage:
        return visitor(static_cast<OpTypeSampledImage const &>(inst));
    case Op::TypeArray:
        return visitor(static_cast<OpTypeArray const &>(inst));
    case Op::TypeRuntimeArray:
        return visitor(static_cast<OpTypeRuntimeArray const &>(inst));
    case Op::TypeStruct:
        return visitor(static_cast<OpTypeStruct const &>(inst));
    case Op::TypeOpaque:
        return visitor(static_cast<OpTypeOpaque const &>(inst));
    case Op::TypePointer:
        return visitor(static_cast<OpTypePointer const &>(inst));
    case Op::TypeFunction:
        return visitor(static_cast<OpTypeFunction const &>(inst));
    case Op::TypeEvent:
        return visitor(static_cast<OpTypeEvent const &>(inst));
    case Op::TypeDeviceEvent:
        return visitor(static_cast<OpTypeDeviceEvent const &>(inst));
    case Op::TypeReserveId:
        return visitor(static_cast<OpTypeReserveId const &>(inst));
    case Op::TypeQueue:
        return visitor(static_cast<OpTypeQueue const &>(inst));
    case Op::TypePipe:
        return visitor(static_cast<OpTypePipe const &>(inst));
    case Op::TypeForwardPointer:
        return visitor(static_cast<OpTypeForwardPointer const &>(inst));
    case Op::ConstantTrue:
        return visitor(static_cast<OpConstantTrue const &>(inst));
    case Op::ConstantFalse:
        return visitor(static_cast<OpConstantFalse const &>(inst));
    case Op::Constant:
        return visitor(static_cast<OpConstant const &>(inst));
    case Op::ConstantComposite:
        return visitor(static_cast<OpConstantComposite const &>(inst));
    case Op::ConstantSampler:
        return visitor(static_cast<OpConstantSampler const &>(inst));
    case Op::ConstantNull:
        return visitor(static_cast<OpConstantNull const &>(inst));
    case Op::Function:
        return visitor(static_cast<OpFunction const &>(inst));
    case Op::FunctionParameter:
        return visitor(static_cast<OpFunctionParameter const &>(inst));
    case Op::FunctionEnd:
        return visitor(static_cast<OpFunctionEnd const &>(inst));
    case Op::FunctionCall:
        return visitor(static_cast<OpFunctionCall const &>(inst));
    case Op::Variable:
        return visitor(static_cast<OpVariable const &>(inst));
    case Op::ImageTexelPointer:
        return visitor(static_cast<OpImageTexelPointer const &>(inst));
    case Op::Load:
        return visitor(static_cast<OpLoad const &>(inst));
    case Op::Store:
        return visitor(static_cast<OpStore const &>(inst));
    case Op::CopyMemory:
        return visitor(static_cast<OpCopyMemory const &>(inst));
    case Op::CopyMemorySized:
        return visitor(static_cast<OpCopyMemorySized const &>(inst));
    case Op::AccessChain:
        return visitor(static_cast<OpAccessChain const &>(inst));
    case Op::InBoundsAccessChain:
        return visitor(static_cast<OpInBoundsAccessChain const &>(inst));
    case Op::PtrAccessChain:
        return visitor(static_cast<OpPtrAccessChain const &>(inst));
    case Op::ArrayLength:
        return visitor(static_cast<OpArrayLength const &>(inst));
    case Op::GenericPtrMemSemantics:
        return visitor(static_cast<OpGenericPtrMemSemantics const &>(inst));
    case Op::InBoundsPtrAccessChain:
        return visitor(static_cast<OpInBoundsPtrAccessChain const &>(inst));
    case Op::Decorate:
        return visitor(static_cast<OpDecorate const &>(inst));
    case Op::MemberDecorate:
        return visitor(static_cast<OpMemberDecorate const &>(inst));
    case Op::DecorationGroup:
        return visitor(static_cast<OpDecorationGroup const &>(inst));
    case Op::GroupDecorate:
        return visitor(static_cast<OpGroupDecorate const &>(inst));
    case Op::GroupMemberDecorate:
        return visitor(static_cast<OpGroupMemberDecorate const &>(inst));
    case Op::VectorExtractDynamic:
        return visitor(static_cast<OpVectorExtractDynamic const &>(inst));
    case Op::VectorInsertDynamic:
        return visitor(static_cast<OpVectorInsertDynamic const &>(inst));
    case Op::VectorShuffle:
        return visitor(static_cast<OpVectorShuffle const &>(inst));
    case Op::CompositeConstruct:
        return visitor(static_cast<OpCompositeConstruct const &>(inst));
    case Op::CompositeExtract:
        return visitor(static_cast<OpCompositeExtract const &>(inst));
    case Op::CompositeInsert:
        return visitor(static_cast<OpCompositeInsert const &>(inst));
    case Op::CopyObject:
        return visitor(static_cast<OpCopyObject const &>(inst));
    case Op::Transpose:
        return visitor(static_cast<OpTranspose const &>(inst));
    case Op::SampledImage:
        return visitor(static_cast<OpSampledImage const &>(inst));
    case Op::ImageSampleImplicitLod:
        return visitor(static_cast<OpImageSampleImplicitLod const &>(inst));
    case Op::ImageSampleExplicitLod:
        return visitor(static_cast<OpImageSampleExplicitLod const &>(inst));
    case Op::ImageSampleDrefImplicitLod:
        return visitor(static_cast<OpImageSampleDrefImplicitLod const &>(inst));
    case Op::ImageSampleDrefExplicitLod:
        return visitor(static_cast<OpImageSampleDrefExplicitLod const &>(inst));
    case Op::ImageSampleProjImplicitLod:
        return visitor(static_cast<OpImageSampleProjImplicitLod const &>(inst));
    case Op::ImageSampleProjExplicitLod:
        return visitor(static_cast<OpImageSampleProjExplicitLod const &>(inst));
    case Op::ImageSampleProjDrefImplicitLod:
        return visitor(static_cast<OpImageSampleProjDrefImplicitLod const &>(inst));
    case Op::ImageSampleProjDrefExplicitLod:
        return visitor(static_cast<OpImageSampleProjDrefExplicitLod const &>(inst));
    case Op::ImageFetch:
        return visitor(static_cast<OpImageFetch const &>(inst));
    case Op::ImageGather:
        return visitor(static_cast<OpImageGather const &>(inst));
    case Op::ImageDrefGather:
        return visitor(static_cast<OpImageDrefGather const &>(inst));
    case Op::ImageRead:
        return visitor(static_cast<OpImageRead const &>(inst));
    case Op::ImageWrite:
        return visitor(static_cast<OpImageWrite const &>(inst));
    case Op::Image:
        return visitor(static_cast<OpImage const &>(inst));
    case Op::ImageQueryFormat:
        return visitor(static_cast<OpImageQueryFormat const &>(inst));
    case Op::ImageQueryOrder:
        return visitor(static_cast<OpImageQueryOrder const &>(inst));
    case Op::ImageQuerySizeLod:
        return visitor(static_cast<OpImageQuerySizeLod const &>(inst));
    case Op::ImageQuerySize:
        return visitor(static_cast<OpImageQuerySize const &>(inst));
    case Op::ImageQueryLod:
        return visitor(static_cast<OpImageQueryLod const &>(inst));
    case Op::ImageQueryLevels:
        return visitor(static_cast<OpImageQueryLevels const &>(inst));
    case Op::ImageQuerySamples:
        return visitor(static_cast<OpImageQuerySamples const &>(inst));
    case Op::ConvertFToU:
        return visitor(static_cast<OpConvertFToU const &>(inst));
    case Op::ConvertFToS:
        return visitor(static_cast<OpConvertFToS const &>(inst));
    case Op::ConvertSToF:
        return visitor(static_cast<OpConvertSToF const &>(inst));
    case Op::ConvertUToF:
        return visitor(static_cast<OpConvertUToF const &>(inst));
    case Op::UConvert:
        return visitor(static_cast<OpUConvert const &>(inst));
    case Op::SConvert:
        return visitor(static_cast<OpSConvert const &>(inst));
    case Op::FConvert:
        return visitor(static_cast<OpFConvert const &>(inst));
    case Op::QuantizeToF16:
        return visitor(static_cast<OpQuantizeToF16 const &>(inst));
    case Op::ConvertPtrToU:
        return visitor(static_cast<OpConvertPtrToU const &>(inst));
    case Op::SatConvertSToU:
        return visitor(static_cast<OpSatConvertSToU const &>(inst));
    case Op::SatConvertUToS:
        return visitor(static_cast<OpSatConvertUToS const &>(inst));
    case Op::ConvertUToPtr:
        return visitor(static_cast<OpConvertUToPtr const &>(inst));
    case Op::PtrCastToGeneric:
        return visitor(static_cast<OpPtrCastToGeneric const &>(inst));
    case Op::GenericCastToPtr:
        return visitor(static_cast<OpGenericCastToPtr const &>(inst));
    case Op::GenericCastToPtrExplicit:
        return visitor(static_cast<OpGenericCastToPtrExplicit const &>(inst));
    case Op::Bitcast:
        return visitor(static_cast<OpBitcast const &>(inst));
    case Op::SNegate:
        return visitor(static_cast<OpSNegate const &>(inst));
    case Op::FNegate:
        return visitor(static_cast<OpFNegate const &>(inst));
    case Op::IAdd:
        return visitor(static_cast<OpIAdd const &>(inst));
    case Op::FAdd:
        return visitor(static_cast<OpFAdd const &>(inst));
    case Op::ISub:
        return visitor(static_cast<OpISub const &>(inst));
    case Op::FSub:
        return visitor(static_cast<OpFSub const &>(inst));
    case Op::IMul:
        return visitor(static_cast<OpIMul const &>(inst));
    case Op::FMul:
        return visitor(static_cast<OpFMul const &>(inst));
    case Op::UDiv:
        return visitor(static_cast<OpUDiv const &>(inst));
    case Op::SDiv:
        return visitor(static_cast<OpSDiv const &>(inst));
    case Op::FDiv:
        return visitor(static_cast<OpFDiv const &>(inst));
    case Op::UMod:
        return visitor(static_cast<OpUMod const &>(inst));
    case Op::SRem:
        return visitor(static_cast<OpSRem const &>(inst));
    case Op::SMod:
        return visitor(static_cast<OpSMod const &>(inst));
    case Op::FRem:
        return visitor(static_cast<OpFRem const &>(inst));
    case Op::FMod:
        return visitor(static_cast<OpFMod const &>(inst));
    case Op::VectorTimesScalar:
        return visitor(static_cast<OpVectorTimesScalar const &>(inst));
    case Op::MatrixTimesScalar:
        return visitor(static_cast<OpMatrixTimesScalar const &>(inst));
    case Op::VectorTimesMatrix:
        return visitor(static_cast<OpVectorTimesMatrix const &>(inst));
    case Op::MatrixTimesVector:
        return visitor(static_cast<OpMatrixTimesVector const &>(inst));
    case Op::MatrixTimesMatrix:
        return visitor(static_cast<OpMatrixTimesMatrix const &>(inst));
    case Op::OuterProduct:
        return visitor(static_cast<OpOuterProduct const &>(inst));
    case Op::Dot:
        return visitor(static_cast<OpDot const &>(inst));
    case Op::IAddCarry:
        return visitor(static_cast<OpIAddCarry const &>(inst));
    case Op::ISubBorrow:
        return visitor(static_cast<OpISubBorrow const &>(inst));
    case Op::UMulExtended:
        return visitor(static_cast<OpUMulExtended const &>(inst));
    case Op::SMulExtended:
        return visitor(static_cast<OpSMulExtended const &>(inst));
    case Op::Any:
        return visitor(static_cast<OpAny const &>(inst));
    case Op::All:
        return visitor(static_cast<OpAll const &>(inst));
    case Op::IsNan:
        return visitor(static_cast<OpIsNan const &>(inst));
    case Op::IsInf:
        return visitor(static_cast<OpIsInf const &>(inst));
    case Op::IsFinite:
        return visitor(static_cast<OpIsFinite const &>(inst));
    case Op::IsNormal:
        return visitor(static_cast<OpIsNormal const &>(inst));
    case Op::SignBitSet:
        return visitor(static_cast<OpSignBitSet const &>(inst));
    case Op::LessOrGreater:
        return visitor(static_cast<OpLessOrGreater const &>(inst));
    case Op::Ordered:
        return visitor(static_cast<OpOrdered const &>(inst));
    case Op::Unordered:
        return visitor(static_cast<OpUnordered const &>(inst));
    case Op::LogicalEqual:
        return visitor(static_cast<OpLogicalEqual const &>(inst));
    case Op::LogicalNotEqual:
        return visitor(static_cast<OpLogicalNotEqual const &>(inst));
    case Op::LogicalOr:
        return visitor(static_cast<OpLogicalOr const &>(inst));
    case Op::LogicalAnd:
        return visitor(static_cast<OpLogicalAnd const &>(inst));
    case Op::LogicalNot:
        return visitor(static_cast<OpLogicalNot const &>(inst));
    case Op::Select:
        return visitor(static_cast<OpSelect const &>(inst));
    case Op::IEqual:
        return visitor(static_cast<OpIEqual const &>(inst));
    case Op::INotEqual:
        return visitor(static_cast<OpINotEqual const &>(inst));
    case Op::UGreaterThan:
        return visitor(static_cast<OpUGreaterThan const &>(inst));
    case Op::SGreaterThan:
        return visitor(static_cast<OpSGreaterThan const &>(inst));
    case Op::UGreaterThanEqual:
        return visitor(static_cast<OpUGreaterThanEqual const &>(inst));
    case Op::SGreaterThanEqual:
        return visitor(static_cast<OpSGreaterThanEqual const &>(inst));
    case Op::ULessThan:
        return visitor(static_cast<OpULessThan const &>(inst));
    case Op::SLessThan:
        return visitor(static_cast<OpSLessThan const &>(inst));
    case Op::ULessThanEqual:
        return visitor(static_cast<OpULessThanEqual const &>(inst));
    case Op::SLessThanEqual:
        return visitor(static_cast<OpSLessThanEqual const &>(inst));
    case Op::FOrdEqual:
        return visitor(static_cast<OpFOrdEqual const &>(inst));
    case Op::FUnordEqual:
        return visitor(static_cast<OpFUnordEqual const &>(inst));
    case Op::FOrdNotEqual:
        return visitor(static_cast<OpFOrdNotEqual const &>(inst));
    case Op::FUnordNotEqual:
        return visitor(static_cast<OpFUnordNotEqual const &>(inst));
    case Op::FOrdLessThan:
        return visitor(static_cast<OpFOrdLessThan const &>(inst));
    case Op::FUnordLessThan:
        return visitor(static_cast<OpFUnordLessThan const &>(inst));
    case Op::FOrdGreaterThan:
        return visitor(static_cast<OpFOrdGreaterThan const &>(inst));
    case Op::FUnordGreaterThan:
        return visitor(static_cast<OpFUnordGreaterThan const &>(inst));
    case Op::FOrdLessThanEqual:
        return visitor(static_cast<OpFOrdLessThanEqual const &>(inst));
    case Op::FUnordLessThanEqual:
        return visitor(static_cast<OpFUnordLessThanEqual const &>(inst));
    case Op::FOrdGreaterThanEqual:
        return visitor(static_cast<OpFOrdGreaterThanEqual const &>(inst));
    case Op::FUnordGreaterThanEqual:
        return visitor(static_cast<OpFUnordGreaterThanEqual const &>(inst));
    case Op::ShiftRightLogical:
        return visitor(static_cast<OpShiftRightLogical const &>(inst));
    case Op::ShiftRightArithmetic:
        return visitor(static_cast<OpShiftRightArithmetic const &>(inst));
    case Op::ShiftLeftLogical:
        return visitor(static_cast<OpShiftLeftLogical const &>(inst));
    case Op::BitwiseOr:
        return visitor(static_cast<OpBitwiseOr const &>(inst));
    case Op::BitwiseXor:
        return visitor(static_cast<OpBitwiseXor const &>(inst));
    case Op::BitwiseAnd:
        return visitor(static_cast<OpBitwiseAnd const &>(inst));
    case Op::Not:
        return visitor(static_cast<OpNot const &>(inst));
    case Op::BitFieldInsert:
        return visitor(static_cast<OpBitFieldInsert const &>(inst));
    case Op::BitFieldSExtract:
        return visitor(static_cast<OpBitFieldSExtract const &>(inst));
    case Op::BitFieldUExtract:
        return visitor(static_cast<OpBitFieldUExtract const &>(inst));
    case Op::BitReverse:
        return visitor(static_cast<OpBitReverse const &>(inst));
    case Op::BitCount:
        return visitor(static_cast<OpBitCount const &>(inst));
    case Op::DPdx:
        return visitor(static_cast<OpDPdx const &>(inst));
    case Op::DPdy:
        return visitor(static_cast<OpDPdy const &>(inst));
    case Op::Fwidth:
        return visitor(static_cast<OpFwidth const &>(inst));
    case Op::DPdxFine:
        return visitor(static_cast<OpDPdxFine const &>(inst));
    case Op::DPdyFine:
        return visitor(static_cast<OpDPdyFine const &>(inst));
    case Op::FwidthFine:
        return visitor(static_cast<OpFwidthFine const &>(inst));
    case Op::DPdxCoarse:
        return visitor(static_cast<OpDPdxCoarse const &>(inst));
    case Op::DPdyCoarse:
        return visitor(static_cast<OpDPdyCoarse const &>(inst));
    case Op::FwidthCoarse:
        return visitor(static_cast<OpFwidthCoarse const &>(inst));
    case Op::EmitVertex:
        return visitor(static_cast<OpEmitVertex const &>(inst));
    case Op::EndPrimitive:
        return visitor(static_cast<OpEndPrimitive const &>(inst));
    case Op::EmitStreamVertex:
        return visitor(static_cast<OpEmitStreamVertex const &>(inst));
    case Op::EndStreamPrimitive:
        return visitor(static_cast<OpEndStreamPrimitive const &>(inst));
    case Op::ControlBarrier:
        return visitor(static_cast<OpControlBarrier const &>(inst));
    case Op::MemoryBarrier:
        return visitor(static_cast<OpMemoryBarrier const &>(inst));
    case Op::AtomicLoad:
        return visitor(static_cast<OpAtomicLoad const &>(inst));
    case Op::AtomicStore:
        return visitor(static_cast<OpAtomicStore const &>(inst));
    case Op::AtomicExchange:
        return visitor(static_cast<OpAtomicExchange const &>(inst));
    case Op::AtomicCompareExchange:
        return visitor(static_cast<OpAtomicCompareExchange const &>(inst));
    case Op::AtomicCompareExchangeWeak:
        return visitor(static_cast<OpAtomicCompareExchangeWeak const &>(inst));
    case Op::AtomicIIncrement:
        return visitor(static_cast<OpAtomicIIncrement const &>(inst));
    case Op::AtomicIDecrement:
        return visitor(static_cast<OpAtomicIDecrement const &>(inst));
    case Op::AtomicIAdd:
        return visitor(static_cast<OpAtomicIAdd const &>(inst));
    case Op::AtomicISub:
        return visitor(static_cast<OpAtomicISub const &>(inst));
    case Op::AtomicSMin:
        return visitor(static_cast<OpAtomicSMin const &>(inst));
    case Op::AtomicUMin:
        return visitor(static_cast<OpAtomicUMin const &>(inst));
    case Op::AtomicSMax:
        return visitor(static_cast<OpAtomicSMax const &>(inst));
    case Op::AtomicUMax:
        return visitor(static_cast<OpAtomicUMax const &>(inst));
    case Op::AtomicAnd:
        return visitor(static_cast<OpAtomicAnd const &>(inst));
    case Op::AtomicOr:
        return visitor(static_cast<OpAtomicOr const &>(inst));
    case Op::AtomicXor:
        return visitor(static_cast<OpAtomicXor const &>(inst));
    case Op::Phi:
        return visitor(static_cast<OpPhi const &>(inst));
    case Op::LoopMerge:
        return visitor(static_cast<OpLoopMerge const &>(inst));
    case Op::SelectionMerge:
        return visitor(static_cast<OpSelectionMerge const &>(inst));
    case Op::Label:
        return visitor(static_cast<OpLabel const &>(inst));
    case Op::Branch:
        return visitor(static_cast<OpBranch const &>(inst));
    case Op::BranchConditional:
        return visitor(static_cast<OpBranchConditional const &>(inst));
    case Op::Switch:
        return visitor(static_cast<OpSwitch const &>(inst));
    case Op::Kill:
        return visitor(static_cast<OpKill const &>(inst));
    case Op::Return:
        return visitor(static_cast<OpReturn const &>(inst));
    case Op::ReturnValue:
        return visitor(static_cast<OpReturnValue const &>(inst));
    case Op::Unreachable:
        return visitor(static_cast<OpUnreachable const &>(inst));
    case Op::LifetimeStart:
        return visitor(static_cast<OpLifetimeStart const &>(inst));
    case Op::LifetimeStop:
        return visitor(static_cast<OpLifetimeStop const &>(inst));
    case Op::GroupAsyncCopy:
        return visitor(static_cast<OpGroupAsyncCopy const &>(inst));
    case Op::GroupWaitEvents:
        return visitor(static_cast<OpGroupWaitEvents const &>(inst));
    case Op::GroupAll:
        return visitor(static_cast<OpGroupAll const &>(inst));
    case Op::GroupAny:
        return visitor(static_cast<OpGroupAny const &>(inst));
    case Op::GroupBroadcast:
        return visitor(static_cast<OpGroupBroadcast const &>(inst));
    case Op::GroupIAdd:
        return visitor(static_cast<OpGroupIAdd const &>(inst));
    case Op::GroupFAdd:
        return visitor(static_cast<OpGroupFAdd const &>(inst));
    case Op::GroupFMin:
        return visitor(static_cast<OpGroupFMin const &>(inst));
    case Op::GroupUMin:
        return visitor(static_cast<OpGroupUMin const &>(inst));
    case Op::GroupSMin:
        return visitor(static_cast<OpGroupSMin const &>(inst));
    case Op::GroupFMax:
        return visitor(static_cast<OpGroupFMax const &>(inst));
    case Op::GroupUMax:
        return visitor(static_cast<OpGroupUMax const &>(inst));
    case Op::GroupSMax:
        return visitor(static_cast<OpGroupSMax const &>(inst));
    case Op::ReadPipe:
        return visitor(static_cast<OpReadPipe const &>(inst));
    case Op::WritePipe:
        return visitor(static_cast<OpWritePipe const &>(inst));
    case Op::ReservedReadPipe:
        return visitor(static_cast<OpReservedReadPipe const &>(inst));
    case Op::ReservedWritePipe:
        return visitor(static_cast<OpReservedWritePipe const &>(inst));
    case Op::ReserveReadPipePackets:
        return visitor(static_cast<OpReserveReadPipePackets const &>(inst));
    case Op::ReserveWritePipePackets:
        return visitor(static_cast<OpReserveWritePipePackets const &>(inst));
    case Op::CommitReadPipe:
        return visitor(static_cast<OpCommitReadPipe const &>(inst));
    case Op::CommitWritePipe:
        return visitor(static_cast<OpCommitWritePipe const &>(inst));
    case Op::IsValidReserveId:
        return visitor(static_cast<OpIsValidReserveId const &>(inst));
    case Op::GetNumPipePackets:
        return visitor(static_cast<OpGetNumPipePackets const &>(inst));
    case Op::GetMaxPipePackets:
        return visitor(static_cast<OpGetMaxPipePackets const &>(inst));
    case Op::GroupReserveReadPipePackets:
        return visitor(static_cast<OpGroupReserveReadPipePackets const &>(inst));
    case Op::GroupReserveWritePipePackets:
        return visitor(static_cast<OpGroupReserveWritePipePackets const &>(inst));
    case Op::GroupCommitReadPipe:
        return visitor(static_cast<OpGroupCommitReadPipe const &>(inst));
    case Op::GroupCommitWritePipe:
        return visitor(static_cast<OpGroupCommitWritePipe const &>(inst));
    case Op::EnqueueMarker:
        return visitor(static_cast<OpEnqueueMarker const &>(inst));
    case Op::EnqueueKernel:
        return visitor(static_cast<OpEnqueueKernel const &>(inst));
    case Op::GetKernelNDrangeSubGroupCount:
        return visitor(static_cast<OpGetKernelNDrangeSubGroupCount const &>(inst));
    case Op::GetKernelNDrangeMaxSubGroupSize:
        return visitor(static_cast<OpGetKernelNDrangeMaxSubGroupSize const &>(inst));
    case Op::GetKernelWorkGroupSize:
        return visitor(static_cast<OpGetKernelWorkGroupSize const &>(inst));
    case Op::GetKernelPreferredWorkGroupSizeMultiple:
        return visitor(static_cast<OpGetKernelPreferredWorkGroupSizeMultiple const &>(inst));
    case Op::RetainEvent:
        return visitor(static_cast<OpRetainEvent const &>(inst));
    case Op::ReleaseEvent:
        return visitor(static_cast<OpReleaseEvent const &>(inst));
    case Op::CreateUserEvent:
        return visitor(static_cast<OpCreateUserEvent const &>(inst));
    case Op::IsValidEvent:
        return visitor(static_cast<OpIsValidEvent const &>(inst));
    case Op::SetUserEventStatus:
        return visitor(static_cast<OpSetUserEventStatus const &>(inst));
    case Op::CaptureEventProfilingInfo:
        return visitor(static_cast<OpCaptureEventProfilingInfo const &>(inst));
    case Op::GetDefaultQueue:
        return visitor(static_cast<OpGetDefaultQueue const &>(inst));
    case Op::BuildNDRange:
        return visitor(static_cast<OpBuildNDRange const &>(inst));
    case Op::ImageSparseSampleImplicitLod:
        return visitor(static_cast<OpImageSparseSampleImplicitLod const &>(inst));
    case Op::ImageSparseSampleExplicitLod:
        return visitor(static_cast<OpImageSparseSampleExplicitLod const &>(inst));
    case Op::ImageSparseSampleDrefImplicitLod:
        return visitor(static_cast<OpImageSparseSampleDrefImplicitLod const &>(inst));
    case Op::ImageSparseSampleDrefExplicitLod:
        return visitor(static_cast<OpImageSparseSampleDrefExplicitLod const &>(inst));
    case Op::ImageSparseSampleProjImplicitLod:
        return visitor(static_cast<OpImageSparseSampleProjImplicitLod const &>(inst));
    case Op::ImageSparseSampleProjExplicitLod:
        return visitor(static_cast<OpImageSparseSampleProjExplicitLod const &>(inst));
    case Op::ImageSparseSampleProjDrefImplicitLod:
        return visitor(static_cast<OpImageSparseSampleProjDrefImplicitLod const &>(inst));
    case Op::ImageSparseSampleProjDrefExplicitLod:
        return visitor(static_cast<OpImageSparseSampleProjDrefExplicitLod const &>(inst));
    case Op::ImageSparseFetch:
        return visitor(static_cast<OpImageSparseFetch const &>(inst));
    case Op::ImageSparseGather:
        return visitor(static_cast<OpImageSparseGather const &>(inst));
    case Op::ImageSparseDrefGather:
        return visitor(static_cast<OpImageSparseDrefGather const &>(inst));
    case Op::ImageSparseTexelsResident:
        return visitor(static_cast<OpImageSparseTexelsResident const &>(inst));
    case Op::NoLine:
        return visitor(static_cast<OpNoLine const &>(inst));
    case Op::AtomicFlagTestAndSet:
        return visitor(static_cast<OpAtomicFlagTestAndSet const &>(inst));
    case Op::AtomicFlagClear:
        return visitor(static_cast<OpAtomicFlagClear const &>(inst));
    case Op::ImageSparseRead:
        return visitor(static_cast<OpImageSparseRead const &>(inst));
    case Op::SizeOf:
        return visitor(static_cast<OpSizeOf const &>(inst));
    case Op::TypePipeStorage:
        return visitor(static_cast<OpTypePipeStorage const &>(inst));
    case Op::ConstantPipeStorage:
        return visitor(static_cast<OpConstantPipeStorage const &>(inst));
    case Op::CreatePipeFromPipeStorage:
        return visitor(static_cast<OpCreatePipeFromPipeStorage const &>(inst));
    case Op::GetKernelLocalSizeForSubgroupCount:
        return visitor(static_cast<OpGetKernelLocalSizeForSubgroupCount const &>(inst));
    case Op::GetKernelMaxNumSubgroups:
        return visitor(static_cast<OpGetKernelMaxNumSubgroups const &>(inst));
    case Op::TypeNamedBarrier:
        return visitor(static_cast<OpTypeNamedBarrier const &>(inst));
    case Op::NamedBarrierInitialize:
        return visitor(static_cast<OpNamedBarrierInitialize const &>(inst));
    case Op::MemoryNamedBarrier:
        return visitor(static_cast<OpMemoryNamedBarrier const &>(inst));
    case Op::ModuleProcessed:
        return visitor(static_cast<OpModuleProcessed const &>(inst));
    case Op::ExecutionModeId:
        return visitor(static_cast<OpExecutionModeId const &>(inst));
    case Op::DecorateId:
        return visitor(static_cast<OpDecorateId const &>(inst));
    case Op::GroupNonUniformElect:
        return visitor(static_cast<OpGroupNonUniformElect const &>(inst));
    case Op::GroupNonUniformAll:
        return visitor(static_cast<OpGroupNonUniformAll const &>(inst));
    case Op::GroupNonUniformAny:
        return visitor(static_cast<OpGroupNonUniformAny const &>(inst));
    case Op::GroupNonUniformAllEqual:
        return visitor(static_cast<OpGroupNonUniformAllEqual const &>(inst));
    case Op::GroupNonUniformBroadcast:
        return visitor(static_cast<OpGroupNonUniformBroadcast const &>(inst));
    case Op::GroupNonUniformBroadcastFirst:
        return visitor(static_cast<OpGroupNonUniformBroadcastFirst const &>(inst));
    case Op::GroupNonUniformBallot:
        return visitor(static_cast<OpGroupNonUniformBallot const &>(inst));
    case Op::GroupNonUniformInverseBallot:
        return visitor(static_cast<OpGroupNonUniformInverseBallot const &>(inst));
    case Op::GroupNonUniformBallotBitExtract:
        return visitor(static_cast<OpGroupNonUniformBallotBitExtract const &>(inst));
    case Op::GroupNonUniformBallotBitCount:
        return visitor(static_cast<OpGroupNonUniformBallotBitCount const &>(inst));
    case Op::GroupNonUniformBallotFindLSB:
        return visitor(static_cast<OpGroupNonUniformBallotFindLSB const &>(inst));
    case Op::GroupNonUniformBallotFindMSB:
        return visitor(static_cast<OpGroupNonUniformBallotFindMSB const &>(inst));
    case Op::GroupNonUniformShuffle:
        return visitor(static_cast<OpGroupNonUniformShuffle const &>(inst));
    case Op::GroupNonUniformShuffleXor:
        return visitor(static_cast<OpGroupNonUniformShuffleXor const &>(inst));
    case Op::GroupNonUniformShuffleUp:
        return visitor(static_cast<OpGroupNonUniformShuffleUp const &>(inst));
    case Op::GroupNonUniformShuffleDown:
        return visitor(static_cast<OpGroupNonUniformShuffleDown const &>(inst));
    case Op::GroupNonUniformIAdd:
        return visitor(static_cast<OpGroupNonUniformIAdd const &>(inst));
    case Op::GroupNonUniformFAdd:
        return visitor(static_cast<OpGroupNonUniformFAdd const &>(inst));
    case Op::GroupNonUniformIMul:
        return visitor(static_cast<OpGroupNonUniformIMul const &>(inst));
    case Op::GroupNonUniformFMul:
        return visitor(static_cast<OpGroupNonUniformFMul const &>(inst));
    case Op::GroupNonUniformSMin:
        return visitor(static_cast<OpGroupNonUniformSMin const &>(inst));
    case Op::GroupNonUniformUMin:
        return visitor(static_cast<OpGroupNonUniformUMin const &>(inst));
    case Op::GroupNonUniformFMin:
        return visitor(static_cast<OpGroupNonUniformFMin const &>(inst));
    case Op::GroupNonUniformSMax:
        return visitor(static_cast<OpGroupNonUniformSMax const &>(inst));
    case Op::GroupNonUniformUMax:
        return visitor(static_cast<OpGroupNonUniformUMax const &>(inst));
    case Op::GroupNonUniformFMax:
        return visitor(static_cast<OpGroupNonUniformFMax const &>(inst));
    case Op::GroupNonUniformBitwiseAnd:
        return visitor(static_cast<OpGroupNonUniformBitwiseAnd const &>(inst));
    case Op::GroupNonUniformBitwiseOr:
        return visitor(static_cast<OpGroupNonUniformBitwiseOr const &>(inst));
    case Op::GroupNonUniformBitwiseXor:
        return visitor(static_cast<OpGroupNonUniformBitwiseXor const &>(inst));
    case Op::GroupNonUniformLogicalAnd:
        return visitor(static_cast<OpGroupNonUniformLogicalAnd const &>(inst));
    case Op::GroupNonUniformLogicalOr:
        return visitor(static_cast<OpGroupNonUniformLogicalOr const &>(inst));
    case Op::GroupNonUniformLogicalXor:
        return visitor(static_cast<OpGroupNonUniformLogicalXor const &>(inst));
    case Op::GroupNonUniformQuadBroadcast:
        return visitor(static_cast<OpGroupNonUniformQuadBroadcast const &>(inst));
    case Op::GroupNonUniformQuadSwap:
        return visitor(static_cast<OpGroupNonUniformQuadSwap const &>(inst));
    case Op::CopyLogical:
        return visitor(static_cast<OpCopyLogical const &>(inst));
    case Op::PtrEqual:
        return visitor(static_cast<OpPtrEqual const &>(inst));
    case Op::PtrNotEqual:
        return visitor(static_cast<OpPtrNotEqual const &>(inst));
    case Op::PtrDiff:
        return visitor(static_cast<OpPtrDiff const &>(inst));
    case Op::TypeCooperativeMatrixKHR:
        return visitor(static_cast<OpTypeCooperativeMatrixKHR const &>(inst));
    case Op::CooperativeMatrixLoadKHR:
        return visitor(static_cast<OpCooperativeMatrixLoadKHR const &>(inst));
    case Op::CooperativeMatrixStoreKHR:
        return visitor(static_cast<OpCooperativeMatrixStoreKHR const &>(inst));
    case Op::CooperativeMatrixMulAddKHR:
        return visitor(static_cast<OpCooperativeMatrixMulAddKHR const &>(inst));
    case Op::CooperativeMatrixLengthKHR:
        return visitor(static_cast<OpCooperativeMatrixLengthKHR const &>(inst));
    case Op::AtomicFMinEXT:
        return visitor(static_cast<OpAtomicFMinEXT const &>(inst));
    case Op::AtomicFMaxEXT:
        return visitor(static_cast<OpAtomicFMaxEXT const &>(inst));
    case Op::AtomicFAddEXT:
        return visitor(static_cast<OpAtomicFAddEXT const &>(inst));
    }
    throw internal_compiler_error();
}

template <typename Derived, bool IsConst = true> class default_visitor {
  public:
    template <typename T> using const_t = std::conditional_t<IsConst, std::add_const_t<T>, T>;
    auto pre_visit(const_t<spv_inst> &) {}
    auto visit_result(const_t<spv_inst> &) {}
    auto post_visit(const_t<spv_inst> &) {}
    auto operator()(const_t<OpNop> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpUndef> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpSourceContinued> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpSource> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        if (in.op2()) {
            static_cast<Derived *>(this)->operator()(*in.op2());
        }

        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpSourceExtension> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpName> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpMemberName> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpString> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpLine> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpExtension> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpExtInstImport> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpExtInst> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        for (auto &op : in.op2()) {
            static_cast<Derived *>(this)->operator()(op);
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpMemoryModel> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpEntryPoint> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        for (auto &op : in.op3()) {
            static_cast<Derived *>(this)->operator()(op);
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpExecutionMode> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpCapability> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpTypeVoid> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpTypeBool> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpTypeInt> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpTypeFloat> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        if (in.op1()) {
            static_cast<Derived *>(this)->operator()(*in.op1());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpTypeVector> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpTypeMatrix> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpTypeImage> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->operator()(in.op4());
        static_cast<Derived *>(this)->operator()(in.op5());
        static_cast<Derived *>(this)->operator()(in.op6());
        if (in.op7()) {
            static_cast<Derived *>(this)->operator()(*in.op7());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpTypeSampler> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpTypeSampledImage> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpTypeArray> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpTypeRuntimeArray> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpTypeStruct> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->visit_result(in);
        for (auto &op : in.op0()) {
            static_cast<Derived *>(this)->operator()(op);
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpTypeOpaque> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpTypePointer> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpTypeFunction> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        for (auto &op : in.op1()) {
            static_cast<Derived *>(this)->operator()(op);
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpTypeEvent> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpTypeDeviceEvent> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpTypeReserveId> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpTypeQueue> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpTypePipe> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpTypeForwardPointer> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpConstantTrue> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpConstantFalse> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpConstant> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpConstantComposite> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        for (auto &op : in.op0()) {
            static_cast<Derived *>(this)->operator()(op);
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpConstantSampler> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpConstantNull> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpFunction> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpFunctionParameter> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpFunctionEnd> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpFunctionCall> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        for (auto &op : in.op1()) {
            static_cast<Derived *>(this)->operator()(op);
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpVariable> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        if (in.op1()) {
            static_cast<Derived *>(this)->operator()(*in.op1());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageTexelPointer> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpLoad> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        if (in.op1()) {
            static_cast<Derived *>(this)->operator()(*in.op1());
        }

        if (in.op2()) {
            static_cast<Derived *>(this)->operator()(*in.op2());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpStore> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        if (in.op2()) {
            static_cast<Derived *>(this)->operator()(*in.op2());
        }

        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpCopyMemory> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        if (in.op2()) {
            static_cast<Derived *>(this)->operator()(*in.op2());
        }

        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }

        if (in.op4()) {
            static_cast<Derived *>(this)->operator()(*in.op4());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpCopyMemorySized> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }

        if (in.op4()) {
            static_cast<Derived *>(this)->operator()(*in.op4());
        }

        if (in.op5()) {
            static_cast<Derived *>(this)->operator()(*in.op5());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpAccessChain> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        for (auto &op : in.op1()) {
            static_cast<Derived *>(this)->operator()(op);
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpInBoundsAccessChain> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        for (auto &op : in.op1()) {
            static_cast<Derived *>(this)->operator()(op);
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpPtrAccessChain> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        for (auto &op : in.op2()) {
            static_cast<Derived *>(this)->operator()(op);
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpArrayLength> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGenericPtrMemSemantics> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpInBoundsPtrAccessChain> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        for (auto &op : in.op2()) {
            static_cast<Derived *>(this)->operator()(op);
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpDecorate> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        if (in.op2()) {
            static_cast<Derived *>(this)->operator()(*in.op2());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpMemberDecorate> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpDecorationGroup> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupDecorate> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        for (auto &op : in.op1()) {
            static_cast<Derived *>(this)->operator()(op);
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupMemberDecorate> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        for (auto &op : in.op1()) {
            static_cast<Derived *>(this)->operator()(op);
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpVectorExtractDynamic> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpVectorInsertDynamic> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpVectorShuffle> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        for (auto &op : in.op2()) {
            static_cast<Derived *>(this)->operator()(op);
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpCompositeConstruct> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        for (auto &op : in.op0()) {
            static_cast<Derived *>(this)->operator()(op);
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpCompositeExtract> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        for (auto &op : in.op1()) {
            static_cast<Derived *>(this)->operator()(op);
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpCompositeInsert> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        for (auto &op : in.op2()) {
            static_cast<Derived *>(this)->operator()(op);
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpCopyObject> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpTranspose> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpSampledImage> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageSampleImplicitLod> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        if (in.op2()) {
            static_cast<Derived *>(this)->operator()(*in.op2());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageSampleExplicitLod> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageSampleDrefImplicitLod> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageSampleDrefExplicitLod> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageSampleProjImplicitLod> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        if (in.op2()) {
            static_cast<Derived *>(this)->operator()(*in.op2());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageSampleProjExplicitLod> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageSampleProjDrefImplicitLod> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageSampleProjDrefExplicitLod> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageFetch> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        if (in.op2()) {
            static_cast<Derived *>(this)->operator()(*in.op2());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageGather> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageDrefGather> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageRead> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        if (in.op2()) {
            static_cast<Derived *>(this)->operator()(*in.op2());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageWrite> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImage> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageQueryFormat> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageQueryOrder> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageQuerySizeLod> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageQuerySize> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageQueryLod> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageQueryLevels> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageQuerySamples> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpConvertFToU> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpConvertFToS> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpConvertSToF> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpConvertUToF> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpUConvert> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpSConvert> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpFConvert> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpQuantizeToF16> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpConvertPtrToU> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpSatConvertSToU> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpSatConvertUToS> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpConvertUToPtr> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpPtrCastToGeneric> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGenericCastToPtr> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGenericCastToPtrExplicit> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpBitcast> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpSNegate> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpFNegate> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpIAdd> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpFAdd> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpISub> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpFSub> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpIMul> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpFMul> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpUDiv> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpSDiv> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpFDiv> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpUMod> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpSRem> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpSMod> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpFRem> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpFMod> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpVectorTimesScalar> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpMatrixTimesScalar> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpVectorTimesMatrix> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpMatrixTimesVector> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpMatrixTimesMatrix> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpOuterProduct> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpDot> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpIAddCarry> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpISubBorrow> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpUMulExtended> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpSMulExtended> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpAny> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpAll> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpIsNan> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpIsInf> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpIsFinite> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpIsNormal> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpSignBitSet> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpLessOrGreater> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpOrdered> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpUnordered> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpLogicalEqual> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpLogicalNotEqual> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpLogicalOr> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpLogicalAnd> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpLogicalNot> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpSelect> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpIEqual> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpINotEqual> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpUGreaterThan> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpSGreaterThan> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpUGreaterThanEqual> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpSGreaterThanEqual> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpULessThan> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpSLessThan> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpULessThanEqual> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpSLessThanEqual> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpFOrdEqual> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpFUnordEqual> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpFOrdNotEqual> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpFUnordNotEqual> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpFOrdLessThan> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpFUnordLessThan> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpFOrdGreaterThan> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpFUnordGreaterThan> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpFOrdLessThanEqual> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpFUnordLessThanEqual> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpFOrdGreaterThanEqual> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpFUnordGreaterThanEqual> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpShiftRightLogical> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpShiftRightArithmetic> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpShiftLeftLogical> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpBitwiseOr> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpBitwiseXor> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpBitwiseAnd> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpNot> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpBitFieldInsert> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpBitFieldSExtract> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpBitFieldUExtract> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpBitReverse> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpBitCount> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpDPdx> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpDPdy> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpFwidth> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpDPdxFine> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpDPdyFine> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpFwidthFine> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpDPdxCoarse> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpDPdyCoarse> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpFwidthCoarse> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpEmitVertex> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpEndPrimitive> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpEmitStreamVertex> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpEndStreamPrimitive> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpControlBarrier> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpMemoryBarrier> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpAtomicLoad> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpAtomicStore> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpAtomicExchange> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpAtomicCompareExchange> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->operator()(in.op4());
        static_cast<Derived *>(this)->operator()(in.op5());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpAtomicCompareExchangeWeak> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->operator()(in.op4());
        static_cast<Derived *>(this)->operator()(in.op5());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpAtomicIIncrement> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpAtomicIDecrement> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpAtomicIAdd> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpAtomicISub> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpAtomicSMin> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpAtomicUMin> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpAtomicSMax> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpAtomicUMax> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpAtomicAnd> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpAtomicOr> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpAtomicXor> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpPhi> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        for (auto &op : in.op0()) {
            static_cast<Derived *>(this)->operator()(op);
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpLoopMerge> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpSelectionMerge> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpLabel> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpBranch> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpBranchConditional> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        for (auto &op : in.op3()) {
            static_cast<Derived *>(this)->operator()(op);
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpSwitch> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        for (auto &op : in.op2()) {
            static_cast<Derived *>(this)->operator()(op);
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpKill> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpReturn> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpReturnValue> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpUnreachable> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpLifetimeStart> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpLifetimeStop> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupAsyncCopy> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->operator()(in.op4());
        static_cast<Derived *>(this)->operator()(in.op5());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupWaitEvents> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupAll> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupAny> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupBroadcast> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupIAdd> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupFAdd> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupFMin> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupUMin> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupSMin> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupFMax> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupUMax> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupSMax> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpReadPipe> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpWritePipe> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpReservedReadPipe> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->operator()(in.op4());
        static_cast<Derived *>(this)->operator()(in.op5());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpReservedWritePipe> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->operator()(in.op4());
        static_cast<Derived *>(this)->operator()(in.op5());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpReserveReadPipePackets> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpReserveWritePipePackets> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpCommitReadPipe> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpCommitWritePipe> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpIsValidReserveId> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGetNumPipePackets> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGetMaxPipePackets> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupReserveReadPipePackets> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->operator()(in.op4());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupReserveWritePipePackets> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->operator()(in.op4());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupCommitReadPipe> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->operator()(in.op4());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupCommitWritePipe> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->operator()(in.op4());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpEnqueueMarker> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpEnqueueKernel> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->operator()(in.op4());
        static_cast<Derived *>(this)->operator()(in.op5());
        static_cast<Derived *>(this)->operator()(in.op6());
        static_cast<Derived *>(this)->operator()(in.op7());
        static_cast<Derived *>(this)->operator()(in.op8());
        static_cast<Derived *>(this)->operator()(in.op9());
        for (auto &op : in.op10()) {
            static_cast<Derived *>(this)->operator()(op);
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGetKernelNDrangeSubGroupCount> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->operator()(in.op4());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGetKernelNDrangeMaxSubGroupSize> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->operator()(in.op4());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGetKernelWorkGroupSize> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGetKernelPreferredWorkGroupSizeMultiple> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpRetainEvent> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpReleaseEvent> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpCreateUserEvent> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpIsValidEvent> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpSetUserEventStatus> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpCaptureEventProfilingInfo> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGetDefaultQueue> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpBuildNDRange> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageSparseSampleImplicitLod> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        if (in.op2()) {
            static_cast<Derived *>(this)->operator()(*in.op2());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageSparseSampleExplicitLod> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageSparseSampleDrefImplicitLod> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageSparseSampleDrefExplicitLod> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageSparseSampleProjImplicitLod> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        if (in.op2()) {
            static_cast<Derived *>(this)->operator()(*in.op2());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageSparseSampleProjExplicitLod> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageSparseSampleProjDrefImplicitLod> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageSparseSampleProjDrefExplicitLod> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageSparseFetch> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        if (in.op2()) {
            static_cast<Derived *>(this)->operator()(*in.op2());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageSparseGather> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageSparseDrefGather> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageSparseTexelsResident> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpNoLine> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpAtomicFlagTestAndSet> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpAtomicFlagClear> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpImageSparseRead> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        if (in.op2()) {
            static_cast<Derived *>(this)->operator()(*in.op2());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpSizeOf> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpTypePipeStorage> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpConstantPipeStorage> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpCreatePipeFromPipeStorage> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGetKernelLocalSizeForSubgroupCount> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->operator()(in.op4());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGetKernelMaxNumSubgroups> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpTypeNamedBarrier> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpNamedBarrierInitialize> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpMemoryNamedBarrier> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpModuleProcessed> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpExecutionModeId> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpDecorateId> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformElect> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformAll> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformAny> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformAllEqual> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformBroadcast> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformBroadcastFirst> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformBallot> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformInverseBallot> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformBallotBitExtract> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformBallotBitCount> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformBallotFindLSB> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformBallotFindMSB> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformShuffle> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformShuffleXor> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformShuffleUp> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformShuffleDown> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformIAdd> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformFAdd> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformIMul> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformFMul> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformSMin> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformUMin> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformFMin> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformSMax> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformUMax> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformFMax> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformBitwiseAnd> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformBitwiseOr> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformBitwiseXor> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformLogicalAnd> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformLogicalOr> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformLogicalXor> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformQuadBroadcast> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpGroupNonUniformQuadSwap> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpCopyLogical> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpPtrEqual> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpPtrNotEqual> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpPtrDiff> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpTypeCooperativeMatrixKHR> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->operator()(in.op4());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpCooperativeMatrixLoadKHR> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        if (in.op2()) {
            static_cast<Derived *>(this)->operator()(*in.op2());
        }

        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }

        if (in.op4()) {
            static_cast<Derived *>(this)->operator()(*in.op4());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpCooperativeMatrixStoreKHR> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }

        if (in.op4()) {
            static_cast<Derived *>(this)->operator()(*in.op4());
        }

        if (in.op5()) {
            static_cast<Derived *>(this)->operator()(*in.op5());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpCooperativeMatrixMulAddKHR> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }

        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpCooperativeMatrixLengthKHR> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpAtomicFMinEXT> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpAtomicFMaxEXT> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->post_visit(in);
    }
    auto operator()(const_t<OpAtomicFAddEXT> &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->visit_result(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->post_visit(in);
    }
};

} // namespace tinytc::spv

#endif // GENERATED_VISIT_20241111_HPP
