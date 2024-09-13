// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PASSES_20240314_HPP
#define PASSES_20240314_HPP

#include "node/program_node.hpp"
#include "tinytc/types.h"

namespace tinytc {

template <typename FunctionPass> void run_function_pass(FunctionPass &&pass, tinytc_prog const &p) {
    for (auto const &func : p.functions()) {
        pass.run_on_function(*func);
    }
}

} // namespace tinytc

#endif // PASSES_20240314_HPP
