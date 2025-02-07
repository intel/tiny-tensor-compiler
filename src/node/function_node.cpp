// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "node/function_node.hpp"
#include "error.hpp"
#include "tinytc/types.hpp"

#include <utility>

using namespace tinytc;

tinytc_func::tinytc_func(std::string name, tinytc::array_view<tinytc_data_type_t> params,
                         tinytc_location const &lc)
    : name_(std::move(name)), work_group_size_{0, 0}, subgroup_size_{0}, loc_{lc} {
    body_.kind(tinytc::region_kind::collective);
    body_.loc(loc_);
    body_.set_params(std::move(params));
}

void tinytc_func::aligned(std::int32_t arg_no, std::int32_t alignment) {
    if (arg_no < 0 || arg_no >= num_params()) {
        throw compilation_error(loc(), status::invalid_arguments);
    }
    if (static_cast<std::int64_t>(align_.size()) != num_params()) {
        align_.resize(num_params(), 0);
    }
    align_[arg_no] = alignment;
}
auto tinytc_func::aligned(std::int32_t arg_no) const -> std::int32_t {
    if (arg_no < 0 || arg_no >= num_params()) {
        throw compilation_error(loc(), status::invalid_arguments);
    }
    if (align_.empty()) {
        return 0;
    }
    return align_[arg_no];
}

