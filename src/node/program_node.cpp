// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "node/program_node.hpp"
#include "node/function_node.hpp"
#include "tinytc/tinytc.h"

#include <utility>

using namespace tinytc;

extern "C" {

tinytc_prog::tinytc_prog(tinytc::compiler_context ctx, tinytc_location const &lc)
    : ctx_{std::move(ctx)} {
    loc(lc);
}
}

