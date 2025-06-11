// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

// This file is generated
// Do not edit manually

#ifndef GENERATED_NONSEMANTIC_SHADER_DEBUGINFO_100_20250610_HPP
#define GENERATED_NONSEMANTIC_SHADER_DEBUGINFO_100_20250610_HPP

namespace tinytc::spv {

constexpr char const *NonSemanticShaderDebugInfo100_name = "NonSemantic.Shader.DebugInfo.100";

enum class NonSemanticShaderDebugInfo100 {
    DebugInfoNone = 0,
    DebugCompilationUnit = 1,
    DebugTypeBasic = 2,
    DebugTypePointer = 3,
    DebugTypeQualifier = 4,
    DebugTypeArray = 5,
    DebugTypeVector = 6,
    DebugTypedef = 7,
    DebugTypeFunction = 8,
    DebugTypeEnum = 9,
    DebugTypeComposite = 10,
    DebugTypeMember = 11,
    DebugTypeInheritance = 12,
    DebugTypePtrToMember = 13,
    DebugTypeTemplate = 14,
    DebugTypeTemplateParameter = 15,
    DebugTypeTemplateTemplateParameter = 16,
    DebugTypeTemplateParameterPack = 17,
    DebugGlobalVariable = 18,
    DebugFunctionDeclaration = 19,
    DebugFunction = 20,
    DebugLexicalBlock = 21,
    DebugLexicalBlockDiscriminator = 22,
    DebugScope = 23,
    DebugNoScope = 24,
    DebugInlinedAt = 25,
    DebugLocalVariable = 26,
    DebugInlinedVariable = 27,
    DebugDeclare = 28,
    DebugValue = 29,
    DebugOperation = 30,
    DebugExpression = 31,
    DebugMacroDef = 32,
    DebugMacroUndef = 33,
    DebugImportedEntity = 34,
    DebugSource = 35,
    DebugFunctionDefinition = 101,
    DebugSourceContinued = 102,
    DebugLine = 103,
    DebugNoLine = 104,
    DebugBuildIdentifier = 105,
    DebugStoragePath = 106,
    DebugEntryPoint = 107,
    DebugTypeMatrix = 108,
};

auto to_string(NonSemanticShaderDebugInfo100 op) -> char const *;

} // namespace tinytc::spv

#endif // GENERATED_NONSEMANTIC_SHADER_DEBUGINFO_100_20250610_HPP
