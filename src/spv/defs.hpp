// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

// This file is generated
// Do not edit manually

#ifndef GENERATED_DEFS_20250131_HPP
#define GENERATED_DEFS_20250131_HPP

#include "enums.hpp"
#include "support/ilist_base.hpp"
#include "tinytc/tinytc.hpp"

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
    std::variant<std::int8_t, std::int16_t, std::int32_t, std::int64_t, half, float, double>;
using LiteralString = std::string;
using LiteralInteger = std::int32_t;
using LiteralExtInstInteger = std::int32_t;
using IdResultType = spv_inst *;
using IdRef = spv_inst *;
using IdScope = spv_inst *;
using IdMemorySemantics = spv_inst *;
using LoopControlAttr = std::int32_t;
using MemoryAccessAttr = std::int32_t;
using PairIdRefIdRef = std::pair<spv_inst *, spv_inst *>;
using PairLiteralIntegerIdRef =
    std::pair<std::variant<std::int8_t, std::int16_t, std::int32_t, std::int64_t>, spv_inst *>;
using PairIdRefLiteralInteger = std::pair<spv_inst *, std::int32_t>;

class OpNop;                                     // IWYU pragma: export
class OpUndef;                                   // IWYU pragma: export
class OpSourceContinued;                         // IWYU pragma: export
class OpSource;                                  // IWYU pragma: export
class OpSourceExtension;                         // IWYU pragma: export
class OpName;                                    // IWYU pragma: export
class OpMemberName;                              // IWYU pragma: export
class OpString;                                  // IWYU pragma: export
class OpLine;                                    // IWYU pragma: export
class OpExtension;                               // IWYU pragma: export
class OpExtInstImport;                           // IWYU pragma: export
class OpExtInst;                                 // IWYU pragma: export
class OpMemoryModel;                             // IWYU pragma: export
class OpEntryPoint;                              // IWYU pragma: export
class OpExecutionMode;                           // IWYU pragma: export
class OpCapability;                              // IWYU pragma: export
class OpTypeVoid;                                // IWYU pragma: export
class OpTypeBool;                                // IWYU pragma: export
class OpTypeInt;                                 // IWYU pragma: export
class OpTypeFloat;                               // IWYU pragma: export
class OpTypeVector;                              // IWYU pragma: export
class OpTypeMatrix;                              // IWYU pragma: export
class OpTypeImage;                               // IWYU pragma: export
class OpTypeSampler;                             // IWYU pragma: export
class OpTypeSampledImage;                        // IWYU pragma: export
class OpTypeArray;                               // IWYU pragma: export
class OpTypeRuntimeArray;                        // IWYU pragma: export
class OpTypeStruct;                              // IWYU pragma: export
class OpTypeOpaque;                              // IWYU pragma: export
class OpTypePointer;                             // IWYU pragma: export
class OpTypeFunction;                            // IWYU pragma: export
class OpTypeEvent;                               // IWYU pragma: export
class OpTypeDeviceEvent;                         // IWYU pragma: export
class OpTypeReserveId;                           // IWYU pragma: export
class OpTypeQueue;                               // IWYU pragma: export
class OpTypePipe;                                // IWYU pragma: export
class OpTypeForwardPointer;                      // IWYU pragma: export
class OpConstantTrue;                            // IWYU pragma: export
class OpConstantFalse;                           // IWYU pragma: export
class OpConstant;                                // IWYU pragma: export
class OpConstantComposite;                       // IWYU pragma: export
class OpConstantSampler;                         // IWYU pragma: export
class OpConstantNull;                            // IWYU pragma: export
class OpFunction;                                // IWYU pragma: export
class OpFunctionParameter;                       // IWYU pragma: export
class OpFunctionEnd;                             // IWYU pragma: export
class OpFunctionCall;                            // IWYU pragma: export
class OpVariable;                                // IWYU pragma: export
class OpImageTexelPointer;                       // IWYU pragma: export
class OpLoad;                                    // IWYU pragma: export
class OpStore;                                   // IWYU pragma: export
class OpCopyMemory;                              // IWYU pragma: export
class OpCopyMemorySized;                         // IWYU pragma: export
class OpAccessChain;                             // IWYU pragma: export
class OpInBoundsAccessChain;                     // IWYU pragma: export
class OpPtrAccessChain;                          // IWYU pragma: export
class OpArrayLength;                             // IWYU pragma: export
class OpGenericPtrMemSemantics;                  // IWYU pragma: export
class OpInBoundsPtrAccessChain;                  // IWYU pragma: export
class OpDecorate;                                // IWYU pragma: export
class OpMemberDecorate;                          // IWYU pragma: export
class OpDecorationGroup;                         // IWYU pragma: export
class OpGroupDecorate;                           // IWYU pragma: export
class OpGroupMemberDecorate;                     // IWYU pragma: export
class OpVectorExtractDynamic;                    // IWYU pragma: export
class OpVectorInsertDynamic;                     // IWYU pragma: export
class OpVectorShuffle;                           // IWYU pragma: export
class OpCompositeConstruct;                      // IWYU pragma: export
class OpCompositeExtract;                        // IWYU pragma: export
class OpCompositeInsert;                         // IWYU pragma: export
class OpCopyObject;                              // IWYU pragma: export
class OpTranspose;                               // IWYU pragma: export
class OpSampledImage;                            // IWYU pragma: export
class OpImageSampleImplicitLod;                  // IWYU pragma: export
class OpImageSampleExplicitLod;                  // IWYU pragma: export
class OpImageSampleDrefImplicitLod;              // IWYU pragma: export
class OpImageSampleDrefExplicitLod;              // IWYU pragma: export
class OpImageSampleProjImplicitLod;              // IWYU pragma: export
class OpImageSampleProjExplicitLod;              // IWYU pragma: export
class OpImageSampleProjDrefImplicitLod;          // IWYU pragma: export
class OpImageSampleProjDrefExplicitLod;          // IWYU pragma: export
class OpImageFetch;                              // IWYU pragma: export
class OpImageGather;                             // IWYU pragma: export
class OpImageDrefGather;                         // IWYU pragma: export
class OpImageRead;                               // IWYU pragma: export
class OpImageWrite;                              // IWYU pragma: export
class OpImage;                                   // IWYU pragma: export
class OpImageQueryFormat;                        // IWYU pragma: export
class OpImageQueryOrder;                         // IWYU pragma: export
class OpImageQuerySizeLod;                       // IWYU pragma: export
class OpImageQuerySize;                          // IWYU pragma: export
class OpImageQueryLod;                           // IWYU pragma: export
class OpImageQueryLevels;                        // IWYU pragma: export
class OpImageQuerySamples;                       // IWYU pragma: export
class OpConvertFToU;                             // IWYU pragma: export
class OpConvertFToS;                             // IWYU pragma: export
class OpConvertSToF;                             // IWYU pragma: export
class OpConvertUToF;                             // IWYU pragma: export
class OpUConvert;                                // IWYU pragma: export
class OpSConvert;                                // IWYU pragma: export
class OpFConvert;                                // IWYU pragma: export
class OpQuantizeToF16;                           // IWYU pragma: export
class OpConvertPtrToU;                           // IWYU pragma: export
class OpSatConvertSToU;                          // IWYU pragma: export
class OpSatConvertUToS;                          // IWYU pragma: export
class OpConvertUToPtr;                           // IWYU pragma: export
class OpPtrCastToGeneric;                        // IWYU pragma: export
class OpGenericCastToPtr;                        // IWYU pragma: export
class OpGenericCastToPtrExplicit;                // IWYU pragma: export
class OpBitcast;                                 // IWYU pragma: export
class OpSNegate;                                 // IWYU pragma: export
class OpFNegate;                                 // IWYU pragma: export
class OpIAdd;                                    // IWYU pragma: export
class OpFAdd;                                    // IWYU pragma: export
class OpISub;                                    // IWYU pragma: export
class OpFSub;                                    // IWYU pragma: export
class OpIMul;                                    // IWYU pragma: export
class OpFMul;                                    // IWYU pragma: export
class OpUDiv;                                    // IWYU pragma: export
class OpSDiv;                                    // IWYU pragma: export
class OpFDiv;                                    // IWYU pragma: export
class OpUMod;                                    // IWYU pragma: export
class OpSRem;                                    // IWYU pragma: export
class OpSMod;                                    // IWYU pragma: export
class OpFRem;                                    // IWYU pragma: export
class OpFMod;                                    // IWYU pragma: export
class OpVectorTimesScalar;                       // IWYU pragma: export
class OpMatrixTimesScalar;                       // IWYU pragma: export
class OpVectorTimesMatrix;                       // IWYU pragma: export
class OpMatrixTimesVector;                       // IWYU pragma: export
class OpMatrixTimesMatrix;                       // IWYU pragma: export
class OpOuterProduct;                            // IWYU pragma: export
class OpDot;                                     // IWYU pragma: export
class OpIAddCarry;                               // IWYU pragma: export
class OpISubBorrow;                              // IWYU pragma: export
class OpUMulExtended;                            // IWYU pragma: export
class OpSMulExtended;                            // IWYU pragma: export
class OpAny;                                     // IWYU pragma: export
class OpAll;                                     // IWYU pragma: export
class OpIsNan;                                   // IWYU pragma: export
class OpIsInf;                                   // IWYU pragma: export
class OpIsFinite;                                // IWYU pragma: export
class OpIsNormal;                                // IWYU pragma: export
class OpSignBitSet;                              // IWYU pragma: export
class OpLessOrGreater;                           // IWYU pragma: export
class OpOrdered;                                 // IWYU pragma: export
class OpUnordered;                               // IWYU pragma: export
class OpLogicalEqual;                            // IWYU pragma: export
class OpLogicalNotEqual;                         // IWYU pragma: export
class OpLogicalOr;                               // IWYU pragma: export
class OpLogicalAnd;                              // IWYU pragma: export
class OpLogicalNot;                              // IWYU pragma: export
class OpSelect;                                  // IWYU pragma: export
class OpIEqual;                                  // IWYU pragma: export
class OpINotEqual;                               // IWYU pragma: export
class OpUGreaterThan;                            // IWYU pragma: export
class OpSGreaterThan;                            // IWYU pragma: export
class OpUGreaterThanEqual;                       // IWYU pragma: export
class OpSGreaterThanEqual;                       // IWYU pragma: export
class OpULessThan;                               // IWYU pragma: export
class OpSLessThan;                               // IWYU pragma: export
class OpULessThanEqual;                          // IWYU pragma: export
class OpSLessThanEqual;                          // IWYU pragma: export
class OpFOrdEqual;                               // IWYU pragma: export
class OpFUnordEqual;                             // IWYU pragma: export
class OpFOrdNotEqual;                            // IWYU pragma: export
class OpFUnordNotEqual;                          // IWYU pragma: export
class OpFOrdLessThan;                            // IWYU pragma: export
class OpFUnordLessThan;                          // IWYU pragma: export
class OpFOrdGreaterThan;                         // IWYU pragma: export
class OpFUnordGreaterThan;                       // IWYU pragma: export
class OpFOrdLessThanEqual;                       // IWYU pragma: export
class OpFUnordLessThanEqual;                     // IWYU pragma: export
class OpFOrdGreaterThanEqual;                    // IWYU pragma: export
class OpFUnordGreaterThanEqual;                  // IWYU pragma: export
class OpShiftRightLogical;                       // IWYU pragma: export
class OpShiftRightArithmetic;                    // IWYU pragma: export
class OpShiftLeftLogical;                        // IWYU pragma: export
class OpBitwiseOr;                               // IWYU pragma: export
class OpBitwiseXor;                              // IWYU pragma: export
class OpBitwiseAnd;                              // IWYU pragma: export
class OpNot;                                     // IWYU pragma: export
class OpBitFieldInsert;                          // IWYU pragma: export
class OpBitFieldSExtract;                        // IWYU pragma: export
class OpBitFieldUExtract;                        // IWYU pragma: export
class OpBitReverse;                              // IWYU pragma: export
class OpBitCount;                                // IWYU pragma: export
class OpDPdx;                                    // IWYU pragma: export
class OpDPdy;                                    // IWYU pragma: export
class OpFwidth;                                  // IWYU pragma: export
class OpDPdxFine;                                // IWYU pragma: export
class OpDPdyFine;                                // IWYU pragma: export
class OpFwidthFine;                              // IWYU pragma: export
class OpDPdxCoarse;                              // IWYU pragma: export
class OpDPdyCoarse;                              // IWYU pragma: export
class OpFwidthCoarse;                            // IWYU pragma: export
class OpEmitVertex;                              // IWYU pragma: export
class OpEndPrimitive;                            // IWYU pragma: export
class OpEmitStreamVertex;                        // IWYU pragma: export
class OpEndStreamPrimitive;                      // IWYU pragma: export
class OpControlBarrier;                          // IWYU pragma: export
class OpMemoryBarrier;                           // IWYU pragma: export
class OpAtomicLoad;                              // IWYU pragma: export
class OpAtomicStore;                             // IWYU pragma: export
class OpAtomicExchange;                          // IWYU pragma: export
class OpAtomicCompareExchange;                   // IWYU pragma: export
class OpAtomicCompareExchangeWeak;               // IWYU pragma: export
class OpAtomicIIncrement;                        // IWYU pragma: export
class OpAtomicIDecrement;                        // IWYU pragma: export
class OpAtomicIAdd;                              // IWYU pragma: export
class OpAtomicISub;                              // IWYU pragma: export
class OpAtomicSMin;                              // IWYU pragma: export
class OpAtomicUMin;                              // IWYU pragma: export
class OpAtomicSMax;                              // IWYU pragma: export
class OpAtomicUMax;                              // IWYU pragma: export
class OpAtomicAnd;                               // IWYU pragma: export
class OpAtomicOr;                                // IWYU pragma: export
class OpAtomicXor;                               // IWYU pragma: export
class OpPhi;                                     // IWYU pragma: export
class OpLoopMerge;                               // IWYU pragma: export
class OpSelectionMerge;                          // IWYU pragma: export
class OpLabel;                                   // IWYU pragma: export
class OpBranch;                                  // IWYU pragma: export
class OpBranchConditional;                       // IWYU pragma: export
class OpSwitch;                                  // IWYU pragma: export
class OpKill;                                    // IWYU pragma: export
class OpReturn;                                  // IWYU pragma: export
class OpReturnValue;                             // IWYU pragma: export
class OpUnreachable;                             // IWYU pragma: export
class OpLifetimeStart;                           // IWYU pragma: export
class OpLifetimeStop;                            // IWYU pragma: export
class OpGroupAsyncCopy;                          // IWYU pragma: export
class OpGroupWaitEvents;                         // IWYU pragma: export
class OpGroupAll;                                // IWYU pragma: export
class OpGroupAny;                                // IWYU pragma: export
class OpGroupBroadcast;                          // IWYU pragma: export
class OpGroupIAdd;                               // IWYU pragma: export
class OpGroupFAdd;                               // IWYU pragma: export
class OpGroupFMin;                               // IWYU pragma: export
class OpGroupUMin;                               // IWYU pragma: export
class OpGroupSMin;                               // IWYU pragma: export
class OpGroupFMax;                               // IWYU pragma: export
class OpGroupUMax;                               // IWYU pragma: export
class OpGroupSMax;                               // IWYU pragma: export
class OpReadPipe;                                // IWYU pragma: export
class OpWritePipe;                               // IWYU pragma: export
class OpReservedReadPipe;                        // IWYU pragma: export
class OpReservedWritePipe;                       // IWYU pragma: export
class OpReserveReadPipePackets;                  // IWYU pragma: export
class OpReserveWritePipePackets;                 // IWYU pragma: export
class OpCommitReadPipe;                          // IWYU pragma: export
class OpCommitWritePipe;                         // IWYU pragma: export
class OpIsValidReserveId;                        // IWYU pragma: export
class OpGetNumPipePackets;                       // IWYU pragma: export
class OpGetMaxPipePackets;                       // IWYU pragma: export
class OpGroupReserveReadPipePackets;             // IWYU pragma: export
class OpGroupReserveWritePipePackets;            // IWYU pragma: export
class OpGroupCommitReadPipe;                     // IWYU pragma: export
class OpGroupCommitWritePipe;                    // IWYU pragma: export
class OpEnqueueMarker;                           // IWYU pragma: export
class OpEnqueueKernel;                           // IWYU pragma: export
class OpGetKernelNDrangeSubGroupCount;           // IWYU pragma: export
class OpGetKernelNDrangeMaxSubGroupSize;         // IWYU pragma: export
class OpGetKernelWorkGroupSize;                  // IWYU pragma: export
class OpGetKernelPreferredWorkGroupSizeMultiple; // IWYU pragma: export
class OpRetainEvent;                             // IWYU pragma: export
class OpReleaseEvent;                            // IWYU pragma: export
class OpCreateUserEvent;                         // IWYU pragma: export
class OpIsValidEvent;                            // IWYU pragma: export
class OpSetUserEventStatus;                      // IWYU pragma: export
class OpCaptureEventProfilingInfo;               // IWYU pragma: export
class OpGetDefaultQueue;                         // IWYU pragma: export
class OpBuildNDRange;                            // IWYU pragma: export
class OpImageSparseSampleImplicitLod;            // IWYU pragma: export
class OpImageSparseSampleExplicitLod;            // IWYU pragma: export
class OpImageSparseSampleDrefImplicitLod;        // IWYU pragma: export
class OpImageSparseSampleDrefExplicitLod;        // IWYU pragma: export
class OpImageSparseSampleProjImplicitLod;        // IWYU pragma: export
class OpImageSparseSampleProjExplicitLod;        // IWYU pragma: export
class OpImageSparseSampleProjDrefImplicitLod;    // IWYU pragma: export
class OpImageSparseSampleProjDrefExplicitLod;    // IWYU pragma: export
class OpImageSparseFetch;                        // IWYU pragma: export
class OpImageSparseGather;                       // IWYU pragma: export
class OpImageSparseDrefGather;                   // IWYU pragma: export
class OpImageSparseTexelsResident;               // IWYU pragma: export
class OpNoLine;                                  // IWYU pragma: export
class OpAtomicFlagTestAndSet;                    // IWYU pragma: export
class OpAtomicFlagClear;                         // IWYU pragma: export
class OpImageSparseRead;                         // IWYU pragma: export
class OpSizeOf;                                  // IWYU pragma: export
class OpTypePipeStorage;                         // IWYU pragma: export
class OpConstantPipeStorage;                     // IWYU pragma: export
class OpCreatePipeFromPipeStorage;               // IWYU pragma: export
class OpGetKernelLocalSizeForSubgroupCount;      // IWYU pragma: export
class OpGetKernelMaxNumSubgroups;                // IWYU pragma: export
class OpTypeNamedBarrier;                        // IWYU pragma: export
class OpNamedBarrierInitialize;                  // IWYU pragma: export
class OpMemoryNamedBarrier;                      // IWYU pragma: export
class OpModuleProcessed;                         // IWYU pragma: export
class OpExecutionModeId;                         // IWYU pragma: export
class OpDecorateId;                              // IWYU pragma: export
class OpGroupNonUniformElect;                    // IWYU pragma: export
class OpGroupNonUniformAll;                      // IWYU pragma: export
class OpGroupNonUniformAny;                      // IWYU pragma: export
class OpGroupNonUniformAllEqual;                 // IWYU pragma: export
class OpGroupNonUniformBroadcast;                // IWYU pragma: export
class OpGroupNonUniformBroadcastFirst;           // IWYU pragma: export
class OpGroupNonUniformBallot;                   // IWYU pragma: export
class OpGroupNonUniformInverseBallot;            // IWYU pragma: export
class OpGroupNonUniformBallotBitExtract;         // IWYU pragma: export
class OpGroupNonUniformBallotBitCount;           // IWYU pragma: export
class OpGroupNonUniformBallotFindLSB;            // IWYU pragma: export
class OpGroupNonUniformBallotFindMSB;            // IWYU pragma: export
class OpGroupNonUniformShuffle;                  // IWYU pragma: export
class OpGroupNonUniformShuffleXor;               // IWYU pragma: export
class OpGroupNonUniformShuffleUp;                // IWYU pragma: export
class OpGroupNonUniformShuffleDown;              // IWYU pragma: export
class OpGroupNonUniformIAdd;                     // IWYU pragma: export
class OpGroupNonUniformFAdd;                     // IWYU pragma: export
class OpGroupNonUniformIMul;                     // IWYU pragma: export
class OpGroupNonUniformFMul;                     // IWYU pragma: export
class OpGroupNonUniformSMin;                     // IWYU pragma: export
class OpGroupNonUniformUMin;                     // IWYU pragma: export
class OpGroupNonUniformFMin;                     // IWYU pragma: export
class OpGroupNonUniformSMax;                     // IWYU pragma: export
class OpGroupNonUniformUMax;                     // IWYU pragma: export
class OpGroupNonUniformFMax;                     // IWYU pragma: export
class OpGroupNonUniformBitwiseAnd;               // IWYU pragma: export
class OpGroupNonUniformBitwiseOr;                // IWYU pragma: export
class OpGroupNonUniformBitwiseXor;               // IWYU pragma: export
class OpGroupNonUniformLogicalAnd;               // IWYU pragma: export
class OpGroupNonUniformLogicalOr;                // IWYU pragma: export
class OpGroupNonUniformLogicalXor;               // IWYU pragma: export
class OpGroupNonUniformQuadBroadcast;            // IWYU pragma: export
class OpGroupNonUniformQuadSwap;                 // IWYU pragma: export
class OpCopyLogical;                             // IWYU pragma: export
class OpPtrEqual;                                // IWYU pragma: export
class OpPtrNotEqual;                             // IWYU pragma: export
class OpPtrDiff;                                 // IWYU pragma: export
class OpTypeCooperativeMatrixKHR;                // IWYU pragma: export
class OpCooperativeMatrixLoadKHR;                // IWYU pragma: export
class OpCooperativeMatrixStoreKHR;               // IWYU pragma: export
class OpCooperativeMatrixMulAddKHR;              // IWYU pragma: export
class OpCooperativeMatrixLengthKHR;              // IWYU pragma: export
class OpSubgroupBlockReadINTEL;                  // IWYU pragma: export
class OpSubgroupBlockWriteINTEL;                 // IWYU pragma: export
class OpAsmTargetINTEL;                          // IWYU pragma: export
class OpAsmINTEL;                                // IWYU pragma: export
class OpAsmCallINTEL;                            // IWYU pragma: export
class OpAtomicFMinEXT;                           // IWYU pragma: export
class OpAtomicFMaxEXT;                           // IWYU pragma: export
class OpAtomicFAddEXT;                           // IWYU pragma: export
class OpConvertFToBF16INTEL;                     // IWYU pragma: export
class OpConvertBF16ToFINTEL;                     // IWYU pragma: export
class OpCooperativeMatrixLoadCheckedINTEL;       // IWYU pragma: export
class OpCooperativeMatrixStoreCheckedINTEL;      // IWYU pragma: export

} // namespace tinytc::spv

#endif // GENERATED_DEFS_20250131_HPP
