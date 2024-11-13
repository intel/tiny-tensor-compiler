// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

// This file is generated
// Do not edit manually

#include "capex_util.hpp"

namespace tinytc::spv {

auto capabilities(ExecutionModel e) -> array_view<Capability> {
    switch (e) {
    case ExecutionModel::Vertex: {
        constexpr static Capability values[] = {Capability::Shader};
        return {values, 1};
    }
    case ExecutionModel::TessellationControl: {
        constexpr static Capability values[] = {Capability::Tessellation};
        return {values, 1};
    }
    case ExecutionModel::TessellationEvaluation: {
        constexpr static Capability values[] = {Capability::Tessellation};
        return {values, 1};
    }
    case ExecutionModel::Geometry: {
        constexpr static Capability values[] = {Capability::Geometry};
        return {values, 1};
    }
    case ExecutionModel::Fragment: {
        constexpr static Capability values[] = {Capability::Shader};
        return {values, 1};
    }
    case ExecutionModel::GLCompute: {
        constexpr static Capability values[] = {Capability::Shader};
        return {values, 1};
    }
    case ExecutionModel::Kernel: {
        constexpr static Capability values[] = {Capability::Kernel};
        return {values, 1};
    }
    case ExecutionModel::TaskNV: {
        constexpr static Capability values[] = {Capability::MeshShadingNV};
        return {values, 1};
    }
    case ExecutionModel::MeshNV: {
        constexpr static Capability values[] = {Capability::MeshShadingNV};
        return {values, 1};
    }
    case ExecutionModel::RayGenerationKHR: {
        constexpr static Capability values[] = {Capability::RayTracingNV,
                                                Capability::RayTracingKHR};
        return {values, 2};
    }
    case ExecutionModel::IntersectionKHR: {
        constexpr static Capability values[] = {Capability::RayTracingNV,
                                                Capability::RayTracingKHR};
        return {values, 2};
    }
    case ExecutionModel::AnyHitKHR: {
        constexpr static Capability values[] = {Capability::RayTracingNV,
                                                Capability::RayTracingKHR};
        return {values, 2};
    }
    case ExecutionModel::ClosestHitKHR: {
        constexpr static Capability values[] = {Capability::RayTracingNV,
                                                Capability::RayTracingKHR};
        return {values, 2};
    }
    case ExecutionModel::MissKHR: {
        constexpr static Capability values[] = {Capability::RayTracingNV,
                                                Capability::RayTracingKHR};
        return {values, 2};
    }
    case ExecutionModel::CallableKHR: {
        constexpr static Capability values[] = {Capability::RayTracingNV,
                                                Capability::RayTracingKHR};
        return {values, 2};
    }
    case ExecutionModel::TaskEXT: {
        constexpr static Capability values[] = {Capability::MeshShadingEXT};
        return {values, 1};
    }
    case ExecutionModel::MeshEXT: {
        constexpr static Capability values[] = {Capability::MeshShadingEXT};
        return {values, 1};
    }
    default:
        return {};
    }
}
auto capabilities(AddressingModel e) -> array_view<Capability> {
    switch (e) {
    case AddressingModel::Physical32: {
        constexpr static Capability values[] = {Capability::Addresses};
        return {values, 1};
    }
    case AddressingModel::Physical64: {
        constexpr static Capability values[] = {Capability::Addresses};
        return {values, 1};
    }
    case AddressingModel::PhysicalStorageBuffer64: {
        constexpr static Capability values[] = {Capability::PhysicalStorageBufferAddresses};
        return {values, 1};
    }
    default:
        return {};
    }
}
auto capabilities(MemoryModel e) -> array_view<Capability> {
    switch (e) {
    case MemoryModel::Simple: {
        constexpr static Capability values[] = {Capability::Shader};
        return {values, 1};
    }
    case MemoryModel::GLSL450: {
        constexpr static Capability values[] = {Capability::Shader};
        return {values, 1};
    }
    case MemoryModel::OpenCL: {
        constexpr static Capability values[] = {Capability::Kernel};
        return {values, 1};
    }
    case MemoryModel::Vulkan: {
        constexpr static Capability values[] = {Capability::VulkanMemoryModel};
        return {values, 1};
    }
    default:
        return {};
    }
}
auto capabilities(ExecutionMode e) -> array_view<Capability> {
    switch (e) {
    case ExecutionMode::Invocations: {
        constexpr static Capability values[] = {Capability::Geometry};
        return {values, 1};
    }
    case ExecutionMode::SpacingEqual: {
        constexpr static Capability values[] = {Capability::Tessellation};
        return {values, 1};
    }
    case ExecutionMode::SpacingFractionalEven: {
        constexpr static Capability values[] = {Capability::Tessellation};
        return {values, 1};
    }
    case ExecutionMode::SpacingFractionalOdd: {
        constexpr static Capability values[] = {Capability::Tessellation};
        return {values, 1};
    }
    case ExecutionMode::VertexOrderCw: {
        constexpr static Capability values[] = {Capability::Tessellation};
        return {values, 1};
    }
    case ExecutionMode::VertexOrderCcw: {
        constexpr static Capability values[] = {Capability::Tessellation};
        return {values, 1};
    }
    case ExecutionMode::PixelCenterInteger: {
        constexpr static Capability values[] = {Capability::Shader};
        return {values, 1};
    }
    case ExecutionMode::OriginUpperLeft: {
        constexpr static Capability values[] = {Capability::Shader};
        return {values, 1};
    }
    case ExecutionMode::OriginLowerLeft: {
        constexpr static Capability values[] = {Capability::Shader};
        return {values, 1};
    }
    case ExecutionMode::EarlyFragmentTests: {
        constexpr static Capability values[] = {Capability::Shader};
        return {values, 1};
    }
    case ExecutionMode::PointMode: {
        constexpr static Capability values[] = {Capability::Tessellation};
        return {values, 1};
    }
    case ExecutionMode::Xfb: {
        constexpr static Capability values[] = {Capability::TransformFeedback};
        return {values, 1};
    }
    case ExecutionMode::DepthReplacing: {
        constexpr static Capability values[] = {Capability::Shader};
        return {values, 1};
    }
    case ExecutionMode::DepthGreater: {
        constexpr static Capability values[] = {Capability::Shader};
        return {values, 1};
    }
    case ExecutionMode::DepthLess: {
        constexpr static Capability values[] = {Capability::Shader};
        return {values, 1};
    }
    case ExecutionMode::DepthUnchanged: {
        constexpr static Capability values[] = {Capability::Shader};
        return {values, 1};
    }
    case ExecutionMode::LocalSizeHint: {
        constexpr static Capability values[] = {Capability::Kernel};
        return {values, 1};
    }
    case ExecutionMode::InputPoints: {
        constexpr static Capability values[] = {Capability::Geometry};
        return {values, 1};
    }
    case ExecutionMode::InputLines: {
        constexpr static Capability values[] = {Capability::Geometry};
        return {values, 1};
    }
    case ExecutionMode::InputLinesAdjacency: {
        constexpr static Capability values[] = {Capability::Geometry};
        return {values, 1};
    }
    case ExecutionMode::Triangles: {
        constexpr static Capability values[] = {Capability::Geometry, Capability::Tessellation};
        return {values, 2};
    }
    case ExecutionMode::InputTrianglesAdjacency: {
        constexpr static Capability values[] = {Capability::Geometry};
        return {values, 1};
    }
    case ExecutionMode::Quads: {
        constexpr static Capability values[] = {Capability::Tessellation};
        return {values, 1};
    }
    case ExecutionMode::Isolines: {
        constexpr static Capability values[] = {Capability::Tessellation};
        return {values, 1};
    }
    case ExecutionMode::OutputVertices: {
        constexpr static Capability values[] = {Capability::Geometry, Capability::Tessellation,
                                                Capability::MeshShadingNV,
                                                Capability::MeshShadingEXT};
        return {values, 4};
    }
    case ExecutionMode::OutputPoints: {
        constexpr static Capability values[] = {Capability::Geometry, Capability::MeshShadingNV,
                                                Capability::MeshShadingEXT};
        return {values, 3};
    }
    case ExecutionMode::OutputLineStrip: {
        constexpr static Capability values[] = {Capability::Geometry};
        return {values, 1};
    }
    case ExecutionMode::OutputTriangleStrip: {
        constexpr static Capability values[] = {Capability::Geometry};
        return {values, 1};
    }
    case ExecutionMode::VecTypeHint: {
        constexpr static Capability values[] = {Capability::Kernel};
        return {values, 1};
    }
    case ExecutionMode::ContractionOff: {
        constexpr static Capability values[] = {Capability::Kernel};
        return {values, 1};
    }
    case ExecutionMode::Initializer: {
        constexpr static Capability values[] = {Capability::Kernel};
        return {values, 1};
    }
    case ExecutionMode::Finalizer: {
        constexpr static Capability values[] = {Capability::Kernel};
        return {values, 1};
    }
    case ExecutionMode::SubgroupSize: {
        constexpr static Capability values[] = {Capability::SubgroupDispatch};
        return {values, 1};
    }
    case ExecutionMode::SubgroupsPerWorkgroup: {
        constexpr static Capability values[] = {Capability::SubgroupDispatch};
        return {values, 1};
    }
    case ExecutionMode::SubgroupsPerWorkgroupId: {
        constexpr static Capability values[] = {Capability::SubgroupDispatch};
        return {values, 1};
    }
    case ExecutionMode::LocalSizeHintId: {
        constexpr static Capability values[] = {Capability::Kernel};
        return {values, 1};
    }
    case ExecutionMode::NonCoherentColorAttachmentReadEXT: {
        constexpr static Capability values[] = {Capability::TileImageColorReadAccessEXT};
        return {values, 1};
    }
    case ExecutionMode::NonCoherentDepthAttachmentReadEXT: {
        constexpr static Capability values[] = {Capability::TileImageDepthReadAccessEXT};
        return {values, 1};
    }
    case ExecutionMode::NonCoherentStencilAttachmentReadEXT: {
        constexpr static Capability values[] = {Capability::TileImageStencilReadAccessEXT};
        return {values, 1};
    }
    case ExecutionMode::SubgroupUniformControlFlowKHR: {
        constexpr static Capability values[] = {Capability::Shader};
        return {values, 1};
    }
    case ExecutionMode::PostDepthCoverage: {
        constexpr static Capability values[] = {Capability::SampleMaskPostDepthCoverage};
        return {values, 1};
    }
    case ExecutionMode::DenormPreserve: {
        constexpr static Capability values[] = {Capability::DenormPreserve};
        return {values, 1};
    }
    case ExecutionMode::DenormFlushToZero: {
        constexpr static Capability values[] = {Capability::DenormFlushToZero};
        return {values, 1};
    }
    case ExecutionMode::SignedZeroInfNanPreserve: {
        constexpr static Capability values[] = {Capability::SignedZeroInfNanPreserve};
        return {values, 1};
    }
    case ExecutionMode::RoundingModeRTE: {
        constexpr static Capability values[] = {Capability::RoundingModeRTE};
        return {values, 1};
    }
    case ExecutionMode::RoundingModeRTZ: {
        constexpr static Capability values[] = {Capability::RoundingModeRTZ};
        return {values, 1};
    }
    case ExecutionMode::EarlyAndLateFragmentTestsAMD: {
        constexpr static Capability values[] = {Capability::Shader};
        return {values, 1};
    }
    case ExecutionMode::StencilRefReplacingEXT: {
        constexpr static Capability values[] = {Capability::StencilExportEXT};
        return {values, 1};
    }
    case ExecutionMode::CoalescingAMDX: {
        constexpr static Capability values[] = {Capability::ShaderEnqueueAMDX};
        return {values, 1};
    }
    case ExecutionMode::IsApiEntryAMDX: {
        constexpr static Capability values[] = {Capability::ShaderEnqueueAMDX};
        return {values, 1};
    }
    case ExecutionMode::MaxNodeRecursionAMDX: {
        constexpr static Capability values[] = {Capability::ShaderEnqueueAMDX};
        return {values, 1};
    }
    case ExecutionMode::StaticNumWorkgroupsAMDX: {
        constexpr static Capability values[] = {Capability::ShaderEnqueueAMDX};
        return {values, 1};
    }
    case ExecutionMode::ShaderIndexAMDX: {
        constexpr static Capability values[] = {Capability::ShaderEnqueueAMDX};
        return {values, 1};
    }
    case ExecutionMode::MaxNumWorkgroupsAMDX: {
        constexpr static Capability values[] = {Capability::ShaderEnqueueAMDX};
        return {values, 1};
    }
    case ExecutionMode::StencilRefUnchangedFrontAMD: {
        constexpr static Capability values[] = {Capability::StencilExportEXT};
        return {values, 1};
    }
    case ExecutionMode::StencilRefGreaterFrontAMD: {
        constexpr static Capability values[] = {Capability::StencilExportEXT};
        return {values, 1};
    }
    case ExecutionMode::StencilRefLessFrontAMD: {
        constexpr static Capability values[] = {Capability::StencilExportEXT};
        return {values, 1};
    }
    case ExecutionMode::StencilRefUnchangedBackAMD: {
        constexpr static Capability values[] = {Capability::StencilExportEXT};
        return {values, 1};
    }
    case ExecutionMode::StencilRefGreaterBackAMD: {
        constexpr static Capability values[] = {Capability::StencilExportEXT};
        return {values, 1};
    }
    case ExecutionMode::StencilRefLessBackAMD: {
        constexpr static Capability values[] = {Capability::StencilExportEXT};
        return {values, 1};
    }
    case ExecutionMode::QuadDerivativesKHR: {
        constexpr static Capability values[] = {Capability::QuadControlKHR};
        return {values, 1};
    }
    case ExecutionMode::RequireFullQuadsKHR: {
        constexpr static Capability values[] = {Capability::QuadControlKHR};
        return {values, 1};
    }
    case ExecutionMode::SharesInputWithAMDX: {
        constexpr static Capability values[] = {Capability::ShaderEnqueueAMDX};
        return {values, 1};
    }
    case ExecutionMode::OutputLinesEXT: {
        constexpr static Capability values[] = {Capability::MeshShadingNV,
                                                Capability::MeshShadingEXT};
        return {values, 2};
    }
    case ExecutionMode::OutputPrimitivesEXT: {
        constexpr static Capability values[] = {Capability::MeshShadingNV,
                                                Capability::MeshShadingEXT};
        return {values, 2};
    }
    case ExecutionMode::DerivativeGroupQuadsKHR: {
        constexpr static Capability values[] = {Capability::ComputeDerivativeGroupQuadsKHR};
        return {values, 2};
    }
    case ExecutionMode::DerivativeGroupLinearKHR: {
        constexpr static Capability values[] = {Capability::ComputeDerivativeGroupLinearKHR};
        return {values, 2};
    }
    case ExecutionMode::OutputTrianglesEXT: {
        constexpr static Capability values[] = {Capability::MeshShadingNV,
                                                Capability::MeshShadingEXT};
        return {values, 2};
    }
    case ExecutionMode::PixelInterlockOrderedEXT: {
        constexpr static Capability values[] = {Capability::FragmentShaderPixelInterlockEXT};
        return {values, 1};
    }
    case ExecutionMode::PixelInterlockUnorderedEXT: {
        constexpr static Capability values[] = {Capability::FragmentShaderPixelInterlockEXT};
        return {values, 1};
    }
    case ExecutionMode::SampleInterlockOrderedEXT: {
        constexpr static Capability values[] = {Capability::FragmentShaderSampleInterlockEXT};
        return {values, 1};
    }
    case ExecutionMode::SampleInterlockUnorderedEXT: {
        constexpr static Capability values[] = {Capability::FragmentShaderSampleInterlockEXT};
        return {values, 1};
    }
    case ExecutionMode::ShadingRateInterlockOrderedEXT: {
        constexpr static Capability values[] = {Capability::FragmentShaderShadingRateInterlockEXT};
        return {values, 1};
    }
    case ExecutionMode::ShadingRateInterlockUnorderedEXT: {
        constexpr static Capability values[] = {Capability::FragmentShaderShadingRateInterlockEXT};
        return {values, 1};
    }
    case ExecutionMode::SharedLocalMemorySizeINTEL: {
        constexpr static Capability values[] = {Capability::VectorComputeINTEL};
        return {values, 1};
    }
    case ExecutionMode::RoundingModeRTPINTEL: {
        constexpr static Capability values[] = {Capability::RoundToInfinityINTEL};
        return {values, 1};
    }
    case ExecutionMode::RoundingModeRTNINTEL: {
        constexpr static Capability values[] = {Capability::RoundToInfinityINTEL};
        return {values, 1};
    }
    case ExecutionMode::FloatingPointModeALTINTEL: {
        constexpr static Capability values[] = {Capability::RoundToInfinityINTEL};
        return {values, 1};
    }
    case ExecutionMode::FloatingPointModeIEEEINTEL: {
        constexpr static Capability values[] = {Capability::RoundToInfinityINTEL};
        return {values, 1};
    }
    case ExecutionMode::MaxWorkgroupSizeINTEL: {
        constexpr static Capability values[] = {Capability::KernelAttributesINTEL};
        return {values, 1};
    }
    case ExecutionMode::MaxWorkDimINTEL: {
        constexpr static Capability values[] = {Capability::KernelAttributesINTEL};
        return {values, 1};
    }
    case ExecutionMode::NoGlobalOffsetINTEL: {
        constexpr static Capability values[] = {Capability::KernelAttributesINTEL};
        return {values, 1};
    }
    case ExecutionMode::NumSIMDWorkitemsINTEL: {
        constexpr static Capability values[] = {Capability::FPGAKernelAttributesINTEL};
        return {values, 1};
    }
    case ExecutionMode::SchedulerTargetFmaxMhzINTEL: {
        constexpr static Capability values[] = {Capability::FPGAKernelAttributesINTEL};
        return {values, 1};
    }
    case ExecutionMode::MaximallyReconvergesKHR: {
        constexpr static Capability values[] = {Capability::Shader};
        return {values, 1};
    }
    case ExecutionMode::FPFastMathDefault: {
        constexpr static Capability values[] = {Capability::FloatControls2};
        return {values, 1};
    }
    case ExecutionMode::StreamingInterfaceINTEL: {
        constexpr static Capability values[] = {Capability::FPGAKernelAttributesINTEL};
        return {values, 1};
    }
    case ExecutionMode::RegisterMapInterfaceINTEL: {
        constexpr static Capability values[] = {Capability::FPGAKernelAttributesv2INTEL};
        return {values, 1};
    }
    case ExecutionMode::NamedBarrierCountINTEL: {
        constexpr static Capability values[] = {Capability::VectorComputeINTEL};
        return {values, 1};
    }
    case ExecutionMode::MaximumRegistersINTEL: {
        constexpr static Capability values[] = {Capability::RegisterLimitsINTEL};
        return {values, 1};
    }
    case ExecutionMode::MaximumRegistersIdINTEL: {
        constexpr static Capability values[] = {Capability::RegisterLimitsINTEL};
        return {values, 1};
    }
    case ExecutionMode::NamedMaximumRegistersINTEL: {
        constexpr static Capability values[] = {Capability::RegisterLimitsINTEL};
        return {values, 1};
    }
    default:
        return {};
    }
}
auto extensions(ExecutionModel e) -> array_view<char const *> {
    switch (e) {
    default:
        return {};
    }
}
auto extensions(AddressingModel e) -> array_view<char const *> {
    switch (e) {
    case AddressingModel::PhysicalStorageBuffer64: {
        constexpr static char const *values[] = {"SPV_EXT_physical_storage_buffer",
                                                 "SPV_KHR_physical_storage_buffer"};
        return {values, 2};
    }
    default:
        return {};
    }
}
auto extensions(MemoryModel e) -> array_view<char const *> {
    switch (e) {
    case MemoryModel::Vulkan: {
        constexpr static char const *values[] = {"SPV_KHR_vulkan_memory_model"};
        return {values, 1};
    }
    default:
        return {};
    }
}
auto extensions(ExecutionMode e) -> array_view<char const *> {
    switch (e) {
    case ExecutionMode::SubgroupUniformControlFlowKHR: {
        constexpr static char const *values[] = {"SPV_KHR_subgroup_uniform_control_flow"};
        return {values, 1};
    }
    case ExecutionMode::PostDepthCoverage: {
        constexpr static char const *values[] = {"SPV_KHR_post_depth_coverage"};
        return {values, 1};
    }
    case ExecutionMode::DenormPreserve: {
        constexpr static char const *values[] = {"SPV_KHR_float_controls"};
        return {values, 1};
    }
    case ExecutionMode::DenormFlushToZero: {
        constexpr static char const *values[] = {"SPV_KHR_float_controls"};
        return {values, 1};
    }
    case ExecutionMode::SignedZeroInfNanPreserve: {
        constexpr static char const *values[] = {"SPV_KHR_float_controls"};
        return {values, 1};
    }
    case ExecutionMode::RoundingModeRTE: {
        constexpr static char const *values[] = {"SPV_KHR_float_controls"};
        return {values, 1};
    }
    case ExecutionMode::RoundingModeRTZ: {
        constexpr static char const *values[] = {"SPV_KHR_float_controls"};
        return {values, 1};
    }
    case ExecutionMode::EarlyAndLateFragmentTestsAMD: {
        constexpr static char const *values[] = {"SPV_AMD_shader_early_and_late_fragment_tests"};
        return {values, 1};
    }
    case ExecutionMode::StencilRefReplacingEXT: {
        constexpr static char const *values[] = {"SPV_EXT_shader_stencil_export"};
        return {values, 1};
    }
    case ExecutionMode::StencilRefUnchangedFrontAMD: {
        constexpr static char const *values[] = {"SPV_AMD_shader_early_and_late_fragment_tests",
                                                 "SPV_EXT_shader_stencil_export"};
        return {values, 2};
    }
    case ExecutionMode::StencilRefGreaterFrontAMD: {
        constexpr static char const *values[] = {"SPV_AMD_shader_early_and_late_fragment_tests",
                                                 "SPV_EXT_shader_stencil_export"};
        return {values, 2};
    }
    case ExecutionMode::StencilRefLessFrontAMD: {
        constexpr static char const *values[] = {"SPV_AMD_shader_early_and_late_fragment_tests",
                                                 "SPV_EXT_shader_stencil_export"};
        return {values, 2};
    }
    case ExecutionMode::StencilRefUnchangedBackAMD: {
        constexpr static char const *values[] = {"SPV_AMD_shader_early_and_late_fragment_tests",
                                                 "SPV_EXT_shader_stencil_export"};
        return {values, 2};
    }
    case ExecutionMode::StencilRefGreaterBackAMD: {
        constexpr static char const *values[] = {"SPV_AMD_shader_early_and_late_fragment_tests",
                                                 "SPV_EXT_shader_stencil_export"};
        return {values, 2};
    }
    case ExecutionMode::StencilRefLessBackAMD: {
        constexpr static char const *values[] = {"SPV_AMD_shader_early_and_late_fragment_tests",
                                                 "SPV_EXT_shader_stencil_export"};
        return {values, 2};
    }
    case ExecutionMode::OutputLinesEXT: {
        constexpr static char const *values[] = {"SPV_NV_mesh_shader", "SPV_EXT_mesh_shader"};
        return {values, 2};
    }
    case ExecutionMode::OutputPrimitivesEXT: {
        constexpr static char const *values[] = {"SPV_NV_mesh_shader", "SPV_EXT_mesh_shader"};
        return {values, 2};
    }
    case ExecutionMode::DerivativeGroupQuadsKHR: {
        constexpr static char const *values[] = {"SPV_NV_compute_shader_derivatives",
                                                 "SPV_KHR_compute_shader_derivatives"};
        return {values, 2};
    }
    case ExecutionMode::DerivativeGroupLinearKHR: {
        constexpr static char const *values[] = {"SPV_NV_compute_shader_derivatives",
                                                 "SPV_KHR_compute_shader_derivatives"};
        return {values, 2};
    }
    case ExecutionMode::OutputTrianglesEXT: {
        constexpr static char const *values[] = {"SPV_NV_mesh_shader", "SPV_EXT_mesh_shader"};
        return {values, 2};
    }
    case ExecutionMode::PixelInterlockOrderedEXT: {
        constexpr static char const *values[] = {"SPV_EXT_fragment_shader_interlock"};
        return {values, 1};
    }
    case ExecutionMode::PixelInterlockUnorderedEXT: {
        constexpr static char const *values[] = {"SPV_EXT_fragment_shader_interlock"};
        return {values, 1};
    }
    case ExecutionMode::SampleInterlockOrderedEXT: {
        constexpr static char const *values[] = {"SPV_EXT_fragment_shader_interlock"};
        return {values, 1};
    }
    case ExecutionMode::SampleInterlockUnorderedEXT: {
        constexpr static char const *values[] = {"SPV_EXT_fragment_shader_interlock"};
        return {values, 1};
    }
    case ExecutionMode::ShadingRateInterlockOrderedEXT: {
        constexpr static char const *values[] = {"SPV_EXT_fragment_shader_interlock"};
        return {values, 1};
    }
    case ExecutionMode::ShadingRateInterlockUnorderedEXT: {
        constexpr static char const *values[] = {"SPV_EXT_fragment_shader_interlock"};
        return {values, 1};
    }
    case ExecutionMode::MaxWorkgroupSizeINTEL: {
        constexpr static char const *values[] = {"SPV_INTEL_kernel_attributes"};
        return {values, 1};
    }
    case ExecutionMode::MaxWorkDimINTEL: {
        constexpr static char const *values[] = {"SPV_INTEL_kernel_attributes"};
        return {values, 1};
    }
    case ExecutionMode::NoGlobalOffsetINTEL: {
        constexpr static char const *values[] = {"SPV_INTEL_kernel_attributes"};
        return {values, 1};
    }
    case ExecutionMode::NumSIMDWorkitemsINTEL: {
        constexpr static char const *values[] = {"SPV_INTEL_kernel_attributes"};
        return {values, 1};
    }
    case ExecutionMode::MaximallyReconvergesKHR: {
        constexpr static char const *values[] = {"SPV_KHR_maximal_reconvergence"};
        return {values, 1};
    }
    default:
        return {};
    }
}

} // namespace tinytc::spv
