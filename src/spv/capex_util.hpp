// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

// This file is generated
// Do not edit manually

#ifndef GENERATED_CAPEX_UTIL_20241126_HPP
#define GENERATED_CAPEX_UTIL_20241126_HPP

#include "tinytc/tinytc.hpp"

namespace tinytc::spv {

enum class Capability;
enum class AddressingModel;
enum class ExecutionMode;
enum class ExecutionModel;
enum class MemoryModel;
auto capabilities(ExecutionModel op) -> array_view<Capability>;
auto capabilities(AddressingModel op) -> array_view<Capability>;
auto capabilities(MemoryModel op) -> array_view<Capability>;
auto capabilities(ExecutionMode op) -> array_view<Capability>;
auto extensions(ExecutionModel op) -> array_view<char const *>;
auto extensions(AddressingModel op) -> array_view<char const *>;
auto extensions(MemoryModel op) -> array_view<char const *>;
auto extensions(ExecutionMode op) -> array_view<char const *>;

} // namespace tinytc::spv

#endif // GENERATED_CAPEX_UTIL_20241126_HPP
