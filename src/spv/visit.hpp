// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

// This file is generated
// Do not edit manually

#ifndef GENERATED_VISIT_20241111_HPP
#define GENERATED_VISIT_20241111_HPP

namespace tinytc::spv {

template <class... Ts> struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

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
template <typename Derived> class default_visitor {
  public:
    auto pre_visit(spv_inst const &) {}
    auto operator()(OpNop const &in) { static_cast<Derived *>(this)->pre_visit(in); }
    auto operator()(OpUndef const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
    }
    auto operator()(OpSourceContinued const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpSource const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        if (in.op2()) {
            static_cast<Derived *>(this)->operator()(*in.op2());
        }

        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }
    }
    auto operator()(OpSourceExtension const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpName const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpMemberName const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpString const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpLine const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpExtension const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpExtInstImport const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpExtInst const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        for (auto const &op : in.op2()) {
            static_cast<Derived *>(this)->operator()(op);
        }
    }
    auto operator()(OpMemoryModel const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpEntryPoint const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        for (auto const &op : in.op3()) {
            static_cast<Derived *>(this)->operator()(op);
        }
    }
    auto operator()(OpExecutionMode const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpCapability const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpTypeVoid const &in) { static_cast<Derived *>(this)->pre_visit(in); }
    auto operator()(OpTypeBool const &in) { static_cast<Derived *>(this)->pre_visit(in); }
    auto operator()(OpTypeInt const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpTypeFloat const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        if (in.op1()) {
            static_cast<Derived *>(this)->operator()(*in.op1());
        }
    }
    auto operator()(OpTypeVector const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpTypeMatrix const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpTypeImage const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
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
    }
    auto operator()(OpTypeSampler const &in) { static_cast<Derived *>(this)->pre_visit(in); }
    auto operator()(OpTypeSampledImage const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpTypeArray const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpTypeRuntimeArray const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpTypeStruct const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        for (auto const &op : in.op0()) {
            static_cast<Derived *>(this)->operator()(op);
        }
    }
    auto operator()(OpTypeOpaque const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpTypePointer const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpTypeFunction const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        for (auto const &op : in.op1()) {
            static_cast<Derived *>(this)->operator()(op);
        }
    }
    auto operator()(OpTypeEvent const &in) { static_cast<Derived *>(this)->pre_visit(in); }
    auto operator()(OpTypeDeviceEvent const &in) { static_cast<Derived *>(this)->pre_visit(in); }
    auto operator()(OpTypeReserveId const &in) { static_cast<Derived *>(this)->pre_visit(in); }
    auto operator()(OpTypeQueue const &in) { static_cast<Derived *>(this)->pre_visit(in); }
    auto operator()(OpTypePipe const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpTypeForwardPointer const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpConstantTrue const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
    }
    auto operator()(OpConstantFalse const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
    }
    auto operator()(OpConstant const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpConstantComposite const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        for (auto const &op : in.op0()) {
            static_cast<Derived *>(this)->operator()(op);
        }
    }
    auto operator()(OpConstantSampler const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpConstantNull const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
    }
    auto operator()(OpFunction const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpFunctionParameter const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
    }
    auto operator()(OpFunctionEnd const &in) { static_cast<Derived *>(this)->pre_visit(in); }
    auto operator()(OpFunctionCall const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        for (auto const &op : in.op1()) {
            static_cast<Derived *>(this)->operator()(op);
        }
    }
    auto operator()(OpVariable const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        if (in.op1()) {
            static_cast<Derived *>(this)->operator()(*in.op1());
        }
    }
    auto operator()(OpImageTexelPointer const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpLoad const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        if (in.op1()) {
            static_cast<Derived *>(this)->operator()(*in.op1());
        }

        if (in.op2()) {
            static_cast<Derived *>(this)->operator()(*in.op2());
        }
    }
    auto operator()(OpStore const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        if (in.op2()) {
            static_cast<Derived *>(this)->operator()(*in.op2());
        }

        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }
    }
    auto operator()(OpCopyMemory const &in) {
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
    }
    auto operator()(OpCopyMemorySized const &in) {
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
    }
    auto operator()(OpAccessChain const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        for (auto const &op : in.op1()) {
            static_cast<Derived *>(this)->operator()(op);
        }
    }
    auto operator()(OpInBoundsAccessChain const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        for (auto const &op : in.op1()) {
            static_cast<Derived *>(this)->operator()(op);
        }
    }
    auto operator()(OpPtrAccessChain const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        for (auto const &op : in.op2()) {
            static_cast<Derived *>(this)->operator()(op);
        }
    }
    auto operator()(OpArrayLength const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpGenericPtrMemSemantics const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpInBoundsPtrAccessChain const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        for (auto const &op : in.op2()) {
            static_cast<Derived *>(this)->operator()(op);
        }
    }
    auto operator()(OpDecorate const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        if (in.op2()) {
            static_cast<Derived *>(this)->operator()(*in.op2());
        }
    }
    auto operator()(OpMemberDecorate const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpDecorationGroup const &in) { static_cast<Derived *>(this)->pre_visit(in); }
    auto operator()(OpGroupDecorate const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        for (auto const &op : in.op1()) {
            static_cast<Derived *>(this)->operator()(op);
        }
    }
    auto operator()(OpGroupMemberDecorate const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        for (auto const &op : in.op1()) {
            static_cast<Derived *>(this)->operator()(op);
        }
    }
    auto operator()(OpVectorExtractDynamic const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpVectorInsertDynamic const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpVectorShuffle const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        for (auto const &op : in.op2()) {
            static_cast<Derived *>(this)->operator()(op);
        }
    }
    auto operator()(OpCompositeConstruct const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        for (auto const &op : in.op0()) {
            static_cast<Derived *>(this)->operator()(op);
        }
    }
    auto operator()(OpCompositeExtract const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        for (auto const &op : in.op1()) {
            static_cast<Derived *>(this)->operator()(op);
        }
    }
    auto operator()(OpCompositeInsert const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        for (auto const &op : in.op2()) {
            static_cast<Derived *>(this)->operator()(op);
        }
    }
    auto operator()(OpCopyObject const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpTranspose const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpSampledImage const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpImageSampleImplicitLod const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        if (in.op2()) {
            static_cast<Derived *>(this)->operator()(*in.op2());
        }
    }
    auto operator()(OpImageSampleExplicitLod const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpImageSampleDrefImplicitLod const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }
    }
    auto operator()(OpImageSampleDrefExplicitLod const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
    }
    auto operator()(OpImageSampleProjImplicitLod const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        if (in.op2()) {
            static_cast<Derived *>(this)->operator()(*in.op2());
        }
    }
    auto operator()(OpImageSampleProjExplicitLod const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpImageSampleProjDrefImplicitLod const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }
    }
    auto operator()(OpImageSampleProjDrefExplicitLod const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
    }
    auto operator()(OpImageFetch const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        if (in.op2()) {
            static_cast<Derived *>(this)->operator()(*in.op2());
        }
    }
    auto operator()(OpImageGather const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }
    }
    auto operator()(OpImageDrefGather const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }
    }
    auto operator()(OpImageRead const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        if (in.op2()) {
            static_cast<Derived *>(this)->operator()(*in.op2());
        }
    }
    auto operator()(OpImageWrite const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }
    }
    auto operator()(OpImage const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpImageQueryFormat const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpImageQueryOrder const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpImageQuerySizeLod const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpImageQuerySize const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpImageQueryLod const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpImageQueryLevels const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpImageQuerySamples const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpConvertFToU const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpConvertFToS const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpConvertSToF const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpConvertUToF const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpUConvert const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpSConvert const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpFConvert const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpQuantizeToF16 const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpConvertPtrToU const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpSatConvertSToU const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpSatConvertUToS const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpConvertUToPtr const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpPtrCastToGeneric const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpGenericCastToPtr const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpGenericCastToPtrExplicit const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpBitcast const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpSNegate const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpFNegate const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpIAdd const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpFAdd const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpISub const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpFSub const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpIMul const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpFMul const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpUDiv const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpSDiv const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpFDiv const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpUMod const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpSRem const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpSMod const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpFRem const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpFMod const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpVectorTimesScalar const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpMatrixTimesScalar const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpVectorTimesMatrix const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpMatrixTimesVector const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpMatrixTimesMatrix const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpOuterProduct const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpDot const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpIAddCarry const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpISubBorrow const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpUMulExtended const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpSMulExtended const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpAny const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpAll const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpIsNan const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpIsInf const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpIsFinite const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpIsNormal const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpSignBitSet const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpLessOrGreater const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpOrdered const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpUnordered const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpLogicalEqual const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpLogicalNotEqual const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpLogicalOr const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpLogicalAnd const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpLogicalNot const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpSelect const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpIEqual const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpINotEqual const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpUGreaterThan const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpSGreaterThan const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpUGreaterThanEqual const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpSGreaterThanEqual const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpULessThan const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpSLessThan const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpULessThanEqual const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpSLessThanEqual const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpFOrdEqual const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpFUnordEqual const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpFOrdNotEqual const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpFUnordNotEqual const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpFOrdLessThan const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpFUnordLessThan const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpFOrdGreaterThan const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpFUnordGreaterThan const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpFOrdLessThanEqual const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpFUnordLessThanEqual const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpFOrdGreaterThanEqual const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpFUnordGreaterThanEqual const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpShiftRightLogical const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpShiftRightArithmetic const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpShiftLeftLogical const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpBitwiseOr const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpBitwiseXor const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpBitwiseAnd const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpNot const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpBitFieldInsert const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
    }
    auto operator()(OpBitFieldSExtract const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpBitFieldUExtract const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpBitReverse const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpBitCount const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpDPdx const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpDPdy const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpFwidth const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpDPdxFine const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpDPdyFine const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpFwidthFine const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpDPdxCoarse const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpDPdyCoarse const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpFwidthCoarse const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpEmitVertex const &in) { static_cast<Derived *>(this)->pre_visit(in); }
    auto operator()(OpEndPrimitive const &in) { static_cast<Derived *>(this)->pre_visit(in); }
    auto operator()(OpEmitStreamVertex const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpEndStreamPrimitive const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpControlBarrier const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpMemoryBarrier const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpAtomicLoad const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpAtomicStore const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
    }
    auto operator()(OpAtomicExchange const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
    }
    auto operator()(OpAtomicCompareExchange const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->operator()(in.op4());
        static_cast<Derived *>(this)->operator()(in.op5());
    }
    auto operator()(OpAtomicCompareExchangeWeak const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->operator()(in.op4());
        static_cast<Derived *>(this)->operator()(in.op5());
    }
    auto operator()(OpAtomicIIncrement const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpAtomicIDecrement const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpAtomicIAdd const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
    }
    auto operator()(OpAtomicISub const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
    }
    auto operator()(OpAtomicSMin const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
    }
    auto operator()(OpAtomicUMin const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
    }
    auto operator()(OpAtomicSMax const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
    }
    auto operator()(OpAtomicUMax const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
    }
    auto operator()(OpAtomicAnd const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
    }
    auto operator()(OpAtomicOr const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
    }
    auto operator()(OpAtomicXor const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
    }
    auto operator()(OpPhi const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        for (auto const &op : in.op0()) {
            static_cast<Derived *>(this)->operator()(op);
        }
    }
    auto operator()(OpLoopMerge const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpSelectionMerge const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpLabel const &in) { static_cast<Derived *>(this)->pre_visit(in); }
    auto operator()(OpBranch const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpBranchConditional const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        for (auto const &op : in.op3()) {
            static_cast<Derived *>(this)->operator()(op);
        }
    }
    auto operator()(OpSwitch const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        for (auto const &op : in.op2()) {
            static_cast<Derived *>(this)->operator()(op);
        }
    }
    auto operator()(OpKill const &in) { static_cast<Derived *>(this)->pre_visit(in); }
    auto operator()(OpReturn const &in) { static_cast<Derived *>(this)->pre_visit(in); }
    auto operator()(OpReturnValue const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpUnreachable const &in) { static_cast<Derived *>(this)->pre_visit(in); }
    auto operator()(OpLifetimeStart const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpLifetimeStop const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpGroupAsyncCopy const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->operator()(in.op4());
        static_cast<Derived *>(this)->operator()(in.op5());
    }
    auto operator()(OpGroupWaitEvents const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpGroupAll const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpGroupAny const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpGroupBroadcast const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpGroupIAdd const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpGroupFAdd const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpGroupFMin const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpGroupUMin const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpGroupSMin const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpGroupFMax const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpGroupUMax const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpGroupSMax const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpReadPipe const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
    }
    auto operator()(OpWritePipe const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
    }
    auto operator()(OpReservedReadPipe const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->operator()(in.op4());
        static_cast<Derived *>(this)->operator()(in.op5());
    }
    auto operator()(OpReservedWritePipe const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->operator()(in.op4());
        static_cast<Derived *>(this)->operator()(in.op5());
    }
    auto operator()(OpReserveReadPipePackets const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
    }
    auto operator()(OpReserveWritePipePackets const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
    }
    auto operator()(OpCommitReadPipe const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
    }
    auto operator()(OpCommitWritePipe const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
    }
    auto operator()(OpIsValidReserveId const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpGetNumPipePackets const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpGetMaxPipePackets const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpGroupReserveReadPipePackets const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->operator()(in.op4());
    }
    auto operator()(OpGroupReserveWritePipePackets const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->operator()(in.op4());
    }
    auto operator()(OpGroupCommitReadPipe const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->operator()(in.op4());
    }
    auto operator()(OpGroupCommitWritePipe const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->operator()(in.op4());
    }
    auto operator()(OpEnqueueMarker const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
    }
    auto operator()(OpEnqueueKernel const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
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
        for (auto const &op : in.op10()) {
            static_cast<Derived *>(this)->operator()(op);
        }
    }
    auto operator()(OpGetKernelNDrangeSubGroupCount const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->operator()(in.op4());
    }
    auto operator()(OpGetKernelNDrangeMaxSubGroupSize const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->operator()(in.op4());
    }
    auto operator()(OpGetKernelWorkGroupSize const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
    }
    auto operator()(OpGetKernelPreferredWorkGroupSizeMultiple const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
    }
    auto operator()(OpRetainEvent const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpReleaseEvent const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpCreateUserEvent const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
    }
    auto operator()(OpIsValidEvent const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpSetUserEventStatus const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpCaptureEventProfilingInfo const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpGetDefaultQueue const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
    }
    auto operator()(OpBuildNDRange const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpImageSparseSampleImplicitLod const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        if (in.op2()) {
            static_cast<Derived *>(this)->operator()(*in.op2());
        }
    }
    auto operator()(OpImageSparseSampleExplicitLod const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpImageSparseSampleDrefImplicitLod const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }
    }
    auto operator()(OpImageSparseSampleDrefExplicitLod const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
    }
    auto operator()(OpImageSparseSampleProjImplicitLod const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        if (in.op2()) {
            static_cast<Derived *>(this)->operator()(*in.op2());
        }
    }
    auto operator()(OpImageSparseSampleProjExplicitLod const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpImageSparseSampleProjDrefImplicitLod const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }
    }
    auto operator()(OpImageSparseSampleProjDrefExplicitLod const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
    }
    auto operator()(OpImageSparseFetch const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        if (in.op2()) {
            static_cast<Derived *>(this)->operator()(*in.op2());
        }
    }
    auto operator()(OpImageSparseGather const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }
    }
    auto operator()(OpImageSparseDrefGather const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }
    }
    auto operator()(OpImageSparseTexelsResident const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpNoLine const &in) { static_cast<Derived *>(this)->pre_visit(in); }
    auto operator()(OpAtomicFlagTestAndSet const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpAtomicFlagClear const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpImageSparseRead const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        if (in.op2()) {
            static_cast<Derived *>(this)->operator()(*in.op2());
        }
    }
    auto operator()(OpSizeOf const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpTypePipeStorage const &in) { static_cast<Derived *>(this)->pre_visit(in); }
    auto operator()(OpConstantPipeStorage const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpCreatePipeFromPipeStorage const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpGetKernelLocalSizeForSubgroupCount const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->operator()(in.op4());
    }
    auto operator()(OpGetKernelMaxNumSubgroups const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
    }
    auto operator()(OpTypeNamedBarrier const &in) { static_cast<Derived *>(this)->pre_visit(in); }
    auto operator()(OpNamedBarrierInitialize const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpMemoryNamedBarrier const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpModuleProcessed const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpExecutionModeId const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpDecorateId const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpGroupNonUniformElect const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpGroupNonUniformAll const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpGroupNonUniformAny const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpGroupNonUniformAllEqual const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpGroupNonUniformBroadcast const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpGroupNonUniformBroadcastFirst const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpGroupNonUniformBallot const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpGroupNonUniformInverseBallot const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpGroupNonUniformBallotBitExtract const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpGroupNonUniformBallotBitCount const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpGroupNonUniformBallotFindLSB const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpGroupNonUniformBallotFindMSB const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpGroupNonUniformShuffle const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpGroupNonUniformShuffleXor const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpGroupNonUniformShuffleUp const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpGroupNonUniformShuffleDown const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpGroupNonUniformIAdd const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }
    }
    auto operator()(OpGroupNonUniformFAdd const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }
    }
    auto operator()(OpGroupNonUniformIMul const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }
    }
    auto operator()(OpGroupNonUniformFMul const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }
    }
    auto operator()(OpGroupNonUniformSMin const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }
    }
    auto operator()(OpGroupNonUniformUMin const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }
    }
    auto operator()(OpGroupNonUniformFMin const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }
    }
    auto operator()(OpGroupNonUniformSMax const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }
    }
    auto operator()(OpGroupNonUniformUMax const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }
    }
    auto operator()(OpGroupNonUniformFMax const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }
    }
    auto operator()(OpGroupNonUniformBitwiseAnd const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }
    }
    auto operator()(OpGroupNonUniformBitwiseOr const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }
    }
    auto operator()(OpGroupNonUniformBitwiseXor const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }
    }
    auto operator()(OpGroupNonUniformLogicalAnd const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }
    }
    auto operator()(OpGroupNonUniformLogicalOr const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }
    }
    auto operator()(OpGroupNonUniformLogicalXor const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }
    }
    auto operator()(OpGroupNonUniformQuadBroadcast const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpGroupNonUniformQuadSwap const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
    }
    auto operator()(OpCopyLogical const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpPtrEqual const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpPtrNotEqual const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpPtrDiff const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
    }
    auto operator()(OpTypeCooperativeMatrixKHR const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
        static_cast<Derived *>(this)->operator()(in.op4());
    }
    auto operator()(OpCooperativeMatrixLoadKHR const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
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
    }
    auto operator()(OpCooperativeMatrixStoreKHR const &in) {
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
    }
    auto operator()(OpCooperativeMatrixMulAddKHR const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        if (in.op3()) {
            static_cast<Derived *>(this)->operator()(*in.op3());
        }
    }
    auto operator()(OpCooperativeMatrixLengthKHR const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
    }
    auto operator()(OpAtomicFMinEXT const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
    }
    auto operator()(OpAtomicFMaxEXT const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
    }
    auto operator()(OpAtomicFAddEXT const &in) {
        static_cast<Derived *>(this)->pre_visit(in);
        static_cast<Derived *>(this)->operator()(in.type());
        static_cast<Derived *>(this)->operator()(in.op0());
        static_cast<Derived *>(this)->operator()(in.op1());
        static_cast<Derived *>(this)->operator()(in.op2());
        static_cast<Derived *>(this)->operator()(in.op3());
    }
};

} // namespace tinytc::spv

#endif // GENERATED_VISIT_20241111_HPP
