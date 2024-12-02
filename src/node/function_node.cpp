// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "node/function_node.hpp"
#include "error.hpp"
#include "tinytc/types.hpp"

using namespace tinytc;

void tinytc_func::align(std::int32_t arg_no, std::int32_t alignment) {
    if (arg_no < 0 || arg_no >= num_params()) {
        throw compilation_error(loc(), status::invalid_arguments);
    }
    if (static_cast<std::int64_t>(align_.size()) != num_params()) {
        align_.resize(num_params());
    }
    align_[arg_no] = alignment;
}
auto tinytc_func::align(std::int32_t arg_no) const -> std::int32_t {
    if (arg_no < 0 || arg_no >= num_params()) {
        throw compilation_error(loc(), status::invalid_arguments);
    }
    if (align_.empty()) {
        return 0;
    }
    return align_[arg_no];
}

