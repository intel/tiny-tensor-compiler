// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "tinytc/ir/prog.hpp"
#include "tinytc/ir/func.hpp"
#include "tinytc/ir/internal/program_node.hpp"

#include <memory>
#include <utility>

namespace tinytc::ir {

prog::prog(func fun)
    : prog(std::make_shared<internal::program>(std::vector<func>{std::move(fun)})) {}

prog::prog(std::vector<func> funs) : prog(std::make_shared<internal::program>(std::move(funs))) {}

} // namespace tinytc::ir
