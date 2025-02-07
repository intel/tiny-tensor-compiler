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

void tinytc_func::param_attr(std::int32_t param_no, tinytc_attr_t a) {
    if (param_no < 0 || param_no >= num_params()) {
        throw compilation_error(loc(), status::invalid_arguments);
    }
    if (static_cast<std::int64_t>(param_attr_.size()) != num_params()) {
        param_attr_.resize(num_params(), nullptr);
    }
    param_attr_[param_no] = a;
}
auto tinytc_func::param_attr(std::int32_t param_no) const -> tinytc_attr_t {
    if (param_no < 0 || param_no >= num_params()) {
        throw compilation_error(loc(), status::invalid_arguments);
    }
    if (param_attr_.empty()) {
        return nullptr;
    }
    return param_attr_[param_no];
}

