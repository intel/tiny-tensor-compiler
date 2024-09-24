// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "node/program_node.hpp"
#include "node/function_node.hpp"

using namespace tinytc;

extern "C" {

tinytc_prog::tinytc_prog(tinytc::compiler_context ctx, tinytc_location const &lc)
    : ctx_{std::move(ctx)} {
    loc(lc);
}

tinytc_prog::~tinytc_prog() {
    for (auto &f : functions()) {
        tinytc_func_destroy(f);
    }
}
}

